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
    doyminus1=$(( $doy - 1 ))
    doyplus1=$(( $doy + 1 ))
#    echo " doy -1 ${doyminus1}"
#    echo " doy +1 ${doyminus1}"
    yearminus1=$year
#    echo " year -1 ${yearminus1}"
    yearplus1=$year
#    echo " doy +1 ${yearplus1}"
    if [ $doy == 1 ]
    then
        echo "doy -1 < 1"
        yearminus1=$(( $year - 1 ))
	if [ $yearminus1 == 1984 -o $yearminus1 == 1980 ]
        then
            doyminus1=366
        else
            doyminus1=365
        fi
    fi

    if [ $doyplus1 == 366 ]
    then
        echo "doy +1 = 366"
	if [ $yearplus1 == 1984 -o $yearplus1 == 1980 ]
        then
            echo "leap year"
        else
            doyplus1=1
            yearplus1=$(( $year + 1 ))
        fi
    fi
    
    if [ $doyplus1 == 367 ]
    then
        echo "doy +1 = 367"
        doyplus1=1
        yearplus1=$(( $year + 1 ))
    fi

#    echo "final answers ${doyminus1} ${yearminus1} ${doy} ${year} ${doyplus1} ${yearplus1}"

#    echo $month
#    echo $year$monthm1$dayminus1
#    echo $year$month$day
#    echo $yearp$monthp1$dayplus1
#    echo $month$day

    doyminus1=$( printf "%03g" $doyminus1 )
    doy=$( printf "%03g" $doy )
    doyplus1=$( printf "%03g" $doyplus1 )
           
    cat /scratch/summit/moha2290/$4_lists/$4.$yearminus1.$doyminus1 \
	/scratch/summit/moha2290/$4_lists/$4.$year.$doy \
	/scratch/summit/moha2290/$4_lists/$4.$yearplus1.$doyplus1 \
	>& /scratch/summit/moha2290/$4_lists/$4.$year.$doy.NS
done


