#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 4 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR DOY_START DOY_STOP SRC"
    echo "  Make a list of all GSX input files available for each "
    echo "  doy in the year. "
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  DOY_STOP: stop day of year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo ""
    exit 1
fi

year=$1
for doy in `seq $2 $3`
do
    #    echo $doy
    realdoy=$(( $doy - 1 ))
    day=`date -d "$year-01-01 + $realdoy days" +%d`
    month=`date -d "$year-01-01 + $realdoy days" +%m`
    
    ls /scratch/summit/${USER}/$4_GSX/*$year$month$day* >& /scratch/summit/${USER}/$4_lists/$4.$year$month$day
done

echo $year$month$day

