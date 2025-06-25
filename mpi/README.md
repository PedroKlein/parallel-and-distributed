# MPI Matrix Multiplication: A Performance Analysis of Communication Strategies

## 1. Overview

This project is an analysis of the performance of different MPI communication strategies in the context of a parallel matrix multiplication algorithm. The goal is to investigate how synchronous and asynchronous communication methods impact the overall efficiency of the algorithm by measuring performance metrics like execution time across various workloads and process counts. 

The core task is to multiply two square matrices, `C = A * B`, by partitioning matrix `A` among multiple MPI processes and broadcasting matrix `B` to all of them.

This work is part of the "Programação Distribuída e Paralela" course at the Universidade Federal do Rio Grande do Sul. 

- **Course:** Programação Distribuída e Paralela 
- **University:** Universidade Federal do Rio Grande do Sul 
- **Due Date:** June 30, 2025 

## 2. Project Structure

The project is organized into source, include, and build directories, managed by CMake and a Makefile for easy automation.

```
.
├── CMakeLists.txt
├── Makefile
├── batch_executor.py
├── include/
│   └── comm_strategies.h
└── src/
    ├── main.c
    ├── collective.c
    ├── sync.c
    ├── async.c
    └── async_new.c
```

## 3. Implemented Communication Strategies

Four distinct communication strategies have been implemented for comparison:

-   **`collective`**: Uses high-level MPI collective operations (`MPI_Scatter`, `MPI_Bcast`, `MPI_Gather`) for data distribution and collection.
-   **`sync`**: Uses blocking, point-to-point communication (`MPI_Send`, `MPI_Recv`) for explicit data handling.
-   **`async`**: A non-blocking implementation (`MPI_Isend`, `MPI_Irecv`) where `MPI_Wait` is called immediately after initiating the communication. This pattern simulates blocking behavior and serves as a baseline for comparison against a true non-blocking approach.
-   **`async_new`**: An improved non-blocking implementation that initiates multiple communication requests and uses `MPI_Waitall` to wait for their completion, aiming to overlap communication with computation where possible.

## 4. Prerequisites

To build and run this project, you will need:
- A C compiler (e.g., GCC, Clang)
- An MPI implementation (e.g., Open MPI, MPICH)
- CMake (version 3.10 or higher)
- Make
- Python 3 (for the batch testing script)

## 5. How to Build

A `Makefile` is provided to simplify the build process, which uses CMake internally. From the project's root directory, simply run:

```bash
make
```
This command will create a `build/` directory, run CMake to configure the project, and compile the source code, resulting in an executable at `build/mpi_matmult`.

## 6. How to Run

The `Makefile` also provides several targets to easily run the application in different modes, including inside Docker Compose for local multi-node testing.

### Single Run (Native)

To execute a single instance of the program for a quick test. You can override the default parameters (`NP`, `N`, `COMM_TYPE`) directly from the command line.

```bash
# Run with default parameters (4 processes, 256x256 matrix, collective)
make run

# Run with custom parameters
make run NP=8 N=1024 COMM_TYPE=async
```

### Validation Run

To verify that the parallel computation is correct, you can run any configuration with result validation. This will compute the result sequentially and compare it against the parallel result.

**Note:** The sequential calculation is slow for large matrices. It is recommended to use this mode with smaller matrix sizes (e.g., N <= 512).

```bash
# Run validation with default parameters
make validate

# Run validation with custom parameters
make validate NP=2 N=128 COMM_TYPE=sync
```

### Batch Tests for Performance Analysis

To run the full suite of tests and generate a CSV file for analysis, use the `test` target. This will execute the `batch_executor.py` script.

```bash
make test
```
This script will iterate through all combinations of communication types, matrix sizes, and process counts defined within it. Upon completion, it will generate a `mpi_results.csv` file containing aggregated performance data (mean, standard deviation) for each configuration, which can then be used for plotting and analysis.

### Running in Docker Compose (Local Multi-Node Testing)

To test your MPI code locally with multiple nodes using Docker Compose:

1. Build and start the cluster:

   ```bash
   docker-compose build
   docker-compose up -d
   ```

2. Run the MPI application in the cluster:

   ```bash
   make run-docker NP=4 N=1024 COMM_TYPE=collective
   ```
   - This runs the application across the main and 3 worker nodes.

3. Run the batch test in the cluster:

   ```bash
   make batch-test-docker
   ```
   - This executes the batch test script inside the main node container.

4. To stop the cluster:

   ```bash
   docker-compose down
   ```

## 7. Cleaning Up

To remove the `build` directory and all generated files (including `mpi_results.csv`), run:

```bash
make clean
```

## 8. Running a Local SLURM Cluster with Docker Compose

You can simulate an HPC SLURM environment locally using Docker Compose. This allows you to submit jobs to a master node, which will schedule them across worker nodes, just like on a real cluster.

### Quick Start

1. **Start the cluster:**

   ```bash
   make start-docker-cluster
   ```

2. **SSH into the master node:**

   ```bash
   make ssh-master
   # or manually:
   ssh root@localhost -p 2222
   # (password: root)
   ```

3. **Submit a SLURM batch job:**

   ```bash
   make submit-job
   # or, inside the master node:
   sbatch /workspace/hype.slurm
   ```

4. **Check SLURM job status:**

   ```bash
   make slurm-status
   # or, inside the master node:
   squeue -u root
   ```

5. **Cancel the last SLURM job:**

   ```bash
   make slurm-cancel
   # or, inside the master node:
   scancel $(squeue -u root -h -o '%A' | head -n 1)
   ```

6. **Stop the cluster:**

   ```bash
   make stop-docker-cluster
   ```

### Notes
- All project files are shared between your host and the containers via `/workspace`.
- Only the master node exposes SSH (port 2222).
- You can edit code on your host and immediately run jobs in the cluster.
- The default SSH password is `root` (for local testing only).