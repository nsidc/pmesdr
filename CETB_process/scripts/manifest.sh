#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 2 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] SRC ENVPATH"
    echo "  Make a manifest for deliver of data to DAAC."
    echo "  Run this script once you have confirmed you have the expected"
    echo "  number of files."
    echo "  Run this script from the \$SRC_scripts directory."
    echo "Arguments:"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

src=$1
envpath=$2
source ${envpath}/summit_set_pmesdr_environment.sh

# Clean up any prior sbatch file with same name
outfile=${src}_manifest
if [ -f $outfile ] ; then
    echo "Removing old $outfile"
    rm $outfile
fi

date
cd ../${src}_sir
for FILE in `find . -name "*.nc"`
do
    basen=`basename $FILE`
    echo $basen >> ../${outfile}
done
date
cd /scratch/summit/${USER}/${src}_scripts

exit 0
	

