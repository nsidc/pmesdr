#!/bin/bash
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J daily_regression

#
# Set a walltime for the job. The time format is HH:MM:SS - In this case we run for 5 minutes.

#SBATCH --time=02:15:00

# Set the output file and embed the job number in the filename
#SBATCH -o output/daily_regression-%j.out

# Select the janus QOS 
#SBATCH --qos=janus
#
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org,brodzik@nsidc.org
#
# The following commands will be executed when this script is run.

source ${PMESDR_TOP_DIR}/src/prod/set_pmesdr_environment.sh -a
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_make
pwd
echo 'running make'
make rss_ease
if [ $? -ne 0 ]; then
  echo 'rss_ease make failed'
  exit -1
fi
make csu_ease
if [ $? -ne 0 ]; then
  echo 'csu_ease make failed'
  exit -1
fi
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_setup/meas_meta_setup_RSS
pwd
echo 'running setup'
make rss_ease
if [ $? -ne 0 ]; then
  echo 'rss_ease setup failed'
  exit -1
fi
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_setup/meas_meta_setup_CSU
pwd
echo 'running setup'
make csu_ease
if [ $? -ne 0 ]; then
  echo 'csu_ease setup failed'
  exit -1
fi
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_sir
make rss_ease
if [ $? -ne 0 ]; then
  echo 'rss_ease SIR failed'
  exit -1
fi
make rss_ease_validate
if [ $? -ne 0 ]; then
  echo 'rss_ease SIR validate failed'
  exit -1
fi
make csu_ease
if [ $? -ne 0 ]; then
  echo 'csu_ease SIR failed'
  exit -1
fi
make csu_ease_validate
if [ $? -ne 0 ]; then
  echo 'csu_ease SIR validate failed'
  exit -1
fi
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_bgi
make rss_ease
if [ $? -ne 0 ]; then
  echo 'rss_ease BGI failed'
  exit -1
fi
make rss_ease_validate
if [ $? -ne 0 ]; then
  echo 'rss_ease BGI validate failed'
  exit -1
fi
make csu_ease
if [ $? -ne 0 ]; then
  echo 'csu_ease BGI failed'
  exit -1
fi
make csu_ease_validate
if [ $? -ne 0 ]; then
  echo 'csu_ease BGI validate failed'
  exit -1
fi


# End of example job shell script
# 
