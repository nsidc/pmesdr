#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 5 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR DOY_START DOY_STOP SRC ENVPATH"
    echo "  Creates an sbatch file to create meas_meta_make files."
    echo "  It creates 4 different commands, for 19-22 NS, 19-22 T, "
    echo "  37-91 NS and 37-91 T, and appends them all to a single "
    echo "  output file."
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  DOY_STOP: stop day of year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
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

    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.19-22.NS.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi19-22hv_NS.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.19-22.T.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi19-22hv_T.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37-91.NS.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi37-85hv_NS.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37-91.T.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi37-85hv_T.def \
	    /scratch/summit/${USER}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${SRC}_make_list

done
#
date
