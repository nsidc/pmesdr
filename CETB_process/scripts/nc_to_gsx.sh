#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 2 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] GSX_TYPE SRC"
    echo "  Rips through all of the CSU .nc files for a particular"
    echo "  SRC and creates a file for the loadbalancer to run"
    echo "  and convert them from CSU to GSX format files."
    echo "Arguments:"
    echo "  GSX_TYPE: type of gsx translation to do: SSMI-CSU, SSMIS-CSU, AMSRE etc"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo ""
    exit 1
fi

gsx_type=$1
src=$2
for file in `find ../${src} -name "*.nc"`
do
    basen=`basename $file`
    echo "gsx $gsx_type $file /scratch/summit/${USER}/${src}_GSX/GSX_$basen" >> /scratch/summit/${USER}/${src}_scripts/gsx_lb_list_summit
done

