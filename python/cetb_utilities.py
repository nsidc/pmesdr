'''
CETB utilities

Utility functions for the Passive Microwave ESDR project data, known as
Calibrated EASE-Grid 2.0 TB (CETB) data.

2015-01-16 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
National Snow & Ice Data Center, University of Colorado, Boulder
Copyright (C) 2015 Regents of University of Colorado and Brigham-Young University

'''

from netCDF4 import Dataset
import inspect
import numpy as np
import glob

def compare_cetb_directories( dir1, dir2, verbose=False, tolerance=0 ):
    """
    Compares 2 CETB output directories, by matching up filenames in each one,
    and calling compare_cetb_files on the pairs.
    Returns 0 when dirs compare as the "same", and 1 otherwise.
    """
    if ( verbose ):
        this_program = inspect.stack()[ 0 ][ 3 ]
        print "\n"
        print "> " + this_program + ": dir1: " + dir1
        print "> " + this_program + ": dir2: " + dir2
        print "> " + this_program + ": tolerance: " + str( tolerance )

    # Read the files in dir1 and the files in dir2
    list1 = sorted( glob.glob( dir1 + "/*.nc" ) )
    list2 = sorted( glob.glob( dir2 + "/*.nc" ) )

    # For now, we will not tolerate any extra files in our directories.
    # If we have the same number of files in each directory, we will
    # just compare them, in order.  (We may need to make this smarter later.)
    if len( list1 ) != len( list2 ):
        return False

    for i in np.arange( len( list1 ) ):
        if not compare_cetb_files( list1[ i ], list2[ i ], verbose=verbose ):
            print "These files aren't the same! "
            return False

    if verbose:
        print "All files match."
        
    return True
    
def compare_cetb_files( file1, file2, verbose=False, tolerance=0 ):
    """
    Compares 2 CETB files.
    CETB files are netCDF files, with global attributes and a number of variable arrays.
    Eventually we will compare all the pieces of the two files, displaying any differences.
    For now, we will just compare the data array contained in variable a_image.
    Returns True when files compare as the "same", and False otherwise.
    """
    if ( verbose ):
        this_program = inspect.stack()[ 0 ][ 3 ]
        print "\n"
        print "> " + this_program + ": file1: " + file1
        print "> " + this_program + ": file2: " + file2
        print "> " + this_program + ": tolerance: " + str( tolerance )

    # Open and read the a_image data from both files
    # Note this cool netCDF4 trick: just printing the Dataset f
    # with "print( f )" will print out a bunch of stuff from the file
    f1 = Dataset( file1, 'r', 'NETCDF4' )
    image1 = f1.variables[ 'a_image' ][ : ]

    f2 = Dataset( file2, 'r', 'NETCDF4' )
    image2 = f2.variables[ 'a_image' ][ : ]

    # If the arrays are equal, we are done
    # If they are not, then check for differences less than |tolerance|
    if ( np.array_equal( image1, image2 ) ):
        return True
    else:
        diff = abs( image2 - image1 )
        if ( np.max( diff ) <= abs( tolerance ) ):
            return True
        else:
            return False












