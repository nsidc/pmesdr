#!/bin/bash
cd meas_meta_make
make csu_quick
cd ../meas_meta_setup
make csu_quick
cd ../meas_meta_sir
make csu_quick
make csu_quick_validate
cd ..
