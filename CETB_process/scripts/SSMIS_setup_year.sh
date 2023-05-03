#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR SRC ENVPATH"
    echo "  Creates an sbatch script to run meas_meta_setup for 1 year of data"
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

YEAR=$1
SRC=$2
envpath=$3
top_level=$4
source ${envpath}/alpine_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
direc=/scratch/alpine/${USER}/${top_level}/
OUTDIR=${direc}/${SRC}_make/
#
#
# run meas_meta_setup with specific parameters
#
date
for FILE in `find ${OUTDIR}/*.${YEAR}.*`
do
    echo "$BINDIR/meas_meta_setup $FILE ${direc}/${SRC}_setup" >> ${direc}/${SRC}_scripts/${SRC}_setup_list_${YEAR}
done
#
date
