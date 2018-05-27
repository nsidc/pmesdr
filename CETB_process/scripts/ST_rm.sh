#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 1 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] SRC"
    echo "  Creates an sbatch script to remove T"
    echo "  files for this sensor."
    echo "Arguments:"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo ""
    exit 1
fi

src=$1
DIR=/scratch/summit/${USER}/${src}_sir

# Make a fresh output file
outfile=/scratch/summit/${USER}/${src}_rm_ST_files
if [ -f $outfile ] ; then
    echo "Removing old $outfile"
    rm $outfile
fi

#
# find files to remove
#
date
echo "Looking for S and T files in $DIR..."
for FILE in `find $DIR -name "NSIDC-0630-EASE2_S*.nc" -o -name "NSIDC-0630-EASE2_T*.nc"`
do
    echo "rm $FILE" >> $outfile
done
date
   
	

