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
#SBATCH -J ssmi_sir

#
# Set a walltime for the job. The time format is HH:MM:SS - In this case we run for almost 24 hours

#SBATCH --time=00:12:00

# Set the output file and embed the job number in the filename
#SBATCH -o output/ssmi_sir-%j.out

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
# It's necessary to create the SHORTYEAR variable because currently the setup files are
# all in the same directory and DGL's convention uses short year and my scripting isn't
# that great.....
#
date
if [ $YEAR -eq 2003 ]
then
    SHORTYEAR=03
else
    SHORTYEAR=04
fi
SRCFILES=$OUTDIR/setup/F5$CHANNEL*$SHORTYEAR-$DAY*.setup
#
# The larger grids - 3 km for 37 and 85 can't all run at the same time on one node
#
if [ $CHANNEL -ge 4 ]
then
    SRCFILES=$OUTDIR/setup/F5$CHANNEL*-E2[N-S]$SHORTYEAR-$DAY*.setup
    for file in $SRCFILES
    do
	echo "running $file"
	$BINDIR/meas_meta_sir $file $FINDIR/$YEAR$DAY &
	echo "command line $BINDIR/meas_meta_sir $file $FINDIR/$YEAR$DAY" &
    done
    wait
    SRCFILES=$OUTDIR/setup/F5$CHANNEL*-E2T$SHORTYEAR-$DAY*.setup
fi

#
#
# Now SIR output from the previously created setup files
#
for file in $SRCFILES
do 
   echo "running  $file"
   $BINDIR/meas_meta_sir $file $FINDIR/$YEAR$DAY & 
done
wait
date

   
	

