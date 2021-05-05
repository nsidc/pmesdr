#!/bin/bash
#
# script to run NRT daily Step2:
#   this is a cleanup script that moves the sir files to the peta library
#   and deletes the setup files
#

#SBATCH --qos normal
#SBATCH --job-name runNRTdailyStep2
#SBATCH --account=ucb135_summit2
#SBATCH --time=00:50:00
#SBATCH --ntasks-per-node=6
#SBATCH --nodes=1
#SBATCH -o /scratch/summit/moha2290/NRTdaily_output/runNRTdailyStep2-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=FAIL,REQUEUE,STAGE_OUT
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
    echo "" 1>&2
}

list_of_emails="molly\\.hardman\\@colorado\\.edu jessica\\.calme\\@colorado\\.edu"

PROGNAME=$(basename $0)

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" 1>&2
    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" | \
	mailx -s "NRT Step2 error" \
	      -r "molly\.hardman\@colorado\.edu" ${list_of_emails}
    exit 1
}

top_level=""
while getopts "t:h" opt; do
    case $opt in
	t) top_level=$OPTARG;;
	h) usage
	   exit 1;;
	?) printf "Usage: %s: [-t] args\n" $0
           exit 1;;
	esac
done

date

shift $(($OPTIND - 1))
[[ "$#" -eq 1 ]] || error_exit "Line $LINENO: Unexpected number of arguments."
src=$1
suffix=LRM
sat_top=${src}_${suffix}
# Note that SMAP doesn't use a top_level directory
pl_top=nsidc0738_v2
case $top_level in
    nsidc0751_CSU_ICDR)
	suffix=SSMIS
	sat_top=${src}_${suffix}
	pl_top=nsidc0757_v1;;
    nsidc0752_CSU_GPM_XCAL_NRT_Tbs)
	suffix=SSMIS
	sat_top=${src}_${suffix}
	pl_top=nsidc0763_v1;;
esac

direc=/scratch/summit/${USER}/${top_level}/
SETUPDIR=${direc}/${src}_setup/
SCRIPTDIR=${direc}/${src}_scripts/

outfile=${SCRIPTDIR}/${src}_moving_files
outfile_rm=${SCRIPTDIR}/${src}_rm_sir_files
if [[ -f ${outfile} ]]; then
    rm ${outfile}
    echo "removed old move file for ${src}"
else
    echo " no old move file to remove for ${src}"
fi

if [[ -f ${outfile_rm} ]]; then
    rm ${outfile_rm}
    echo "removed old delete *.nc file for ${src}"
fi

for file in `find ${direc}/${src}_sir/*.nc`
do
    basen=`basename $file`
    year=`echo $basen | grep -o ${src}_${suffix}-.... | sed 's/^.*-//'`
    hemi=`echo $basen | grep -o EASE2_. | sed 's/^.*_//'`
    echo "cp $file /pl/active/PMESDR/${pl_top}/${sat_top}/${hemi}/EASE2_${hemi}/${year}/" >> ${outfile}
    echo "rm $file" >> ${outfile_rm}
done

setup_rm_file=${SCRIPTDIR}/${src}_setup_rm
if [[ -f ${setup_rm_file} ]]; then
    rm ${setup_rm_file}
    echo "removed old rm setup file for ${src} ${top_level}"
fi
for file in `find ${SETUPDIR}/*.setup`
do
    echo "rm $file" >> ${setup_rm_file}
done

ml intel
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:ofi lb $outfile || \
    error_exit "Line $LINENO: mpirun cp *.nc files"
mpirun -genv I_MPI_FABRICS=shm:ofi lb $setup_rm_file || \
    error_exti "Line $LINENO: mpirun remove setup and scratch output files"
mpirun -genv I_MPI_FABRICS=shm:ofi lb $outfile_rm || \
    error_exit "Line $LINENO: mpirun rm *.nc files"

echo "${PROGNAME}: Step2 for ${pl_top} {src} completed" | \
	mailx -s "NRT Step2 Completed" \
	      -r "molly\.hardman\@colorado\.edu" ${list_of_emails}
date






