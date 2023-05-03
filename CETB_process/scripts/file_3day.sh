#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 4 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR DOY_START DOY_STOP SRC"
    echo "  Make a new list of input files for the NS lists, "
    echo "  which need 3 days of input files for each day processed."
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  DOY_STOP: stop day of year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  TOP_LEVEL: for NRT processing"
    echo ""
    exit 1
fi

year=$1
startdoy=$2
stopdoy=$3
src=$4
top_level=$5
direc=/scratch/alpine/${USER}/${top_level}/

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" 1>&2
    exit 1
}

enddaydate=`date -d "$year-01-01 + $(( 10#${stopdoy} - 1 )) days" +%d`
startdaydate=`date -d "$year-01-01 + $(( 10#${startdoy} - 1 )) days" +%d`
for doy in `seq ${startdoy} ${stopdoy}`
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

    if [[ 10#${doy} -lt 10#${stopdoy} ]]
    then
	cat ${direc}/${src}_lists/${src}.$yearm1$monthm1$dayminus1 \
	    ${direc}/${src}_lists/${src}.$year$month$day \
	    ${direc}/${src}_lists/${src}.$yearp$monthp1$dayplus1 \
	    >& ${direc}/${src}_lists/${src}.$year$month$day.NS 2>/dev/null
    else
	cat ${direc}/${src}_lists/${src}.$yearm1$monthm1$dayminus1 \
	    ${direc}/${src}_lists/${src}.$year$month$day \
	    >& ${direc}/${src}_lists/${src}.$year$month$day.NS 2>/dev/null
    fi
	
done


