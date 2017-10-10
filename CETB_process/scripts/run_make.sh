#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#
#SBATCH --qos normal
#SBATCH --job-name CETB_make
#SBATCH --partition=shas
#SBATCH --time=00:15:00
#SBATCH --ntasks-per-node 24
#SBATCH --nodes 1
#SBATCH --account=ucb13_summit1
#SBATCH -o output/make_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
envpath=$2
file=/scratch/summit/${USER}/${src}_scripts/${src}_make_list
source ${envpath}/summit_set_pmesdr_environment.sh
ml intel
ml impi
ml netcdf/4.3.3.1
ml udunits
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 24 lb $file
date

