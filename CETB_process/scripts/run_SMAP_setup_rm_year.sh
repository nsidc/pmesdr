#!/bin/bash
#
# Arguments:
#  year: 4-digit year
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#  suffix : resolution is either 25, 24 or 36
#
#SBATCH --qos normal
#SBATCH --job-name CETB_rm
#SBATCH --partition=shas
#SBATCH --time=00:15:00
#SBATCH --account=ucb135_summit1
#SBATCH --ntasks=24
#SBATCH --cpus-per-task=1
#SBATCH -o output/rm_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
year=$1
src=$2
envpath=$3
suffix=$4
FILE=/scratch/summit/${USER}/${src}_scripts/${src}_setup_rm_${year}_${suffix}
source ${envpath}/summit_set_pmesdr_environment.sh
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 lb $FILE
date

