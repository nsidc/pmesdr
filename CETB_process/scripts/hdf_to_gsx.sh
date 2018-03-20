#!/bin/sh
# getfiles.sh
# Created Mon Nov 14 2016 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# This script rips through all of the .hdf files for a particular satellite and creates a file
# for the loadbalancer to run and convert them from CSU to GSX input files
#
# first argument is the type of gsx translation to do
# second argument is the name of the platform, F08, F10, F19 etc
if [ "$1" == "-h" ] || [ "$#" -ne 2 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] GSX_TYPE SRC"
    echo "  Rips through all of the .hdf files for a particular"
    echo "  SRC and creates a file for the loadbalancer to run"
    echo "  and convert them from CSU to GSX format files."
    echo "Arguments:"
    echo "  GSX_TYPE: type of gsx translation to do: AMSRE, SMMR etc"
    echo "  SRC: input sensor source of data: AMSRE, SMMR etc"
    echo ""
    exit 1
fi
gsx_type=$1
src=$2
for file in `find ../${src} -name "*.hdf"`
do
    basen=`basename $file`
    echo "gsx $gsx_type $file /scratch/summit/${USER}/${src}_GSX/GSX_$basen.nc" >> /scratch/summit/${USER}/${src}_scripts/gsx_lb_list_summit
done

