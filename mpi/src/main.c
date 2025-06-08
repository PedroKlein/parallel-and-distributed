#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comm_strategies.h"

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
    if (argc != 3)
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        if (rank == 0)
        {
            printf("Uso: mpirun -np <num_procs> %s <tam_matriz> <tipo_comm>\n", argv[0]);
            printf("Tipos de comunicação: collective, sync, async, async_naive\n");
        }
        MPI_Finalize();
        return 1;
    }

    int size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = atoi(argv[1]);
    char *comm_type = argv[2];

    if (n % size != 0)
    {
        if (rank == 0)
        {
            printf("Erro: O tamanho da matriz (n) deve ser divisível pelo número de processos (size).\n");
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

    // Cleanup
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