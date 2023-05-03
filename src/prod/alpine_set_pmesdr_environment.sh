#!/bin/bash -l
# set_pmesdr_environment.sh
#
# usage: summit_set_pmesdr_environment.sh or set_pmesdr_environment.sh
#

#  This script is used by sourcing it
#
#  ". /this/script/location/summit_set_pmesdr_environment.sh"
#
#  This script sets up the summit environment to use the icc compiler
#  By doing this, you set up the environment necessary for building and running the
#  Passive Microwave ESDR system software.
#  When this script is finished, the bash environment variables
#  that are used by our Makefiles and system processing will be set, and the correct set
#  of expected summit/alpine modules will be loaded.
#
# 2014-04-15 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2014 Regents of University of Colorado and Brigham-Young University
#========================================================================
#
# First check for compiler environment variable - cmd line args will override
#
compiler=icc
export PMESDR_COMPILER=$compiler

# Grab the full path to this script, regardless of where it is called from.
export PMESDR_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# The Project's Top Level Directory is two directories up from this file:
export PMESDR_TOP_DIR=$(dirname $(dirname "$PMESDR_SCRIPT_DIR"))
echo "PMESDR_TOP_DIR is" $PMESDR_TOP_DIR
# PMESDR environment variables for running the system
export meas_home=$PMESDR_TOP_DIR
export SIR_areas=$PMESDR_TOP_DIR/ref/locs
export SIR_region=$PMESDR_TOP_DIR/ref/regiondef1.dat
export RSS_path=$PMESDR_TOP_DIR/ref
regression_yyyymmdd=20180920
export PMESDR_REGRESS_DIR=$PMESDR_TOP_DIR/../pmesdr_regression_data/${regression_yyyymmdd}

# Determine the LOCALE, a function of host and compiler.
thisHostname=`hostname --fqdn`
if [[ "$thisHostname"  == *"int.nsidc.org"* ]]; then

  export LOCALE=int.nsidc.org
  export PATH=/opt/anaconda/bin:$PATH
  export PMESDR_COMPARE_TOLERANCE=0.01
  export PMESDR_MAX_DIFF_PIXELS=100
  export PMESDR_REGRESS_DIR=/projects/PMESDR/pmesdr_regression_data/${regression_yyyymmdd}
  export PMESDR_TESTDATA_DIR=$PMESDR_TOP_DIR/sample_data/test_gsx

fi # endif hostname

if [[ "$thisHostname" == *"shas"* \
	  || "$thisHostname" == *"rc.colorado.edu" \
	  || "$thisHostname" == *"c3"* \
	  || "$thisHostname" == "node"* ]]; then

  echo "Setting environment..."
  export PMESDR_RUN=${PMESDR_TOP_DIR}/CETB_process/scripts
  export PMESDR_COMPARE_TOLERANCE=0.06
  export PMESDR_MAX_DIFF_PIXELS=100
  export PMESDR_REGRESS_DIR=/projects/moha2290/pmesdr_regression_data/${regression_yyyymmdd}

  if [[ "$thisHostname" == *"shas"* \
	  || "$thisHostname" == "node"* ]]; then
      echo "Setting netcdf for the icc compiler on Summit"
      module purge all
      ml intel
      ml netcdf/4.4.1.1
      ml udunits/2.2.25
      export LOCALE=SUMMITicc
      echo "Compiler set to $PMESDR_COMPILER"
      module list
  fi

  if [[ "$thisHostname" == *"c3"* ]]; then
      echo "Setting netcdf for the icc compiler on Alpine"
#      module purge all
      ml intel/2022.1.2
      ml netcdf/4.8.1
      ml udunits/2.2.25
      export LOCALE=ALPINEicc
      echo "Compiler set to $PMESDR_COMPILER"
      module list
  fi

  echo "It is expected that you will use conda environments for specific versions of python"

fi

echo "PMESDR system LOCALE=$LOCALE, ready to use the PMESDR system."
