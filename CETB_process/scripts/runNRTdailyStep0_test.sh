#!/bin/bash
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
#SBATCH --account=ucb265_alpine1
#SBATCH --partition=amilan
#SBATCH --constraint=ib
#SBATCH --time=00:01:00
#SBATCH --nodes=1
#SBATCH --cpus-per-task=1
#SBATCH --ntasks=20
#SBATCH -o /scratch/alpine/%u/NRTdaily_output/runNRTdailyStep0-%j.out
# Set the system up to notify upon completion

OPTIND=1

usage() {
    echo "" 1>&2
    echo "Usage: `basename $0` [-trf] [-h] GSX_TYPE CONDAENV" 1>&2
    echo "  GSX_TYPE CONDAENV" 1>&2
    echo "Options: "  1>&2
    echo "  -t: top level data location under /scratch/alpine/${USER}" 1>&2
    echo "  -r: set base resolution default is 25km r -1 is 36km r -2 is 24km" 1>&2
    echo "  -h: display help message and exit" 1>&2
    echo "  -f: do the ftp pull(default is no ftp pull)" 1>&2
    echo "  GSX_TYPE : type of gsx translation to do (see gsx --help)" 1>&2
    echo "           : also controls which platforms to process, eg F17 or AQUA etc" 1>&2
    echo "  CONDAENV : conda environment for gsx translation" 1>&2
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
	mailx -s "NRT Step0 error jobid ${SLURM_JOB_ID}" \
	      -r "molly\.hardman\@colorado\.edu" ${list_of_emails}
    exit 1
}

top_level=""
arg_string=""
res_string=""
base_resolution=0
do_ftp=
ftp_string=""

while getopts "fr:t:h" opt; do
    case $opt in
	f) do_ftp=1
	   ftp_string="-f";;
	t) top_level=$OPTARG
	   arg_string="-t ${top_level}";;
	r) base_resolution=$OPTARG
	   res_string="-r ${base_resolution}";;
	h) usage
	   exit 1;;
	?) printf "Usage: %s: [-tf] args\n" $0
           exit 1;;
	esac
done

source /projects/moha2290/measures-byu-alpine/src/prod/alpine_set_pmesdr_environment.sh
    
date
# Now set up sensor based on GSX type

echo "$PROGNAME: top_level = $top_level"
direc=/scratch/alpine/${USER}/${top_level}
echo "$PROGNAME: scratch directory $direc"

shift $(($OPTIND - 1))

[[ "$#" -eq 2 ]] || error_exit "Line $LINENO: Unexpected number of arguments."

gsx_type=$1
condaenv=$2

# start sbatch for the next day
sbatch --begin=04:30:00 --account=$SLURM_JOB_ACCOUNT ${PMESDR_RUN}/runNRTdailyStep0.sh ${ftp_string} ${res_string} ${arg_string} ${gsx_type} ${condaenv}
ml purge
ml intel/2022.1.2
ml python/3
ml gnu_parallel
ml
date

thisHost=$SLURM_NODELIST
thisDate=$(date)
echo "$PROGNAME: Begin on hostname=$thisHost on $thisDate"
echo "$PROGNAME: SLURM_SCRATCH=$SLURM_SCRATCH"
echo "$PROGNAME: SLURM_JOB_ID=$SLURM_JOB_ID"
echo "$PROGNAME: with gsx_type=$gsx_type and condaenv=$condaenv base_resolution=$base_resolution"


case $gsx_type in
    
    SSMIS-L1C)
    fetch_file="/projects/${USER}/swathfetcher/ftp_nrt_l1c_alpine.py"
    suffix="*.RT-H5"
    run_dir="/projects/${USER}/swathfetcher"
    make_file="all_SSMIS_make_for_sensor.sh"
    platforms="F16 F17 F18";;
    
    AMSR2-L1C)
    fetch_file="/projects/${USER}/swathfetcher/ftp_nrt_amsr2_l1c_alpine.py"
    suffix="*.RT-H5"
    run_dir="/projects/${USER}/swathfetcher"
    make_file="all_SSMIS_make_for_sensor.sh"
    platforms="AMSR2";;
    
    SSMIS-CSU-ICDR)
    fetch_file="/projects/${USER}/swathfetcher/ftp_nrt_csu_alpine.py"
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
    source activate base
#Go here so that correct secret files are used or SMAP downloaded to correct location
    cd ${run_dir}
    python ${fetch_file} || error_exit "Line $LINENO: ftp error."
    echo "Done with ftp fetch from ${fetch_file}"
# Change back to original directory
    cd ${cur_dir}

#after files are retrieved, create input file list for gsx of files with
# modification date less than 1 day old
    source activate $condaenv
    date
    for src in $platforms
    do
	echo "working this src ${src}"
	if [[ -f ${direc}/${src}_scripts/gsx_lb_list_alpine ]]; then
	    rm ${direc}/${src}_scripts/gsx_lb_list_alpine
	    echo "removed old gsx_lb_file for ${src}"
	fi
	for file in `find ${direc}/${src} -name "${suffix}" -mtime 0`
	do
	    basen=`basename ${file}`
	    echo "gsx ${gsx_type} ${file} ${direc}/${src}_GSX/GSX_${basen}.nc" \
		 >> ${direc}/${src}_scripts/gsx_lb_list_alpine
	done
	echo "parallel -a ${direc}/${src}_scripts/gsx_lb_list_alpine"
	parallel -a ${direc}/${src}_scripts/gsx_lb_list_alpine || \
	    error_exit "Line $LINENO: mpirun gsx ${src} error."
    done
fi
date

#next sort the files into daily lists

startyear=`date '+%Y' -d "7 days ago"`
startdoy=`date '+%j' -d "7 days ago"`
endyear=`date '+%Y'`
enddoy=`date '+%j'`
echo "$PROGNAME: $startyear=start year $startdoy=start doy $endyear=end year $enddoy=end doy"
echo "$PROGRAME: $platforms=platforms $src=src"

# Note the CSU ICDR data stream is 1-2 days behind today
# This workaround gets around a bug in meas_meta_setup that fails if there
# are no input files in the make file
if [[ ${gsx_type} == SSMIS-CSU-ICDR ]]; then
    endyear=`date '+%Y' -d "2 days ago"`
    enddoy=`date '+%j' -d "2 days ago"`
fi

echo "$PROGNAME: $startyear=start year $startdoy=start doy $endyear=end year $enddoy=end doy"
echo "$PROGRAME: $platforms=platforms $src=src"

suffix=""
if [[ "${base_resolution}" == "1" ]]
then
    suffix="_36"
fi
if [[ "$base_resolution" == "2" ]]
then
    suffix="_24"
fi

echo "$PROGNAME: suffix = $suffix"

for src in ${platforms}
do
    echo "$PROGNAME: starty startd endy endd $startyear $startdoy $endyear $enddoy"
    echo "$PROGNAME: $src - platform "
    out=$(${PMESDR_RUN}/all_lists_for_sensor.sh $startyear $startdoy $endyear $enddoy $src $top_level) \
	|| error_exit "Line $LINENO: all_lists_for_sensor ${src} error."
    echo "back from all_lists_for_sensor.sh"
    grep -l such $direc/${src}_lists/* | xargs sed -i '/such/d'
    echo "$PROGNAME: $make_file $startyear ${res_string} $startdoy $endyear $enddoy $src"
    echo "$PROGNAME: ${PMESDR_SCRIPT_DIR} $top_level"
    out=$(${PMESDR_RUN}/${make_file} ${res_string} $startyear $startdoy $endyear \
	   $enddoy $src ${PMESDR_SCRIPT_DIR} $top_level) || \
	error_exit "Line $LINENO: all_SSMIS(or SMAP)_make_for_sensor ${src} error."
    ml netcdf/4.8.1
    ml udunits/2.2.25
    ml
    echo "parallel -a ${direc}/${src}_scripts/${src}_make_list${suffix}"
    parallel -a ${direc}/${src}_scripts/${src}_make_list${suffix} || \
	error_exit "Line $LINENO: parallel meas_meta_make ${src} error."
done

#finally set off Step1
for src in $platforms
do
    echo "Start Step1 for ${src}"
    echo "sbatch --account=$SLURM_JOB_ACCOUNT --dependency=afterok:$SLURM_JOB_ID ${PMESDR_RUN}/runNRTdailyStep1.sh ${res_string} ${arg_string} ${src}"
    sbatch --dependency=afterok:$SLURM_JOB_ID --account=$SLURM_JOB_ACCOUNT ${PMESDR_RUN}/runNRTdailyStep1.sh ${res_string} ${arg_string} ${src}
done

thisDate=$(date)
echo "$PROGNAME: Done on hostname=$thisHost on $thisDate with this jobid=$SLURM_JOB_ID"