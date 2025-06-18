#include "comm_strategies.h"
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern bool verbose;

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
        // Receive part of A and wait immediately.
        MPI_Irecv(local_A, elements_per_proc, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }

    // Broadcast B and wait immediately.
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
        // Receive from each worker and wait inside the loop.
        for (int i = 1; i < size; i++)
        {
            MPI_Irecv(C + i * elements_per_proc, elements_per_proc, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, MPI_STATUS_IGNORE);
        }
    }
    else
    {
        // Send the result and wait immediately.
        MPI_Isend(local_C, elements_per_proc, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    }
    comm_time += MPI_Wtime() - comm_phase_start;
    total_end_time = MPI_Wtime();

    if (rank == 0)
    {
        double total_time = total_end_time - total_start_time;
        double comp_time = comp_end_time - comp_start_time;
        if (verbose)
        {
            printf("[VERBOSE] CSV Output: comm_type=async_naive, matrix_size=%d, num_procs=%d, total_time=%.6f, "
                   "comm_time=%.6f, comp_time=%.6f\n",
                   n, size, total_time, comm_time, comp_time);
        }
        else
        {
            printf("async_naive,%d,%d,%.6f,%.6f,%.6f\n", n, size, total_time, comm_time, comp_time);
        }
    }
}