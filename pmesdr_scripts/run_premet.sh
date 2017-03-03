#!/bin/bash
#
#SBATCH --qos normal
#SBATCH --job-name F08_sir
#SBATCH --partition=shas
#SBATCH --time=00:20:00
#SBATCH --nodes 10
#SBATCH --ntasks-per-node 24
#SBATCH -o output/premet_lb-%j.out
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
source activate cetbtools
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 -n 240 lb /scratch/summit/moha2290/F08_scripts/F08_premet_list
date
