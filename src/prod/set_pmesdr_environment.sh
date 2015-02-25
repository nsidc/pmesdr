#!/usr/bin/env sh
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

# Parse command line
compiler=gcc
do_anaconda=0
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

# Grab the full path to this script, regardless of where it is called from.
export PMESDR_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# The Project's Top Level Directory is two directories up from this file:
export PMESDR_TOP_DIR=$(dirname $(dirname "$PMESDR_SCRIPT_DIR"))

# PMESDR environment variables for running the system
export meas_home=$PMESDR_TOP_DIR
export SIR_areas=$PMESDR_TOP_DIR/ref/locs
export SIR_region=$PMESDR_TOP_DIR/ref/regiondef1.dat
export RSS_path=$PMESDR_TOP_DIR/ref
export PMESDR_REGRESS_DIR=$PMESDR_TOP_DIR/../pmesdr_regression_data/20150209

# netCDF libraries are different, depending on compiler
icc_netcdf=netcdf/netcdf4-4.3_hdf5-1.8.11_hdf4-4.2.9_szip-2.1_zlib-1.2.78_jpeglib-8d_intel-13.0.0
gcc_netcdf=netcdf/netcdf4-4.3.2_hdf5-1.8.13_hdf4-4.2.10_szip-2.1_zlib-1.2.8_jpeglib-9a_openmpi-1.8.2_gcc-4.9.1

# Determine the LOCALE, a function of host and compiler.
# Janus needs to load compiler-specific modules before building
if [[ "$HOSTNAME" == *[Jj]"anus"* || "$HOSTNAME" == *"rc.colorado.edu" ]]; then
    
  module load slurm
  
  if [[ "$compiler" == "gcc" ]]; then
    echo "Setting netcdf for the gcc compiler"
    module unload $icc_netcdf
    module load $gcc_netcdf
    export LOCALE=JANUSgcc
    if [[ $do_anaconda == 1 ]]; then
	module load python/anaconda-2.1.0
	export PATH=~/.conda/envs/pmesdr/bin:$PATH
    else
	module unload python/anaconda-2.1.0
    fi
  fi
  
  if [[ "$compiler" == "icc" ]]; then
    echo "Setting netcdf for the icc compiler"
    module load python/anaconda-2.1.0
    module unload $gcc_netcdf
    module load $icc_netcdf
    export LOCALE=JANUSicc
    export PATH=~/.conda/envs/pmesdr/bin:$PATH
  fi
  
  module list
  
elif [[ "$HOSTNAME" == "snow"* ]]; then
    export LOCALE=NSIDCsnow
    # Initialize the virtualenv that was built for running on snow
    . ~brodzik/.virtual_envs_snow/pmesdr/bin/activate
elif [[ "$HOSTNAME" == "brodzik" ]]; then
    export LOCALE=NSIDCdev
    # Initialize the virtualenv that was built for running on snow
    . ~brodzik/.virtual_envs/pmesdr/bin/activate
else
    export LOCALE=BYU
fi # endif janus

echo "PMESDR system LOCALE=$LOCALE, ready to make the system."
