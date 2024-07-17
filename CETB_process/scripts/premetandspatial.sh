#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 2  ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] SRC ENVPATH"
    echo "  Make an sbatch script to make premet and spatial metadata"
    echo "  files for a collection of CETB files to be migrated to DAAC."
    echo "  Run this script from the \$SRC_scripts directory."
    echo "Arguments:"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

src=$1
envpath=$2
year=$3
top_level=$4
echo $src $envpath $year $top_level
source ${envpath}/set_pmesdr_environment.sh

# Clean up any prior sbatch file with same name
direc=$PMESDR_SCRATCH_DIR/${top_level}
outfile=${direc}/${src}_scripts/${src}_premet_list_cetb_${year}
if [ -f $outfile ] ; then
    echo "Removing old $outfile"
    rm $outfile
fi

date
for FILE in `find ${direc}/${src}_sir/${year} -name "*.nc"`
do
    echo "generate_premetandspatial.py $FILE" >> ${outfile}
done
date

   
	

