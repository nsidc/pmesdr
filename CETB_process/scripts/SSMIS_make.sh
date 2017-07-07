#!/bin/sh
# SSMIS_make.sh
# Created May 2017 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# This script builds the command lines to feed a load balance run to create make files
# This script works for SSMIS instruments - takes 4 arguments
# $1 - year - is the year to process
# $2 - doy1 - is the start doy
# $3 - doy2 - is the end doy
# $4 - src - is the input one of F16, F17, F18 or F19
# The script creates 4 different files, for 19-22 NS, 19-22 T, 37-91 NS and 37-91 T
# Usually combine them into 1 files afterwards and run all "makes" together
# $Id$
# $Log$
#
echo "year $1"
echo "doy1 $2"
echo "doy2 $3"
echo "SRC $4"
YEAR=$1
SRC=$4
DOY1=$2
DOY2=$3
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/${SRC}_make/
#
#
# run meas_meta_make with specific parameters
#
date
echo $YEAR
echo $DOY1
echo $DOY2
for DOY in `seq $DOY1 $DOY2`
do
    DOYM1=$(( $DOY - 1 ))
#    echo $DOYM1
    DAY=`date -d "$YEAR-01-01 + $DOYM1 days" +%d`
#    echo $DAY
#    echo `$DOY - 1`
    MONTH=`date -d "$YEAR-01-01 + $DOYM1 days" +%m`
#    echo $MONTH
    DOYA=$( printf "%03g" $DOY )
#    echo $DOYA
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.19-22.NS.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi19-22hv_NS.def \
	    /scratch/summit/moha2290/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.19-22.T.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi19-22hv_T.def \
	    /scratch/summit/moha2290/${SRC}_lists/$SRC.$YEAR$MONTH$DAY" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37-91.NS.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi37-85hv_NS.def \
	    /scratch/summit/moha2290/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37-91.T.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi37-85hv_T.def \
	    /scratch/summit/moha2290/${SRC}_lists/$SRC.$YEAR$MONTH$DAY" >> ${SRC}_make_list
#    echo " done with day ${DOY} "
done
#
date
