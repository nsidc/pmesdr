#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 3 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] SRC YYYYMM_START YYYYMM_STOP"
    echo "  Create a set of year and month subdirs in the \$SRC_sir"
    echo "    directory in $PMESDR_SCRATCH_DIR/top_level.  Dates are"
    echo "    inclusive. top_level is optional"
    echo "Arguments:"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  YYYYMM_START: 4-digit year and 2-digit month to start"
    echo "  YYYYMM_STOP: 4-digit year and 2-digit month to stop"
    echo "  top_level - directory under the scratch dir"
    echo ""
    return
fi

SRC=$1
YYYYMM_START=$2
YYYYMM_STOP=$3
top_level=$4

yyyy_start=${YYYYMM_START:0:4}
mm_start=${YYYYMM_START:4:2}
yyyy_stop=${YYYYMM_STOP:0:4}
mm_stop=${YYYYMM_STOP:4:2}

for year in $(seq $yyyy_start $yyyy_stop) ; do
    for month in $(seq 1 12) ; do 
	if ((year == yyyy_start)) ; then
	    if ((month < mm_start)) ; then continue ; fi
	elif ((year == yyyy_stop)) ; then
	    if ((month > mm_stop)) ; then continue ; fi
	fi
	mm=$(printf "%02d" $month)
	dir=$PMESDR_SCRATCH_DIR/$top_level/${SRC}_sir/${year}_${mm}
	mkdir -pv $dir
    done
done



