#!/bin/bash -l
# set_pmesdr_environment.sh
#
# usage: set_pmesdr_environment.sh
#
#  This script is used by sourcing it
#
#  ". /this/script/location/set_pmesdr_environment.sh"
#
#  This script sets up the environment to use the icc compiler
#  By doing this, you set up the environment necessary for building and running the
#  Passive Microwave ESDR system software.
#
#  When this script is finished, the bash environment variables
#  that are used by our Makefiles and system processing will be set.
#  When the hostname seems to be a CU supercomputer node, the correct set
#  of expected slurm modules will be loaded.
#
# 2014-04-15 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2014 Regents of University of Colorado and Brigham-Young University
#========================================================================

# Make no assumptions about starting LOCALE
export LOCALE=

# Grab the full path to this script, regardless of where it is called from.
export PMESDR_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# The Project's Top Level Directory is two directories up from this file:
export PMESDR_TOP_DIR=$(dirname $(dirname "$PMESDR_SCRIPT_DIR"))
echo "PMESDR_TOP_DIR is" $PMESDR_TOP_DIR

# PMESDR environment variables for running the system
export meas_home=$PMESDR_TOP_DIR

# Regression data is stored by the date it was created/modified
regression_yyyymmdd=20230125
export PMESDR_REGRESS_DIR=$PMESDR_TOP_DIR/../pmesdr_regression_data/${regression_yyyymmdd}
export PMESDR_TESTDATA_DIR=$PMESDR_TOP_DIR/sample_data/test_gsx
export PMESDR_CONDAENV=pmesdrEnv

# Determine the LOCALE and set compiler and more environment variables
thisHostname=`hostname -f`

echo "Setting environment..."
if [[ "$thisHostname"  == *"int.nsidc.org"* ]]; then

  export PMESDR_COMPILER=gcc
  export LOCALE=int.nsidc.org
  export PATH=/opt/anaconda/bin:$PATH
  export PMESDR_COMPARE_TOLERANCE=0.01
  export PMESDR_MAX_DIFF_PIXELS=100
  export PMESDR_SCRATCH_DIR=/projects/PMESDR/vagrant/$(shell hostname)  
  export PMESDR_TEST_OUT_DIR=/projects/PMESDR/vagrant/NSIDCtest/$(shell hostname)  

elif [[ "$thisHostname" == *"rc.colorado.edu" \
	  || "$thisHostname" == *"c3"* ]]; then

  export PMESDR_COMPILER=icc
  export LOCALE=ALPINEicc
  export PMESDR_COMPARE_TOLERANCE=0.06
  export PMESDR_MAX_DIFF_PIXELS=100
  export PMESDR_SCRATCH_DIR=/scratch/alpine/${USER}
  export PMESDR_TEST_OUT_DIR=${PMESDR_TOP_DIR}/NSIDCtest

  module purge all
  ml intel/2022.1.2
  ml netcdf/4.8.1
  ml udunits/2.2.25
  ml git-lfs/3.1.2
  module list

else

  export PMESDR_COMPILER=gcc
  export LOCALE=default
  export PMESDR_COMPARE_TOLERANCE=0.06
  export PMESDR_MAX_DIFF_PIXELS=100
  export PMESDR_SCRATCH_DIR=/home/${USER}
  export PMESDR_TEST_OUT_DIR=/home/${USER}/NSIDCtest
  
fi                                                                                                                   

if [[ -z $LOCALE ]]; then
  echo "Unrecognized locale, please define a new block with this locale's environment"
else
  echo "Hostname: $thisHostname"
  echo "Compiler: $PMESDR_COMPILER"
  echo "Conda env: $PMESDR_CONDAENV"
  echo "Regression data: $PMESDR_REGRESS_DIR"
  echo "Scratch dir: $PMESDR_SCRATCH_DIR"
  echo "It is expected that you will use conda environments for specific versions of python"
  echo "PMESDR system LOCALE=$LOCALE, ready to use the PMESDR system."
fi
