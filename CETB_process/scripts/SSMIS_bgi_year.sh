#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR SRC ENVPATH"
    echo "  Creates an sbatch script to run meas_meta_sir processing"
    echo "  for this sensor and year."
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

longyear=$1
src=$2
envpath=$3
shortyear=${longyear:2:2}
source ${envpath}/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/${USER}/${src}_setup/

# run meas_meta_sir with specific parameters
rm -f ${src}_bgi_list_${longyear}
mkdir ../${src}_bgi/${longyear}
date
for FILE in `find ../${src}_setup/ -name *E2[NST]${shortyear}*`
do
    echo "$BINDIR/meas_meta_bgi $OUTDIR/$FILE /scratch/summit/${USER}/${src}_bgi/${longyear}" >> ${src}_bgi_list_${longyear}
done
#
date
