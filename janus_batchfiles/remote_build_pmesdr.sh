#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#

# The following commands will be executed when this script is run.

ssh -n -o StrictHostKeyChecking=no janus-compile1 "set -e ; echo -e '\nBegin PMESDR system build...\n'; source /etc/profile.d/modules.sh; source /curc/tools/utils/switch_lmod.sh; source ${PMESDR_TOP_DIR}/src/prod/set_pmesdr_environment.sh ; source ${PMESDR_TOP_DIR}/janus_batchfiles/build_pmesdr.sh ;echo -e '\nEnd PMESDR system build.\n' "
# End of compile job shell script
#
# 
