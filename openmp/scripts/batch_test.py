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
    65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608
]]
thresholds = [1024, 2048, 4096]
thread_counts = [1, 2, 4, 8]
repetitions = 5


output_csv = "batch_results.csv"


with open(output_csv, mode="w", newline='') as csvfile:
    csvwriter = csv.writer(csvfile)

    csvwriter.writerow(["input_file", "N", "task_threshold", "omp_num_threads", "repetitions",
                        "mean_time", "std_deviation", "individual_times"])
    
    # Iterate over all combinations.
    for input_file in input_files:
        for thr in thread_counts:
            for thres in thresholds:
                times = []
                reported_N = None
                # Run the execution multiple times.
                for rep in range(repetitions):
                    # Set environment variable for number of OpenMP threads.
                    env = os.environ.copy()
                    env["OMP_NUM_THREADS"] = str(thr)
                    
                    # Build the command.
                    cmd = [binary, "-i", input_file, "-t", str(thres), "-csv"]
                    try:
                        result = subprocess.run(cmd, env=env, stdout=subprocess.PIPE, 
                                                stderr=subprocess.PIPE, universal_newlines=True, check=True)
                    except subprocess.CalledProcessError as e:
                        print(f"Error running command: {' '.join(cmd)}\nError: {e.stderr}")
                        continue

                    lines = result.stdout.strip().splitlines()
                    for line in lines:
                        if line.startswith("input_file"):
                            continue

                        parts = line.split(",")
                        if len(parts) == 5:
                            _, N_val, _, omp_threads_val, time_val = parts
                            try:
                                runtime = float(time_val)
                                times.append(runtime)
                                reported_N = N_val
                            except ValueError:
                                pass
                    
                if times:
                    mean_time = statistics.mean(times)
                    std_dev = statistics.stdev(times) if len(times) > 1 else 0.0

                    csvwriter.writerow([input_file, reported_N, thres, thr, repetitions,
                                        f"{mean_time:.6f}", f"{std_dev:.6f}", times])
                    print(f"Config (input: {input_file}, threshold: {thres}, threads: {thr}) -> "
                          f"Mean: {mean_time:.6f} s, Std Dev: {std_dev:.6f} s, Runs: {times}")
                else:
                    print(f"Config (input: {input_file}, threshold: {thres}, threads: {thr}) produced no data.")
