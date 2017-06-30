#!/bin/sh
# F15_premetandspatial.sh
# Created Apr 2017 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
#
# This script takes a platform/src s input argument and generates a files suitable
# for the load balancer to run create premet and spatial files
#
# $Id$
# $Log$
#
src=$1
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
#
date
for FILE in `find ../${src}_sir -name "*.nc"`
do
    echo "/projects/moha2290/miniconda3/envs/cetbtools/bin/generate_premetandspatial.py $FILE" >> ${src}_premet_list_cetb
done
date
   
	

