#!/bin/bash
#
# script to run NRT daily Step2:
#   this is a cleanup script that moves the sir files to the peta library
#   and deletes the setup files
#

#SBATCH --qos normal
#SBATCH --job-name runNRTdailyStep2
#SBATCH --account=ucb135_summit2
#SBATCH --time=00:30:00
#SBATCH --ntasks-per-node=6
#SBATCH --nodes=1
#SBATCH -o /scratch/summit/moha2290/NRTdaily_output/runNRTdailyStep2-%j.out
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

case $top_level in
    nsidc0751_CSU_ICDR) pl_top=nsidc0757_v1;;
    nsidc0752_CSU_GPM_XCAL_NRT_Tbs) pl_top=nsidc0763_v1;;
esac

shift $(($OPTIND - 1))
[[ "$#" -eq 1 ]] || error_exit "Line $LINENO: Unexpected number of arguments."
src=$1

outfile=/scratch/summit/${USER}/${top_level}/${src}_scripts/${src}_moving_files
if [[ -f ${outfile} ]]; then
    rm ${outfile}
    echo "removed old move file for ${src}"
fi

for file in `find /scratch/summit/${USER}/${top_level}/${src}_sir/*.nc`
do
    basen=`basename $file`
    year=`echo $basen | grep -o ${src}_SSMIS-.... | sed 's/^.*-//'`
    hemi=`echo $basen | grep -o EASE2_. | sed 's/^.*_//'`
    echo "cp $file /pl/active/PMESDR/${pl_top}/${src}_SSMIS/${hemi}/EASE2_${hemi}/${year}/" >> ${outfile}
done

setup_rm_file=/scratch/summit/${USER}/${top_level}/${src}_scripts/${src}_setup_rm
if [[ -f ${setup_rm_file} ]]; then
    rm ${setup_rm_file}
    echo "removed old move file for ${src}"
fi
for file in `find /scratch/summit/${USER}/${top_level}/${src}_setup/*.setup`
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
date






