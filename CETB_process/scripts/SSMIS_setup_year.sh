#!/bin/sh
# SSMIS_setup_year.sh
# Created May 2017 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Takes 2 arguments, year and input src - i.e. F16, F17, F18, F19
# creates a file suitable to run the loadbalancer for 1 year of setup files
YEAR=$1
SRC=$2
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/${SRC}_make/
#
#
# run meas_meta_setup with specific parameters
#
date
for FILE in `find ../${SRC}_make/*.${YEAR}.*`
do
    echo "$BINDIR/meas_meta_setup $OUTDIR/$FILE /scratch/summit/moha2290/${SRC}_setup" >> ${SRC}_setup_list_${YEAR}
done
#
date
   
	

