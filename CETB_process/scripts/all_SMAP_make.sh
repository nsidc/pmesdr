#!/bin/sh
res_string=""
resolution=0
OPTIND=1

usage() {
    echo "" 1>&2
    echo "Usage: `basename $0` [-h] [-r] resolution YEAR_START DOY_START YEAR_END DOY_END SRC ENVPATH" 1>&2
    echo "  Makes a list of meas_meta_make commands" 1>&2
    echo "  this script is a wrapper for the yearly meas_meta_make list" 1>&2
    echo "  the script calls the yearly script for each year in the period" 1>&2
    echo "Arguments:" 1>&2
    echo "  YEAR_START: start 4-digit year" 1>&2
    echo "  DOY_START: start day of year" 1>&2
    echo "  YEAR_END: end 4-digit year" 1>&2
    echo "  DOY_END: end day of year" 1>&2
    echo "  SRC: input sensor source of data: F08, F10, etc" 1>&2
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script" 1>&2
    echo "" 1>&2
}

res_string=""
resolution=0
top_string=""
top_level=""

while getopts r:t:h opt; do
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

[[ "$#" -eq 6 ]] || error_exit "Line $LINENO: $# Unexpected number of arguments."

startyear=$1
startdoy=$2
endyear=$3
enddoy=$4
sensor=$5
envpath=$6

suffix=""
if [[ resolution -eq 1 ]]
then
    suffix="_36"
elif [[ resolution -eq 2 ]]
then
    suffix="_24"
fi

# Delete the output file.  It will be appended to by each iteration of the
# year loop, below.
outfile=/scratch/summit/${USER}/${top_level}/${sensor}_scripts/${sensor}_make_list${suffix}
rm -f ${outfile}
echo "$0: removing make file: $outfile"

# Call SMAP_make.sh for each year to process
for year in `seq $startyear $endyear`
do
    if [ "$year" -eq "$startyear" ]; then
	thisbegindoy=$startdoy
    else
	thisbegindoy=1
    fi

    if [ "$year" -eq "$endyear" ]; then
	thisenddoy=$enddoy
    else
	# Test good for years from 1901 - 2099
	if ! ((year % 4)); then
	    thisenddoy=366
	else
	    thisenddoy=365
	fi
    fi
    
    echo "$0: SMAP_make for: $year $thisbegindoy $thisenddoy $sensor"
    source $PMESDR_RUN/SMAP_make.sh ${res_string} ${top_string} $year $thisbegindoy $thisenddoy $sensor $envpath
    
done    

echo "$0: All make output written to: $outfile"	    
