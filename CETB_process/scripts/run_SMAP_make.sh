#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#  suffix : resolution is either 25, 24 or 36
#
#SBATCH --qos normal
#SBATCH --job-name CETB_make
#SBATCH --partition=shas
#SBATCH --time=00:15:00
#SBATCH --ntasks=24
#SBATCH --cpus-per-task=1
#SBATCH --account=ucb135_summit1
#SBATCH -o output/make_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
envpath=$2
suffix=$3
file=/scratch/summit/${USER}/${src}_scripts/${src}_make_list_${suffix}
source ${envpath}/summit_set_pmesdr_environment.sh
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 lb $file
date

