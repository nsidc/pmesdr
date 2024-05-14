#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.

condaenv=$1

acompile
ssh -n -o StrictHostKeyChecking=no $SLURM_NODELIST "set -e ; echo -e '\nBegin PMESDR system build...\n'; source ${PMESDR_TOP_DIR}/src/prod/single_set_pmesdr_environment.sh ; source ${PMESDR_TOP_DIR}/alpine_batchfiles/build_pmesdr.sh ${condaenv} ; echo -e '\nEnd PMESDR system build.\n' "
# End of compile job shell script
#
# 
