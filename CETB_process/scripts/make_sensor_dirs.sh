#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -gt 2 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] SRC"
    echo "  Creates all the required sensor directories for running"
    echo "  the system in the current $PMESDR_SCRATCH_DIR/top_level directory."
    echo "  Note that the top_level is an optional argument"
    echo "  If any of the required directories exist, this script will"
    echo "  do nothing for that directory."
    echo "Arguments:"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo ""
    return
fi

SRC=$1
top_level=$2
scratchdir=$PMESDR_SCRATCH_DIR/${top_level}
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


