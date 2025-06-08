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
            fprintf(stderr, "Usage: mpirun -np <procs> %s <n> <comm_type> [--validate]\n", argv[0]);
            fprintf(stderr, "Communication types: collective, sync, async, async_naive\n");
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
            fprintf(stderr, "Error: Matrix size (n) must be divisible by the number of processes (size).\n");
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
            fprintf(stderr, "Error: Invalid communication type '%s'.\n", comm_type);
        }
    }

    // --- Validation Step (Outside timing) ---
    if (rank == 0 && validation_enabled)
    {
        printf("--- Starting Validation ---\n");
        double *C_sequential = (double *)malloc(n * n * sizeof(double));
        if (C_sequential == NULL)
        {
            fprintf(stderr, "Failed to allocate memory for validation matrix.\n");
        }
        else
        {
            printf("Calculating sequential result for comparison...\n");
            sequential_matrix_multiplication(n, A, B, C_sequential);

            printf("Comparing parallel result with sequential...\n");
            validate_results(n, C, C_sequential);

            free(C_sequential);
        }
        printf("--- Validation Finished ---\n");
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

/**
 * @brief Computes matrix multiplication C = A * B sequentially.
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
 * @brief Compares two matrices, C_parallel and C_sequential, element by element.
 * @return Returns true if they are equal (within a tolerance), false otherwise.
 */
bool validate_results(int n, double *C_parallel, double *C_sequential)
{
    const double epsilon = 1e-6; // tolerance for floating point errors

    for (int i = 0; i < n * n; i++)
    {
        if (fabs(C_parallel[i] - C_sequential[i]) > epsilon)
        {
            int row = i / n;
            int col = i % n;
            fprintf(stderr, "VALIDATION ERROR at position [%d][%d]!\n", row, col);
            fprintf(stderr, "  - Parallel Value:   %f\n", C_parallel[i]);
            fprintf(stderr, "  - Sequential Value: %f\n", C_sequential[i]);
            fprintf(stderr, "  - Difference:       %e\n", fabs(C_parallel[i] - C_sequential[i]));
            printf("VALIDATION FAILED.\n");
            return false;
        }
    }

    printf("VALIDATION SUCCESSFUL: The parallel result is correct.\n");
    return true;
}