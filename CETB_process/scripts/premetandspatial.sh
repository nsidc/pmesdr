#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 2 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] SRC ENVPATH"
    echo "  Make an sbatch script to make premet and spatial metadata"
    echo "  files for a collection of CETB files to be migrated to DAAC."
    echo "  Run this script from the \$SRC_scripts directory."
    echo "Arguments:"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

src=$1
envpath=$2
source ${envpath}/alpine_set_pmesdr_environment.sh

# Clean up any prior sbatch file with same name
outfile=${src}_premet_list_cetb
if [ -f $outfile ] ; then
    echo "Removing old $outfile"
    rm $outfile
fi

date
for FILE in `find ../${src}_sir -name "*.nc"`
do
    echo "generate_premetandspatial.py $FILE" >> ${outfile}
done
date

   
	

