#!/bin/sh
# Runs either quick or daily regression
#
# Quick: runs fast, only does N Hem output for 1 channel
# Daily: runs slower, does NST for multiple channels
#
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.

#
# The following commands will be executed when this script is run.
# It is assumed that the caller has set the system location to
# $PMESDR_TOP_DIR
regressiontype=$1
condaenv=$2

if [ "$1" == "-h" ] || [ "$#" -ne 2 ] ; then
    echo ""
    echo "Usage: `basename $0` [-h] REGRESSIONTYPE CONDAENV"
    echo "  Runs quick (fast, does less) or "
    echo "  daily (slower, does more) regression tests"
    echo ""
    echo "Assumes the system location is set in PMESDR_TOP_DIR"
    echo ""
    echo "Arguments:"
    echo "  REGRESSIONTYPE: quick or daily"
    echo "  CONDENV: name of conda env with python2 and nose"
    echo ""
    exit 1
fi

if [ "$regressiontype" != "quick" ] && [ "$regressiontype" != "daily" ] ; then
    echo "`basename $0`: invalid regressiontype, should be quick or daily"
    exit 1
fi    

maketarget=${regressiontype}
if [ "$regressiontype" == "daily" ] ; then
    maketarget="ease"
fi

echo "`basename $0`: Regression type is $regressiontype"
echo "`basename $0`: Conda env is       $condaenv"
echo "`basename $0`: Make target is     $maketarget"

source ${PMESDR_TOP_DIR}/src/prod/single_set_pmesdr_environment.sh
eval "$(conda shell.bash hook)"
conda activate ${condaenv}
date

echo "`basename $0`: running make"
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_make
pwd
make csu_${maketarget}
if [ $? -ne 0 ]; then
  echo "`basename $0`: csu_${maketarget} make failed"
  exit -1
fi
echo "`basename $0`: running setup"
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_setup
pwd
make csu_${maketarget}
if [ $? -ne 0 ]; then
  echo "`basename $0`: csu_$maketarget setup failed"
  exit -1
fi
echo "`basename $0`: running sir"
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_sir
pwd
make csu_${maketarget}
if [ $? -ne 0 ]; then
  echo "`basename $0`: csu_${maketarget} SIR failed"
  exit -1
fi
make csu_${maketarget}_validate
if [ $? -ne 0 ]; then
  echo "`basename $0`: csu_$maketarget SIR validate failed"
  exit -1
fi

echo "`basename $0`: running make at 36 km"
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_make
pwd
make csu_${maketarget}_36
if [ $? -ne 0 ]; then
  echo "`basename $0`: csu_${maketarget}_36 make failed"
  exit -1
fi
echo "`basename $0`: running setup at 36 km"
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_setup
pwd
make csu_${maketarget}_36
if [ $? -ne 0 ]; then
  echo "`basename $0`: csu_${maketarget}_36 setup failed"
  exit -1
fi
echo "`basename $0`: running sir at 36 km"
cd ${PMESDR_TOP_DIR}/src/prod/meas_meta_sir
pwd
make csu_${maketarget}_36
if [ $? -ne 0 ]; then
  echo "`basename $0`: csu_${maketarget}_36 SIR failed"
  exit -1
fi
make csu_${maketarget}_validate_36
if [ $? -ne 0 ]; then
  echo "`basename $0`: csu_$maketarget_36 SIR validate failed"
  exit -1
fi
echo "`basename $0`: Finished $regressiontype regression"
