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
#SBATCH --account=ucb135_summit2
#SBATCH --ntasks 120
#SBATCH --cpus-per-task=1
#SBATCH -o output/premet_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
SRC=$1
condaenv=$2
file=/scratch/summit/${USER}/${SRC}_scripts/${SRC}_premet_list_cetb
source activate $condaenv
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:ofi  lb $file
date
