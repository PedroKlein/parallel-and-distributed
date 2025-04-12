#!/usr/bin/env python3
import csv
import os
import subprocess
import statistics
import sys

if len(sys.argv) != 2:
    print("Usage: batch_test.py <path_to_binary>")
    sys.exit(1)

binary = sys.argv[1]
input_files = [f"in_{num}.in" for num in [
    1, 2, 4, 8, 16, 32, 64, 128,
    256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
    65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216
]]
# Only used for parallel sort methods.
thresholds = [1024, 2048, 4096]

# For sequential sorts, these parameters are not varied.
sequential_thr = [1]
sequential_thresholds = [0]  # Dummy value; not used in sequential routines.
repetitions = 5

# Define the sort methods to test.
sort_methods = ["bitonic", "bitonic_parallel", "mergesort", "mergesort_parallel"]

output_csv = "batch_results.csv"

# Dynamically determine thread counts based on system's CPU count
max_threads = os.cpu_count() or 1  # Fallback to 1 if os.cpu_count() returns None
thread_counts = []
current = max_threads
while current >= 1:
    thread_counts.append(current)
    current //= 2
thread_counts.reverse()

print(f"Using thread counts: {thread_counts}")

with open(output_csv, mode="w", newline='') as csvfile:
    csvwriter = csv.writer(csvfile)
    csvwriter.writerow(["sort_method", "input_file", "N", "task_threshold", "omp_num_threads",
                        "repetitions", "mean_time", "std_deviation", "individual_times"])
    
    # Outer loop: iterate over sort methods.
    for sort_method in sort_methods:
        # Decide which thread and threshold values to use.
        if sort_method in ["bitonic", "mergesort"]:
            thr_list = sequential_thr
            thres_list = sequential_thresholds
        else:
            thr_list = thread_counts
            thres_list = thresholds

        # Iterate over input files and configurations.
        for input_file in input_files:
            for thr in thr_list:
                for thres in thres_list:
                    times = []
                    reported_N = None
                    # Run the execution multiple times.
                    for rep in range(repetitions):
                        # Set environment variable for number of OpenMP threads.
                        env = os.environ.copy()
                        env["OMP_NUM_THREADS"] = str(thr)
                        
                        # Build the command, including the sort method.
                        cmd = [binary, "-i", input_file, "-t", str(thres), "-csv", "-sort", sort_method]
                        try:
                            result = subprocess.run(cmd, env=env, stdout=subprocess.PIPE, 
                                                    stderr=subprocess.PIPE, universal_newlines=True, check=True)
                        except subprocess.CalledProcessError as e:
                            print(f"Error running command: {' '.join(cmd)}\nError: {e.stderr}")
                            continue

                        lines = result.stdout.strip().splitlines()
                        for line in lines:
                            # Skip header lines.
                            if line.startswith("input_file") or line.startswith("sort_method"):
                                continue

                            parts = line.split(",")
                            # The expected CSV format: input_file,N,task_threshold,omp_num_threads,total_time
                            if len(parts) == 5:
                                _, N_val, _, _, time_val = parts
                                try:
                                    runtime = float(time_val)
                                    times.append(runtime)
                                    reported_N = N_val
                                except ValueError:
                                    pass

                    if times:
                        mean_time = statistics.mean(times)
                        std_dev = statistics.stdev(times) if len(times) > 1 else 0.0

                        csvwriter.writerow([sort_method, input_file, reported_N, thres, thr, repetitions,
                                            f"{mean_time:.6f}", f"{std_dev:.6f}", times])
                        print(f"Sort: {sort_method} | Config (input: {input_file}, threshold: {thres}, threads: {thr}) -> "
                              f"Mean: {mean_time:.6f} s, Std Dev: {std_dev:.6f} s, Runs: {times}")
                    else:
                        print(f"Sort: {sort_method} | Config (input: {input_file}, threshold: {thres}, threads: {thr}) produced no data.")
