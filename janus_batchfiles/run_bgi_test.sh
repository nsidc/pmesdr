#!/bin/bash
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J meta_bgi_small

#
# Not setting a wall time.....

#SBATCH --time=00:59:00

#
# Select one node
# SBATCH -N1

# Select 1 task per node
# SBATCH --ntasks-per-node 1

# Set the output file and embed the job number in the filename
#SBATCH -o output/meta_bgi_small-%j.out

# Select the janus QOS 
#SBATCH --qos=janus

# The following commands will be executed when this script is run.

cd /projects/moha2290/PMESDR/mahworking/measures/src/prod
source set_pmesdr_environment.sh
cd ../..
date
rm -fr NSIDCtest/bgiRSS/e2n
mkdir -p NSIDCtest/bgiRSS/e2n
bin/meas_meta_bgi NSIDCtest/setupRSS/e2n/F131-E2N97-061-061.setup NSIDCtest/bgiRSS/e2n
date