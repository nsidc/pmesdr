#!/bin/bash
#
# Arguments:
#  year: 4-digit year
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#
#SBATCH --qos normal
#SBATCH --job-name CETB_rm
#SBATCH --partition=shas
#SBATCH --time=00:15:00
#SBATCH --account=ucb13_summit1
#SBATCH --ntasks-per-node 24
#SBATCH --nodes 1
#SBATCH -o output/rm_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
year=$1
src=$2
envpath=$3
FILE=/scratch/summit/${USER}/${src}_scripts/${src}_setup_rm_${year}
source ${envpath}/summit_set_pmesdr_environment.sh
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 24 lb $FILE
date
