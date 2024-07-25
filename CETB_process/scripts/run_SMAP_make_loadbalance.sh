#!/bin/bash
#
# Arguments:
#  src : sensor source (F08, F10, etc)
#  envpath : location of summit_set_pmesdr_environment.sh script
#  suffix : resolution is either 25, 24 or 36
#
#SBATCH --qos normal
#SBATCH --job-name CETB_make
#SBATCH --partition=amilan
#SBATCH --time=00:15:00
#SBATCH --ntasks=24
#SBATCH --cpus-per-task=1
#SBATCH --account=ucb286_asc2
#SBATCH -o output/make_lb-%j.out
#SBATCH --constraint=ib
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
OPTIND=1
usage() {
    echo "" 1>&2
    echo "Usage: `basename $0`[-r resolution] [-h] ENVPATH" 1>&2
    echo "  Creates an sbatch script to run meas_meta_setup for 1 year of data" 1>&2
    echo "Arguments:" 1>&2
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

[[ "$#" -lt 1 ]] || error_exit "Line $LINENO: Unexpected number of arguments."
envpath=$1
top_level=$2

suffix=""
if [[ "${base_resolution}" == "1" ]]
then
    suffix="_36"
fi
if [[ "${base_resolution}" == "2" ]]
then
    suffix="_24"
fi


module purge 
# Now load any other software modules you need:
source ${envpath}/set_pmesdr_environment.sh

file=${PMESDR_SCRATCH_DIR}/${top_level}/SMAP_scripts/SMAP_make_list${suffix}
echo "file=${file}"
echo "suffix, src, path, ${suffix}, ${src} ${envpath} ${top_level}"

# Load the Load Balancer module *first*
module load loadbalance/0.2

ml
date

$CURC_LB_BIN/mpirun lb ${file}
date


