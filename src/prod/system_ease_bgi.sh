#!/bin/bash
cd meas_meta_make
make csu_ease
make rss_ease
cd ../meas_meta_setup
make csu_ease
make rss_ease
cd ../meas_meta_bgi
make csu_ease
make rss_ease
