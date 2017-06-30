#!/bin/sh
# SSMIS_sir.sh
# Created May 2017 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Note this is a 2 digit year because of the setup file naming convention
# Also note that running the script will require creation of the Fxx_sir/year directory
#
year=$1
src=$2
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/${src}_setup/
#
#
# run meas_meta_sir with specific parameters
#
mkdir ../${src}_sir/20${year}
date
for FILE in `find ../${src}_setup/ -name *E2[NST]${year}*`
do
    echo "$BINDIR/meas_meta_sir $OUTDIR/$FILE /scratch/summit/moha2290/${src}_sir/20${year}" >> ${src}_sir_list_${year}
done
#
date
   
	

