#include <math.h>
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm_strategies.h"

void sequential_matrix_multiplication(int n, double *A, double *B, double *C_sequential);
bool validate_results(int n, double *C_parallel, double *C_sequential);

void initialize_matrices(int n, double *A, double *B, double *C)
{
    for (int i = 0; i < n * n; i++)
    {
        A[i] = (double)(i % 100);
        B[i] = (double)((i % 100) + 1);
        C[i] = 0.0;
    }
}

int main(int argc, char *argv[])
{
    int rank = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3 || argc > 4)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Uso: mpirun -np <procs> %s <n> <comm_type> [--validate]\n", argv[0]);
            fprintf(stderr, "Tipos de comunicação: collective, sync, async, async_naive\n");
        }
        MPI_Finalize();
        return 1;
    }

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = atoi(argv[1]);
    char *comm_type = argv[2];
    bool validation_enabled = (argc == 4 && strcmp(argv[3], "--validate") == 0);

    if (n % size != 0)
    {
        if (rank == 0)
        {
            fprintf(stderr, "Erro: O tamanho da matriz (n) deve ser divisível pelo número de processos (size).\n");
        }
        MPI_Finalize();
        return 1;
    }

    double *A = NULL, *B = NULL, *C = NULL;
    int rows_per_proc = n / size;
    int elements_per_proc = n * rows_per_proc;

    B = (double *)malloc(n * n * sizeof(double));
    double *local_A = (double *)malloc(elements_per_proc * sizeof(double));
    double *local_C = (double *)malloc(elements_per_proc * sizeof(double));

    if (rank == 0)
    {
        A = (double *)malloc(n * n * sizeof(double));
        C = (double *)malloc(n * n * sizeof(double));
        initialize_matrices(n, A, B, C);
    }

    if (strcmp(comm_type, "collective") == 0)
    {
        run_collective(n, rank, size, A, B, C, local_A, local_C);
    }
    else if (strcmp(comm_type, "sync") == 0)
    {
        run_sync(n, rank, size, A, B, C, local_A, local_C);
    }
    else if (strcmp(comm_type, "async") == 0)
    {
        run_async(n, rank, size, A, B, C, local_A, local_C);
    }
    else if (strcmp(comm_type, "async_naive") == 0)
    {
        run_async_naive(n, rank, size, A, B, C, local_A, local_C);
    }
    else
    {
        if (rank == 0)
        {
            fprintf(stderr, "Erro: Tipo de comunicação '%s' inválido.\n", comm_type);
        }
    }

    // --- Etapa de Validação (Fora da cronometragem) ---
    if (rank == 0 && validation_enabled)
    {
        printf("--- Iniciando Validação ---\n");
        double *C_sequential = (double *)malloc(n * n * sizeof(double));
        if (C_sequential == NULL)
        {
            fprintf(stderr, "Falha ao alocar memória para a matriz de validação.\n");
        }
        else
        {
            printf("Calculando resultado sequencial para comparação...\n");
            sequential_matrix_multiplication(n, A, B, C_sequential);

            printf("Comparando resultado paralelo com sequencial...\n");
            validate_results(n, C, C_sequential);

            free(C_sequential);
        }
        printf("--- Validação Concluída ---\n");
    }

    // Limpeza
    free(B);
    free(local_A);
    free(local_C);
    if (rank == 0)
    {
        free(A);
        free(C);
    }

    MPI_Finalize();
    return 0;
}

/**
 * @brief Calcula a multiplicação de matrizes C = A * B de forma puramente sequencial.
 */
void sequential_matrix_multiplication(int n, double *A, double *B, double *C_sequential)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            double sum = 0.0;
            for (int k = 0; k < n; k++)
            {
                sum += A[i * n + k] * B[k * n + j];
            }
            C_sequential[i * n + j] = sum;
        }
    }
}

/**
 * @brief Compara duas matrizes, C_parallel e C_sequential, elemento por elemento.
 * @return Retorna true se forem iguais (dentro de uma tolerância), false caso contrário.
 */
bool validate_results(int n, double *C_parallel, double *C_sequential)
{
    const double epsilon = 1e-6; // tolerância para erros de ponto flutuante

    for (int i = 0; i < n * n; i++)
    {
        if (fabs(C_parallel[i] - C_sequential[i]) > epsilon)
        {
            int row = i / n;
            int col = i % n;
            fprintf(stderr, "ERRO DE VALIDAÇÃO na posição [%d][%d]!\n", row, col);
            fprintf(stderr, "  - Valor Paralelo:   %f\n", C_parallel[i]);
            fprintf(stderr, "  - Valor Sequencial: %f\n", C_sequential[i]);
            fprintf(stderr, "  - Diferença:        %e\n", fabs(C_parallel[i] - C_sequential[i]));
            printf("VALIDAÇÃO FALHOU.\n");
            return false;
        }
    }

    printf("VALIDAÇÃO BEM-SUCEDIDA: O resultado paralelo está correto.\n");
    return true;
}