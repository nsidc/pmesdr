#!/bin/bash
cd meas_meta_make
make ssmis_cross
cd ../meas_meta_setup
make ssmis_cross
# cd ../meas_meta_sir
#make ssmis_cross
cd ..
