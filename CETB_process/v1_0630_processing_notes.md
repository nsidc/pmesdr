# Operational processing notes for nsidc0630v1

Set the email notification address as an option to sbatch commands as follows:

``` bash
sbatch --mail-user=yourname@nsidc.org run_setup_year.sh yyyy Fxx
```

For multiple users, you can include a double-quoted, comma-separated list.

Note that some links in the following instructions are pointed to internal NSIDC wiki
pages and may not be visible without access to NSIDC systems.

## To produce a batch of nsidc0630v1 data:

1. Transfer all of the input files to CURC system, see [Running
   processing](https://nsidc.atlassian.net/wiki/spaces/PMESDR/pages/51249491/Running+processing+on+CURC+systems)
   notes for installation, and notes on transferring input files to CURC
   systems.
   
     a. Start a processing info file, e.g. /projects/${USER}/F16_info to keep
     track of notes on files and days processed

2. Take note of the start doy, end doy, total doys, total number of input files.

	The required shell scripts are all in the

    `CETB_process/scripts/`
    
    directory.  Add this to your $PATH, or set an ENV variable with
    this location.
    
    Note that for SSM/I and SSMIS, you will generate 42 setup files per day and
    84 *.nc CETB data files per day

3. Make the set of directories that are required for operational processing.

	Run make_sensor_dirs.sh to just do them all:

	``` bash
	cd ${PMESDR_SCRATCH_DIR}
    make_sensor_dirs.sh $SENSOR
	```

4. Convert input swath files to gsx format.

	``` bash
	cd to your location of `$SENSOR_scripts`
    nc_to_gsx.sh (to create script in the scripts directory called
	gsx_lb_list_summit to convert input to gsx - arguments SSMIS-CSU and source,
	e.g., F18)
	```
	
    This takes about 3-4 minutes.

5.  Verify contents of generated file with head, tail and wc -l
    (should have same number of lines as files you are expecting to process)

6.  From `$SENSOR_scripts`, do

	``` bash
    sbatch --mail-user=yourname@nsidc.org run_gsx.sh Fxx $CONDAENV
	```

    to create gsx files and verify file count in `xxx_GSX directory`.
    Stderr will be written to `$SENSOR_scripts/output/` directory

      a. Use `squeue --user=$USER --long` to monitor the queue activity

7.  From `$SENSOR_scripts`, do

    run `all_lists_for_sensor.sh` to create the list of input files for each day in series,

    Arguments are `start_year start_doy` `end_year end_doy` `sensor`

    These are stored in directory `$SENSOR_lists`.  There will be 2 files per
    day, first file is just the GSX files for that day and second file *.NS is
    the rolling set of day-1, day, day+1 files.
    
8.  Count to confirm the correct number of files created, 2 files per day for
    total number of days.

9.  Use grep to check for no "such" messages in files created - should only be
    in NS files for first and last doys - this is useful because it will also
    show if there are missing days of files.

10. Use sed to edit files to remove missing file messages:

	``` bash
	grep -l such ../Fxx_lists/* | xargs sed -i '/such/d'
	```

11. From `$SENSOR_scripts`, do

    `all_SSMIS_make_for_sensor.sh` to create list for make process

    For the time period to process.

    Arguments are `start_year start_doy` `end_year end_doy` `sensor` and
    path to `set_pmesdr_environment.sh` script, which will depend
    on where you installed the system, usually `${PMESDR_TOP_DIR}/src/prod`.

    N.B. If you run this for multiple years, they will each append to the end of
    the `$SENSOR_make_list` file, currently there is no check for this file
    existing to begin with, so be careful if you get interrupted to clean it up
    before restarting from the first year.

12. Count number of lines in input file to run_make, there should be 6 * number
    of days to process

13. From `$SENSOR_scripts`, do

	``` bash
    sbatch --mail-user=yourname@nsidc.org run_make.sh Fxx /path/to/set_env
	```

    which takes 2 arguments, the sensor and the path to
    set_pmesdr_environment.sh script, which will depend on where you
    installed the system, usually `${PMESDR_TOP_DIR}/src/prod`.

    This generates make files for setup process - files are stored in
    `$SENSOR_make`

---

From this point processing should proceed by year.  (Note that 1 year of SSMIS
setup files is around 15 - 17 TB, so plan accordingly.)

14. From `$SENSOR_scripts` dir, execute `SSMIS_setup_year.sh` with arguments of
    4-digit year platform and path to `set_pmesdr_environement.sh` script,
    to generate list of commands for loadbalancer.

    You can do these all at once with a bash loop:
	
	``` bash
	for y in $(seq 2005 2017); do echo $y ; $PMESDR_RUN/SSMIS_setup_year.sh $y F16 $PMESDR_SCRIPT_DIR; done
	```

15. Use `sbatch run_setup_year.sh` to run 1 year -- only run 3 of these at one time:

	``` bash
    sbatch --mail-user=yourname@nsidc.org run_setup_year.sh YYYY Fxx /path/to/set_env
	```

    Use `grep -i error` and `grep -i warning` on batch stderr output.

	A successful setup batch should return no warnings or errors.

    I think there's a way to do this with regex's, but a way to do this with a loop is:

	``` bash
    for t in warning error; do echo "looking for $t..."; grep -i $t setup_lb-745501.out ; done
	```
    
16. After setup completion, create list of sir commands for loadbalancer with:

	``` bash
	cd $SENSOR_scripts
    SSMIS_sir_year.sh yyyy platform /path/to/set_env
	```

17. Schedule the sir command batch with:

	``` bash
    sbatch --mail-user=yourname@nsidc.org run_sir_year.sh YYYY Fxx /path/to/set_env
	```
	
    Use `grep -i error` and `grep -i warning` on batch stderr output.

	A successful sir batch should return no warnings or errors.
    
18. Once setup has been completed for a given year (but before sir is completed),
    you can set up a batch job that will remove the very large setup files, contingent
    on the success of the sbatch that runs the sir commands.  To do this:
    
    From `$SENSOR_scripts` dir, execute `SSMIS_setup_rm_year.sh`
    with arguments of 4-digit year, platform and
    path to `set_pmesdr_environement.sh` script,
    to create list of setup files to remove:

	``` bash
    SSMIS_setup_rm_year.sh yyyy platform /path/to/set_env
	```

19. Then the sbatch job to remove the input files can be scheduled to only run
    if the sir job completes successfully.  So you do sbatch
    `run_setup_rm_year.sh`, with dependency as follows:-

	``` bash
    sbatch --dependency=afterok:jobid_from_sir run_setup_rm_year.sh yyyy Fxx /path/to/set_env
	```

20. At this point I usually edit the list of sir commands using sed to create
    the lists for further years

21. Also edit the list of setup rm commands with sed to change year

22. Now you can proceed to chain setup, sir and setup_rm jobs with dependencies:

	``` bash
	for y in $(seq 2011 2017) ; do echo "enxt" ; echo $y ; for f in $(ls $y/*N25*${y}001-37V-[ME]* ) ; do ncdump -h $f | grep temporal ; done ; for f in $(ls $y/*T25*${y}001-37V*) ; do ncdump -h $f | grep input ; done ; done
	```

## To move files to ingest locations and produce premet and spatial metadata

Once the processing has completed, you have to move the files around and run the
premet and spatial file generation.

1. Run the script `file_create_dirs.sh` - takes the platform and start/stop
   yyyymm as arguments

2. Execute the script `file_move_dir.sh` (yyyy, Fxx and type as arguments) to
    create a list of mv commands by year cat all of the output files into
    `moving_files_all`

	``` bash
    for y in $(seq 1990 1997) ; do file_move_dir.sh $y $SRC SSMI ; done
	```

3. `sbatch run_mv_files.sh` to move the files - two arguments are
    Fxx (src) and path to `set_pmesdr_environement.sh` script,

	``` bash
    sbatch --mail-user=yourname@nsidc.org run_mv_files.sh Fxx /path/to/set_env
	```

4. From `$SENSOR_scripts` dir, execute premetandspatial.sh with Fxx and path to
    `set_pmesdr_environement.sh` script, as argument to generate list for
    loadbalancer:

	``` bash
    premetandspatial.sh Fxx /path/to/set_env
	```

5. Make sure your conda env CONDAENV has cetbtools installed, and sbatch
    `run_premet_cetb.sh`, second arg is CONDAENV:

	``` bash
    sbatch --mail-user=yourname@nsidc.org run_premet_cetb.sh Fxx CONDAENV
	```
    
6. From `Fxx_scripts` directory, run manifest.sh to create the manifest list for OPS:

	``` bash
    manifest.sh Fxx /path/to/set_env
	```

7. Make sure that group read permissions are set recursively on `Fxx_sir`, and on
    ${PMESDR_SCRATCH_DIR} and on the `Fxx_manifest` file for the DAAC operator's
    user name.  At this point, the operators are not in any shared groups on
    the CURC systems, so this requires setting dirs to o+r, o+x and all files to o+r.

    At some point, my workflow created .tramp_history files, remove these so
    they don't get transferred over.

    Send email to DAAC about new batch of processing, wait for DAAC confirmation
    of ingest before deleting `Fxx_sir`. Save the `Fxx_scripts` directory to
    someplace off scratch.