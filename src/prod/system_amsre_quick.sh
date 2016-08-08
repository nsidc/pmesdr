#!/bin/bash
cd meas_meta_make
make amsre_quick
cd ../meas_meta_setup
make amsre_quick
cd ../meas_meta_sir
make amsre_quick
cd ..
