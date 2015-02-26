#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#

# The following commands will be executed when this script is run.


cd ${PMESDR_TOP_DIR}/src/prod
git pull
make clean
make all
make install
#
source ${PMESDR_TOP_DIR}/src/prod/set_pmesdr_environment.sh -a
cd ${PMESDR_TOP_DIR}/python
nosetests test_cetb_utilities.py
# End of example job shell script
# 
