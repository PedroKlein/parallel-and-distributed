#!/usr/bin/env python3

import csv
import os
import subprocess
import statistics
import sys
import time
from typing import List

def calculate_stats(times: List[float]):
    """Calculate the mean and standard deviation of a list of times."""
    if not times:
        return 0.0, 0.0
    
    mean = statistics.mean(times)
    std_dev = statistics.stdev(times) if len(times) > 1 else 0.0
    return mean, std_dev

def main():
    """
    Script to run batch tests of matrix multiplication with MPI.
    Works both in a local environment and in a SLURM-managed cluster.
    """
    # --- Environment Detection ---
    IS_SLURM_RUN = 'SLURM_JOB_ID' in os.environ

    # --- Argument Validation ---
    if IS_SLURM_RUN:
        if len(sys.argv) != 3:
            print(f"SLURM Usage: {sys.argv[0]} <executable> <machinefile>")
            sys.exit(1)
        executable = sys.argv[1]
        machinefile = sys.argv[2]
    else: # Local Execution
        if len(sys.argv) != 2:
            print(f"Local Usage: {sys.argv[0]} <executable>")
            sys.exit(1)
        executable = sys.argv[1]
        machinefile = None # Not used locally

    if not os.path.isfile(executable):
        print(f"Error: Executable file '{executable}' not found.")
        sys.exit(1)

    # --- Test Configuration ---
    job_id = os.environ.get('SLURM_JOB_ID', 'local')
    output_csv_file = f"mpi_results_{job_id}.csv"
    repetitions = 10

    comm_types = ["collective", "sync", "async", "async_new"]
    matrix_sizes = [1024, 2048, 4096]

    # Set the number of processes based on the environment
    if IS_SLURM_RUN:
        total_tasks = int(os.environ.get('SLURM_NTASKS', 1))
        print("--- Running in SLURM environment ---")
        process_counts = [total_tasks]  # Always use the number of processes defined in SLURM
    else: # Local
        total_tasks = os.cpu_count() or 4
        print("--- Running in LOCAL environment ---")
        process_counts = [total_tasks]  # Always use all available CPUs locally
        
    print("Starting test batch...")
    print(f"Executable: {executable}")
    print(f"Communication Types: {comm_types}")
    print(f"Matrix Sizes: {matrix_sizes}")
    print(f"Number of Processes: {process_counts}")
    print(f"Repetitions per test: {repetitions}")
    print("-" * 50)

    start_time = time.time()

    with open(output_csv_file, mode="w", newline='') as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerow([
            "comm_type", "matrix_size", "num_procs", "environment",
            "total_time_mean", "total_time_std", "comm_time_mean", "comm_time_std",
            "comp_time_mean", "comp_time_std", "repetitions"
        ])

        for comm_type in comm_types:
            for n in matrix_sizes:
                for p in process_counts:
                    if n % p != 0:
                        continue

                    print(f"Testing: {comm_type}, n={n}, p={p}...")
                    
                    times_data = {'total': [], 'comm': [], 'comp': []}

                    for rep in range(repetitions):
                        # --- Command Construction Specific to Environment ---
                        if IS_SLURM_RUN:
                            # TODO: test with bind-to socket
                            cmd = [
                                "mpirun", "-np", str(p), "-machinefile", machinefile,
                                "--mca", "btl", "^openib", "--mca", "btl_tcp_if_include", "eno2",
                                "--mca", "mtl", "^ofi", executable, str(n), comm_type
                            ]
                        else: # Simple command for local execution
                            cmd = ["mpirun", "-np", str(p), executable, str(n), comm_type]
                        
                        try:
                            result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, check=True)
                            parts = result.stdout.strip().split(',')
                            if len(parts) == 6:
                                _, _, _, total_t, comm_t, comp_t = parts
                                times_data['total'].append(float(total_t))
                                times_data['comm'].append(float(comm_t))
                                times_data['comp'].append(float(comp_t))
                        except subprocess.CalledProcessError as e:
                            print(f"\nERROR running: {' '.join(cmd)}\n  {e.stderr.strip()}")
                            break 

                    if times_data['total']:
                        total_mean, total_std = calculate_stats(times_data['total'])
                        comm_mean, comm_std = calculate_stats(times_data['comm'])
                        comp_mean, comp_std = calculate_stats(times_data['comp'])
                        
                        csv_writer.writerow([
                            comm_type, n, p, job_id,
                            f"{total_mean:.6f}", f"{total_std:.6f}",
                            f"{comm_mean:.6f}", f"{comm_std:.6f}",
                            f"{comp_mean:.6f}", f"{comp_std:.6f}",
                            repetitions
                        ])
                        csvfile.flush()  # Ensure data is written immediately
                        print(f"  -> Result: Total Mean = {total_mean:.6f}s")

    print("-" * 50)
    print("Test batch completed.")
    end_time = time.time()
    elapsed = end_time - start_time
    print(f"Total test execution time: {elapsed:.2f} seconds.")

if __name__ == "__main__":
    main()