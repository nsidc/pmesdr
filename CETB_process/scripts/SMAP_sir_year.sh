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

longyear=$1
SRC=$2
envpath=$3
resolution=$4
top_level=$5
shortyear=${longyear:2:2}
source ${envpath}/alpine_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
direc=/scratch/alpine/${USER}/${top_level}/
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
direc=/scratch/alpine/${USER}/${top_level}
OUTDIR=${direc}/${SRC}_setup${suffix}/
#
#
# run meas_meta_setup with specific parameters
#
date
for FILE in `find ${OUTDIR}/ -name *E2[NST]${shortyear}*`
do
    echo "$BINDIR/meas_meta_sir $FILE ${direc}/${SRC}_sir${suffix}" >> ${direc}/${SRC}_scripts/${SRC}_sir_list_${YEAR}${suffix}
done
#
date
