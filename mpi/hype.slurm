#!/bin/bash
#SBATCH --job-name=matmult_batch_test
#SBATCH --partition=hype
#SBATCH --nodes=2
#SBATCH --ntasks=40
#SBATCH --time=23:59:59
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err

echo "--- Job Setup ---"
echo "Job ID: $SLURM_JOB_ID"
echo "Job Name: $SLURM_JOB_NAME"
echo "Running on nodes: $SLURM_JOB_NODELIST"
echo "Number of tasks: $SLURM_NTASKS"
echo "-------------------"

# --- Application Execution ---
# First, ensure the project is compiled
make build

# Create the machinefile for mpirun, listing the allocated nodes
MACHINEFILE="nodes.$SLURM_JOB_ID"
srun -l hostname | sort -n | awk '{print $2}' > $MACHINEFILE
echo "Created machinefile: $MACHINEFILE"


# Define the path to your executable and scripts
EXECUTABLE="build/mpi_matmult"
PYTHON_SCRIPT="scripts/batch_test.py"

echo "--- Running MPI Matrix Multiplication Batch Test ---"
# Execute the Python batch test script.
# We pass it the path to the C executable and the generated machinefile.
python3 -u $PYTHON_SCRIPT $EXECUTABLE $MACHINEFILE

echo "--- Job Finished ---"