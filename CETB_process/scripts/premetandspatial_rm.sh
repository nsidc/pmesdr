#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 1 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] SRC"
    echo "  Creates an sbatch script to remove premet and spatial"
    echo "  files for this sensor."
    echo "Arguments:"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo ""
    exit 1
fi

src=$1
DIR=/scratch/summit/${USER}/${src}_sir

# Make a fresh output file
outfile=/scratch/summit/${USER}/${src}_rm_premet_and_spatial
if [ -f $outfile ] ; then
    echo "Removing old $outfile"
    rm $outfile
fi

#
# find premet and spatial files to remove
#
date
echo "Looking for premet/spatial files in $DIR..."
for FILE in `find $DIR -name "*.nc.premet" -o -name "*.nc.spatial"`
do
    echo "rm $FILE" >> $outfile
done
date
   
	

