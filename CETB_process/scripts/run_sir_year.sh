#!/bin/bash
#
#SBATCH --qos normal
#SBATCH --job-name F17_sir
#SBATCH --partition=shas
#SBATCH --account=ucb13_summit1
#SBATCH --time=04:00:00
#SBATCH --nodes 5
#SBATCH --ntasks-per-node 24
#SBATCH -o output/sir_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
year=$1
src=$2
envpath=$3
file=/scratch/summit/${USER}/${src}_scripts/${src}_sir_list_${year}
source ${envpath}/summit_set_pmesdr_environment.sh
ml intel
ml impi
ml netcdf/4.3.3.1
ml udunits
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 120 lb ${file}
date
