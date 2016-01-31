#!/bin/sh
# driver_ssmi_bgi_t.sh
# Created Fri Jan 29 2016 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# This script will set the environment variables for year day and channel and then call the slurm shell
# script to start the job
# arg 1 is CSU or RSS
# arg 2 is year yyyy
# arg 3 is CHANNEL 1-7
# arg $4 $5 are begin and end days to process in day of year
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
    case $CHANNEL in
	[1-2])
	    SBATCH_TIMELIMIT=09:45:00
	    ;;
	3)
	    SBATCH_TIMELIMIT=06:55:00
	    ;;
	[4-5])
	    SBATCH_TIMELIMIT=09:45:00
	    ;;
	[6-7])
	    SBATCH_TIMELIMIT=00:40:00
	    ;;
    esac
    sbatch /projects/moha2290/measures-byu/janus_batchfiles/ssmi_bgi_ta.sh
    sbatch /projects/moha2290/measures-byu/janus_batchfiles/ssmi_bgi_td.sh
done
#
echo "result $?"

