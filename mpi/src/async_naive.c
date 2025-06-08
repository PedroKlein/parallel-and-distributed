#include "comm_strategies.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void run_async_naive(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C)
{
    int elements_per_proc = n * n / size;
    double total_start_time, total_end_time, comm_time = 0.0, comp_start_time, comp_end_time, comm_phase_start;
    MPI_Request request;

    MPI_Barrier(MPI_COMM_WORLD);
    total_start_time = MPI_Wtime();

    comm_phase_start = MPI_Wtime();
    if (rank == 0)
    {
        for (int i = 1; i < size; i++)
        {
            MPI_Isend(A + i * elements_per_proc, elements_per_proc, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &request);
        }
        for (int i = 0; i < elements_per_proc; i++)
        {
            local_A[i] = A[i];
        }
    }
    else
    {
        // Recebe a parte de A e espera imediatamente.
        MPI_Irecv(local_A, elements_per_proc, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }

    // Broadcast de B e espera imediatamente.
    MPI_Ibcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE);
    comm_time += MPI_Wtime() - comm_phase_start;

    comp_start_time = MPI_Wtime();
    for (int i = 0; i < n / size; i++)
    {
        for (int j = 0; j < n; j++)
        {
            local_C[i * n + j] = 0.0;
            for (int k = 0; k < n; k++)
            {
                local_C[i * n + j] += local_A[i * n + k] * B[k * n + j];
            }
        }
    }
    comp_end_time = MPI_Wtime();

    comm_phase_start = MPI_Wtime();
    if (rank == 0)
    {
        for (int i = 0; i < elements_per_proc; i++)
        {
            C[i] = local_C[i];
        }
        // Recebe de cada trabalhador e espera dentro do loop.
        for (int i = 1; i < size; i++)
        {
            MPI_Irecv(C + i * elements_per_proc, elements_per_proc, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, MPI_STATUS_IGNORE);
        }
    }
    else
    {
        // Envia o resultado e espera imediatamente.
        MPI_Isend(local_C, elements_per_proc, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }
    comm_time += MPI_Wtime() - comm_phase_start;
    total_end_time = MPI_Wtime();

    if (rank == 0)
    {
        double total_time = total_end_time - total_start_time;
        double comp_time = comp_end_time - comp_start_time;
        printf("async_naive,%d,%d,%.6f,%.6f,%.6f\n", n, size, total_time, comm_time, comp_time);
    }
}