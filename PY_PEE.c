#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#define MAX_E   20
#define MAX_D   10
#define MAX_Y  200
#define MAX_S  230
#define INF    1e30
#define EPS    1e-9
#define BIGM   1e7
#define MAX_IT 10000

typedef struct {
    int    num_estados, num_decisiones;
    int    decision_valida[MAX_E][MAX_D];
    double P[MAX_D][MAX_E][MAX_E];
    double C[MAX_E][MAX_D];
    char   nombre_estado[MAX_E][32];
    char   nombre_decision[MAX_D][32];
    int    es_maximizar;
    int    max_iteraciones;
} ModeloDecision;

void sep_doble(void) {
    printf("\n");
    printf("================================================================\n");
}
void sep_simple(void) {
    printf("----------------------------------------------------------------\n");
}
void titulo(const char *s) {
    sep_doble();
    printf("  %s\n", s);
    sep_doble();
}
void subtitulo(const char *s) {
    printf("\n  >> %s\n", s);
    printf("  ");
    int n = (int)strlen(s) + 5;
    for (int i = 0; i < n; i++) putchar('-');
    printf("\n");
}

void print_politica(ModeloDecision *p, int *pol, const char *lbl) {
    printf("  %s: [ ", lbl);
    for (int i = 0; i < p->num_estados; i++) {
        printf("%s->%s", p->nombre_estado[i], p->nombre_decision[pol[i]]);
        if (i < p->num_estados - 1) printf(" | ");
    }
    printf(" ]\n");
}

void leer_modelo(ModeloDecision *p) {
    int i, j, k;
    titulo("INGRESO DE DATOS");

    printf("  Ingrese el numero total de estados del sistema  : ");
    scanf("%d", &p->num_estados);
    printf("  Ingrese el numero total de decisiones posibles  : ");
    scanf("%d", &p->num_decisiones);

    printf("\n  Asigne un nombre a cada estado del sistema:\n");
    for (i = 0; i < p->num_estados; i++) {
        printf("    Estado %d  --> nombre: ", i+1);
        scanf("%31s", p->nombre_estado[i]);
    }

    printf("\n  Asigne un nombre a cada decision disponible:\n");
    for (k = 0; k < p->num_decisiones; k++) {
        printf("    Decision %d --> nombre: ", k+1);
        scanf("%31s", p->nombre_decision[k]);
    }

    printf("\n  Indique que decisiones son validas en cada estado (1 = aplica, 0 = no aplica):\n");
    for (i = 0; i < p->num_estados; i++) {
        printf("\n  Estado [%s]:\n", p->nombre_estado[i]);
        for (k = 0; k < p->num_decisiones; k++) {
            printf("    La decision [%-12s] es valida aqui? (1/0): ", p->nombre_decision[k]);
            scanf("%d", &p->decision_valida[i][k]);
        }
    }

    printf("\n  Ingrese las probabilidades de transicion P(estado_siguiente | estado_actual, decision):\n");
    printf("  (Recuerde: la suma de cada fila debe ser igual a 1.0)\n");
    for (k = 0; k < p->num_decisiones; k++) {
        int hay = 0;
        for (i = 0; i < p->num_estados; i++) if (p->decision_valida[i][k]) { hay = 1; break; }
        if (!hay) continue;
        printf("\n  === Decision [%s] ===\n", p->nombre_decision[k]);
        for (i = 0; i < p->num_estados; i++) {
            if (!p->decision_valida[i][k]) continue;
            printf("  Desde el estado [%s], probabilidad de ir a:\n", p->nombre_estado[i]);
            for (j = 0; j < p->num_estados; j++) {
                printf("    --> %-15s : P(%s | %s, %s) = ",
                       p->nombre_estado[j],
                       p->nombre_estado[j], p->nombre_estado[i], p->nombre_decision[k]);
                scanf("%lf", &p->P[k][i][j]);
            }
        }
    }

    printf("\n  Verificando que las probabilidades de cada fila sumen 1...\n");
    int ok = 1;
    for (k = 0; k < p->num_decisiones; k++)
        for (i = 0; i < p->num_estados; i++) {
            if (!p->decision_valida[i][k]) continue;
            double s = 0;
            for (j = 0; j < p->num_estados; j++) s += p->P[k][i][j];
            if (fabs(s - 1.0) > 1e-4) {
                printf("  [ERROR] Decision [%s], Estado [%s]: la suma es %.4f (debe ser 1.0)\n",
                       p->nombre_decision[k], p->nombre_estado[i], s);
                ok = 0;
            }
        }
    if (ok) printf("  Todas las filas suman correctamente 1.0  [OK]\n");

    printf("\n  Seleccione el criterio de optimizacion:\n");
    printf("    0 = Minimizar costos\n");
    printf("    1 = Maximizar ganancias\n");
    printf("  Su eleccion: "); scanf("%d", &p->es_maximizar);

    printf("  Numero maximo de iteraciones permitidas (ingrese 0 para sin limite): ");
    scanf("%d", &p->max_iteraciones);

    printf("\n  Ingrese los %s asociados a cada par (estado, decision) valido:\n",
           p->es_maximizar ? "valores de ganancia" : "valores de costo");
    for (i = 0; i < p->num_estados; i++)
        for (k = 0; k < p->num_decisiones; k++) {
            if (!p->decision_valida[i][k]) continue;
            printf("    %s al aplicar [%-12s] en estado [%-12s]: ",
                   p->es_maximizar ? "Ganancia" : "Costo",
                   p->nombre_decision[k], p->nombre_estado[i]);
            scanf("%lf", &p->C[i][k]);
        }

    printf("\n  Todos los datos han sido ingresados exitosamente.\n");
}

int    es_mejor(ModeloDecision *p, double nv, double ac) {
    return p->es_maximizar ? (nv > ac + EPS) : (nv < ac - EPS);
}
double peor_valor(ModeloDecision *p) { return p->es_maximizar ? -INF : INF; }

int gauss_jordan(double A[][MAX_E+1], int n, double *x) {
    int i, j, k;
    double aug[MAX_E][MAX_E+1];
    for (i = 0; i < n; i++)
        for (j = 0; j <= n; j++) aug[i][j] = A[i][j];
    for (k = 0; k < n; k++) {
        int piv = k;
        for (i = k+1; i < n; i++)
            if (fabs(aug[i][k]) > fabs(aug[piv][k])) piv = i;
        double tmp[MAX_E+1];
        memcpy(tmp, aug[k], (n+1)*sizeof(double));
        memcpy(aug[k], aug[piv], (n+1)*sizeof(double));
        memcpy(aug[piv], tmp, (n+1)*sizeof(double));
        if (fabs(aug[k][k]) < EPS) return 0;
        double d = aug[k][k];
        for (j = k; j <= n; j++) aug[k][j] /= d;
        for (i = 0; i < n; i++) {
            if (i == k) continue;
            double f = aug[i][k];
            for (j = k; j <= n; j++) aug[i][j] -= f * aug[k][j];
        }
    }
    for (i = 0; i < n; i++) x[i] = aug[i][n];
    return 1;
}

int vector_estacionario(ModeloDecision *p, int *pol, double *pi) {
    int n = p->num_estados, i, j;
    double A[MAX_E][MAX_E+1];
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            A[i][j] = p->P[pol[j]][j][i];
            if (i == j) A[i][j] -= 1.0;
        }
        A[i][n] = 0.0;
    }
    for (j = 0; j < n; j++) A[n-1][j] = 1.0;
    A[n-1][n] = 1.0;
    return gauss_jordan(A, n, pi);
}

int evaluar_mp(ModeloDecision *p, int *pol, double *g, double *V) {
    int n = p->num_estados, i, j;
    double A[MAX_E][MAX_E+1];
    for (i = 0; i < n; i++) {
        int k = pol[i];
        A[i][0] = 1.0;
        for (j = 0; j < n-1; j++)
            A[i][j+1] = (i == j ? 1.0 : 0.0) - p->P[k][i][j];
        A[i][n] = p->C[i][k];
    }
    double x[MAX_E];
    if (!gauss_jordan(A, n, x)) return 0;
    *g = x[0];
    for (j = 0; j < n-1; j++) V[j] = x[j+1];
    V[n-1] = 0.0;
    return 1;
}

int evaluar_desc(ModeloDecision *p, int *pol, double alpha, double *V) {
    int n = p->num_estados, i, j;
    double A[MAX_E][MAX_E+1];
    for (i = 0; i < n; i++) {
        int k = pol[i];
        for (j = 0; j < n; j++)
            A[i][j] = (i == j ? 1.0 : 0.0) - alpha * p->P[k][i][j];
        A[i][n] = p->C[i][k];
    }
    return gauss_jordan(A, n, V);
}

void enumeracion_exhaustiva(ModeloDecision *p) {
    titulo("METODO 1: ENUMERACION EXHAUSTIVA DE POLITICAS");

    int n = p->num_estados, i, k, num = 0;
    int politica_actual[MAX_E], politica_optima[MAX_E];
    double valor_optimo = peor_valor(p);

    for (i = 0; i < n; i++) {
        politica_actual[i] = -1;
        for (k = 0; k < p->num_decisiones; k++)
            if (p->decision_valida[i][k]) { politica_actual[i] = k; break; }
        if (politica_actual[i] < 0) { printf("  [ERROR] Estado %d sin decisiones.\n", i+1); return; }
    }

    int total = 1;
    for (i = 0; i < n; i++) {
        int cnt = 0;
        for (k = 0; k < p->num_decisiones; k++) if (p->decision_valida[i][k]) cnt++;
        total *= cnt;
    }
    printf("  Total de politicas: %d\n", total);

    while (1) {
        num++;
        sep_simple();
        printf("  Politica %d/%d: ", num, total);
        for (i = 0; i < n; i++)
            printf("[%s->%s] ", p->nombre_estado[i], p->nombre_decision[politica_actual[i]]);
        printf("\n");

        double pi[MAX_E];
        if (!vector_estacionario(p, politica_actual, pi)) {
            printf("  Sistema singular — se omite.\n");
        } else {
            double ec = 0;
            printf("  pi = ( ");
            for (i = 0; i < n; i++) {
                printf("%.5f", pi[i]);
                if (i < n-1) printf("  ");
                ec += pi[i] * p->C[i][politica_actual[i]];
            }
            printf(" )\n");
            printf("  %s = ", p->es_maximizar ? "G(C)" : "E(C)");
            for (i = 0; i < n; i++) {
                printf("%.5f*%.2f", pi[i], p->C[i][politica_actual[i]]);
                if (i < n-1) printf(" + ");
            }
            printf(" = %.4f\n", ec);
            if (es_mejor(p, ec, valor_optimo)) {
                valor_optimo = ec;
                memcpy(politica_optima, politica_actual, n * sizeof(int));
            }
        }


        int pos = n-1;
        while (pos >= 0) {
            int sig = politica_actual[pos] + 1;
            while (sig < p->num_decisiones && !p->decision_valida[pos][sig]) sig++;
            if (sig < p->num_decisiones) { politica_actual[pos] = sig; break; }
            for (k = 0; k < p->num_decisiones; k++)
                if (p->decision_valida[pos][k]) { politica_actual[pos] = k; break; }
            pos--;
        }
        if (pos < 0) break;
    }

    sep_doble();
    printf("  RESULTADO FINAL\n");
    sep_doble();
    print_politica(p, politica_optima, "Politica optima");
    printf("  %s = %.4f\n",
           p->es_maximizar ? "Ganancia maxima" : "Costo minimo", valor_optimo);
}

typedef struct {
    int    m, n;
    double T[MAX_S][MAX_S];
    int    base[MAX_S];
} Simp;

void simp_init(Simp *s, int m, int n) {
    s->m = m; s->n = n;
    memset(s->T, 0, sizeof(s->T));
}
void simp_eq(Simp *s, int row, double *c, double rhs) {
    int j, n = s->n, m = s->m;
    for (j = 0; j < n; j++) s->T[row][j] = c[j];
    s->T[row][n+row] = 1.0;
    s->T[row][n+m]   = rhs;
    s->base[row]     = n+row;
}
void simp_obj(Simp *s, double *obj, int maximizar) {
    int i, j, n = s->n, m = s->m;
    for (j = 0; j < n; j++)
        s->T[m][j] = maximizar ? -obj[j] : obj[j];
    for (j = n; j < n+m; j++) s->T[m][j] = BIGM;
    s->T[m][n+m] = 0.0;
    for (i = 0; i < m; i++) {
        double f = s->T[m][s->base[i]];
        if (fabs(f) < EPS) continue;
        for (j = 0; j <= n+m; j++) s->T[m][j] -= f * s->T[i][j];
    }
}
int simp_solve(Simp *s, double *sol) {
    int it, i, j, m = s->m, n = s->n, tot = n+m;
    for (it = 0; it < MAX_IT; it++) {
        int ent = -1; double mn = EPS;
        for (j = 0; j < tot; j++)
            if (s->T[m][j] < -mn) { mn = -s->T[m][j]; ent = j; }
        if (ent < 0) break;
        int sal = -1; double mr = INF;
        for (i = 0; i < m; i++) {
            if (s->T[i][ent] < EPS) continue;
            double r = s->T[i][tot] / s->T[i][ent];
            if (r < mr - EPS) { mr = r; sal = i; }
        }
        if (sal < 0) return -1;
        double pv = s->T[sal][ent];
        for (j = 0; j <= tot; j++) s->T[sal][j] /= pv;
        for (i = 0; i <= m; i++) {
            if (i == sal) continue;
            double f = s->T[i][ent];
            if (fabs(f) < EPS) continue;
            for (j = 0; j <= tot; j++) s->T[i][j] -= f * s->T[sal][j];
        }
        s->base[sal] = ent;
    }
    for (j = 0; j < tot; j++) sol[j] = 0.0;
    for (i = 0; i < m; i++) sol[s->base[i]] = s->T[i][tot];
    return 0;
}

void programacion_lineal(ModeloDecision *p) {
    titulo(p->es_maximizar
           ? "METODO 2: PROGRAMACION LINEAL (Maximizar)"
           : "METODO 2: PROGRAMACION LINEAL (Minimizar)");

    int n = p->num_estados, i, j, k, v;
    int idx[MAX_E][MAX_D], vi[MAX_Y], vk[MAX_Y], nv = 0;
    for (i = 0; i < n; i++)
        for (k = 0; k < p->num_decisiones; k++) {
            idx[i][k] = -1;
            if (p->decision_valida[i][k]) { idx[i][k] = nv; vi[nv]=i; vk[nv]=k; nv++; }
        }

    subtitulo("PLANTEAMIENTO DEL PPL");
    printf("  %s Z =\n", p->es_maximizar ? "Max" : "Min");
    for (v = 0; v < nv; v++)
        printf("    %c %.2f * Y(%s,%s)\n",
               v==0?' ':'+',
               p->C[vi[v]][vk[v]], p->nombre_estado[vi[v]], p->nombre_decision[vk[v]]);

    printf("\n  s.a.\n");
    printf("  [R0] ");
    for (v = 0; v < nv; v++) {
        if (v > 0) printf(" + ");
        printf("Y(%d,%d)", vi[v]+1, vk[v]+1);
    }
    printf(" = 1\n");

    for (j = 0; j < n; j++) {
        printf("  [R%d] ", j+1);
        int prim = 1;
        for (k = 0; k < p->num_decisiones; k++) {
            if (!p->decision_valida[j][k]) continue;
            if (!prim) printf(" + ");
            printf("Y(%d,%d)", j+1, k+1);
            prim = 0;
        }
        for (v = 0; v < nv; v++) {
            double pij = p->P[vk[v]][vi[v]][j];
            if (fabs(pij) < EPS) continue;
            printf(" - %.4g*Y(%d,%d)", pij, vi[v]+1, vk[v]+1);
        }
        printf(" = 0\n");
    }
    printf("  Y(i,k) >= 0\n");

    subtitulo("RESOLUCION (Simplex Big-M)");
    int nr = 1 + n;
    Simp S; simp_init(&S, nr, nv);
    double row[MAX_Y];

    for (v = 0; v < nv; v++) row[v] = 1.0;
    simp_eq(&S, 0, row, 1.0);
    for (j = 0; j < n; j++) {
        memset(row, 0, nv*sizeof(double));
        for (v = 0; v < nv; v++) {
            if (vi[v] == j) row[v] += 1.0;
            row[v] -= p->P[vk[v]][vi[v]][j];
        }
        simp_eq(&S, j+1, row, 0.0);
    }
    double obj[MAX_Y];
    for (v = 0; v < nv; v++) obj[v] = p->C[vi[v]][vk[v]];
    simp_obj(&S, obj, p->es_maximizar);

    double sol[MAX_S];
    if (simp_solve(&S, sol) != 0) {
        printf("  [ERROR] Problema no acotado.\n"); return;
    }

    subtitulo("SOLUCION");
    double Z = 0;
    printf("  %-30s  %10s\n", "Variable", "Valor");
    sep_simple();
    for (v = 0; v < nv; v++) {
        printf("  Y(%d,%d) = %10.6f\n", vi[v]+1, vk[v]+1, sol[v]);
        Z += p->C[vi[v]][vk[v]] * sol[v];
    }
    sep_simple();
    printf("  Z* = %.4f\n\n", Z);

    int pol_opt[MAX_E];
    printf("  Politica optima:\n");
    for (i = 0; i < n; i++) {
        double sm = 0;
        for (k = 0; k < p->num_decisiones; k++) if (p->decision_valida[i][k]) sm += sol[idx[i][k]];
        pol_opt[i] = 0; double best = -INF;
        for (k = 0; k < p->num_decisiones; k++) {
            if (!p->decision_valida[i][k]) continue;
            double D = (sm > EPS) ? sol[idx[i][k]] / sm : 0.0;
            if (D > best) { best = D; pol_opt[i] = k; }
        }
        printf("    E%d -> D%d  Y=%.6f\n", i+1, pol_opt[i]+1, sol[idx[i][pol_opt[i]]]);
    }
    sep_doble();
    printf("  Pol optima: [ ");
    for (i=0;i<n;i++) { printf("E%d->D%d",i+1,pol_opt[i]+1); if(i<n-1) printf(" | "); }
    printf(" ]\n");
    printf("  Z* = %.4f\n", Z);
}

/* Funcion auxiliar para calcular el valor de mejoramiento sin descuento */
static double calc_mejoramiento(int estado, int decision, int n,
                                ModeloDecision *p, double *V) {
    int j;
    double suma = 0.0;
    for (j = 0; j < n; j++)
        suma += p->P[decision][estado][j] * V[j];
    return p->C[estado][decision] + suma - V[estado];
}

void mejoramiento_politicas(ModeloDecision *p) {
    titulo("METODO 3: MEJORAMIENTO DE POLITICAS");

    int n = p->num_estados, i, k, iter;
    int politica_actual[MAX_E], politica_nueva[MAX_E];
    double V[MAX_E], g;

    /* --- Politica inicial --- */
    printf("  Ingrese la politica inicial (numero de decision del 1 al %d por cada estado):\n",
           p->num_decisiones);
    for (i = 0; i < n; i++) {
        int d;
        do {
            printf("    Decision inicial para el estado %d [%s]: ",
                   i+1, p->nombre_estado[i]);
            scanf("%d", &d);
            d--;
            if (d < 0 || d >= p->num_decisiones || !p->decision_valida[i][d])
                printf("    Opcion no valida, intente de nuevo.\n");
        } while (d < 0 || d >= p->num_decisiones || !p->decision_valida[i][d]);
        politica_actual[i] = d;
    }

    printf("\n  Politica inicial: ");
    print_politica(p, politica_actual, "R0");
    printf("  Maximo de iteraciones configurado: %d\n", p->max_iteraciones);

    /* --- Iteraciones --- */
    for (iter = 1; iter <= MAX_IT; iter++) {
        sep_doble();
        printf("  ITERACION %d\n", iter);
        sep_doble();

        /* Paso 1: evaluar politica actual */
        subtitulo("PASO 1 — Evaluacion de la politica actual");
        printf("  Sistema: g + Vi - sum_j pij(k)*Vj = C(i,k)   [V_{n-1} = 0]\n\n");
        for (i = 0; i < n; i++) {
            int k2 = politica_actual[i], j2;
            printf("  Ec.%d  [%s | %s]:  g", i+1,
                   p->nombre_estado[i], p->nombre_decision[k2]);
            for (j2 = 0; j2 < n-1; j2++) {
                double c = (i==j2 ? 1.0 : 0.0) - p->P[k2][i][j2];
                if (fabs(c) < EPS) continue;
                printf(" %+.4g*V_%s", c, p->nombre_estado[j2]);
            }
            printf("  =  %.4f\n", p->C[i][k2]);
        }
        if (!evaluar_mp(p, politica_actual, &g, V)) {
            printf("  [ERROR] Sistema singular.\n"); return;
        }
        printf("\n  Costo promedio  g = %.10f\n", g);
        for (i = 0; i < n; i++)
            printf("  V[%s] = %.10f\n", p->nombre_estado[i], V[i]);

        /* Paso 2: mejoramiento */
        subtitulo("PASO 2 — Mejoramiento de la politica");
        printf("  Criterio: %s\n\n", p->es_maximizar ? "Maximizar" : "Minimizar");

        printf("  Politica actual:   ");
        print_politica(p, politica_actual, "");

        for (i = 0; i < n; i++) {
            int mejor_k = politica_actual[i];
            double mejor_val = calc_mejoramiento(i, mejor_k, n, p, V);
            printf("\n  Estado [%s]:\n", p->nombre_estado[i]);
            for (k = 0; k < p->num_decisiones; k++) {
                if (!p->decision_valida[i][k]) continue;
                double val = calc_mejoramiento(i, k, n, p, V);
                printf("    Decision [%-12s]: valor = %.10f", p->nombre_decision[k], val);
                if (val < mejor_val - 1e-8) { mejor_val = val; mejor_k = k; }
                if (mejor_k == k) printf("  <-- MEJOR");
                printf("\n");
            }
            politica_nueva[i] = mejor_k;
        }

        printf("\n  Politica mejorada: ");
        print_politica(p, politica_nueva, "");

        /* Verificar convergencia */
        int igual = 1;
        for (i = 0; i < n; i++)
            if (politica_nueva[i] != politica_actual[i]) { igual = 0; break; }

        if (igual) {
            sep_doble();
            printf("  POLITICA OPTIMA ENCONTRADA\n");
            sep_doble();
            printf("  Politica optima: ");
            print_politica(p, politica_actual, "");
            printf("\n  Costo promedio optimo g = %.10f\n", g);
            return;
        }

        for (i = 0; i < n; i++) politica_actual[i] = politica_nueva[i];

        if (p->max_iteraciones > 0 && iter >= p->max_iteraciones) {
            printf("\n  >>> Limite de %d iteraciones alcanzado.\n", p->max_iteraciones);
            break;
        }
    }

    sep_doble();
    printf("  RESULTADO FINAL (maximo de iteraciones alcanzado)\n");
    sep_doble();
    printf("  La politica obtenida puede no ser optima.\n");
    printf("  Ultima politica: ");
    print_politica(p, politica_actual, "");
    printf("\n  g = %.10f\n", g);
}

/* Funcion auxiliar: valor con descuento para un estado/decision */
static double calc_mejoramiento_desc(int estado, int decision, int n,
                                     double alpha, ModeloDecision *p, double *V) {
    int j;
    double suma = 0.0;
    for (j = 0; j < n; j++)
        suma += p->P[decision][estado][j] * V[j];
    return p->C[estado][decision] + alpha * suma;
}

void mejoramiento_descuento(ModeloDecision *p) {
    titulo("METODO 4: MEJORAMIENTO DE POLITICAS CON DESCUENTO");

    int n = p->num_estados, i, k, iter;
    int politica_actual[MAX_E], politica_nueva[MAX_E];
    double V[MAX_E];
    double alpha;

    /* --- Alpha --- */
    printf("  Ingrese el factor de descuento alpha (0 < alpha < 1): ");
    scanf("%lf", &alpha);
    if (alpha <= 0.0 || alpha >= 1.0) {
        printf("  [ERROR] alpha debe estar en el intervalo (0, 1).\n"); return;
    }
    printf("  alpha = %.4f\n\n", alpha);

    /* --- Politica inicial --- */
    printf("  Ingrese la politica inicial (numero de decision del 1 al %d por cada estado):\n",
           p->num_decisiones);
    for (i = 0; i < n; i++) {
        do {
            printf("    Decision inicial para el estado %d [%s]: ",
                   i+1, p->nombre_estado[i]);
            scanf("%d", &politica_actual[i]);
            politica_actual[i]--;
            if (!p->decision_valida[i][politica_actual[i]])
                printf("    Decision no permitida, intente de nuevo.\n");
        } while (!p->decision_valida[i][politica_actual[i]]);
    }

    printf("\n  Politica inicial: ");
    print_politica(p, politica_actual, "R0");

    /* --- Iteraciones --- */
    printf("\n  ITERACIONES\n");
    for (iter = 1; iter <= MAX_IT; iter++) {
        sep_doble();
        printf("  ITERACION %d\n", iter);
        sep_doble();

        /* Paso 1: resolver sistema Vi - alpha * sum_j pij*Vj = C(i,k) */
        subtitulo("PASO 1 — Calculo de valores con descuento");
        printf("  Sistema: Vi - alpha*sum_j pij(k)*Vj = C(i,k)   alpha=%.4f\n\n", alpha);
        for (i = 0; i < n; i++) {
            int k2 = politica_actual[i], j2;
            printf("  Ec.%d  [%s | %s]:  V_%s",
                   i+1, p->nombre_estado[i], p->nombre_decision[k2], p->nombre_estado[i]);
            for (j2 = 0; j2 < n; j2++) {
                if (j2 == i) continue;
                double c = -alpha * p->P[k2][i][j2];
                if (fabs(c) < EPS) continue;
                printf(" %+.4g*V_%s", c, p->nombre_estado[j2]);
            }
            printf("  =  %.4f\n", p->C[i][k2]);
        }
        if (!evaluar_desc(p, politica_actual, alpha, V)) {
            printf("  [ERROR] Sistema singular.\n"); return;
        }
        printf("\n");
        for (i = 0; i < n; i++)
            printf("  V[%s] = %.10f\n", p->nombre_estado[i], V[i]);

        /* Paso 2: mejoramiento */
        subtitulo("PASO 2 — Mejoramiento de la politica");
        printf("  Criterio: %s  (C(i,k) + alpha*sum_j pij(k)*Vj)\n\n",
               p->es_maximizar ? "Maximizar" : "Minimizar");

        printf("  Politica actual:   ");
        print_politica(p, politica_actual, "");

        for (i = 0; i < n; i++) {
            int mejor_k = politica_actual[i];
            double mejor_val = calc_mejoramiento_desc(i, mejor_k, n, alpha, p, V);
            printf("\n  Estado [%s]:\n", p->nombre_estado[i]);
            for (k = 0; k < p->num_decisiones; k++) {
                if (!p->decision_valida[i][k]) continue;
                double val = calc_mejoramiento_desc(i, k, n, alpha, p, V);
                printf("    Decision [%-12s]: %.4f + %.4f*(...) = %.10f",
                       p->nombre_decision[k], p->C[i][k], alpha, val);
                if (val < mejor_val - 1e-8) { mejor_val = val; mejor_k = k; }
                if (mejor_k == k) printf("  <-- MEJOR");
                printf("\n");
            }
            politica_nueva[i] = mejor_k;
        }

        printf("\n  Politica mejorada: ");
        print_politica(p, politica_nueva, "");

        /* Verificar convergencia */
        int igual = 1;
        for (i = 0; i < n; i++)
            if (politica_nueva[i] != politica_actual[i]) { igual = 0; break; }

        if (igual) {
            sep_doble();
            printf("  POLITICA OPTIMA ENCONTRADA\n");
            sep_doble();
            printf("  Politica optima: ");
            print_politica(p, politica_actual, "");
            printf("\n  alpha = %.4f\n", alpha);
            for (i = 0; i < n; i++)
                printf("  V[%s] = %.10f\n", p->nombre_estado[i], V[i]);
            return;
        }

        for (i = 0; i < n; i++) politica_actual[i] = politica_nueva[i];

        if (p->max_iteraciones > 0 && iter >= p->max_iteraciones) {
            printf("\n  >>> Limite de %d iteraciones alcanzado.\n", p->max_iteraciones);
            break;
        }
    }

    sep_doble();
    printf("  MAXIMO DE ITERACIONES ALCANZADO\n");
    sep_doble();
    printf("  La politica obtenida puede no ser optima.\n");
    printf("  Ultima politica: ");
    print_politica(p, politica_actual, "");
    printf("\n  alpha = %.4f\n", alpha);
    for (i = 0; i < n; i++)
        printf("  V[%s] = %.10f\n", p->nombre_estado[i], V[i]);
}

void aproximaciones_sucesivas(ModeloDecision *p) {
    titulo("METODO 5: APROXIMACIONES SUCESIVAS");

    int n = p->num_estados, i, j, k, iter;
    double alpha, epsilon;
    int N;

    /* --- Parametros --- */
    printf("  Ingrese el factor de descuento alpha (0 < alpha <= 1): ");
    scanf("%lf", &alpha);
    if (alpha <= 0.0 || alpha > 1.0) {
        printf("  [ERROR] alpha invalido.\n"); return;
    }
    printf("  Ingrese la tolerancia de convergencia epsilon (ej. 0.0001): ");
    scanf("%lf", &epsilon);

    N = p->max_iteraciones > 0 ? p->max_iteraciones : 100;
    printf("  alpha = %.4f   epsilon = %.6f   Max. iteraciones = %d\n\n", alpha, epsilon, N);

    double V_anterior[MAX_E], V_nuevo[MAX_E];
    int politica[MAX_E];

    /* =========================================
       ITERACION 1: V_i^1 = min{ C(i,k) }
    ========================================= */
    sep_simple();
    printf("  ITERACION 1\n");
    printf("  V_i^1 = %s{ C(i,k) }\n", p->es_maximizar ? "max" : "min");
    sep_simple();

    for (i = 0; i < n; i++) {
        int primera = 1;
        double mejor_val = 0.0;
        int mejor_k = 0;
        for (k = 0; k < p->num_decisiones; k++) {
            if (!p->decision_valida[i][k]) continue;
            printf("    Estado [%s], Decision [%s]: C = %.4f\n",
                   p->nombre_estado[i], p->nombre_decision[k], p->C[i][k]);
            if (primera || p->C[i][k] < mejor_val) {
                mejor_val = p->C[i][k];
                mejor_k   = k;
                primera   = 0;
            }
        }
        V_anterior[i] = mejor_val;
        politica[i]   = mejor_k;
        printf("    => V[%s]^1 = %.10f   Decision = [%s]\n\n",
               p->nombre_estado[i], V_anterior[i], p->nombre_decision[mejor_k]);
    }

    /* =========================================
       ITERACIONES SUCESIVAS
    ========================================= */
    for (iter = 2; iter <= N; iter++) {
        int convergio    = 1;
        int pol_igual    = 1;
        int politica_ant[MAX_E];

        for (i = 0; i < n; i++) politica_ant[i] = politica[i];

        sep_simple();
        printf("  ITERACION %d\n", iter);
        sep_simple();

        for (i = 0; i < n; i++) {
            int primera = 1;
            double mejor_val = 0.0;
            int mejor_k = 0;

            printf("  Estado [%s]:\n", p->nombre_estado[i]);
            for (k = 0; k < p->num_decisiones; k++) {
                if (!p->decision_valida[i][k]) continue;
                double suma = 0.0;
                for (j = 0; j < n; j++)
                    suma += p->P[k][i][j] * V_anterior[j];
                double val = p->C[i][k] + alpha * suma;
                printf("    Decision [%-12s]: %.4f + %.4f * suma = %.10f",
                       p->nombre_decision[k], p->C[i][k], alpha, val);
                if (primera || val < mejor_val) {
                    mejor_val = val;
                    mejor_k   = k;
                    primera   = 0;
                }
                if (mejor_k == k) printf("  <-- MEJOR");
                printf("\n");
            }

            V_nuevo[i]  = mejor_val;
            politica[i] = mejor_k;
            printf("    => V[%s]^%d = %.10f   Decision = [%s]\n\n",
                   p->nombre_estado[i], iter, V_nuevo[i], p->nombre_decision[mejor_k]);

            if (fabs(V_nuevo[i] - V_anterior[i]) >= epsilon) convergio = 0;
            if (politica[i] != politica_ant[i])               pol_igual = 0;
        }

        /* Prueba de optimalidad */
        if (convergio && pol_igual) {
            sep_doble();
            printf("  PRUEBA DE OPTIMALIDAD CUMPLIDA  (convergencia en iteracion %d)\n", iter);
            sep_doble();
            printf("  Politica optima: ");
            print_politica(p, politica, "");
            printf("\n\n  Valores optimos:\n");
            for (i = 0; i < n; i++)
                printf("  V[%s] = %.10f\n", p->nombre_estado[i], V_nuevo[i]);
            return;
        }

        for (i = 0; i < n; i++) V_anterior[i] = V_nuevo[i];
    }

    /* No convergió */
    sep_doble();
    printf("  SE ALCANZO EL NUMERO MAXIMO DE ITERACIONES (%d)\n", N);
    sep_doble();
    printf("  La solucion obtenida puede no ser optima.\n\n");
    printf("  Ultima politica: ");
    print_politica(p, politica, "");
    printf("\n\n  Ultimos valores:\n");
    for (i = 0; i < n; i++)
        printf("  V[%s] = %.10f\n", p->nombre_estado[i], V_nuevo[i]);
}

int main(void) {
    ModeloDecision modelo;
    memset(&modelo, 0, sizeof(modelo));

    sep_doble();
    printf("  PROYECTO DE PROCESOS ESTOCASTICOS\n");
    sep_doble();
    printf("  INTEGRANTES:\n");
    printf("    - Hernandez Montes Victoria del Carmen\n");
    printf("    - Estefani Michelle Lira\n");
    printf("    - Leonardo Escareno\n");
    sep_doble();

    leer_modelo(&modelo);

    int op;
    do {
        sep_doble();
        printf("  MENU PRINCIPAL\n");
        sep_doble();
        printf("  1. Enumeracion Exhaustiva\n");
        printf("  2. Programacion Lineal (Simplex)\n");
        printf("  3. Mejoramiento de Politicas\n");
        printf("  4. Mejoramiento con Descuento\n");
        printf("  5. Aproximaciones Sucesivas\n");
        printf("  0. Salir\n");
        sep_simple();
        printf("  Opcion: "); scanf("%d", &op);

        switch (op) {
            case 1: enumeracion_exhaustiva(&modelo);  break;
            case 2: programacion_lineal(&modelo);     break;
            case 3: mejoramiento_politicas(&modelo);  break;
            case 4: mejoramiento_descuento(&modelo);  break;
            case 5: aproximaciones_sucesivas(&modelo); break;
            case 0: printf("  Hasta luego.\n");  break;
            default: printf("  Opcion invalida.\n");
        }
    } while (op != 0);

    return 0;
}
