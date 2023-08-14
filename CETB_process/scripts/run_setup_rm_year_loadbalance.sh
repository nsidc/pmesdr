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
#SBATCH --account=ucb286_asc1
#SBATCH --ntasks=24
#SBATCH --cpus-per-task=1
#SBATCH -o output/rm_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
year=$1
src=$2
envpath=$3
FILE=/scratch/summit/${USER}/${src}_scripts/${src}_setup_rm_${year}

module purge 

# Load the Load Balancer module *first*
module load loadbalance/0.2

# Now load any other software modules you need:
source ${envpath}/single_set_pmesdr_environment.sh

ml
date

$CURC_LB_BIN/mpirun lb ${file}
date

