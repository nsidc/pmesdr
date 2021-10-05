#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -lt 6 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR_START DOY_START YEAR_END DOY_END SRC ENVPATH"
    echo "  Makes a list of meas_meta_make commands"
    echo "  this script is a wrapper for the yearly meas_meta_make list"
    echo "  the script calls the yearly script for each year in the period"
    echo "Arguments:"
    echo "  YEAR_START: start 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  YEAR_END: end 4-digit year"
    echo "  DOY_END: end day of year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script"
    echo "  top_level: path for top_level in scratch used in NRT processing"
    echo ""
    exit 1
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
outfile=/scratch/summit/${USER}/${top_level}/${sensor}_scripts/${sensor}_make_list
rm -f ${outfile}
echo "$0: removing make file: $outfile"

# Call SSMIS_make.sh for each year to process
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
    
    echo "$0: SSMIS_make for: $year $thisbegindoy $thisenddoy $sensor $envpath $top_level"
    source $PMESDR_RUN/SSMIS_make.sh $year $thisbegindoy $thisenddoy $sensor $envpath $top_level
    echo "$? exit from SSMIS-make.sh called from all_SSMIS-make"
    
done    

echo "$0: All make output written to: $outfile"	    
