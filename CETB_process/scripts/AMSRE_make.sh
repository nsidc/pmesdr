#!/bin/sh
# F15_make.sh
# Created Jan 30 2016 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
echo "year $1"
echo "doy1 $2"
echo "doy2 $3"
YEAR=$1
SRC=$4
DOY1=$2
DOY2=$3
source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/${SRC}_make/
FINDIR=/scratch/summit/moha2290/prototype-output
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
	    /scratch/summit/moha2290/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list_19-22_NS
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.19-22.T.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi19-22hv_T.def \
	    /scratch/summit/moha2290/${SRC}_lists/$SRC.$YEAR$MONTH$DAY" >> ${SRC}_make_list_19-22_T
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37-85.NS.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi37-85hv_NS.def \
	    /scratch/summit/moha2290/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list_37-85_NS
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37-85.T.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi37-85hv_T.def \
	    /scratch/summit/moha2290/${SRC}_lists/$SRC.$YEAR$MONTH$DAY" >> ${SRC}_make_list_37-85_T
#    echo " done with day ${DOY} "
done
#
date
