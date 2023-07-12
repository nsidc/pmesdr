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

regression_yyyymmdd=20230125
export PMESDR_REGRESS_DIR=$PMESDR_TOP_DIR/../pmesdr_regression_data/${regression_yyyymmdd}

# Determine the LOCALE, a function of host and compiler.
thisHostname=`hostname --fqdn`
echo "hey $thisHostname"
if [[ "$thisHostname"  == *"int.nsidc.org"* ]]; then

  compiler=gcc
  export PMESDR_COMPILER=$compiler
  export LOCALE=int.nsidc.org
  export PATH=/opt/anaconda/bin:$PATH
  export PMESDR_COMPARE_TOLERANCE=0.01
  export PMESDR_MAX_DIFF_PIXELS=100
  export PMESDR_REGRESS_DIR=/projects/PMESDR/pmesdr_regression_data/${regression_yyyymmdd}
  export PMESDR_TESTDATA_DIR=$PMESDR_TOP_DIR/sample_data/test_gsx

fi # endif hostname

if [[ "$thisHostname" == *"rc.colorado.edu" \
	  || "$thisHostname" == *"c3"* ]]; then

  echo "Setting environment..."
  export PMESDR_RUN=${PMESDR_TOP_DIR}/CETB_process/scripts
  export PMESDR_COMPARE_TOLERANCE=0.06
  export PMESDR_MAX_DIFF_PIXELS=100
  export PMESDR_REGRESS_DIR=$PMESDR_TOP_DIR/../pmesdr_regression_data/${regression_yyyymmdd}

  echo "Setting netcdf for the icc compiler on Alpine"
  module purge all
  ml intel/2022.1.2
  ml netcdf/4.8.1
  ml udunits/2.2.25
  export LOCALE=ALPINEicc
  echo "Compiler set to $PMESDR_COMPILER"
      module list
fi

if [[ "$thisHostname" == "pmesdr-nrt" ]]; then                                                                       
  echo "Setting netcdf for the icc compiler on Cumulus"                                                            
  source /opt/intel/oneapi/setvars.sh                                                                              
  export LOCALE=CUMULUSicc                                                                                         
  echo "Compiler set to $PMESDR_COMPILER"                                                                          
  echo "Setting environment..."                                                                                        
  export PMESDR_RUN=${PMESDR_TOP_DIR}/CETB_process/scripts                                                             
  export PMESDR_COMPARE_TOLERANCE=0.06                                                                                 
  export PMESDR_MAX_DIFF_PIXELS=100                                                                                    
  export PMESDR_REGRESS_DIR=${PMESDR_TOP_DIR}/../pmesdr_regression_data/${regression_yyyymmdd}                                 
                                                                                                                       
fi                                                                                                                   

echo "It is expected that you will use conda environments for specific versions of python"

echo "PMESDR system LOCALE=$LOCALE, ready to use the PMESDR system."
