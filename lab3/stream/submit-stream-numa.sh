#!/bin/bash
#SBATCH -A execution2
#SBATCH -p execution2
#SBATCH --output=output-%j.out
#SBATCH --error=error-%j.out
#SBATCH --job-name="stream"

USAGE="\n USAGE: ./submit-stream.sh numthreads thread_node mem_node\n
        numthreads    -> Number of threads\n"

if (test $# -lt 3 || test $# -gt 3)
then
        echo -e $USAGE
        exit 0
fi

export OMP_NUM_THREADS=$1

numactl -m $3 --cpunodebind=$2 ./stream

