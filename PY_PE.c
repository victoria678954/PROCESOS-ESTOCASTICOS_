#include <stdio.h>
#include <stdlib.h>

void limpiarPantalla() {
    system("cls");
}

void esperarEnter() {
    printf("\nPresione ENTER para continuar...");
    getchar();
    getchar();
}

int main() {

    int opcion1, opcion2;
    int filas, columnas;
    int i, j;

    float matriz[100][100];
    float sumaFila;

    /* ================= PRIMERA PANTALLA ================= */

    limpiarPantalla();

    printf("=====================================\n");
    printf("   PROYECTO DE PROCESOS ESTOCASTICOS\n");
    printf("=====================================\n");

    printf("\nPresione ENTER para continuar...");
    getchar();

    /* ================= MENU PRINCIPAL ================= */

    do {

        limpiarPantalla();

        printf("=====================================\n");
        printf("            MENU PRINCIPAL\n");
        printf("=====================================\n");

        printf("1. Leer matriz de transicion\n");
        printf("2. Salir\n");

        printf("\nSeleccione una opcion: ");
        scanf("%d", &opcion1);

        switch(opcion1) {

            case 1:

                limpiarPantalla();

                printf("=====================================\n");
                printf("      MATRIZ DE TRANSICION\n");
                printf("=====================================\n");

                /* Leer filas */

                do {

                    printf("\nCantidad de filas: ");
                    scanf("%d", &filas);

                    if(filas <= 0 || filas > 100) {
                        printf("Error: valor invalido\n");
                    }

                } while(filas <= 0 || filas > 100);

                /* Leer columnas */

                do {

                    printf("Cantidad de columnas: ");
                    scanf("%d", &columnas);

                    if(columnas <= 0 || columnas > 100) {
                        printf("Error: valor invalido\n");
                    }

                } while(columnas <= 0 || columnas > 100);

                /* Leer matriz */

                for(i = 0; i < filas; i++) {

                    sumaFila = 0;

                    printf("\n--- FILA %d ---\n", i);

                    for(j = 0; j < columnas; j++) {

                        do {

                            printf("Elemento [%d][%d]: ", i, j);
                            scanf("%f", &matriz[i][j]);

                            if(matriz[i][j] < 0 || matriz[i][j] > 1) {
                                printf("Error: solo valores entre 0 y 1\n");
                            }

                        } while(matriz[i][j] < 0 || matriz[i][j] > 1);

                        sumaFila += matriz[i][j];
                    }

                    /* Validar suma de fila */

                    if(sumaFila > 1.0001 || sumaFila < 0.9999) {

                        printf("\nERROR:\n");
                        printf("La suma de la fila debe ser igual a 1\n");

                        i--;
                    }
                }

                /* Mostrar matriz */

                limpiarPantalla();

                printf("=====================================\n");
                printf("      MATRIZ INGRESADA\n");
                printf("=====================================\n\n");

                for(i = 0; i < filas; i++) {

                    for(j = 0; j < columnas; j++) {
                        printf("%.2f\t", matriz[i][j]);
                    }

                    printf("\n");
                }

                esperarEnter();

                /* Limpiar pantalla antes del segundo menu */

                limpiarPantalla();

                /* ================= SEGUNDO MENU ================= */

                do {

                    printf("=====================================\n");
                    printf("       PROCESOS ESTOCASTICOS\n");
                    printf("=====================================\n");

                    printf("1. P.E.\n");
                    printf("2. Salir\n");

                    printf("\nSeleccione una opcion: ");
                    scanf("%d", &opcion2);

                    switch(opcion2) {

                        case 1:

                            printf("\nOpcion P.E. seleccionada\n");
                            esperarEnter();
                            limpiarPantalla();

                            break;

                        case 2:

                            printf("\nSaliendo del programa...\n");

                            break;

                        default:

                            printf("\nOpcion invalida\n");
                            esperarEnter();
                            limpiarPantalla();
                    }

                } while(opcion2 != 2);

                break;

            case 2:

                printf("\nSaliendo del programa...\n");

                break;

            default:

                printf("\nOpcion invalida\n");
                esperarEnter();
        }

    } while(opcion1 != 2);

    return 0;
}
