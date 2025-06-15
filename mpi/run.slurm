#!/bin/bash
#SBATCH --job-name=mpi_coletiva
#SBATCH --partition=hype
#SBATCH --nodes=2
#SBATCH --ntasks=32
#SBATCH --time=00:05:00
#SBATCH --output=%x_%j.out
#SBATCH --error=%x_%j.err

MACHINEFILE="nodes.$SLURM_JOB_ID"
srun -l hostname | sort -n | awk '{print $2}' > $MACHINEFILE

mpirun -np $SLURM_NTASKS  \
       -machinefile $MACHINEFILE \
       --mca btl ^openib \
       --mca btl_tcp_if_include eno2 \
       --mca mtl ^ofi \
       ./build/mpi_matmult 128 collective
