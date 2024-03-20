#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR SRC ENVPATH top_level"
    echo "  Creates an sbatch script to run meas_meta_sir processing"
    echo "  for this sensor and year."
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to single_set_pmesdr_environment.sh script"
    echo "  top_level: optional parameter directory below /scratch/summit/${USER}"
    echo ""
    exit 1
fi

longyear=$1
src=$2
envpath=$3
top_level=$4
shortyear=${longyear:2:2}
source ${envpath}/single_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
direc=/scratch/alpine/${USER}/${top_level}/
OUTDIR=/scratch/alpine/${USER}/${top_level}/${src}_setup/

# run meas_meta_sir with specific parameters
rm -f ${direc}/${src}_scripts/${src}_sir_list_${longyear}
mkdir ${direc}/${src}_sir/${longyear}
date
for FILE in `find ${direc}/${src}_setup/ -name *E2[NST]${shortyear}*`
do
    echo "$BINDIR/meas_meta_sir $FILE ${direc}/${src}_sir/${longyear}" >> ${direc}/${src}_scripts/${src}_sir_list_${longyear}
done
#
date
