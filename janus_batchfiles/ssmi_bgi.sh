#!/bin/sh
# ssmi_19h_day.sh
# Created Wed May 20 2015 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J ssmi_bgi

#
# Set a walltime for the job. The time format is HH:MM:SS - In this case we run for almost 24 hours

#SBATCH --time=23:55:00

# Set the output file and embed the job number in the filename
#SBATCH -o output/ssmi_bgi-%j.out

# Select the janus QOS 
#SBATCH --qos=janus
#
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
#
# The following commands will be executed when this script is run.
source ${PMESDR_TOP_DIR}/src/prod/set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/lustre/janus_scratch/moha2290/prototype-output/$SRC
FINDIR=/lustre/janus_scratch/moha2290/prototype-output
#
#
date
SRCFILES=$OUTDIR/setup/F5$CHANNEL*-$DAY*.setup
#
#
# Now SIR output from the previously created setup files
#
for file in $SRCFILES
do 
   echo "running  $file"
   $BINDIR/meas_meta_bgi $file $FINDIR/$YEAR$DAY 
done
date

   
	

