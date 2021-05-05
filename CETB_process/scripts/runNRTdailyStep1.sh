#!/bin/bash
#
# script to run NRT daily Step1:
#   this step runs after Step0 which runs the process through meas_meta_make
#   setup and sir run in this process
#   arguments are
#

#SBATCH --qos normal
#SBATCH --job-name runNRTdailyStep1
#SBATCH --account=ucb135_summit2
#SBATCH --time=05:30:00
#SBATCH --ntasks-per-node=6
#SBATCH --cpus-per-task=2
#SBATCH -o /scratch/summit/moha2290/NRTdaily_output/runNRTdailyStep1-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=END,FAIL,REQUEUE,STAGE_OUT
#SBATCH --mail-user=mhardman@nsidc.org

usage() {
    echo "" 1>&2
    echo "Usage: `basename $0` [-t] [-h] PLATFORM" 1>&2
    echo "  PLATFORM" 1>&2
    echo "Options: "  1>&2
    echo "  -t: top level data location under /scratch/summit/${USER}" 1>&2
    echo "  -h: display help message and exit" 1>&2
    echo "  PLATFORM : F16, F17, F18 AMSRE, SMAP" 1>&2
    echo "Prior to running this script, do:" 1>&2
    echo "  run summit_set_pmesdr_environment.sh" 1>&2
    echo "" 1>&2
}

PROGNAME=$(basename $0)

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" 1>&2
    exit 1
}

top_level=""
arg_string=""
while getopts "t:h" opt; do
    case $opt in
	t) top_level=$OPTARG
	   arg_string="-t ${top_level}";;
	h) usage
	   exit 1;;
	?) printf "Usage: %s: [-tf] args\n" $0
           exit 1;;
	esac
done

if [[ -z ${PMESDR_TOP_DIR} ]];
then
    error_exit "$LINENO: First set the PMESDR environment run summit_set_pmesdr_environment.sh"
fi
    
date

shift $(($OPTIND - 1))
[[ "$#" -eq 1 ]] || error_exit "Line $LINENO: Unexpected number of arguments."
src=$1

module purge
date

direc=/scratch/summit/${USER}/${top_level}/
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
MAKEDIR=${direc}/${src}_make/
SETUPDIR=${direc}/${src}_setup/
SCRIPTDIR=${direc}/${src}_scripts/

# create the list of the latest make files
date
if [[ -f ${SCRIPTDIR}/${src}_setup_list ]]; then
    rm ${SCRIPTDIR}/${src}_setup_list
    echo "removed old setup file for ${src}"
fi

for FILE in `find ${MAKEDIR}/* -mtime 0`
do
    echo "$BINDIR/meas_meta_setup $FILE ${SETUPDIR}" >> ${SCRIPTDIR}/${src}_setup_list
done

ml intel
ml netcdf/4.4.1.1
ml udunits
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:ofi lb ${SCRIPTDIR}/${src}_setup_list || \
    error_exit "Line $LINENO: mpirun setup ${src} error."

# now create list of newly created setup files to feed to rSIR processing
date
if [[ -f ${SCRIPTDIR}/${src}_sir_list ]]; then
    rm ${SCRIPTDIR}/${src}_sir_list
    echo "removed old sir file for ${src}"
fi
for FILE in `find ${SETUPDIR}/* -mtime 0`
do
    echo "$BINDIR/meas_meta_sir $FILE ${direc}/${src}_sir" >> ${SCRIPTDIR}/${src}_sir_list
done
mpirun -genv I_MPI_FABRICS=shm:ofi lb ${SCRIPTDIR}/${src}_sir_list || \
    error_exit "Line $LINENO: mpirun sir ${src} error."

# set off step 2 which copies files to the peta library and deletes the setup files
echo "sbatch --dependency=afterok:$SLURM_JOB_ID ${PMESDR_RUN}/runNRTdailyStep2.sh ${arg_string} ${src}"
sbatch --dependency=afterok:$SLURM_JOB_ID ${PMESDR_RUN}/runNRTdailyStep2.sh ${arg_string} ${src}




    


