#!/bin/bash
# link_regression_files.sh
# Created Wed Jul  1 2015 by Mary Jo Brodzik <brodzik@>
# Refer to Refer to wiki page for filenameing conventions:
# https://nsidc.org/confluence/display/PMESDR/CETB+Data+Set+File+Definition#CETBDataSetFileDefinition-FilenamingConvention

# Make new directories
orig_dir=/projects/PMESDR/pmesdr_regression_data/20150518
root_dir=/projects/PMESDR/pmesdr_regression_data/20150701
mkdir -v $root_dir/daily
mkdir -v $root_dir/daily/bgiCSU
mkdir -v $root_dir/daily/bgiRSS
mkdir -v $root_dir/daily/sirCSU
mkdir -v $root_dir/daily/sirRSS
mkdir -v $root_dir/quick
mkdir -v $root_dir/quick/bgiCSU
mkdir -v $root_dir/quick/bgiRSS
mkdir -v $root_dir/quick/sirCSU
mkdir -v $root_dir/quick/sirRSS

projs=(N S T)

sources=(CSU RSS)

for source in ${sources[@]}; do
    echo "Next source is ${source}";

    for proj in ${projs[@]}; do
	echo "Next proj is ${proj}";

	ltods=(E M)
	if [ 'T' == ${proj} ]; then
	    ltods=(A D)
	fi
	echo "First ltod is ${ltods[0]}"
	echo "Next  ltod is ${ltods[1]}"

	lcproj=$(tr '[A-Z]' '[a-z]' <<< ${proj} )

	lcltod0=$(tr '[A-Z]' '[a-z]' <<< ${ltods[0]} )
	lcltod1=$(tr '[A-Z]' '[a-z]' <<< ${ltods[1]} )

	if [ 'N' == ${proj} ]; then
	    # Link real BGI quick regression filenames to dump filenames
	    ln -s $orig_dir/quick/bgi${source}/e2${lcproj}/FD1${lcltod0}-E2${proj}97-061-061.lis_dump1.nc \
		$root_dir/quick/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19H.${ltods[0]}.BGI.${source}.v0.1.nc
	    ln -s $orig_dir/quick/bgi${source}/e2${lcproj}/FD1${lcltod1}-E2${proj}97-061-061.lis_dump1.nc \
		$root_dir/quick/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19H.${ltods[1]}.BGI.${source}.v0.1.nc
	fi

        # Link real BGI daily regression filenames to dump filenames
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD1${lcltod0}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19H.${ltods[0]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD1${lcltod1}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19H.${ltods[1]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD2${lcltod0}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19V.${ltods[0]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD2${lcltod1}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19V.${ltods[1]}.BGI.${source}.v0.1.nc

	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD3${lcltod0}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.22V.${ltods[0]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD3${lcltod1}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.22V.${ltods[1]}.BGI.${source}.v0.1.nc

	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD4${lcltod0}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.37H.${ltods[0]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD4${lcltod1}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.37H.${ltods[1]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD5${lcltod0}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.37V.${ltods[0]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD5${lcltod1}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.37V.${ltods[1]}.BGI.${source}.v0.1.nc

	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD6${lcltod0}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.85H.${ltods[0]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD6${lcltod1}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.85H.${ltods[1]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD7${lcltod0}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.85V.${ltods[0]}.BGI.${source}.v0.1.nc
	ln -s $orig_dir/daily/bgi${source}/e2${lcproj}/FD7${lcltod1}-E2${proj}97-061-061.lis_dump1.nc \
	    $root_dir/daily/bgi${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.85V.${ltods[1]}.BGI.${source}.v0.1.nc

	if [ 'N' == ${proj} ]; then
	    # Link real SIR quick regression filenames to dump filenames
	    ln -s $orig_dir/quick/sir${source}/e2${lcproj}/FD1${lcltod0}-E2${proj}97-061-061.lis_dump.nc \
		$root_dir/quick/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19H.${ltods[0]}.SIR.${source}.v0.1.nc
	    ln -s $orig_dir/quick/sir${source}/e2${lcproj}/FD1${lcltod1}-E2${proj}97-061-061.lis_dump.nc \
		$root_dir/quick/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19H.${ltods[1]}.SIR.${source}.v0.1.nc
	fi

        # Link real SIR daily regression filenames to dump filenames
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD1${lcltod0}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19H.${ltods[0]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD1${lcltod1}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19H.${ltods[1]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD2${lcltod0}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19V.${ltods[0]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD2${lcltod1}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.19V.${ltods[1]}.SIR.${source}.v0.1.nc

	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD3${lcltod0}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.22V.${ltods[0]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD3${lcltod1}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.22V.${ltods[1]}.SIR.${source}.v0.1.nc

	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD4${lcltod0}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.37H.${ltods[0]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD4${lcltod1}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.37H.${ltods[1]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD5${lcltod0}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.37V.${ltods[0]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD5${lcltod1}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.37V.${ltods[1]}.SIR.${source}.v0.1.nc

	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD6${lcltod0}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.85H.${ltods[0]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD6${lcltod1}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.85H.${ltods[1]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD7${lcltod0}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.85V.${ltods[0]}.SIR.${source}.v0.1.nc
	ln -s $orig_dir/daily/sir${source}/e2${lcproj}/FD7${lcltod1}-E2${proj}97-061-061.lis_dump.nc \
	    $root_dir/daily/sir${source}/EASE2_${proj}12.5km.F13_SSMI.1997061.85V.${ltods[1]}.SIR.${source}.v0.1.nc

    done
done
echo "Done."
