#!/bin/bash
#
# Arguments:
#  year : 4-digit year
#  src : sensor source (F08, F10, etc)
#  envpath : location of alpine_set_pmesdr_environment.sh script
#
#SBATCH --qos normal
#SBATCH --job-name CETB_platform_setup
#SBATCH --partition=amilan
#SBATCH --time=06:00:00
#SBATCH --nodes=6
#SBATCH --ntasks=120
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=7500
#SBATCH --account=ucb286_asc1
#SBATCH -o output/setup_lb-%j.out
#SBATCH --constraint=ib
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
year=$1
src=$2
envpath=$3
top_level=$4
file=/scratch/alpine/${USER}/${top_level}/${src}_scripts/${src}_setup_list_${year}

module purge 

# Load the Load Balancer module *first*
module load loadbalance/0.2

# Now load any other software modules you need:
source ${envpath}/single_set_pmesdr_environment.sh

ml
date
#/curc/sw/install/openmpi/4.1.1/gcc/11.2.0/bin/mpirun lb ${file}
$CURC_LB_BIN/mpirun lb ${file}
date

