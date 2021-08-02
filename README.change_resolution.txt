The measures-byu system only outputs data in EASE grid 2.0
projections

The default is to produce data at a 25 km base resolution - see
the readme in the ref directory to understand how to produce
output at 12.5, 6.25, 3.125 etc km

The system can also produce data with a base resolution of 36 km
or 24 km.  These are achieved with an optional parameter on the
command line when calling meas_meta_make
-r 0  is the default and uses the 25 km base resolution
-r 1  uses 36 km base resolution (and hence 18, 9, 4.5, 2.25 etc)
-r 2  uses 24 km base resolution (and 12, 6 and 3)

Currently this is only used in SMAP processing where for the
NSIDC-0738 dataset we also produce 36, 9 and 3 km resolution
gridded output.  The 24 km output is not used and is discarded,
but is required as a base to get to 3 km.  Also note that the
software ensures for the 24 km base resolution that the
cylindrical grids are M rather than T grids as they go to higher
latitudes than the T grids (which only go to +/- 67 degrees)
