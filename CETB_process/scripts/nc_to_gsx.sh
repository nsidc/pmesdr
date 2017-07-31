#!/bin/sh
# getfiles.sh
# Created Mon Nov 14 2016 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# This script rips through all of the CSU .nc files for a particular satellite and creates a file
# for the loadbalancer to run and convert them from CSU to GSX input files
#
# It assumes it is begin run from /scratch/summit/${USER}/${SRC}_scripts/ directory
#
# first argument is the type of gsx translation to do - e.g. SSMI-CSU, SSMIS-CSU, AMSRE
# second argument is the name of the platform, F08, F10, F19 etc
gsx_type=$1
src=$2
for file in `find ../${src} -name "*.nc"`
do
    basen=`basename $file`
    echo "gsx $gsx_type $file /scratch/summit/${USER}/${src}_GSX/GSX_$basen" >> /scratch/summit/${USER}/${src}_scripts/gsx_lb_list_summit
done

