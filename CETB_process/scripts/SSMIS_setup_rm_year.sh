#!/bin/sh
# Fxx_sir.sh
# Created Jan 30 2016 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
longyear=$1
shortyear=${longyear:2:2}
src=$2
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/${src}_setup
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
   
	

