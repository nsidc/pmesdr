#!/bin/bash
#
# script to run the translation from nc to gsx -
# Arguments:
#  src : sensor source (F08, F10, etc)
#  condaenv : name of conda env where gsx is installed
# top_level : if present is for nsidc0751_CSU_ICDR or nsidc0752_CSU_GPM_XCAL_NRT_Tbs
#
#SBATCH --qos normal
#SBATCH --job-name CETB_GSX
#SBATCH --partition=amilan
#SBATCH --constraint=ib
#SBATCH --time=04:30:00
#SBATCH --ntasks=120
#SBATCH --account=ucb-general
#SBATCH --cpus-per-task=1
#SBATCH -o output/gsx_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
condaenv=$2
top_level=$3
file=/scratch/alpine/${USER}/${top_level}/${src}_scripts/gsx_lb_list_alpine
source activate $condaenv
ml intel/2022.1.2
ml gnu_parallel
ml
date
parallel -a $file
date

