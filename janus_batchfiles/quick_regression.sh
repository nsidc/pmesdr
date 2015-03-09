#!/bin/bash
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J quick_regression

#
# Set a walltime for the job. The time format is HH:MM:SS - In this case we run for 5 minutes.

#SBATCH --time=00:15:00

# Set the output file and embed the job number in the filename
#SBATCH -o output/quick_regression-%j.out

# Select the janus QOS 
#SBATCH --qos=janus

# The following commands will be executed when this script is run.

source ${PMESDR_TOP_DIR}/src/prod/set_pmesdr_environment.sh -a
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_make
pwd
echo 'running make'
make rss_quick
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_setup/meas_meta_setup_RSS
pwd
echo 'running setup'
make rss_quick
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_sir
make rss_quick
make rss_quick_validate
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_bgi
make rss_quick
make rss_quick_validate


# End of example job shell script
# 
