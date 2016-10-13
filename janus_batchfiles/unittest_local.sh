#!/bin/bash

# Set up the environment.

repo=$(readlink -f $(dirname $(readlink -f $0))/..)
source $repo/src/prod/set_pmesdr_environment.sh -a -c gcc
module load ruby/2.2.3
set -ex
export C_INCLUDE_PATH=$CURC_NETCDF_INC:$CURC_UDUNITS_INC:$repo/src/prod/utils/src:$repo/include
export LIBRARY_PATH=$CURC_HDF5_LIB:$CURC_NETCDF_LIB:$CURC_UDUNITS_LIB

# Build.

cd $repo/src/prod
make clean
make all
make install

# Python tests.

cd $repo/python
nosetests test_cetb_utilities.py

# Ceedling tests.

export GEM_HOME=$HOME/.gems
gem install ceedling -v 0.18.0
cd $repo/src/prod/cetb_file
rake test:all
cd $repo/src/prod/gsx
rake test:all
