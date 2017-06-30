#!/bin/sh
# file_lists.sh
# Created May 2017 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# this little script will take a year, start and stop days and satellite as arguments and then create new input files for
#   the NS lists of 3 days of files
year=$1
for doy in `seq $2 $3`
do
#    echo $doy
    realdoy=$(( $doy - 1 ))
    day=`date -d "$year-01-01 + $realdoy days" +%d`
    doyminus1=$(( $realdoy - 1 ))
    doyplus1=$(( $realdoy + 1 ))
#    echo $doyminus1
#    echo $realdoy
#    echo $doyplus1
    dayminus1=`date -d "$year-01-01 + $doyminus1 days" +%d`
    dayplus1=`date -d "$year-01-01 + $doyplus1 days" +%d`
    monthm1=`date -d "$year-01-01 + $doyminus1 days" +%m`
    month=`date -d "$year-01-01 + $realdoy days" +%m`
    monthp1=`date -d "$year-01-01 + $doyplus1 days" +%m`
    yearm1=$year
    yearp=$year
    if [ $day == 01 -a $month == 01 ]
    then
#        echo "1st of the year"
	dayminus1=31
        monthm1=12
	yearm1=$(( $year - 1 ))
    fi
#    echo $month
#    echo $year$monthm1$dayminus1
#    echo $year$month$day
#    echo $yearp$monthp1$dayplus1
#    echo $month$day
    if [ $day == 31 -a $month == 12 ]
    then
	yearp=$(( $year + 1 ))
	dayplus1=01
	monthp1=01
#	echo $yearp$monthp1$dayplus1
    fi
#    echo $yearp$monthp1$dayplus1
           
    cat /scratch/summit/moha2290/$4_lists/$4.$yearm1$monthm1$dayminus1 \
	/scratch/summit/moha2290/$4_lists/$4.$year$month$day \
	/scratch/summit/moha2290/$4_lists/$4.$yearp$monthp1$dayplus1 \
	>& /scratch/summit/moha2290/$4_lists/$4.$year$month$day.NS
done


