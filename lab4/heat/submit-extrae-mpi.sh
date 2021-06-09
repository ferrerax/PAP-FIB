#!/bin/bash

#SBATCH -A execution2
#SBATCH -p execution2
#SBATCH --output=output-%j.out
#SBATCH -D .
#SBATCH --output=submit-extrae.sh.o%j
#SBATCH --error=submit-extrae.sh.e%j
#SBATCH --job-name="heat-mpi-extrae"
#SBATCH --nodes=1
#SBATCH --ntasks=1

USAGE="\n USAGE: ./submit-extrae.sh prog num_threads\n
        prog        -> omp program name\n
        num_threads -> number of threads\n"

#if (test $# -lt 2 || test $# -gt 2)
#then
#	echo -e $USAGE
#        exit 0
#fi


HOST=$(echo $HOSTNAME | cut -f 1 -d'.')
if (test "${HOST}" = "boada-1")
then
    echo "Use sbatch to execute this script"
    exit 0
fi

#export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}



#srun --mpi=pmi2 bash -c "LD_PRELOAD=${EXTRAE_HOME}/lib/libmpitrace.so ./$1 test.dat"
srun --mpi=pmi2 ./trace-mpi.sh ./$1 test.dat
#unset LD_PRELOAD
#mpi2prv -f TRACE.mpits -o $1-$2-${HOST}.prv -e $1 -paraver
