#!/bin/sh
res_string=""
resolution=0
top_string=""
top_level=""
OPTIND=1

usage() {
    echo "" 1>&2
    echo "Usage: `basename $0` [-htr] YEAR DOY_START DOY_STOP SRC ENVPATH" 1>&2
    echo "  Creates an sbatch file to create meas_meta_make files." 1>&2
    echo "  It creates 4 different commands, for 19-22 NS, 19-22 T, " 1>&2
    echo "  37-91 NS and 37-91 T, and appends them all to a single " 1>&2
    echo "  output file." 1>&2
    echo "Arguments:" 1>&2
    echo "  YEAR: 4-digit year" 1>&2
    echo "  DOY_START: start day of year" 1>&2
    echo "  DOY_STOP: stop day of year" 1>&2
    echo "  SRC: input sensor source of data: F08, F10, etc" 1>&2
    echo "  ENVPATH: path to alpine_set_pmesdr_environment.sh script" 1>&2
    echo "" 1>&2
}

while getopts t:r:h opt; do
    case $opt in
	t) top_level=$OPTARG
	   top_string="-t ${top_level}";;
	r) resolution=$OPTARG
	   res_string="-r ${resolution}";;
	h) usage
	   exit 1;;
	?) printf "Usage: %s: [-t] [-r] args\n" $0
           exit 1;;
	esac
done

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" 1>&2
    exit 1
}

shift $(($OPTIND - 1))

[[ "$#" -eq 5 ]] || error_exit "Line $LINENO: Unexpected number of arguments."

echo "year $1"
echo "doy1 $2"
echo "doy2 $3"
echo "SRC $4"
echo "path $5"
echo "resolution ${resolution}"

YEAR=$1
SRC=$4
DOY1=$2
DOY2=$3
envpath=$5
def_file_N="E2N_smap.def"
def_file_S="E2S_smap.def"
def_file_T="E2T_smap.def"

suffix=""
if [[ "${resolution}" == "1" ]]
then
    suffix="_36"
    def_file_N="E2N_smap_9km.def"
    def_file_S="E2S_smap_9km.def"
    def_file_T="E2T_smap_9km.def"
fi
if [[ "$resolution" == "2" ]]
then
    suffix="_24"
fi

echo "suffix=${suffix} resolution=${resolution} res_string=${res_string} ${top_level}"

source ${envpath}/alpine_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/alpine/${USER}/${top_level}/${SRC}_make${suffix}/
direc=/scratch/alpine/${USER}/${top_level}
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

    echo "$BINDIR/meas_meta_make ${res_string} $OUTDIR/$SRC.$DOYA.${YEAR}.N.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/${def_file_N} \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${direc}/${SRC}_scripts/${SRC}_make_list${suffix}
    echo "$BINDIR/meas_meta_make ${res_string} $OUTDIR/$SRC.$DOYA.${YEAR}.T.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/${def_file_T} \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${direc}/${SRC}_scripts/${SRC}_make_list${suffix}
    echo "$BINDIR/meas_meta_make ${res_string} $OUTDIR/$SRC.$DOYA.${YEAR}.S.meta $SRC $DOYA $DOYA $YEAR $TOPDIR/ref/${def_file_S} \
	    ${direc}/${SRC}_lists/$SRC.$YEAR$MONTH$DAY.NS" >> ${direc}/${SRC}_scripts/${SRC}_make_list${suffix}

done
#
date
