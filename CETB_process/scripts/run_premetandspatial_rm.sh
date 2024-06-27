#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#
#SBATCH --qos normal
#SBATCH --job-name CETB_rm_premet_spatial
#SBATCH --partition=shas
#SBATCH --time=00:30:00
#SBATCH --account=ucb286_asc2
#SBATCH --ntasks-per-node 12
#SBATCH --nodes 1
#SBATCH -o premet_spatial_rm_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
FILE=/scratch/summit/${USER}/${src}_rm_premet_and_spatial
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 12 lb $FILE
date

