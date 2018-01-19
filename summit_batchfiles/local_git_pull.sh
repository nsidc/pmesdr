#!/bin/bash -l

echo -e '\nFetching latest software changes to PMESDR_regression...\n'
hostname
source /projects/$USER/PMESDR_regression/src/prod/summit_set_pmesdr_environment.sh
cd $PMESDR_TOP_DIR
git pull
git status

