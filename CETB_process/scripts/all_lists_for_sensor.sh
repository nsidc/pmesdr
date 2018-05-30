#!/bin/sh
if [ "$1" == "-h" ] || [ "$#" -ne 5 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] YEAR_START DOY_START YEAR_END DOY_END SRC"
    echo "  Makes daily and concatenated 3-day file lists of GSX input files for the "
    echo "  requested sensor SRC"
    echo "Arguments:"
    echo "  YEAR_START: start 4-digit year"
    echo "  DOY_START: start day of year"
    echo "  YEAR_END: end 4-digit year"
    echo "  DOY_END: end day of year"
    echo "  SRC: input sensor source of data: F08, F10, etc"
    echo ""
    exit 1
fi

startyear=$1
startdoy=$2
endyear=$3
enddoy=$4
sensor=$5

# Do all the daily lists first, then do the 3-day lists, which just concatenate
# them in rolling groups of 3 days
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
    
    echo "$0: 1day set: $year $thisbegindoy $thisenddoy $sensor"
    source $PMESDR_RUN/daily_file_lists.sh $year $thisbegindoy $thisenddoy $sensor
    
done    

# Now do 3-day lists
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
    
    echo "$0: 3day set: $year $thisbegindoy $thisenddoy $sensor"
    source $PMESDR_RUN/file_3day.sh $year $thisbegindoy $thisenddoy $sensor
    
done    
	    
