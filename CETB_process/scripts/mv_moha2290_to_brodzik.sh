#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 1 ] ; then
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

# Make a fresh output file
outfile=moving_files_${year}
if [ -f $outfile ] ; then
    echo "Removing old $outfile"
    rm $outfile
fi

for file in `find /scratch/summit/moha2290/F15_sir/${year}/*.nc*`
do
    echo "cp $file /scratch/summit/${USER}/F15_sir/${year}/" >> ${outfile}
done

echo "Wrote move commands to $outfile"

exit 0
