import subprocess
import os

def run_test_with_vtune(binary, analysis_type, result_dir, input_file, task_threshold, sort_method, omp_threads):
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = str(omp_threads)
    vtune_cmd = [
        "vtune", "-collect", analysis_type,
        "-result-dir", result_dir, "--",
        binary, "-i", input_file, "-t", str(task_threshold), "-csv", "-sort", sort_method
    ]
    print("Running:", " ".join(vtune_cmd))
    try:
        result = subprocess.run(vtune_cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print("Error:", e.stderr)
        raise

# Example use:
binary = "./build/sort"
input_file = "in_16777216.in"
task_threshold = 2048
sort_method = "bitonic_parallel"
omp_threads = 4 
analysis_type = "hotspots"  # "performance-snapshot" or "hpc-performance"
result_dir = "vtune_results_hotspots"

output = run_test_with_vtune(binary, analysis_type, result_dir, input_file, task_threshold, sort_method, omp_threads)
print("VTune output collected!")
