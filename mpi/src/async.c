#include "comm_strategies.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void run_async(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C)
{
    int elements_per_proc = n * n / size;
    double total_start_time, total_end_time, comm_time = 0.0, comp_start_time, comp_end_time, comm_phase_start;

    MPI_Barrier(MPI_COMM_WORLD);
    total_start_time = MPI_Wtime();

    comm_phase_start = MPI_Wtime();
    MPI_Request bcast_req;
    if (rank == 0)
    {
        MPI_Request *send_requests = (MPI_Request *)malloc((size - 1) * sizeof(MPI_Request));
        for (int i = 1; i < size; i++)
        {
            MPI_Isend(A + i * elements_per_proc, elements_per_proc, MPI_DOUBLE, i, 0, MPI_COMM_WORLD,
                      &send_requests[i - 1]);
        }
        for (int i = 0; i < elements_per_proc; i++)
            local_A[i] = A[i];
        MPI_Ibcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD, &bcast_req);
        MPI_Wait(&bcast_req, MPI_STATUS_IGNORE);
        free(send_requests); // Free send requests, no longer need to be monitored by rank 0
    }
    else
    {
        MPI_Request recv_a_req;
        MPI_Irecv(local_A, elements_per_proc, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &recv_a_req);
        MPI_Ibcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD, &bcast_req);
        MPI_Request all_requests[] = {recv_a_req, bcast_req};
        MPI_Waitall(2, all_requests, MPI_STATUSES_IGNORE);
    }
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
        MPI_Request *recv_requests = (MPI_Request *)malloc((size - 1) * sizeof(MPI_Request));
        for (int i = 1; i < size; i++)
        {
            MPI_Irecv(C + i * elements_per_proc, elements_per_proc, MPI_DOUBLE, i, 1, MPI_COMM_WORLD,
                      &recv_requests[i - 1]);
        }
        MPI_Waitall(size - 1, recv_requests, MPI_STATUSES_IGNORE);
        free(recv_requests);
    }
    else
    {
        MPI_Request send_req;
        MPI_Isend(local_C, elements_per_proc, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &send_req);
        MPI_Wait(&send_req, MPI_STATUS_IGNORE);
    }
    comm_time += MPI_Wtime() - comm_phase_start;

    total_end_time = MPI_Wtime();

    if (rank == 0)
    {
        double total_time = total_end_time - total_start_time;
        double comp_time = comp_end_time - comp_start_time;
        printf("async,%d,%d,%.6f,%.6f,%.6f\n", n, size, total_time, comm_time, comp_time);
    }
}