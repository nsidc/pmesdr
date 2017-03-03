#!/bin/bash
#
#SBATCH --qos normal
#SBATCH --job-name F08_setup
#SBATCH --partition=shas
#SBATCH --time=00:30:00
#SBATCH --ntasks-per-node=6
#SBATCH --nodes=1
#SBATCH -o output/setup_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
ml intel
ml impi
ml netcdf/4.3.3.1
ml udunits/2.2.20
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 6 lb /scratch/summit/moha2290/F08_scripts/F08_setup_list_37-85.reduced
date

