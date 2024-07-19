#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 6 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR_START DOY_START YEAR_END DOY_END SRC ENVPATH TOP_LEVEL"
    echo "  Makes daily and concatenated 3-day file lists of GSX input files for the "
    echo "  requested sensor SRC"
    echo "Arguments:"
    echo "  YEAR_START: start 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  YEAR_END: end 4-digit year"
    echo "  DOY_END: end day of year"
    echo "  SRC: input sensor source of data: AMSRE"
    echo "  ENVPATH: path to set_pmesdr_environment.sh script"
    echo "  TOP_LEVEL: optional directory below $PMESDR_SCRATCH_DIR"
    echo ""
    return
fi

startyear=$1
startdoy=$2
endyear=$3
enddoy=$4
sensor=$5
envpath=$6
top_level=$7

# Delete the output file.  It will be appended to by each iteration of the
# year loop, below.
outfile=$PMESDR_SCRATCH_DIR/${top_level}/${sensor}_scripts/${sensor}_make_list
rm -rf ${outfile}
echo "$0: removing make file: $outfile"

# Call AMSRE_make.sh for each year to process
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
    
    echo "$0: AMSR_make for: $year $thisbegindoy $thisenddoy $sensor"
    source $PMESDR_RUN/AMSR_make.sh $year $thisbegindoy $thisenddoy $sensor $envpath $top_level
    
done    

echo "$0: All make output written to: $outfile"	    
