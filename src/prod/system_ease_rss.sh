#!/bin/bash
cd meas_meta_make
make rss_ease
cd ../meas_meta_setup
make rss_ease
cd ../meas_meta_sir
make rss_ease
make rss_ease_validate
cd ..
