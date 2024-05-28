#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.

condaenv=$1

salloc --nodes=1 --partition=acompile --ntasks=1  --time=01:00:00  --qos=compile --job-name=acompile --oversubscribe srun /bin/bash
ssh -n -o StrictHostKeyChecking=no $SLURM_NODELIST "set -e ; echo -e '\nBegin PMESDR system build...\n'; source ${PMESDR_TOP_DIR}/src/prod/single_set_pmesdr_environment.sh ; source ${PMESDR_TOP_DIR}/alpine_batchfiles/build_pmesdr.sh ${condaenv} ; echo -e '\nEnd PMESDR system build.\n' "
# End of compile job shell script
#
# 
