#!/bin/sh
# swath_to_gsx.sh
# Created Tue August 21 2018 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# This script rips through all of the input files for a particular satellite and creates a file
# for the loadbalancer to run and convert them from the native swath format to GSX input files
#
# first argument is the type of gsx translation to do
# second argument is the name of the platform, F08, F10, F19 etc
if [ "$1" == "-h" ] || [ "$#" -ne 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] GSX_TYPE SRC FILE_SUFFIX"
    echo "  Rips through all of the input files for a particular"
    echo "  SRC and creates a file for the loadbalancer to run"
    echo "  and convert them from the input to GSX format files."
    echo "Arguments:"
    echo "  GSX_TYPE: type of gsx translation to do: AMSRE, SMMR etc"
    echo "  SRC: input sensor source of data: AMSRE, SMMR etc"
    echo "  FILE_SUFFIX: either h5, hdf, or nc - depending on the input"
    echo ""
    echo "  SOURCE_TYPE: The type of input dataset to convert. May be one of:"
    echo "    SMMR      - Nimbus-7 SMMR Pathfinder brightness temperatures"
    echo "    SSMI-CSU  - Colorado State University NETCDF SSM/I brightness temperatures"
    echo "    SSMIS-CSU - Colorado State University NETCDF SSMIS brightness temperatures"
    echo "    RSS       - Remote Sensing Systems NETCDF SSMI brightness temperatures"
    echo "    AMSRE     - Remote Sensing Systems HDF AMSR-E brightness temperatures"
    echo "    SMAP      - SMAP L-Band radiometer brightness temperatures"
    
    exit 1
fi
gsx_type=$1
src=$2
suffix=$3
for file in `find ../${src} -name "*.${suffix}"`
do
    basen=`basename $file`
    echo "gsx $gsx_type $file /scratch/summit/${USER}/${src}_GSX/GSX_$basen.nc" >> /scratch/summit/${USER}/${src}_scripts/gsx_lb_list_summit
done

