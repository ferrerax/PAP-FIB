#!/bin/bash
#SBATCH -A execution2
#SBATCH -p execution2
#SBATCH --output=output-%j.out
#SBATCH --error=error-%j.out
#SBATCH --job-name="stream"

USAGE="\n USAGE: ./submit-stream.sh numthreads \n
        numthreads    -> Number of threads\n"

if (test $# -lt 1 || test $# -gt 1)
then
        echo -e $USAGE
        exit 0
fi

export OMP_NUM_THREADS=$1

./stream

