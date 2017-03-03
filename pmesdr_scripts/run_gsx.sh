#!/bin/bash
#
#SBATCH --qos=janus
#SBATCH --time=03:10:00
#SBATCH --ntasks-per-node=12
#SBATCH --nodes=54
#SBATCH -o output/gsx_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
date
srun lb $1
date
