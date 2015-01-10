#!/bin/bash
#========================================================================
# janus batch file to create a setup file suitable for testing
#
# output of this batch file is the set of setup files created by the Makefile targets
#     csu_ease and rss_ease
#
# 2015-01-09 M. A. Hardman 303-492-2969 mhardman@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2015 Regents of University of Colorado and Brigham-Young University
#========================================================================

# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
base=${0##*/}
base=${base%.*}
#SBATCH -J ${base}

#
# Set a walltime for the job. The time format is HH:MM:SS 
#SBATCH --time=00:15:00

# Set the output file and embed the job number in the filename
#SBATCH -o ${PMESDR_TOP_DIR}/janus_batchfiles/output/${base}-%j.out

# Select the janus QOS 
#SBATCH --qos=janus

# The following commands will be executed when this script is run.

cd ${PMESDR_TOP_DIR}/src/prod

cd meas_meta_make
date
make rss_ease
date
make csu_ease
date

cd ../meas_meta_setup/meas_meta_setup_RSS
date
make rss_ease
date

cd ../meas_meta_setup_CSU
make csu_ease
date

# End of script
# 
