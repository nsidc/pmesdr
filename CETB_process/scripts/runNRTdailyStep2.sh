#!/bin/bash
#
# script to run NRT daily Step2:
#   this is a cleanup script that moves the sir files to the peta library
#   and deletes the setup files
#

#SBATCH --qos normal
#SBATCH --job-name runNRTdailyStep2
#SBATCH --account=ucb135_summit3
#SBATCH --time=01:50:00
#SBATCH --ntasks-per-node=6
#SBATCH --nodes=1
#SBATCH -o /scratch/summit/%u/NRTdaily_output/runNRTdailyStep2-%j.out
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

shift $(($OPTIND - 1))
[[ "$#" -eq 1 ]] || error_exit "Line $LINENO: Unexpected number of arguments."
src=$1
# Note that SMAP doesn't use a top_level directory
pl_top=nsidc0738_v2

if [[ ${src} == SMAP ]]
then
    suffix=LRM
    sat_top=${src}_${suffix}_NRT
    smap_top=${src}_${suffix}
fi

if [[ ${src} == AMSR2 ]]
then
    suffix=""
    sat_top=GCOMW1_${src}
    pl_top=nsidc0763_v1
fi

resolution_suffix=""
case $top_level in
    nsidc0751_CSU_ICDR)
	suffix=SSMIS
	sat_top=${src}_${suffix}
	pl_top=nsidc0630_v1;;
    nsidc0752_CSU_GPM_XCAL_NRT_Tbs)
	suffix=SSMIS
	sat_top=${src}_${suffix}
	pl_top=nsidc0763_v1;;
esac

if [[ resolution -eq 1 ]]
then
    resolution_suffix="_36"
elif [[ resolution -eq 2 ]]
then
    resolution_suffix="_24"
fi

direc=/scratch/summit/${USER}/${top_level}/
SETUPDIR=${direc}/${src}_setup${resolution_suffix}/
SCRIPTDIR=${direc}/${src}_scripts/

outfile=${SCRIPTDIR}/${src}_moving_files${resolution_suffix}
outfile_ps=${SCRIPTDIR}/${src}_premet_files${resolution_suffix}
outfile_premet_fix=${SCRIPTDIR}/${src}_premet_fix${resolution_suffix}
if [[ -f ${outfile} ]]; then
    rm ${outfile}
    echo "removed old move file for ${src}"
else
    echo " no old move file to remove for ${src}"
fi

if [[ -f ${outfile_ps} ]]; then
    rm ${outfile_ps}
    echo "removed old premetandspatial file for ${src}"
fi

for file in `find ${direc}/${src}_sir${resolution_suffix}/*.nc -mtime -2`
do
    basen=`basename $file`
    year=`echo $basen | grep -o ${src}_${suffix}-.... | sed 's/^.*-//'`
    if [[ ${src} == AMSR2 ]]; then
	year=`echo $basen | grep -o ${src}-.... | sed 's/^.*-//'`
    fi
    hemi=`echo $basen | grep -o EASE2_.*km`
    if [[ $SLURM_JOB_USER == "jeca4282" ]]; then
	echo "rsync -avz -e 'ssh -i /home/jeca4282/.ssh/id_ecdsa_summit_archive' ${file} archive@nusnow.colorado.edu:/disks/restricted_ftp/ops_data/incoming/NSIDC0630/${src}/" >> ${outfile}
	echo "rsync -avz -e 'ssh -i /home/jeca4282/.ssh/id_ecdsa_summit_archive' ${file}.premet archive@nusnow.colorado.edu:/disks/restricted_ftp/ops_data/incoming/NSIDC0630/${src}/" >> ${outfile}
	echo "rsync -avz -e 'ssh -i /home/jeca4282/.ssh/id_ecdsa_summit_archive' ${file}.spatial archive@nusnow.colorado.edu:/disks/restricted_ftp/ops_data/incoming/NSIDC0630/${src}/" >> ${outfile}
	echo "generate_premetandspatial.py ${file}" >> ${outfile_ps}
    else
	echo "rsync -avz ${file} /pl/active/PMESDR/${pl_top}/${sat_top}/${hemi}/${year}/" >> ${outfile}
    fi
done

setup_rm_file=${SCRIPTDIR}/${src}_setup_rm${resolution_suffix}
if [[ -f ${setup_rm_file} ]]; then
    rm ${setup_rm_file}
    echo "removed old rm setup file for ${src} ${top_level} ${res_string}"
fi
for file in `find ${SETUPDIR}/*.setup`
do
    echo "rm $file" >> ${setup_rm_file}
done

ml intel
ml impi
ml loadbalance
ml python/3.6.5
ml
date
if [[ $SLURM_JOB_USER == "jeca4282" ]]; then
    source activate /projects/jeca4282/miniconda3/envs/cetb3
    mpirun -genv I_MPI_FABRICS=shm:ofi lb ${outfile_ps} || \
	error_exit "Line $LINENO: mpirun premetandspatial"
    grep :60 ${direc}/${src}_sir${resolution_suffix}/*.premet > ${outfile_premet_fix}
    sed -i '/CSU-v1/d' ${outfile_premet_fix}
    sed -i 's/nc.premet:Begin.*$/nc/' ${outfile_premet_fix}
    python /projects/moha2290/testing/coverage_list.py ${outfile_premet_fix}
    echo "${PROGNAME}: rerun premet and spatial after *.nc time metadata corrected"
    mpirun -genv I_MPI_FABRICS=shm:ofi lb ${outfile_ps} || \
	error_exit "Line $LINENO: mpirun premetandspatial"
    echo "${PROGNAME}: premet fix has run"
    sbatch --account=$SLURM_JOB_ACCOUNT --dependency=afterok:$SLURM_JOB_ID ${PMESDR_RUN}/runNRTdailyStep3.sh ${src}
fi

mpirun -genv I_MPI_FABRICS=shm:ofi lb $outfile || \
    error_exit "Line $LINENO: mpirun cp *.nc files"
mpirun -genv I_MPI_FABRICS=shm:ofi lb $setup_rm_file || \
    error_exit "Line $LINENO: mpirun remove setup and scratch output files"

echo "${PROGNAME}: Step2 for ${pl_top} ${res_string} ${src} completed" | \
	mailx -s "NRT Step2 Completed jobid ${SLURM_JOB_ID}" \
	      -r "molly\.hardman\@colorado\.edu" ${list_of_emails}
echo "${PROGNAME}: Step2 jobid=${SLURM_JOB_ID} for ${pl_top} ${res_string} ${src} completed" 

date






