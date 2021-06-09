#!/bin/bash

source /Soft/PAR/extrae/3.5/etc/extrae.sh

export OMP_NUM_THREADS=4
export EXTRAE_CONFIG_FILE=../extrae.xml
export LD_PRELOAD=${EXTRAE_HOME}/lib/libompitrace.so # For C apps
export LD_LIBRARY_PATH=/scratch/nas/1/pap0016/PAP-FIB/lab4/heat:${LD_LIBRARY_PATH}
#export LD_PRELOAD=${EXTRAE_HOME}/lib/libompitracef.so # For Fortran apps

## Run the desired program
$*

