#include "comm_strategies.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void run_sync(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C)
{
    int elements_per_proc = n * n / size;
    double total_start_time, total_end_time, comm_time = 0.0, comp_start_time, comp_end_time, comm_phase_start;

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

    total_end_time = MPI_Wtime();

    if (rank == 0)
    {
        double total_time = total_end_time - total_start_time;
        double comp_time = comp_end_time - comp_start_time;
        printf("sync,%d,%d,%.6f,%.6f,%.6f\n", n, size, total_time, comm_time, comp_time);
    }
}