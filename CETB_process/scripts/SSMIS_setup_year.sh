#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 3 ] ; then
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
source ${envpath}/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/${USER}/${SRC}_make/
#
#
# run meas_meta_setup with specific parameters
#
date
for FILE in `find ../${SRC}_make/*.${YEAR}.*`
do
    echo "$BINDIR/meas_meta_setup $OUTDIR/$FILE /scratch/summit/${USER}/${SRC}_setup" >> ${SRC}_setup_list_${YEAR}
done
#
date
