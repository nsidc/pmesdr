#!/bin/sh
# amsre_premetandspatial.sh
# Created Dec 5 2016 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J amsre_premetandspatial

#
# Set a walltime for the job. The time format is HH:MM:SS - this is set via an environment variable in
# the calling script, based on the channel number being processed


# Set the output file and embed the job number in the filename
#SBATCH -o output/amsre_setup-%j.out

# Select the janus QOS 
#SBATCH --qos=janus-debug
#SBATCH --time=10:00
#
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
#
date
for FILE in `find ../F08_sir/*.nc`
do
    echo "/projects/moha2290/miniconda3/envs/cetbtools/bin/generate_premetandspatial.py $FILE" >> F08_premet_list_cetb
done
date
   
	

