#!/bin/bash
#SBATCH -A execution2
#SBATCH -p execution2
#SBATCH --output=output-%j.out
#SBATCH --error=error-%j.out
#SBATCH --job-name="hpl"
#SBATCH --nodes=1
#SBATCH --ntasks=1

echo "Linpack execution with ${SLURM_JOB_NUM_NODES} nodes and ${SLURM_NTASKS} processes"

srun --mpi=pmi2 ./xhpl

