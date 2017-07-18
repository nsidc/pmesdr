#!/bin/sh
# file_lists.sh
# Created Thu Sep 15 2016 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# this little script will take a year as argument and then create a list of all
#   SSMI/SSMIS GSX input files available for each date in the year
# $1 is the year to process
# $2 is the start doy for the year
# $3 is the stop doy for the year
# $4 is the platform, e.g. F08, F10
year=$1
for doy in `seq $2 $3`
do
    #    echo $doy
    realdoy=$(( $doy - 1 ))
    day=`date -d "$year-01-01 + $realdoy days" +%d`
#    echo $day
    month=`date -d "$year-01-01 + $realdoy days" +%m`
#    echo $month
    ls /scratch/summit/moha2290/$4_GSX/*$year$month$day* >& /scratch/summit/moha2290/$4_lists/$4.$year$month$day
done

echo $year$month$day

