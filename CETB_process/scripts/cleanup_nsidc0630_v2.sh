#!/bin/bash -l
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.

condaenv=$1
CONDA_BASE=$(conda info --base)
source $CONDA_BASE/etc/profile.d/conda.sh


conda activate $condaenv
for sat in F16 F17 F18; do echo $sat; for dir in N25 N6.25 N3.125 S25 S6.25 S3.125 T25  T6.25 T3.125; do echo $dir; python $PMESDR_TOP_DIR/scripts/remove_duplicate_files.py -i /pl/active/PMESDR/nsidc0630_v2/${sat}_SSMIS/EASE2_${dir}km/2024 -p ${sat}; done; done
for sat in GCOMW1_AMSR2; do echo $sat; for dir in N25 N12.5 N6.25 N3.125 S25 S12.5 S6.25 S3.125 T25 T12.5 T6.25 T3.125; do echo $dir; python $PMESDR_TOP_DIR/scripts/remove_duplicate_files.py -i /pl/active/nsidc0630_v2/${sat}/EASE2_${dir}km/2024 -p AMSR2; done; done
# 
#
# 
