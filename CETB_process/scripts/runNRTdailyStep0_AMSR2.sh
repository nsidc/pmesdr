#!/bin/bash
#
# script to run NRT daily Step0 for AMSR2 for v2 of 0630:
#   This is a special Step0 because of the way the input files have to be combined
#   Step1 can proceed as usual
#   fetch latest data - this script gets AMSR2 L1C and AMSR2 JAXA and combines them
#   run gsx on input files
#   create file lists and run meas_meta_make
#   kick off the batch job to run setup and sir
#   schedul Step0 for tomorrow
#
#SBATCH --qos normal
#SBATCH --job-name AMSR2_runNRTdailyStep0
#SBATCH --account=ucb286_asc1
#SBATCH --partition=amilan
# SBATCH --constraint=ib
#SBATCH --time=02:00:00
#SBATCH --nodes=1
#SBATCH --cpus-per-task=2
#SBATCH --ntasks=20
#SBATCH -o /scratch/alpine/%u/NRTdaily_output/%x-%j.out
# Set the system up to notify upon completion
#SBATCH --mail-type=FAIL,REQUEUE,STAGE_OUT
#SBATCH --mail-user=mhardman@nsidc.org

OPTIND=1

usage() {
    echo "" 1>&2
    echo "Usage: `basename $0` [-tf] [-h] CONDAENV" 1>&2
    echo "  CONDAENV" 1>&2
    echo "Options: "  1>&2
    echo "  -t: top level data location under /scratch/alpine/${USER}" 1>&2
    echo "  -h: display help message and exit" 1>&2
    echo "  -f: do the ftp pull(default is no ftp pull)" 1>&2
    echo "  CONDAENV : conda environment for gsx translation" 1>&2
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
thisScriptDir="$( cd "$( dirname "${PROGNAME}" )" && pwd )"
echo "running from this directory ${thisScriptDir}"

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
do_ftp=
ftp_string=""
start_string="now+24hour"

while getopts "ft:h" opt; do
    case $opt in
	f) do_ftp=1
	   ftp_string="-f";;
	t) top_level=$OPTARG
	   arg_string="-t ${top_level}";;
	h) usage
	   exit 1;;
	?) printf "Usage: %s: [-tf] args\n" $0
           exit 1;;
	esac
done

source $thisScriptDir/../../src/prod/alpine_set_pmesdr_environment.sh
    
date
# Now set up sensor based on GSX type

echo "$PROGNAME: top_level = $top_level"
direc=/scratch/alpine/${USER}/${top_level}
echo "$PROGNAME: scratch directory $direc"

shift $(($OPTIND - 1))

[[ "$#" -eq 1 ]] || error_exit "Line $LINENO: Unexpected number of arguments."

condaenv=$1
echo "conda environment $condaenv"

# start sbatch for the next day
echo "sbatch --begin=${start_string} --account=$SLURM_JOB_ACCOUNT ${PMESDR_RUN}/runNRTdailyStep0_AMSR2.sh ${ftp_string} ${arg_string} ${condaenv}"
sbatch --begin=${start_string} --account=$SLURM_JOB_ACCOUNT ${PMESDR_RUN}/runNRTdailyStep0_AMSR2.sh ${ftp_string} ${arg_string} ${condaenv}
ml purge
ml intel/2022.1.2
ml gnu_parallel
source /projects/${USER}/miniconda3/bin/activate
ml
date

thisHost=$SLURM_NODELIST
thisDate=$(date)
echo "$PROGNAME: Begin on hostname=$thisHost on $thisDate"
echo "$PROGNAME: SLURM_SCRATCH=$SLURM_SCRATCH"
echo "$PROGNAME: SLURM_JOB_ID=$SLURM_JOB_ID"
echo "$PROGNAME: running AMSR2 and condaenv=$condaenv"

make_file="all_SSMIS_make_for_sensor.sh"
platforms="AMSR2-L1C AMSR2-JAXA"
    

cur_dir=${PWD}
echo "$PROGNAME: $platforms"

# if -f is set download files from ftp
run_dir="/projects/${USER}/swathfetcher"
if [[ $do_ftp ]]; then
    source activate base
#Go here so that correct secret files are used or SMAP downloaded to correct location
    cd ${run_dir}
    echo "/projects/${USER}/swathfetcher/ftp_nrt_amsr2_l1c_v2_alpine.py"
    python  "/projects/${USER}/swathfetcher/ftp_nrt_amsr2_l1c_v2_alpine.py" || error_exit "Line $LINENO: ftp error."
    echo "Done with ftp fetch for L1C"
    source activate $condaenv
    echo "/projects/${USER}/swathfetcher/ftp_nrt_amsr2_jaxa_v2_alpine.py"
    python "/projects/${USER}/swathfetcher/ftp_nrt_amsr2_jaxa_v2_alpine.py" || error_exit "Line $LINENO: ftp error."
# Change back to original directory
    echo "Done with ftp fetch for JAXA"    
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
	case $src in
	    *"L1C"*)
		suffix="*.RT-H5"
		echo "L1C suffix ${suffix}"
		;;
	    *"JAXA"*)
		suffix="*.h5"
		echo "JAXA suffix ${suffix}"
		;;
	esac
	echo "suffix is ${suffix}"
	for file in `find ${direc}/${src}/ -name "${suffix}" -mtime 0`
	do
	    basen=`basename ${file}`
	    echo "gsx ${src} ${file} ${direc}/${src}_GSX/GSX_${basen}.nc.partial" \
		 >> ${direc}/${src}_scripts/gsx_lb_list_alpine
	done
	echo "parallel -a ${direc}/${src}_scripts/gsx_lb_list_alpine"
	parallel -a ${direc}/${src}_scripts/gsx_lb_list_alpine || \
	    error_exit "Line $LINENO: parallel gsx ${src} error."
    done
    # Now combine the L1C and the JAXA files
    if [[ -f ${direc}/AMSR2_scripts/gsx_lb_list_alpine ]]; then
	rm ${direc}/AMSR2_scripts/gsx_lb_list_alpine
	echo "removed old gsx_lb_file for AMSR2"
    fi
    for file in `find ${direc}/AMSR2-L1C_GSX -name "GSX_*.nc.partial"`
    do
	echo "combine_amsr_l1c_jaxa AMSR2 $file ${direc}/AMSR2-JAXA_GSX ${direc}/AMSR2_GSX" \
	     >> ${direc}/AMSR2_scripts/gsx_lb_list_alpine
    done
    parallel -a ${direc}/AMSR2_scripts/gsx_lb_list_alpine || \
	error_exit "Line $LINENO: parallel gsx AMSR2 combine error."
	 
fi
date

#next sort the files into daily lists

startyear=`date '+%Y' -d "7 days ago"`
startdoy=`date '+%j' -d "7 days ago"`
endyear=`date '+%Y'`
enddoy=`date '+%j'`
echo "$PROGNAME: $startyear=start year $startdoy=start doy $endyear=end year $enddoy=end doy"
echo "$PROGRAME: $platforms=platforms $src=src"


echo "$PROGNAME: $startyear=start year $startdoy=start doy $endyear=end year $enddoy=end doy"
echo "$PROGRAME: $platforms=platforms $src=src"


echo "$PROGNAME: suffix = $suffix"

for src in AMSR2
do
    echo "$PROGNAME: starty startd endy endd $startyear $startdoy $endyear $enddoy"
    echo "$PROGNAME: $src - platform "
    out=$(${PMESDR_RUN}/all_lists_for_sensor.sh $startyear $startdoy $endyear $enddoy $src $top_level) \
	|| error_exit "Line $LINENO: all_lists_for_sensor ${src} error."
    echo "back from all_lists_for_sensor.sh"
    grep -l such $direc/${src}_lists/* | xargs sed -i '/such/d'
    echo "$PROGNAME: $make_file $startyear $startdoy $endyear $enddoy $src"
    echo "$PROGNAME: ${PMESDR_SCRIPT_DIR} $top_level"
    out=$(${PMESDR_RUN}/${make_file} $startyear $startdoy $endyear \
	   $enddoy $src ${PMESDR_SCRIPT_DIR} $top_level) || \
	error_exit "Line $LINENO: all_SSMIS_make_for_sensor ${src} error."
    ml netcdf/4.8.1
    ml udunits/2.2.25
    ml
    echo "parallel -a ${direc}/${src}_scripts/${src}_make_list${suffix}"
    parallel -a ${direc}/${src}_scripts/${src}_make_list || \
	error_exit "Line $LINENO: parallel meas_meta_make ${src} error."
done

#finally set off Step1

echo "Start Step1 for AMSR2"
echo "sbatch --account=$SLURM_JOB_ACCOUNT --dependency=afterok:$SLURM_JOB_ID --job-name=AMSR2_S1 ${PMESDR_RUN}/runNRTdailyStep1.sh ${res_string} ${arg_string} AMSR2"
sbatch --dependency=afterok:$SLURM_JOB_ID --job-name=AMSR2_S1 --account=$SLURM_JOB_ACCOUNT ${PMESDR_RUN}/runNRTdailyStep1.sh ${res_string} ${arg_string} AMSR2


thisDate=$(date)
echo "$PROGNAME: Done on hostname=$thisHost on $thisDate with this jobid=$SLURM_JOB_ID"
