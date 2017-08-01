#!/bin/bash
cd meas_meta_make
make csu_ease
cd ../meas_meta_setup
make csu_ease
cd ../meas_meta_sir
make csu_ease
make csu_ease_validate
cd ..
