#!/bin/sh
# F08_make.sh
# Created Jan 30 2016 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J F08_setup

#
# Set a walltime for the job. The time format is HH:MM:SS - this is set via an environment variable in
# the calling script, based on the channel number being processed


# Set the output file and embed the job number in the filename
#SBATCH -o output/F08_setup-%j.out

# Select the janus QOS 
#SBATCH --time=10:00
#
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
YEAR=$1
SRC=F08
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/F08_setup/
#FINDIR=/lustre/janus_scratch/moha2290/prototype-output
#
#
# run meas_meta_sir with specific parameters
#
date
echo $YEAR
for FILE in `find ../F08_setup/`
do
#    echo $DOY
#    DAY=`date -d "$YEAR-01-01 + $DOY days" +%d`
    #    MONTH=`date -d "$YEAR-01-01 + $DOY days" +%m`
    echo "$BINDIR/meas_meta_sir $OUTDIR/$FILE /scratch/summit/moha2290/F08_sir" >> F08_sir_list
    # >> F08_setup_list_36hv
#    echo " done with day ${DOY} "
done
#
date
   
	

