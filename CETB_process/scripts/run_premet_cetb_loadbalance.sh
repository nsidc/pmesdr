#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  condaenv : name of conda env where gsx is installed
#
#SBATCH --qos normal
#SBATCH --job-name CETB_platform_premet
#SBATCH --partition=amilan
#SBATCH --time=00:30:00
#SBATCH --account=ucb286_asc2
#SBATCH --ntasks=24
#SBATCH --cpus-per-task=1
#SBATCH --constraint=ib
#SBATCH -o output/premet_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
year=$2
condaenv=$3
envpath=$4
top_level=$5

module purge 
# Now load any other software modules you need:
source ${envpath}/set_pmesdr_environment.sh
source /projects/${USER}/miniconda3/bin/activate
conda activate $condaenv

file=$PMESDR_SCRATCH_DIR/${top_level}/${src}_scripts/${src}_premet_list_cetb_${year}

ml
date

# Load the Load Balancer module *first*
module load loadbalance/0.2

$CURC_LB_BIN/mpirun lb ${file}
date

