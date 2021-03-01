#!/bin/bash

#SBATCH --job-name=submit-omp.sh
#SBATCH -D .
#SBATCH --output=submit-omp.sh.o%j
#SBATCH --error=submit-omp.sh.e%j

USAGE="\n USAGE: ./submit-extrae.sh prog range slice_size num_threads\n
        prog        -> omp program name\n
        range       -> count primes between 2 and range\n
#        slice_size  -> size of the blocks to sieve (only for sieve2)\n
        num_threads -> number of threads\n"

#if (test $# -lt 4 || test $# -gt 4)
if (test $# -lt 3 || test $# -gt 3)
then
	echo -e $USAGE
        exit 0
fi

make $1-omp

HOST=$(echo $HOSTNAME | cut -f 1 -d'.')
if (test "${HOST}" = "boada-1")
then
    echo "Use sbatch to execute this script"
    exit 0
fi

export OMP_NUM_THREADS=$3
#export OMP_NUM_THREADS=$4

export LD_PRELOAD=${EXTRAE_HOME}/lib/libomptrace.so

./$1-omp $2 $3
unset LD_PRELOAD
mpi2prv -f TRACE.mpits -o $1-$2-$3-${HOST}.prv -e $1 -paraver

#./$1-omp $2 $3 $4
#unset LD_PRELOAD
#mpi2prv -f TRACE.mpits -o $1-$2-$3-$4-${HOST}.prv -e $1 -paraver
