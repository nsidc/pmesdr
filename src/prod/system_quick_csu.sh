#!/bin/bash
cd meas_meta_make
make csu_quick
cd ../meas_meta_setup
make csu_quick
cd ../meas_meta_sir
make csu_quick
make csu_quick_validate
cd ../meas_meta_make
make csu_quick_36
cd ../meas_meta_setup
make csu_quick_36
cd ../meas_meta_sir
make csu_quick_36
make csu_quick_validate_36
cd ..
