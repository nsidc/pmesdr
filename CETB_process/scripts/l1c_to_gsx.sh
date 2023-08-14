#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 2 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] GSX_TYPE SRC"
    echo "  Rips through all a directory of input swath files for a particular"
    echo "  SRC and creates a file to be run in gnu_parallel or via loadbalancer"
    echo "  and convert them from input swath to GSX format files."
    echo "Arguments:"
    echo "  GSX_TYPE: type of gsx translation to do: SSMI-CSU, SSMIS-CSU, AMSRE etc"
    echo "  SRC: input sensor source of data: F08, F10, AMSR2-L1C, AMSR2-JAXA etc"
    echo "  note for DMSP input src is just the satellite, but for AMSR2 it is the same as the gsx type"
    echo "  SUFFIX: input file suffix"
    echo "  top_level is optional"
    echo ""
    exit 1
fi

gsx_type=$1
src=$2
suffix=$3
top_level=$4

if [[ ${src} == *"AMSR"* ]];
then
    gsx_suffix="nc.partial"
else
    gsx_suffix="nc"
fi

echo $gsx_suffix

direc=/scratch/alpine/${USER}/${top_level}/
for file in `find ${direc}/${src} -name "*.${suffix}"`
do
    basen=`basename $file`
    echo "gsx $gsx_type $file ${direc}/${src}_GSX/GSX_$basen.${gsx_suffix}" >> ${direc}/${src}_scripts/gsx_lb_list_alpine
done

