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
#SBATCH --account=ucb286_asc2
#SBATCH --cpus-per-task=1
#SBATCH -o output/gsx_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
condaenv=$2
top_level=$3
envpath=$4
file=/scratch/alpine/${USER}/${top_level}/${src}_scripts/gsx_lb_list_alpine
source /projects/${USER}/miniconda3/bin/activate
conda activate $condaenv

module purge 


# Now load any other software modules you need:
source ${envpath}/single_set_pmesdr_environment.sh

ml
date

# Load the Load Balancer module *first*
module load loadbalance/0.2

$CURC_LB_BIN/mpirun lb ${file}
date


