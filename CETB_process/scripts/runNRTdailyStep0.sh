#!/bin/bash
#
# This script must be preceded by running summit_set_pmesdr_environment.sh
#   which is located in the {measures-byu-repo-dir}/src/prod
#
# script to run NRT daily Step0:
#   fetch latest data - input argument l1c or ICDR or SMAP
#   run gsx on input files
#   create file lists and run meas_meta_make
#   kick off the batch job to run setup and sir
#   schedul Step0 for tomorrow
#
#SBATCH --qos normal
#SBATCH --job-name runNRTdailyStep0
#SBATCH --account=ucb135_summit2
#SBATCH --time=00:30:00
#SBATCH --ntasks-per-node=24
#SBATCH --nodes=1
#SBATCH -o /scratch/summit/moha2290/NRTdaily_output/runNRTdailyStep0-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=END,FAIL,REQUEUE,STAGE_OUT
#SBATCH --mail-user=mhardman@nsidc.org

usage() {
    echo "" 1>&2
    echo "Usage: `basename $0` [-tf] [-h] GSX_TYPE CONDAENV" 1>&2
    echo "  GSX_TYPE CONDAENV" 1>&2
    echo "Options: "  1>&2
    echo "  -t: top level data location under /scratch/summit/${USER}" 1>&2
    echo "  -h: display help message and exit" 1>&2
    echo "  -f: do the ftp pull(default is no ftp pull)" 1>&2
    echo "  GSX_TYPE : type of gsx translation to do (see gsx --help)" 1>&2
    echo "           : also controls which platforms to process, eg F17 or AQUA etc" 1>&2
    echo "  CONDAENV : conda environment for gsx translation" 1>&2
    echo "Prior to running this script, do:" 1>&2
    echo " run summit_set_pmesdr_environment.sh " 1>&2
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
do_ftp=

while getopts "ft:h" opt; do
    case $opt in
	f) do_ftp=1;;
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
# Now set up sensor based on GSX type

echo "$PROGNAME: top_level = $top_level"
direc=/scratch/summit/${USER}/${top_level}
echo "$PROGNAME: scratch directory $direc"

shift $(($OPTIND - 1))

[[ "$#" -eq 2 ]] || error_exit "Line $LINENO: Unexpected number of arguments."

gsx_type=$1
condaenv=$2
source activate $condaenv
# start sbatch for the next day
sbatch  --begin=08:30:00 ${PMESDR_RUN}/runNRTdailyStep0.sh -f ${arg_string} ${gsx_type} ${condaenv}
ml purge
ml intel
ml impi
ml loadbalance
ml python/3.6.5
ml
date

thisHost=$(hostname)
thisDate=$(date)
echo "$PROGNAME: Begin on hostname=$thisHost on $thisDate"
echo "$PROGNAME: SLURM_SCRATCH=$SLURM_SCRATCH"
echo "$PROGNAME: SLURM_JOB_ID=$SLURM_JOB_ID"
echo "$PROGNAME: with gsx_type=$gsx_type and condaenv=$condaenv"


case $gsx_type in
    
    SSMIS-L1C)
    fetch_file="/projects/${USER}/swathfetcher/ftp_nrt_l1c.py"
    suffix="*.RT-H5"
    run_dir="/projects/${USER}/swathfetcher"
    make_file="all_SSMIS_make_for_sensor.sh"
    platforms="F16 F17 F18";;
    
    SSMIS-CSU-ICDR)
    fetch_file="/projects/${USER}/swathfetcher/ftp_nrt_csu.py"
    suffix="*.nc"
    run_dir="/projects/${USER}/swathfetcher"
    make_file="all_SSMIS_make_for_sensor.sh"
    platforms="F16 F17 F18";;
    
    SMAP)
    fetch_file="/projects/${USER}/swathfetcher/ecs_smap_get.py"
    suffix="*.h5"
    run_dir="${direc}/SMAP"
    make_file="all_SMAP_make.sh"
    platforms="SMAP";;

esac

cur_dir=${PWD}
echo "$PROGNAME: $suffix"
echo "$PROGNAME: $platforms"

# if -f is set download files from ftp
if [[ $do_ftp ]]; then
#Go here so that correct secret files are used or SMAP downloaded to correct location
    cd ${run_dir}
    python ${fetch_file} || error_exit "Line $LINENO: ftp error."
    echo "Done with ftp fetch from ${fetch_file}"
# Change back to original directory
    cd ${cur_dir}
fi

#after files are retrieved, create input file list for gsx of files with
# modification date less than 1 day old

date
for src in $platforms
do
    echo "working this src ${src}"
    if [[ -f ${direc}/${src}_scripts/gsx_lb_list_summit ]]; then
	rm ${direc}/${src}_scripts/gsx_lb_list_summit
	echo "removed old gsx_lb_file for ${src}"
    fi
    for file in `find ${direc}/${src} -name "${suffix}" -mtime 0`
    do
	basen=`basename ${file}`
	echo "gsx ${gsx_type} ${file} ${direc}/${src}_GSX/GSX_${basen}.nc" \
	     >> ${direc}/${src}_scripts/gsx_lb_list_summit
    done
    echo "mpirun -genv I_MPI_FABRICS=shm:ofi lb ${direc}/${src}_scripts/gsx_lb_list_summit"
    mpirun -genv I_MPI_FABRICS=shm:ofi lb ${direc}/${src}_scripts/gsx_lb_list_summit || \
	error_exit "Line $LINENO: mpirun gsx ${src} error."
done

date

#next sort the files into daily lists
#source /projects/${USER}/measures-byu/src/prod/summit_set_pmesdr_environment.sh
startyear=`date '+%Y' -d "7 days ago"`
startdoy=`date '+%j' -d "7 days ago"`
endyear=`date '+%Y'`
enddoy=`date '+%j'`

# Note the CSU ICDR data stream is 1-2 days behind today
# This workaround gets around a bug in meas_meta_setup that fails if there
# are no input files in the make file
if [[ ${gsx_type} -eq SSMIS-CSU-ICDR ]]; then
    endyear=`date '+%Y' -d "2 days ago"`
    enddoy=`date '+%j' -d "2 days ago"`
fi

echo "$PROGNAME: $startyear=start year $startdoy=start doy $endyear=end year $enddoy=end doy"

for src in ${platforms}
do
    echo "$PROGNAME: $src - platform "
    source ${PMESDR_RUN}/all_lists_for_sensor.sh $startyear $startdoy $endyear $enddoy $src $top_level \
	|| error_exit "Line $LINENO: all_lists_for_sensor ${src} error."
    grep -l such $direc/${src}_lists/* | xargs sed -i '/such/d' 
    source $PMESDR_RUN/${make_file} $startyear $startdoy $endyear \
	   $enddoy $src ${PMESDR_SCRIPT_DIR} $top_level || \
	error_exit "Line $LINENO: all_SSMIS(or SMAP)_make_for_sensor ${src} error."
    ml intel
    ml netcdf/4.4.1.1
    ml udunits
    ml impi
    ml loadbalance
    echo "mpirun lb ${direc}/${src}_scripts/${src}_make_list"
    mpirun -genv I_MPI_FABRICS=shm:ofi lb ${direc}/${src}_scripts/${src}_make_list || \
	error_exit "Line $LINENO: mpirun meas_meta_make ${src} error."
done

#finally set off Step1
for src in $platforms
do
    echo "Start Step1 for ${src}"
    echo "sbatch --dependency=afterok:$SLURM_JOB_ID ${PMESDR_RUN}/runNRTdailyStep1.sh ${arg_string} ${src}"
    sbatch --dependency=afterok:$SLURM_JOB_ID ${PMESDR_RUN}/runNRTdailyStep1.sh ${arg_string} ${src}
done

thisDate=$(date)
echo "$PROGNAME: Done on hostname=$thisHost on $thisDate"
