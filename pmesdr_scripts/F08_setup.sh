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
#
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/F08_make/
FINDIR=/scratch/summit/moha2290/prototype-output
#
#
# run meas_meta_setup with specific parameters
#
date
for FILE in `find ../F08_make/`
do
    if [[ $FILE == *19-22.NS.* ]]
    then 
	echo "$BINDIR/meas_meta_setup $OUTDIR/$FILE /scratch/summit/moha2290/F08_setup" >> F08_setup_list_19-22.NS
    fi
    if [[ $FILE == *19-22.T.* ]]
    then 
	echo "$BINDIR/meas_meta_setup $OUTDIR/$FILE /scratch/summit/moha2290/F08_setup" >> F08_setup_list_19-22.T
    fi
    if [[ $FILE == *37-85.NS.* ]]
    then 
	echo "$BINDIR/meas_meta_setup $OUTDIR/$FILE /scratch/summit/moha2290/F08_setup" >> F08_setup_list_37-85.NS
    fi
    if [[ $FILE == *37-85.T.* ]]
    then 
	echo "$BINDIR/meas_meta_setup $OUTDIR/$FILE /scratch/summit/moha2290/F08_setup" >> F08_setup_list_37-85.T
    fi
done
#
date
   
	

