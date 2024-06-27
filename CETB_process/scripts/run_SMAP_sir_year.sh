#!/bin/bash
#
# Arguments:
#  year : 4-digit year
#  src : sensor source (F08, F10, etc)
#  envpath : location of alpine_set_pmesdr_environment.sh script
#  suffix : resolution is either 25, 24 or 36
#
#SBATCH --qos normal
#SBATCH --job-name CETB_platform_sir
#SBATCH --partition=amilan
#SBATCH --time=03:59:00
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=7500
#SBATCH --ntasks=120
#SBATCH --account=ucb286_asc2
#SBATCH -o output/sir_lb-%j.out
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
    echo "  ENVPATH: path to alpine_set_pmesdr_environment.sh script" 1>&2
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

[[ "$#" -lt 3 ]] || error_exit "Line $LINENO: Unexpected number of arguments."
year=$1
src=$2
envpath=$3
top_level=$4

suffix=""
if [[ "${base_resolution}" == "1" ]]
then
    suffix="_36"
fi
if [[ "${base_resolution}" == "2" ]]
then
    suffix="_24"
fi

file=/scratch/alpine/${USER}/${top_level}/${src}_scripts/${src}_sir_list_${year}${suffix}
echo "file = ${file}"

module purge
# Load the Load Balancer module *first*
module load loadbalance/0.2

# Now load any other software modules you need:
source ${envpath}/alpine_set_pmesdr_environment.sh

ml
date
#/curc/sw/install/openmpi/4.1.1/gcc/11.2.0/bin/mpirun lb ${file}
$CURC_LB_BIN/mpirun lb ${file}
date
