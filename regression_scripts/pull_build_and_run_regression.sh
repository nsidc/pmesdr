#!/bin/bash -l
#
# Re-builds a fresh copy of the system at PMESDR_TOP_DIR, runs unit tests and
# either quick or daily regression
#
# Assumes the user has set the bash environment with set_pmesdr_environment.sh
#

# If any simple step or pipeline fails, this script will fail
set -eo pipefail

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "pull_build_and_run_regression.sh: ERROR: ${1:-"Unknown Error"}" 1>&2
    return 1
}

REGRESSIONTYPE=$1

source ${PMESDR_TOP_DIR}/regression_scripts/git_pull.sh || \
    error_exit "failed to git pull from remote"
source ${PMESDR_TOP_DIR}/regression_scripts/build_pmesdr.sh ${PMESDR_CONDAENV} || \
    error_exit "failed to build system"
source ${PMESDR_TOP_DIR}/regression_scripts/run_regression.sh \
       ${REGRESSIONTYPE} ${PMESDR_CONDAENV} || \
    error_exit "failed to run regression"

