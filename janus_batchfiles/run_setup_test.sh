#!/bin/bash
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J meta_setup

#
# Set a walltime for the job. The time format is HH:MM:SS 

#SBATCH --time=00:59:59

#
# Select one node
# SBATCH -N1

# Select 1 task per node
# SBATCH --ntasks-per-node 1

# Set the output file and embed the job number in the filename
#SBATCH -o output/meta_setup-%j.out

# Select the janus QOS 
#SBATCH --qos=janus

# The following commands will be executed when this script is run.

cd /projects/moha2290/PMESDR/mahworking/measures/src/prod
source set_pmesdr_environment.sh
# module load  netcdf/netcdf4-4.3_hdf5-1.8.11_hdf4-4.2.9_szip-2.1_zlib-1.2.78_jpeglib-8d_intel-13.0.0

cd meas_meta_setup/meas_meta_setup_CSU
date
make LOCALE=JANUSicc csu_ease
cd ../meas_meta_setup_RSS
date
make LOCALE=JANUSicc rss_ease
date
# End of example job shell script
# 
