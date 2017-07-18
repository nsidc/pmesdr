#!/bin/bash
#
# script to run the translation from nc to gsx - takes src as argument
#
#SBATCH --qos normal
#SBATCH --job-name GSX
#SBATCH --partition=shas
#SBATCH --time=01:30:00
#SBATCH --ntasks-per-node=24
#SBATCH --account=ucb13_summit1
#SBATCH --nodes=5
#SBATCH -o output/gsx_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
condaenv=$2
file=/scratch/summit/moha2290/${src}_scripts/gsx_lb_list_summit
source activate $condaenv
ml intel
ml impi
ml netcdf/4.3.3.1
ml udunits
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 120 lb $file
date

