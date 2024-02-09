#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#  suffix : resolution is either 25, 24 or 36
#
#SBATCH --qos normal
#SBATCH --job-name CETB_make
#SBATCH --partition=shas
#SBATCH --time=00:15:00
#SBATCH --ntasks=24
#SBATCH --cpus-per-task=1
#SBATCH --account=ucb286_asc2
#SBATCH -o output/make_lb-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
OPTIND=1
usage() {
    echo "" 1>&2
    echo "Usage: `basename $0`[-r resolution] [-h] YEAR SRC ENVPATH" 1>&2
    echo "  Creates an sbatch script to run meas_meta_setup for 1 year of data" 1>&2
    echo "Arguments:" 1>&2
    echo "  YEAR: 4-digit year" 1>&2
    echo "  SRC: input sensor source of data: F08, F10, etc" 1>&2
    echo "  ENVPATH: path to summit_set_pmesdr_environment.sh script" 1>&2
    echo "  -r 1 and -r 2 use 36 and 24 km base resolutions respectively" 1>&2
    echo "  -r 0 or nothing is the default 25 km base resolution" 1>&2
    echo "" 1>&2
}

while getopts "r:h" opt; do
    case $opt in
	r) base_resolution=$OPTARG;;
	h) usage
	   exit 1;;
	?) printf "Usage: %s: [-r] args\n" $0
           exit 1;;
	esac
done

shift $(($OPTIND - 1))

[[ "$#" -eq 3 ]] || error_exit "Line $LINENO: Unexpected number of arguments."
year=$1
src=$2
envpath=$3

suffix=""
if [[ "${base_resolution}" == "1" ]]
then
    suffix="_36"
fi
if [[ "${base_resolution}" == "2" ]]
then
    suffix="_24"
fi

file=/scratch/summit/${USER}/${src}_scripts/${src}_make_list${suffix}
echo "file=${file}"
echo "suffix, year, src, path, ${suffix}, ${year}, ${src} ${envpath}"
source ${envpath}/summit_set_pmesdr_environment.sh
ml impi
ml loadbalance
ml
date
mpirun -genv I_MPI_FABRICS=shm:tmi -genv I_MPI_TMI_PROVIDER=psm2 lb $file
date

