# Operational processing notes for nsidc0630v2

These instructions assume that the reader is familiar with the processing notes
for nsidc0630v1.

## Scripts

There are new scripts in CETB_process/scripts:

 * l1c_to_gsx.sh - processes input swath files for v2
 * combine_amsr.sh - combines AMSR2 .nc.partial files into a single AMSR2 GSX input file suitable for processing
 * run_gsx_loadbalance.sh - use to sbatch the gsx conversions
 * run_make_loadbalance.sh - runs the list of make files
 * run_setup_year_loadbalance.sh - to run a year of setup files
 * run_sir_year_loadbalance.sh - to run a year of sir files
 * run_setup_rm_year_loadbalance.sh  - to delete a year of setup files

The l1c_to_gsx.sh script will handle SSMI, SSMIS and AMSR2 conversions into GSX
format - it is similar to the previous nc_to_gsx.sh and hdf_to_gsx.sh, but with
extra arguments

It takes 4 arguments and will then create an output file with a list of files to
be converted from input swath format into GSX format.  Note that for AMSR2 and
AMSRE there is an extra step required AFTER the conversions and that is to
combine the GSX input files - this runs from a separate script

Arguments for l1c_to_gsx.sh
 * gsx_type - one of SSMIS-L1C, AMSR2-L1C, SSMI-L1C, AMSR2-JAXA, AMSRE-L1C or AMSRE-JAXA
 * src - F16, F08 (or all of the other DMSP satellites), for AMSR2 or AMSRE same as gsx_type
 * suffix - the input file suffix eg RT-H5 for L1C files, h5 for JAXA files
 * top_level - same as before - the top level directory under /scratch/alpine/$USER

Arguments for combine_amsr.sh
 * src - either AMSR2 or AMSRE
 * top_level - same as before

## RECIPE for SSMIS for processing a year at a time

First source the environment setup file

``` bash
source /projects/moha2290/pmesdr-v2/src/projects/set_pmesdr_environment.sh
```

If not done already - create the sensor directories by running:

``` bash
source $PMESDR_RUN/make_sensor_dirs.sh SRC top_level
```

where SRC is AMSER2 or SMAP or F08 ot F16 etc and top_level is the top level
directory on scratch in your user area

``` bash
source $PMESDR_RUN/l1c_to_gsx.sh gsx-type src suffix top_level
sbatch $PMESDR_RUN/run_gsx_loadbalance.sh src condaenv top_level $PMESDR_SCRIPT_DIR
source $PMESDR_RUN/all_lists_for_sensor.sh year doy year doy SRC top_level
source $PMESDR_RUN/SSMIS_make.sh year doy doy src $PMESDR_SCRIPT_DIR top_level
sbatch $PMESDR_RUN/run_make_loadbalance.sh src $PMESDR_SCRIPT_DIR top_level
source $PMESDR_RUN/SSMIS_setup_year.sh year src $PMESDR_SCRIPT_DIR top_level
sbatch $PMESDR_RUN/run_setup_year_loadbalance.sh year src $PMESDR_SCRIPT_DIR top_level
source $PMESDR_RUN/SSMIS_sir_year.sh year src $PMESDR_SCRIPT_DIR top_level
sbatch $PMESDR_RUN/run_sir_year_loadbalance.sh year src $PMESDR_SCRIPT_DIR top_level
source $PMESDR_RUN/premetandspatial.sh src $PMESDR_SCRIPT_DIR year top_level
sbatch $PMESDR_RUN/run_premet_cetb_loadbalance.sh src year condaenv $PMESDR_SCRIPT_DIR top_level
```

In between each step - check the "dirver" files that are created in the
`src_scripts` directory for correct entries and correct number of lines. After
each batch job completes, check for correct number of files. Check the output
files in the src_scripts/output directory and grep for errors.
