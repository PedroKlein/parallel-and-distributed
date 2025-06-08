#pragma once

#include <mpi.h>

void run_collective(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C);
void run_sync(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C);
void run_async(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C);
void run_async_naive(int n, int rank, int size, double *A, double *B, double *C, double *local_A, double *local_C);
