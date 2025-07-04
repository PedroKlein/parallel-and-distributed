#include "comm_strategies.h"
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern bool verbose;

void run_sync(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C)
{
    int elements_per_proc = n * n / size;
    double total_start_time, total_end_time;

    // Local timers for each process
    double comm_time = 0.0, comp_time = 0.0;
    double comp_start_time, comp_end_time, comm_phase_start;

    MPI_Barrier(MPI_COMM_WORLD);
    total_start_time = MPI_Wtime();

    comm_phase_start = MPI_Wtime();
    if (rank == 0)
    {
        for (int i = 1; i < size; i++)
        {
            MPI_Send(A + i * elements_per_proc, elements_per_proc, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
        }
        for (int i = 0; i < elements_per_proc; i++)
            local_A[i] = A[i];
    }
    else
    {
        MPI_Recv(local_A, elements_per_proc, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    MPI_Bcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);
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
    comp_time = comp_end_time - comp_start_time; // Store local computation time

    comm_phase_start = MPI_Wtime();
    if (rank == 0)
    {
        for (int i = 0; i < elements_per_proc; i++)
            C[i] = local_C[i];
        for (int i = 1; i < size; i++)
        {
            MPI_Recv(C + i * elements_per_proc, elements_per_proc, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    else
    {
        MPI_Send(local_C, elements_per_proc, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    }
    comm_time += MPI_Wtime() - comm_phase_start;

    MPI_Barrier(MPI_COMM_WORLD);
    total_end_time = MPI_Wtime();

    double max_comm_time, max_comp_time;

    MPI_Reduce(&comm_time, &max_comm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comp_time, &max_comp_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        double total_time = total_end_time - total_start_time;
        if (verbose)
        {
            printf("[VERBOSE] CSV Output: comm_type=sync, matrix_size=%d, num_procs=%d, total_time=%.6f, "
                   "comm_time=%.6f, comp_time=%.6f\n",
                   n, size, total_time, max_comm_time, max_comp_time);
        }
        else
        {
            printf("sync,%d,%d,%.6f,%.6f,%.6f\n", n, size, total_time, max_comm_time, max_comp_time);
        }
    }
}