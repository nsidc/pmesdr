#!/bin/bash
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J meta_bgi_quick

#
# Not setting a wall time.....

#SBATCH --time=00:59:00

#
# Select one node
# SBATCH -N1

# Select 1 task per node
# SBATCH --ntasks-per-node 1

# Set the output file and embed the job number in the filename
#SBATCH -o output/meta_bgi_quick-%j.out

# Select the janus QOS 
#SBATCH --qos=janus

# The following commands will be executed when this script is run.

cd /projects/moha2290/PMESDR/mahworking/mah_icc_test/src/prod
source set_pmesdr_environment.sh -c icc
date
cd meas_meta_bgi
make rss_quick 
date
make rss_quick_validate
date