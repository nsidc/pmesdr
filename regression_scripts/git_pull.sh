#!/bin/bash -l
# If any simple step or pipeline fails, this script will fail
set -eo pipefail

PROGNAME=$(basename $0)

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"
    # Assumes user has done set_pmesdr_environment.sh

    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" 1>&2
    exit -1
}

printf "%s: On %s, fetching latest software changes...\n" ${PROGNAME} $(hostname)

cd $PMESDR_TOP_DIR || error_exit "$LINENO: Error changing to PMESDR_TOP_DIR."
git pull || error_exit "$LINENO: Error on git pull."
git status

printf "%s: Done\n" ${PROGNAME}

