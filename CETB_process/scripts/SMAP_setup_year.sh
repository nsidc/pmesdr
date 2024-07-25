#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR SRC ENVPATH"
    echo "  Creates an sbatch script to run meas_meta_setup for 1 year of data"
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to alpine_set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

YEAR=$1
SRC=$2
envpath=$3
resolution=$4
top_level=$5
source ${envpath}/set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
direc=${PMESDR_SCRATCH_DIR}/${top_level}/
if [[ resolution -eq 0 ]]
then
    suffix=""
elif [[ resolution -eq 1 ]]
then
    suffix="_36"
elif [[ resolution -eq 2 ]]
then
    suffix="_24"
fi
OUTDIR=${direc}/${SRC}_make${suffix}/
#
#
# run meas_meta_setup with specific parameters
#
date
for FILE in `find ${OUTDIR}/*.${YEAR}.*`
do
    echo "$BINDIR/meas_meta_setup $FILE ${direc}/${SRC}_setup${suffix}" >> ${direc}/${SRC}_scripts/${SRC}_setup_list_${YEAR}${suffix}
done
#
date
