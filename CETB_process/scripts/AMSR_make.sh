#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 5 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR DOY_START DOY_STOP SRC ENVPATH"
    echo "  Creates an sbatch file to create meas_meta_make files."
    echo "  It creates 8 different commands, for 6-10 NS, 6-10 T"
    echo "  18-22 NS, 18-22 T, 36 NS, 36 T 91 NS and 91 T"
    echo "  and appends them all to a single output file."
    echo "  output file.  It can be used for AMSRE or AMSR2 sensors,"
    echo "  since they have the same number of channels."
    echo "Arguments:"
    echo "  YEAR: 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  DOY_STOP: stop day of year"
    echo "  SRC: input sensor source of data: AMSRE or AMSR2"
    echo "  ENVPATH: path to alpine_set_pmesdr_environment.sh script"
    echo "  top_level: used for NRT processing"
    echo ""
    return
fi

echo "year $1"
echo "doy1 $2"
echo "doy2 $3"
echo "SRC $4"
echo "path $5"
echo "top_level $6"
YEAR=$1
SRC=$4
DOY1=$2
DOY2=$3
envpath=$5
top_level=$6
source ${envpath}/set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=$PMESDR_SCRATCH_DIR/${top_level}/${SRC}_make/
direc=$PMESDR_SCRATCH_DIR/${top_level}/

if [[ "${SRC}" == "AMSR2" ]] ; then
   sat=GCOMW1
elif [[ "${SRC}" == "AMSRE" ]] ; then
     sat=AQUA
else
    echo "SRC must be either AMSR2 or AMSRE"
    return
fi
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

    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.6-10.NS.meta $sat $DOYA $DOYA $YEAR $TOPDIR/ref/amsr2-6-10hv_NS.def \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" \
	 >> ${direc}/${SRC}_scripts/${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.6-10.T.meta $sat $DOYA $DOYA $YEAR $TOPDIR/ref/amsr2-6-10hv_T.def \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" \
	 >> ${direc}/${SRC}_scripts/${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.18-23.NS.meta $sat $DOYA $DOYA $YEAR $TOPDIR/ref/amsr2-18-23hv_NS.def \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" \
	 >> ${direc}/${SRC}_scripts/${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.18-23.T.meta $sat $DOYA $DOYA $YEAR $TOPDIR/ref/amsr2-18-23hv_T.def \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" \
	 >> ${direc}/${SRC}_scripts/${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37.NS.meta $sat $DOYA $DOYA $YEAR $TOPDIR/ref/amsr2-37hv_NS.def \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" \
	 >> ${direc}/${SRC}_scripts/${SRC}_make_list
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37.T.meta $sat $DOYA $DOYA $YEAR $TOPDIR/ref/amsr2-37hv_T.def \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" \
	 >> ${direc}/${SRC}_scripts/${SRC}_make_list
    echo "$BINDIR/meas_meta_make -t -16 $OUTDIR/$SRC.$DOYA.${YEAR}.89.NS.meta $sat $DOYA $DOYA $YEAR $TOPDIR/ref/amsr2-89abhv_NS.def \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" \
	 >> ${direc}/${SRC}_scripts/${SRC}_make_list
    echo "$BINDIR/meas_meta_make -t -16 $OUTDIR/$SRC.$DOYA.${YEAR}.89.T.meta $sat $DOYA $DOYA $YEAR $TOPDIR/ref/amsr2-89abhv_T.def \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" \
	 >> ${direc}/${SRC}_scripts/${SRC}_make_list

done
#
date
