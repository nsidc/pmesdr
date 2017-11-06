#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR SRC ENVPATH"
    echo "  Creates an sbatch script to remove the very large setup files"
    echo "  for this sensor and year."
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

longyear=$1
shortyear=${longyear:2:2}
src=$2
envpath=$3
source ${envpath}/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/${USER}/${src}_setup
#
#
# find used setup files to remove - by year
#
date
for FILE in `find ../${src}_setup/*E2[NST]${shortyear}*`
do
    echo "rm $OUTDIR/$FILE " >> ${src}_setup_rm_${longyear}
done
#
date
   
	

