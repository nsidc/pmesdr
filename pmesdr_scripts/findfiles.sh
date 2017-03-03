#!/bin/sh
# getfiles.sh
# Created Mon Nov 14 2016 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
year=2002
for doy in `seq 360 365`
do
    echo $doy
    day=`date -d "$year-01-01 + $doy days" +%d`
    echo $day
    month=`date -d "$year-01-01 + $doy days" +%m`
    echo $month
    echo $year$month$day
    for file in `find /lustre/janus_scratch/moha2290/AMSRE -name \*$year$month$day\*.hdf`
    do
	echo $file GSX_$file.nc
	gsx AMSRE $file /lustre/janus_scratch/moha2290/GSX_AMSRE/GSX_$file.nc
    done
done

#for file in `find /lustre/janus_scratch/moha2290/AMSRE -name \*$date\*`
#do
#    echo $file >> AMSRE.20020708;
#    if [ $j -ge 1279999 ]
#    then
#	scancel $j
#	sleep 1
#    fi
#done
