#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.
#
# This script requires the name of a conda env to use to run the unit tests for CETB
# comparison utilities.

condaenv=$1

cd ${PMESDR_TOP_DIR}/src/prod
make clean
make all
make install
#
cd ${PMESDR_TOP_DIR}/python
source activate $condaenv
nosetests test_cetb_utilities.py
# End of example job shell script
# 
