#!/bin/bash
cd meas_meta_make
make smmr_quick
cd ../meas_meta_setup
make smmr_quick
cd ../meas_meta_sir
make smmr_quick
#make csu_quick_validate
cd ..
