#!/bin/sh
# SSMIS_sir.sh
# Created May 2017 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Note arguments to this script are 4 digit year and platform
# This script creates the relevant directory for the year of processing in the sir directory
#
longyear=$1
src=$2
shortyear=${longyear:2:2}
echo $longyear
echo $shortyear
echo $src
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/${src}_setup/
#
#
# run meas_meta_sir with specific parameters
#
rm -f ${src}_sir_list_${longyear}
mkdir ../${src}_sir/${longyear}
date
for FILE in `find ../${src}_setup/ -name *E2[NST]${shortyear}*`
do
    echo "$BINDIR/meas_meta_sir $OUTDIR/$FILE /scratch/summit/moha2290/${src}_sir/${longyear}" >> ${src}_sir_list_${longyear}
done
#
date
   
	

