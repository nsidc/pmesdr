#!/usr/bin/env sh
# set_pmesdr_environment.sh
#
#  This script is used by sourcing it ". /this/script/location/set_pmesdr_environment.sh"
#  By doing this, you set up the environment necessary for building and running the
#  Passive Microwave ESDR system software.
#
# 2014-04-15 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2014 Regents of University of Colorado and Brigham-Young University
#========================================================================

# Grab the full path to this script, regardless of where it is called from.
export PMESDR_SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# The Project's Top Level Directory is two directories up from this file:
export PMESDR_TOP_DIR=$(dirname $(dirname "$PMESDR_SCRIPT_DIR"))

# PMESDR environment variables for running the system
export meas_home=$PMESDR_TOP_DIR
export SIR_areas=$PMESDR_TOP_DIR/ref/locs
export SIR_region=$PMESDR_TOP_DIR/ref/regiondef1.dat
export RSS_path=$PMESDR_TOP_DIR/ref

