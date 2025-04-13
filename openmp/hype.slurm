#!/bin/bash
#SBATCH --job-name=bitonic_sort
#SBATCH --partition=hype
#SBATCH --time=03:00:00
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err

python3 /home/users/paklein/repositories/parallel-and-distributed/openmp/scripts/batch_test.py /home/users/paklein/repositories/parallel-and-distributed/openmp/build/sort