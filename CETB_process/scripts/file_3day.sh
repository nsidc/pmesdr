#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 4 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR DOY_START DOY_STOP SRC"
    echo "  Make a new list of input files for the NS lists, "
    echo "  which need 3 days of input files for each day processed."
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

    realdoy=$(( $doy - 1 ))
    day=`date -d "$year-01-01 + $realdoy days" +%d`
    doyminus1=$(( $realdoy - 1 ))
    doyplus1=$(( $realdoy + 1 ))

    dayminus1=`date -d "$year-01-01 + $doyminus1 days" +%d`
    dayplus1=`date -d "$year-01-01 + $doyplus1 days" +%d`
    monthm1=`date -d "$year-01-01 + $doyminus1 days" +%m`
    month=`date -d "$year-01-01 + $realdoy days" +%m`
    monthp1=`date -d "$year-01-01 + $doyplus1 days" +%m`
    yearm1=$year
    yearp=$year
    if [ $day == 01 -a $month == 01 ]
    then
	dayminus1=31
        monthm1=12
	yearm1=$(( $year - 1 ))
    fi
    if [ $day == 31 -a $month == 12 ]
    then
	yearp=$(( $year + 1 ))
	dayplus1=01
	monthp1=01
    fi
           
    cat /scratch/summit/${USER}/$4_lists/$4.$yearm1$monthm1$dayminus1 \
	/scratch/summit/${USER}/$4_lists/$4.$year$month$day \
	/scratch/summit/${USER}/$4_lists/$4.$yearp$monthp1$dayplus1 \
	>& /scratch/summit/${USER}/$4_lists/$4.$year$month$day.NS
done


