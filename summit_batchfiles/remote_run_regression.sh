#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.
regressiontype=$1  # quick or daily
condaenv=$2

ssh -n -o StrictHostKeyChecking=no scompile "set -e ; echo -e '\nBegin remote PMESDR ${regressiontype} regression...\n'; source ${PMESDR_TOP_DIR}/src/prod/summit_set_pmesdr_environment.sh ; cd ${PMESDR_TOP_DIR}/summit_batchfiles ; sbatch run_regression.sh ${regressiontype} ${condaenv} ; echo -e '\nEnd remote PMESDR ${regressiontype} regression.\n' "
# End of compile job shell script
#
# 
