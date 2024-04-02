#!/bin/bash -l

PROGNAME=$(basename $0)

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" 1>&2
    exit -1
}

printf "%s: On %s, fetching latest software changes...\n" ${PROGNAME} $(hostname)

source /projects/$USER/PMESDR_regression/src/prod/single_set_pmesdr_environment.sh
cd $PMESDR_TOP_DIR || error_exit "$LINENO: Error changing to PMESDR_TOP_DIR."
git pull || error_exit "$LINENO: Error on git pull."
git status

printf "%s: Done\n" ${PROGNAME}

