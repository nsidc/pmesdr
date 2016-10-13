#!/bin/bash

ceedling_version=0.18.0
ruby_version=2.2.3

# Set up the environment.

source /etc/profile.d/modules.sh
repo=$(readlink -f $(dirname $(readlink -f $0))/..)
source $repo/src/prod/set_pmesdr_environment.sh -a -c gcc
module load ruby/$ruby_version
set -ex
export C_INCLUDE_PATH=$CURC_NETCDF_INC:$CURC_UDUNITS_INC:$repo/src/prod/utils/src:$repo/include
export LIBRARY_PATH=$CURC_HDF5_LIB:$CURC_NETCDF_LIB:$CURC_UDUNITS_LIB

# Build.

cd $repo/src/prod
(make clean && make all && make install) >/dev/null

# Python tests.

cd $repo/python
nosetests test_cetb_utilities.py

# Ceedling tests.

export GEM_HOME=$HOME/.gems
gem list | grep -q "ceedling ($ceedling_version)" || gem install ceedling -v $ceedling_version
cd $repo/src/prod/cetb_file
rake test:all
cd $repo/src/prod/gsx
rake test:all
