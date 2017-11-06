#!/bin/bash
cd meas_meta_make
make rss_quick
cd ../meas_meta_setup
make rss_quick
cd ../meas_meta_sir
make rss_quick
make rss_quick_validate
cd ..
