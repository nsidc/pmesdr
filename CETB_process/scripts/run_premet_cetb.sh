#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  condaenv : name of conda env where gsx is installed
#
#SBATCH --qos normal
#SBATCH --job-name CETB_platform_premet
#SBATCH --partition=shas
#SBATCH --time=01:20:00
#SBATCH --nodes 5
#SBATCH --account=ucb13_summit1
#SBATCH --ntasks-per-node 24
#SBATCH -o output/premet_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
SRC=$1
condaenv=$2
file=/scratch/summit/${USER}/${SRC}_scripts/${SRC}_premet_list_cetb
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
