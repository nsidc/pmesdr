#!/bin/bash
#
#SBATCH --qos normal
#SBATCH --job-name mv_files
#SBATCH --partition=shas
#SBATCH --time=01:15:00
#SBATCH --account=ucb13_summit1
#SBATCH --ntasks-per-node 24
#SBATCH --nodes 1
#SBATCH -o output/mv_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
FILE=/scratch/summit/moha2290/${src}_scripts/moving_files_all
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
ml intel
ml impi
ml netcdf/4.3.3.1
ml udunits
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 24 lb $FILE
date

