#!/bin/bash
#
# Arguments:
#  year : 4-digit year
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#
#SBATCH --qos normal
#SBATCH --job-name CETB_run_sir_year
#SBATCH --partition=amilan
#SBATCH --account=ucb286_asc1
#SBATCH --time=04:00:00
#SBATCH --ntasks=120
#SBATCH --cpus-per-task=2
#SBATCH -o output/sir_lb-%j.out
#SBATCH --constraint=ib
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
year=$1
src=$2
envpath=$3
top_level=$4
file=/scratch/alpine/${USER}/${top_level}/${src}_scripts/${src}_sir_list_${year}
source ${envpath}/alpine_set_pmesdr_environment.sh
ml intel/2022.1.2
ml gnu_parallel
ml
date
parallel -a ${file}
date
