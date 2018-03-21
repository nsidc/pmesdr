#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 5 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR DOY_START DOY_STOP SRC ENVPATH"
    echo "  Creates an sbatch file to create meas_meta_make files."
    echo "  It creates 8 different commands, for 6-10 NS, 6-10 T"
    echo "  18-22 NS, 18-22 T, 36 NS, 36 T 91 NS and 91 T"
    echo "  and appends them all to a single output file."
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  DOY_STOP: stop day of year"
    echo "  SRC: input sensor source of data: AMSRE"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

echo "year $1"
echo "doy1 $2"
echo "doy2 $3"
echo "SRC $4"
echo "path $5"
YEAR=$1
SRC=$4
DOY1=$2
DOY2=$3
envpath=$5
source ${envpath}/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/${USER}/${SRC}_make/
#
#
# run meas_meta_make with specific parameters
#
date
echo "$0: $YEAR $DOY1 $DOY2"

for DOY in `seq $DOY1 $DOY2`
do
    DOYM1=$(( $DOY - 1 ))
    DAY=`date -d "$YEAR-01-01 + $DOYM1 days" +%d`
    MONTH=`date -d "$YEAR-01-01 + $DOYM1 days" +%m`
    DOYA=$( printf "%03g" $DOY )

    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.6-10.NS.meta AQUA $DOYA $DOYA $YEAR $TOPDIR/ref/amsre6-10hv_NS.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.6-10.T.meta AQUA $DOYA $DOYA $YEAR $TOPDIR/ref/amsre6-10hv_T.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.18-23.NS.meta AQUA $DOYA $DOYA $YEAR $TOPDIR/ref/amsre18-23hv_NS.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.18-23.T.meta AQUA $DOYA $DOYA $YEAR $TOPDIR/ref/amsre18-23hv_T.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.36.NS.meta AQUA $DOYA $DOYA $YEAR $TOPDIR/ref/amsre36hv_NS.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.36.T.meta AQUA $DOYA $DOYA $YEAR $TOPDIR/ref/amsre36hv_T.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make -t -12 $OUTDIR/$SRC.$DOYA.${YEAR}.89.NS.meta AQUA $DOYA $DOYA $YEAR $TOPDIR/ref/amsre89hvab_NS.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make -t -12 $OUTDIR/$SRC.$DOYA.${YEAR}.89.T.meta AQUA $DOYA $DOYA $YEAR $TOPDIR/ref/amsre89hvab_T.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list

done
#
date
