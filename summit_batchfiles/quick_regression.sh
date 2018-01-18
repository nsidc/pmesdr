#!/bin/bash
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J quick_regression

#
# Set a walltime for the job. The time format is HH:MM:SS - In this case we run for 5 minutes.

#SBATCH --time=00:05:00

# Set the output file and embed the job number in the filename
#SBATCH -o output/quick_regression-%j.out

# Select the summit QOS
#SBATCH --qos normal
#SBATCH --partition=shas
##SBATCH --account=ucb13_summit1
##SBATCH --ntasks-per-node=1
##SBATCH --nodes=1

#
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org,brodzik@nsidc.org
#
# The following commands will be executed when this script is run.
# It is assumed that the caller has executed
condaenv=$1

source ${PMESDR_TOP_DIR}/src/prod/summit_set_pmesdr_environment.sh
source activate ${condaenv}
date

echo 'running make'
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_make
pwd
make csu_quick
if [ $? -ne 0 ]; then
  echo 'csu_quick make failed'
  exit -1
fi
echo 'running setup'
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_setup
pwd
make csu_quick
if [ $? -ne 0 ]; then
  echo 'csu_quick setup failed'
  exit -1
fi
echo 'running sir'
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_sir
pwd
make csu_quick
if [ $? -ne 0 ]; then
  echo 'csu_quick SIR failed'
  exit -1
fi
make csu_quick_validate
if [ $? -ne 0 ]; then
  echo 'csu_quick SIR validate failed'
  exit -1
fi
echo 'end of quick_regression'
