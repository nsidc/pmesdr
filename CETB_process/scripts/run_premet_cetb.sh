#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  condaenv : name of conda env where gsx is installed
#
#SBATCH --qos normal
#SBATCH --job-name CETB_platform_premet
#SBATCH --partition=amilan
#SBATCH --time=01:20:00
#SBATCH --account=ucb286_asc1
#SBATCH --ntasks 120
#SBATCH --cpus-per-task=1
#SBATCH --constraint=ib
#SBATCH -o premet_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
file=/scratch/alpine/${USER}/data/premet_script
source activate cetb
ml gnu_parallel
ml
date
parallel -a $file
date
