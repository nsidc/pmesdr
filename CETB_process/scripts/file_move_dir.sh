#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YYYY SRC TYPE"
    echo "  Make an sbatch script to move CETB files from YYYY directory to "
    echo "  YYYY_MM directories, for migration to DAAC."
    echo "  Call this script from the \$SRC_scripts directory to"
    echo "  create a moving_files_YYYY sbatch file."
    echo "Arguments:"
    echo "  YYYY: 4-digit year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  TYPE: type of data to move: SSMI, SSMIS, AMSRE etc"
    echo ""
    exit 1
fi

year=$1
SRC=$2
type=$3

# Make a fresh output file
outfile=moving_files_${year}
if [ -f $outfile ] ; then
    echo "Removing old $outfile"
    rm $outfile
fi

for file in `find /scratch/summit/${USER}/${SRC}_sir/${year}/*-${SRC}_${type}-${year}*.nc*`
do
    basen=`basename $file`
    index=`echo $basen | grep -bo $year | sed 's/:.*$//'`
    index_day=$(( $index + 4 ))
    doy=${basen:$index_day:3}
    doyminus1=$(( 10#$doy - 1 ))  # note that the 10# forces base 10 otherwise
    day=`date -d "$year-01-01 + $doyminus1 days" +%d`
    month=`date -d "$year-01-01 + $doyminus1 days" +%m`
    echo "mv $file /scratch/summit/${USER}/${SRC}_sir/${year}_$month/" >> ${outfile}
done

echo "Wrote move commands to $outfile"

exit 0
