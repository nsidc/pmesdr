#!/bin/bash
#
# script to run the translation from nc to gsx -
# Arguments:
#  src : sensor source (F08, F10, etc)
#  condaenv : name of conda env where gsx is installed
#
#SBATCH --qos normal
#SBATCH --job-name CETB_GSX
#SBATCH --partition=shas
#SBATCH --time=04:30:00
#SBATCH --ntasks-per-node=24
#SBATCH --account=ucb13_summit1
#SBATCH --nodes=5
#SBATCH -o output/gsx_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
condaenv=$2
file=/scratch/summit/${USER}/${src}_scripts/gsx_lb_list_summit
source activate $condaenv
ml intel
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 120 lb $file
date

