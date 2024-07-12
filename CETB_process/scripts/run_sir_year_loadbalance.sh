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
#SBATCH --account=ucb286_asc2
#SBATCH --time=03:00:00
#SBATCH --nodes=6
#SBATCH --ntasks=120
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=7500
#SBATCH -o output/sir_lb-%j.out
#SBATCH --constraint=ib
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
year=$1
src=$2
envpath=$3
top_level=$4

# Now load any other software modules you need:
module purge
source ${envpath}/set_pmesdr_environment.sh

file=$PMESDR_SCRATCH_DIR/${top_level}/${src}_scripts/${src}_sir_list_${year}
echo $file

# Load the Load Balancer module *first*
module load loadbalance/0.2

ml
date
$CURC_LB_BIN/mpirun lb ${file}
date
