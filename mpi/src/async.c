#include "comm_strategies.h"
#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern bool verbose;

void run_async(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C)
{
    int elements_per_proc = n * n / size;
    MPI_Request request;

    double comm_time = 0.0;
    double comp_time = 0.0;
    double start_block, end_block;
    double total_start_time, total_end_time;

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize all processes before starting the main timer
    total_start_time = MPI_Wtime();

    // --- TIMING BLOCK 1: Communication for distributing A ---
    // This block includes the Isend loop on rank 0 and the Irecv+Wait on workers.
    start_block = MPI_Wtime();
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
    end_block = MPI_Wtime();
    comm_time += end_block - start_block;
    // --------------------------------------------------------

    // --- TIMING BLOCK 2: Communication for distributing B ---
    start_block = MPI_Wtime();
    // Broadcast B and wait immediately.
    MPI_Ibcast(B, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD, &request);
    MPI_Wait(&request, MPI_STATUS_IGNORE);
    end_block = MPI_Wtime();
    comm_time += end_block - start_block;
    // --------------------------------------------------------

    // --- TIMING BLOCK 3: Computation ---
    start_block = MPI_Wtime();
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
    end_block = MPI_Wtime();
    comp_time += end_block - start_block;
    // -----------------------------------

    // --- TIMING BLOCK 4: Communication for gathering C ---
    start_block = MPI_Wtime();
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
    end_block = MPI_Wtime();
    comm_time += end_block - start_block;
    // -----------------------------------------------------

    // Ensure all processes have finished their work before stopping the total timer
    MPI_Barrier(MPI_COMM_WORLD);
    total_end_time = MPI_Wtime();

    // ======================= AGGREGATE AND PRINT TIMING =======================
    // Use MPI_Reduce to find the maximum communication and computation times
    // across all processes and report them from rank 0.
    double max_comm_time;
    double max_comp_time;

    MPI_Reduce(&comm_time, &max_comm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comp_time, &max_comp_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        // The total_time is the wall-clock time from start to finish.
        double total_time = total_end_time - total_start_time;

        // The max_comp_time and max_comm_time are the aggregated results.
        if (verbose)
        {
            printf("[VERBOSE] CSV Output: comm_type=async, matrix_size=%d, num_procs=%d, total_time=%.6f, "
                   "comm_time=%.6f, comp_time=%.6f\n",
                   n, size, total_time, max_comm_time, max_comp_time);
        }
        else
        {
            printf("async,%d,%d,%.6f,%.6f,%.6f\n", n, size, total_time, max_comm_time, max_comp_time);
        }
    }
}