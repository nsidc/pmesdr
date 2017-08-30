#!/bin/sh
# Fxx_sir.sh
# Created Jan 30 2016 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Arguments to this script are
# 4 digit year
# platform
# path to summit_set_pmesdr_environment.sh script
#
# This script creates a list of commands to remove the large setup files for this
# sensor and year
#
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
   
	

