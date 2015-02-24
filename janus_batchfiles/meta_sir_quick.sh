#!/bin/bash
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J meta_setup_quick

#
# Set a walltime for the job. The time format is HH:MM:SS - In this case we run for 5 minutes.

#SBATCH --time=00:05:00

#
# Select one node
# SBATCH -N1

# Select 1 task per node
# SBATCH --ntasks-per-node 1

# Set the output file and embed the job number in the filename
#SBATCH -o output/meta_sir_quick-%j.out

# Select the janus QOS 
#SBATCH --qos=janus

# The following commands will be executed when this script is run.

cd /projects/moha2290/PMESDR/mahworking/mah_icc_test/src/prod
source set_pmesdr_environment.sh -c icc
date
cd meas_meta_sir
make rss_quick
date
# End of example job shell script
# 
