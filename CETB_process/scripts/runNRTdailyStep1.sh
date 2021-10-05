#!/bin/bash
#
# script to run NRT daily Step1:
#   this step runs after Step0 which runs the process through meas_meta_make
#   setup and sir run in this process
#   arguments are
#

#SBATCH --qos normal
#SBATCH --job-name runNRTdailyStep1
#SBATCH --account=ucb135_summit3
#SBATCH --time=04:30:00
#SBATCH --ntasks-per-node=6
#SBATCH --cpus-per-task=3
#SBATCH -o /scratch/summit/%u/NRTdaily_output/runNRTdailyStep1-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=FAIL,REQUEUE,STAGE_OUT
#SBATCH --mail-user=mhardman@nsidc.org

usage() {
    echo "" 1>&2
    echo "Usage: `basename $0` [-t] [-r] [-h] PLATFORM" 1>&2
    echo "  PLATFORM" 1>&2
    echo "Options: "  1>&2
    echo "  -t: top level data location under /scratch/summit/${USER}" 1>&2
    echo "  -r: optional for different base resolution, -r 1 is 36km and -r 2 is 24km" 1>&2
    echo "  -h: display help message and exit" 1>&2
    echo "  PLATFORM : F16, F17, F18 AMSRE, SMAP" 1>&2
    echo "Prior to running this script, do:" 1>&2
    echo "  run summit_set_pmesdr_environment.sh" 1>&2
    echo "" 1>&2
}

list_of_emails="molly\\.hardman\\@colorado\\.edu jessica\\.calme\\@colorado\\.edu"

isBatch=
if [[ ${BASH_SOURCE} == *"slurm_script"* ]]; then
    # Running as slurm
    echo "Running batch job..."
    PROGNAME=(`scontrol show job ${SLURM_JOB_ID} | grep Command | tr -s ' ' | cut -d = -f 2`)
    isBatch=1
else
    echo "Not running as sbatch..."
    PROGNAME=${BASH_SOURCE[0]}
fi

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" 1>&2
    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" | \
	mailx -s "NRT Step1 error jobid ${SLURM_JOB_ID}" \
	      -r "molly\.hardman\@colorado\.edu" ${list_of_emails}
    exit 1
}

top_level=""
arg_string=""
resolution=0
res_string=""

while getopts "r:t:h" opt; do
    case $opt in
	t) top_level=$OPTARG
	   arg_string="-t ${top_level}";;
	r) resolution=$OPTARG
	   res_string="-r ${resolution}";;
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

suffix=""
if [[ resolution -eq 1 ]]
then
    suffix="_36"
elif [[ resolution -eq 2 ]]
then
    suffix="_24"
fi

direc=/scratch/summit/${USER}/${top_level}/
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
MAKEDIR=${direc}/${src}_make${suffix}/
SETUPDIR=${direc}/${src}_setup${suffix}/
SIRDIR=${direc}/${src}_sir${suffix}/
SCRIPTDIR=${direc}/${src}_scripts/

# create the list of the latest make files
date
if [[ -f ${SCRIPTDIR}/${src}_setup_list${suffix} ]]; then
    rm ${SCRIPTDIR}/${src}_setup_list${suffix}
    echo "removed old setup file for ${src} ${res_string}"
fi

for FILE in `find ${MAKEDIR}/* -mtime 0`
do
    echo "$BINDIR/meas_meta_setup $FILE ${SETUPDIR}" >> ${SCRIPTDIR}/${src}_setup_list${suffix}
done

ml intel
ml netcdf/4.4.1.1
ml udunits/2.2.25
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:ofi lb ${SCRIPTDIR}/${src}_setup_list${suffix} || \
    error_exit "Line $LINENO: mpirun setup ${src} error."

# now create list of newly created setup files to feed to rSIR processing
date
if [[ -f ${SCRIPTDIR}/${src}_sir_list${suffix} ]]; then
    rm ${SCRIPTDIR}/${src}_sir_list${suffix}
    echo "removed old sir file for ${src} ${res_string}"
fi
for FILE in `find ${SETUPDIR}/* -mtime 0`
do
    echo "$BINDIR/meas_meta_sir $FILE ${SIRDIR}" >> ${SCRIPTDIR}/${src}_sir_list${suffix}
done
mpirun -genv I_MPI_FABRICS=shm:ofi lb ${SCRIPTDIR}/${src}_sir_list${suffix} || \
    error_exit "Line $LINENO: mpirun sir ${src} error."

#set off step 2 which copies files to the peta library and deletes the setup files
echo "sbatch --account=$SLURM_JOB_ACCOUNT --dependency=afterok:$SLURM_JOB_ID ${PMESDR_RUN}/runNRTdailyStep2.sh ${res_string} ${arg_string} ${src}"
sbatch --account=$SLURM_JOB_ACCOUNT --dependency=afterok:$SLURM_JOB_ID ${PMESDR_RUN}/runNRTdailyStep2.sh ${res_string} ${arg_string} ${src}
