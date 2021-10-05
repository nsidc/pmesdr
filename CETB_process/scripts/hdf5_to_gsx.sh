#!/bin/sh
# hdf5_to_gsx.sh
# Created Mon Nov 14 2016 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# This script rips through all of the hdf5 files with suffix .h5 and creates a file
# for the loadbalancer to run and convert them from hdf5 to GSX input files
#
# first argument is the type of gsx translation to do
# second argument is the name of the platform, SMAP etc
# third argument is optional top_level directory under /scratch/summit/${USER}
gsx_type=$1
src=$2
top_level=$3
direc=/scratch/summit/${USER}/${top_level}/
rm -f ${direc}/${src}_scripts/gsx_lb_list_summit
for file in `find ${direc}/${src} -name "*.h5"`
do
    basen=`basename $file`
    echo "gsx $gsx_type $file ${direc}/${src}_GSX/GSX_$basen.nc" >> ${direc}/${src}_scripts/gsx_lb_list_summit
done

