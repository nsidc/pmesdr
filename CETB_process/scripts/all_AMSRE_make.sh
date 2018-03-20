#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 6 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR_START DOY_START YEAR_END DOY_END SRC ENVPATH"
    echo "  Makes daily and concatenated 3-day file lists of GSX input files for the "
    echo "  requested sensor SRC"
    echo "Arguments:"
    echo "  YEAR_START: start 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  YEAR_END: end 4-digit year"
    echo "  DOY_END: end day of year"
    echo "  SRC: input sensor source of data: AMSRE"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo ""
    exit 1
fi

startyear=$1
startdoy=$2
endyear=$3
enddoy=$4
sensor=$5
envpath=$6

# Delete the output file.  It will be appended to by each iteration of the
# year loop, below.
outfile=${sensor}_make_list
rm -f ${outfile}
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
    
    echo "$0: AMSRE_make for: $year $thisbegindoy $thisenddoy $sensor"
    source $PMESDR_RUN/AMSRE_make.sh $year $thisbegindoy $thisenddoy $sensor $envpath
    
done    

echo "$0: All make output written to: $outfile"	    
