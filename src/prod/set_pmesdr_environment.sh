#!/bin/bash -l
# set_pmesdr_environment.sh
#
# usage: set_pmesdr_environment.sh or set_pmesdr_environment.sh [-a][-c gcc]
#   -a : For janus gcc compiler only, load anaconda python (default is do not load)
#   -c gcc : Sets environment up for gcc compiler
#   -c icc : Sets environment up for icc compiler
#
#  This script is used by sourcing it ". /this/script/location/set_pmesdr_environment.sh"
#  By doing this, you set up the environment necessary for building and running the
#  Passive Microwave ESDR system software.
#  When this script is finished, the bash environment variables
#  that are used by our Makefiles and system processing will be set.
#
# 2014-04-15 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2014 Regents of University of Colorado and Brigham-Young University
#========================================================================
#
# First check for compiler environment variable - cmd line args will override
#
compiler=icc
do_anaconda=0

if [[ "$PMESDR_COMPILER" != "" ]]; then
    compiler="$PMESDR_COMPILER"
fi

#
# Parse command line
set -- $(getopt ac: "$@")
while [ $# -gt 0 ]
do
    case "$1" in
    (-c)
	    compiler="$2";;
    (-a)
	    do_anaconda=1;;
    (*) break;;
    esac
    shift
done
#
# test and then set the $PMESDR_COMPILER environment variable 
#
if [[ "$compiler" != "gcc" ]] && [[ "$compiler" != "icc" ]]; then
    echo "PMESDR_COMPILER must be 'icc' or 'gcc' - cannot be set to " $compiler
    exit -1
fi

export PMESDR_COMPILER=$compiler

# Grab the full path to this script, regardless of where it is called from.
export PMESDR_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# The Project's Top Level Directory is two directories up from this file:
export PMESDR_TOP_DIR=$(dirname $(dirname "$PMESDR_SCRIPT_DIR"))

# PMESDR environment variables for running the system
export meas_home=$PMESDR_TOP_DIR
export SIR_areas=$PMESDR_TOP_DIR/ref/locs
export SIR_region=$PMESDR_TOP_DIR/ref/regiondef1.dat
export RSS_path=$PMESDR_TOP_DIR/ref
regression_yyyymmdd=20230125
export PMESDR_REGRESS_DIR=$PMESDR_TOP_DIR/../pmesdr_regression_data/${regression_yyyymmdd}

# Determine the LOCALE, a function of host and compiler.
if [[ `hostname -d` =~ "int.nsidc.org" ]]; then

  export LOCALE=int.nsidc.org
  export PATH=/opt/anaconda/bin:$PATH
  export PMESDR_COMPARE_TOLERANCE=0.01
  export PMESDR_MAX_DIFF_PIXELS=100
  export PMESDR_REGRESS_DIR=/projects/PMESDR/pmesdr_regression_data/${regression_yyyymmdd}
  export PMESDR_TESTDATA_DIR=$PMESDR_TOP_DIR/sample_data/test_gsx

else

  export LOCALE=BYU

fi # endif hostname

echo "PMESDR system LOCALE=$LOCALE, ready to use the PMESDR system."
