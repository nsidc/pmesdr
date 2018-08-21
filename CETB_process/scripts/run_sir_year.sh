#!/bin/bash
#
# Arguments:
#  year : 4-digit year
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#
#SBATCH --qos normal
#SBATCH --job-name CETB_run_sir_year
#SBATCH --partition=shas
#SBATCH --account=ucb13_summit2
#SBATCH --time=04:00:00
#SBATCH --ntasks=120
#SBATCH --cpus-per-task=1
#SBATCH -o output/sir_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
year=$1
src=$2
envpath=$3
file=/scratch/summit/${USER}/${src}_scripts/${src}_sir_list_${year}
source ${envpath}/summit_set_pmesdr_environment.sh
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 lb ${file}
date
