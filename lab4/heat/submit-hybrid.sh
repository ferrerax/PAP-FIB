#!/bin/bash
#SBATCH -A execution2
#SBATCH -p execution2
#SBATCH --output=output-%j.out
#SBATCH --error=error-%j.out
#SBATCH --job-name="heat-hybrid"
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1

export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}

echo "Heat Hybrid execution with ${SLURM_JOB_NUM_NODES} nodes, ${SLURM_NTASKS} processes and ${OMP_NUM_THREADS} threads"

mpirun.mpich ./heat-hybrid test.dat

