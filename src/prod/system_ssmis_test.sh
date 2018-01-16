#!/bin/bash
cd meas_meta_make
make ssmis_test
cd ../meas_meta_setup
make ssmis_test
cd ../meas_meta_sir
make ssmis_test
cd ..
