#!/bin/bash
#SBATCH --qos normal
#SBATCH --job-name tarring
#SBATCH --partition=shas
#SBATCH --time=04:15:00
#SBATCH --account=ucb135_summit1
#SBATCH --ntasks-per-node 24
#SBATCH --nodes 2
#SBATCH -o output/tarring-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org

FILE=$1
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 13 lb $FILE
date
