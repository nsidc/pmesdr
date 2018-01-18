#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.

condaenv=$1

ssh -n -o StrictHostKeyChecking=no scompile "set -e ; echo -e '\nBegin remote PMESDR quick regression...\n'; source ${PMESDR_TOP_DIR}/src/prod/summit_set_pmesdr_environment.sh ; cd ${PMESDR_TOP_DIR}/summit_batchfiles ; sbatch quick_regression.sh ${condaenv} ; echo -e '\nEnd remote PMESDR quick regression.\n' "
# End of compile job shell script
#
# 
