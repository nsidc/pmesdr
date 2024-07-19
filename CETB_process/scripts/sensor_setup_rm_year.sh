#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR SRC ENVPATH [top_level]"
    echo "  Creates an sbatch script to remove the very large setup files"
    echo "  for this sensor and year."
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo "  top_level: directory under $PMESDR_SCRATCH_DIR - optional argument"
    echo ""
    return
fi

longyear=$1
shortyear=${longyear:2:2}
src=$2
envpath=$3
top_level=$4
source ${envpath}/set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
direc=${PMESDR_SCRATCH_DIR}/${top_level}/

rm -rf ${direc}/${src}_setup_rm_${longyear} 
#
#
# find used setup files to remove - by year
#
date
for FILE in `find ${direc}/${src}_setup/*E2[NST]${shortyear}*`
do
    echo "rm ${direc}/${src}_setup/$FILE " >> ${direc}/${src}_setup_rm_${longyear}
done
#
date
   
	

