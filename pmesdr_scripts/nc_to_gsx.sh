#!/bin/sh
# getfiles.sh
# Created Mon Nov 14 2016 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# This script rips through all of the CSU .nc files for a particular satellite and creates a file
# for the loadbalancer to run and convert them from CSU to GSX input files
#
# It should be improved to take one argument that is the name of the satellite, eg F08, F10 etc
for file in `find /lustre/janus_scratch/moha2290/F08 -name "*.nc"`
do
    basen=`basename $file`
    echo "gsx SSMI-CSU $file /lustre/janus_scratch/moha2290/F08_GSX/GSX_$basen" >> /lustre/janus_scratch/moha2290/F08_scripts/gsx_lb_list
#    gsx AMSRE $file /lustre/janus_scratch/moha2290/GSX_AMSRE/GSX_$file.nc
done

