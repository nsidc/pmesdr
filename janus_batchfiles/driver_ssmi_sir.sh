#!/bin/sh
# driver_ssmi.sh
# Created Thu Dec 10 2015 by Molly Hardman <mhardman@nsidc-driftice.local>
# $Id$
# $Log$
#
#
# This script will set the environment variables for year day and channel and then call the slurm shell
# script to start the job
# arg 1 is CSU or RSS
# arg 2 is year yyyy
# arg 3 is CHANNEL 1-7
# arg $4 - $5 are days to process $4 is start day and $5 is end day in doy
#
# first make sure slurm is loaded
ml slurm

chan_str[0]="no_chan"
chan_str[1]="19h"
chan_str[2]="19v"
chan_str[3]="22v"
chan_str[4]="37h"
chan_str[5]="37v"
chan_str[6]="85h"
chan_str[7]="85v"
export YEAR=$2
export CHANNEL=$3
export CHANNEL_STRING=${chan_str[$CHANNEL]}
export SRC=$1

for list in $(seq -f "%03g" $4 $5) 
do
    export DAY=$list
    sbatch /projects/moha2290/measures-byu/janus_batchfiles/ssmi_sir.sh
    echo "/projects/moha2290/measures-byu/janus_batchfiles/ssmi_${CHANNEL_STRING}_day_$SRC.sh"
    echo "result $? for day $DAY and year $YEAR channel $CHANNEL chan_str $CHANNEL_STRING and input file $SRC$YEAR$DAY"
done
#
echo "result $?"

