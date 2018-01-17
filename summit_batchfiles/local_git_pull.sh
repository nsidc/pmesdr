#!/bin/bash -l

echo -e '\nBegin PMESDR system build...\n'
hostname
source /projects/$USER/PMESDR_quick_regression_test/src/prod/summit_set_pmesdr_environment.sh
cd $PMESDR_TOP_DIR
git pull

