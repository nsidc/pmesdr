#!/bin/bash
#
# script to run NRT daily Step3:
#   this script runs after the daily processing of 0630 and copies the files to
#   the peta library for internal use
#

#SBATCH --qos normal
#SBATCH --job-name runNRTdailyStep2
#SBATCH --account=ucb135_summit3
#SBATCH --time=01:50:00
#SBATCH --ntasks-per-node=6
#SBATCH --nodes=1
#SBATCH -o /scratch/summit/%u/NRTdaily_output/runNRTdailyStep3-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=FAIL,REQUEUE,STAGE_OUT
#SBATCH --mail-user=mhardman@nsidc.org

OPTIND=1

usage() {
    echo "" 1>&2
    echo "Usage: `basename $0` [-t] [-r] [-h] PLATFORM" 1>&2
    echo "  PLATFORM" 1>&2
    echo "Options: "  1>&2
    echo "  -t: top level data location under /scratch/summit/${USER}" 1>&2
    echo "  -r: -r 0 is default base resolution (25km) -r 1 is 36km -r 2 is 24km" 1>&2
    echo "  -h: display help message and exit" 1>&2
    echo "  PLATFORM : F16, F17, F18 AMSRE, SMAP" 1>&2
    echo "Prior to running this script, do:" 1>&2
    echo "" 1>&2
}

list_of_emails="molly\\.hardman\\@colorado\\.edu"

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
	mailx -s "NRT Step2 error jobid ${SLURM_JOB_ID}" \
	      -r "molly\.hardman\@colorado\.edu" ${list_of_emails}
    exit 1
}

top_level=""
res_string=""
resolution=0
while getopts "r:t:h" opt; do
    case $opt in
	t) top_level=$OPTARG;;
	r) resolution=$OPTARG
	   res_string="resolution ${resolution}";;
	h) usage
	   exit 1;;
	?) printf "Usage: %s: [-t] args\n" $0
           exit 1;;
	esac
done

date

echo "argument is $1"
src=$1
for file in `find /scratch/summit/jeca4282/${src}_sir/*.nc -mtime 0`
do
    basen=`basename $file`
    year=`echo $basen | grep -o ${src}_${suffix}-.... | sed 's/^.*-//'`
    hemi=`echo $basen | grep -o EASE2_.*km`

    rsync -avz ${file} /pl/active/PMESDR/nsidc0630_v1/${src}_SSMIS/${hemi}/${year}/
done
