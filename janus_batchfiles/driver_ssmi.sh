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
# arg 4 is input_file found in the testing directory
# arg $5 - $9 are days to process in ddd - i.e. day of year
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
export INPUT_FILE=$4
export CHANNEL_STRING=${chan_str[$CHANNEL]}

for list in $5 $6 $7 $8 $9 
do
    export DAY=$list
    sbatch /projects/moha2290/measures-byu/janus_batchfiles/ssmi_${CHANNEL_STRING}_day_$1.sh
    echo "/projects/moha2290/measures-byu/janus_batchfiles/ssmi_${CHANNEL_STRING}_day_$1.sh"
    echo "result $? for day $DAY and year $YEAR channel $CHANNEL chan_str $CHANNEL_STRING and input file $INPUT_FILE"
done
#
echo "result $?"

