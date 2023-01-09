#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#
#SBATCH --qos normal
#SBATCH --job-name CETB_mv_files
#SBATCH --partition=shas
#SBATCH --time=01:15:00
#SBATCH --account=ucb286_asc1
#SBATCH --ntasks=24
#SBATCH --cpus-per-task=1
#SBATCH -o output/mv_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
envpath=$2
FILE=/scratch/summit/${USER}/${src}_scripts/moving_files_all
source ${envpath}/summit_set_pmesdr_environment.sh
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 lb $FILE
date

