#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.

condaenv=$1

ssh -n -o StrictHostKeyChecking=no scompile "set -e ; echo -e '\nBegin PMESDR system build...\n'; source ${PMESDR_TOP_DIR}/src/prod/summit_set_pmesdr_environment.sh ; source ${PMESDR_TOP_DIR}/summit_batchfiles/build_pmesdr.sh ${condaenv} ; echo -e '\nEnd PMESDR system build.\n' "
# End of compile job shell script
#
# 
