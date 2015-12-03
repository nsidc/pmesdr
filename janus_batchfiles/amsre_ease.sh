#!/bin/sh
# full_day_csu.sh
# Created Wed May 20 2015 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J amsre_full_day

#
# Set a walltime for the job. The time format is HH:MM:SS - In this case we run for almost 24 hours

#SBATCH --time=23:55:00

# Set the output file and embed the job number in the filename
#SBATCH -o output/amsre_ease-%j.out

# Select the janus QOS 
#SBATCH --qos=janus
#
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
#
# The following commands will be executed when this script is run.
source ${PMESDR_TOP_DIR}/src/prod/set_pmesdr_environment.sh -c icc
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/lustre/janus_scratch/moha2290/amsre_ease
#
#
cd $TOPDIR/src/prod/meas_meta_make
make amsre_ease
#
cd $TOPDIR/sample_data
$BINDIR/meas_meta_setup $TOPDIR/NSIDCtest/amsre_e2n/e2ntest_AMSRE.meta $OUTDIR/setupAMSRE
$BINDIR/meas_meta_setup $TOPDIR/NSIDCtest/amsre_e2s/e2stest_AMSRE.meta $OUTDIR/setupAMSRE
$BINDIR/meas_meta_setup $TOPDIR/NSIDCtest/amsre_e2t/e2ttest_AMSRE.meta $OUTDIR/setupAMSRE

AMSREFILES=$OUTDIR/setupAMSRE/A*.setup

# first SIR output
for file in $AMSREFILES; do \
    echo "running $file"; \
    $BINDIR/meas_meta_sir $file $OUTDIR/sirAMSRE; \
done
    
#
# Now BGI output from the same setup files
#
for file in $AMSREFILES; do \
   echo "running  $file"; \
   $BINDIR/meas_meta_bgi $file $OUTDIR/bgiAMSRE; \
done

#for file in $RSSE23KMFILES; do \
#   echo "running  $file"; \
#   $BINDIR/meas_meta_bgi $file $OUTDIR/bgiRSS_3km/gamma.75 2.35619449019234; \
#done

   
	

