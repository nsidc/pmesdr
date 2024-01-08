#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 2 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] SRC"
    echo "  Rips through all a directory of input swath files for a particular"
    echo "  SRC and creates a file to be run in gnu_parallel or via loadbalancer"
    echo "  and convert them from input swath to GSX format files."
    echo "Arguments:"
    echo "  SRC: input sensor source of data: AMSR2 or AMSRE"
    echo "  top_level is optional"
    echo ""
    exit 1
fi

src=$1
top_level=$2

direc=/scratch/alpine/${USER}/${top_level}/
for file in `find ${direc}/${src}-L1C_GSX -name "*.nc.partial"`
do
    echo "combine_amsr_l1c_jaxa $src $file ${direc}/${src}-JAXA_GSX ${direc}/AMSR2_GSX" >> ${direc}/${src}_scripts/gsx_lb_list_alpine
done

