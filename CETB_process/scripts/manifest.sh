#!/bin/sh
# manifest.sh
# Created Dec 5 2016 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Takes src as argument and creates manifest for output sir files
# Only run this once you have verified that you have the correct number of expected files
src=$1
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
#
date
cd ../${src}_sir
for FILE in `find . -name "*.nc"`
do
    basen=`basename $FILE`
    echo $basen >> ../${src}_manifest
done
cd /scratch/summit/moha2290/${src}_scripts
date
   
	

