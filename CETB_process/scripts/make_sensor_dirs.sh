#!/bin/sh
# make_sensor_dirs.sh
# Created Jul 2017 by M. J. Brodzik
#
# This script will create all the required sensor directories for running
# the system on summit in the current directory.
#
# If any of the required directories already exist, this script will
# do nothing for that directory.
#
SRC=$1
scratchdir=/scratch/summit/${USER}
echo "Making ${scratchdir} processing directories for ${SRC}..."

for subdir in ${SRC} \
		  ${SRC}_scripts ${SRC}_scripts/output \
		  ${SRC}_GSX \
		  ${SRC}_lists \
		  ${SRC}_make \
		  ${SRC}_setup \
		  ${SRC}_sir
		     
do
    dir=${scratchdir}/${subdir}
    if [ -d "$dir" ]; then
       echo "$dir already exists..."
    else
       echo "Creating $dir..."   
       `mkdir ${dir}`
    fi

done
echo "Done"

