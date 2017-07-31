#!/bin/sh
# SSMIS_setup_year.sh
# Created May 2017 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Takes 3 arguments,
# year
# input src - i.e. F16, F17, F18, F19
# path to summit_set_pmesdr_environment.sh
#
# creates a file suitable to run the loadbalancer for 1 year of setup files
#
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
