#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#
#SBATCH --qos normal
#SBATCH --job-name CETB_make
#SBATCH --partition=amilan
#SBATCH --time=00:15:00
#SBATCH --ntasks=24
#SBATCH --cpus-per-task=1
#SBATCH --account=ucb286_asc2
#SBATCH -o output/make_lb-%j.out
#SBATCH --constraint=ib
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
src=$1
envpath=$2
top_level=$3
file=/scratch/alpine/${USER}/${top_level}/${src}_scripts/${src}_make_list

module purge 
# Now load any other software modules you need:
source ${envpath}/single_set_pmesdr_environment.sh

# Load the Load Balancer module *first*
module load loadbalance/0.2


ml
date

$CURC_LB_BIN/mpirun lb ${file}
date

