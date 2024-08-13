#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.
#
# Re-builds a fresh copy of the system at PMESDR_TOP_DIR, and
# run unit tests on the python utilities needed for regression testing.
#
# This script requires the name of a conda env to use to run the unit tests for CETB
# comparison utilities.

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "build_pmesdr.sh: ERROR: ${1:-"Unknown Error"}" 1>&2
    return -1
}

condaenv=$1

cd ${PMESDR_TOP_DIR}/src/prod
make clean
make all
make install
#
cd ${PMESDR_TOP_DIR}/python
source activate $condaenv
nosetests test_cetb_utilities.py || error_exit "unit tests failed"

# End of example job shell script
# 
