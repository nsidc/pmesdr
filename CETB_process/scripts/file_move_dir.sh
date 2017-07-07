#!/bin/sh
# script to move SSMIS files from the src_sir directory into year_month directories
# script creates list of mv commands by year
# edit and run file_move.sh first to create monthly directories
# arguments are 4 digit year, src i.e. F18, F10 etc and type eg SSMI, SSMIS, AMSRE
year=$1
SRC=$2
type=$3
for file in `find /scratch/summit/moha2290/${SRC}_sir/${year}/*-${SRC}_${type}-${year}*.nc*`
do
    basen=`basename $file`
    index=`echo $basen | grep -bo $year | sed 's/:.*$//'`
    index_day=$(( $index + 4 ))
    doy=${basen:$index_day:3}
    doyminus1=$(( 10#$doy - 1 ))  # note that the 10# forces base 10 otherwise
    day=`date -d "$year-01-01 + $doyminus1 days" +%d`
    month=`date -d "$year-01-01 + $doyminus1 days" +%m`
    echo "mv $file /scratch/summit/moha2290/${SRC}_sir/${year}_$month/" >> moving_files_${year}
done
