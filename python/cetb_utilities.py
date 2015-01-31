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
import os
import sys

def compare_cetb_directories( dir1, dir2, statistics=False, tolerance=0, verbose=False ):
    """
    Compares 2 CETB output directories, by matching up filenames in each one,
    and calling compare_cetb_files on the pairs.
    Returns 0 when dirs compare as the "same", and 1 otherwise.
    """
    this_program = inspect.stack()[ 0 ][ 3 ]
    if ( verbose ):
        sys.stderr.write( "\n" )
        sys.stderr.write( "> " + this_program + ": dir1: " + dir1 + "\n" )
        sys.stderr.write( "> " + this_program + ": dir2: " + dir2 + "\n" )
        sys.stderr.write( "> " + this_program + ": tolerance: " + str( tolerance ) + "\n" )

    tolerance = float( tolerance )

    if not os.path.isdir( dir1 ) or not os.path.isdir( dir2 ):
        if ( verbose ):
            sys.stderr.write( "> " + this_program + ": One or both directories not found.\n" )
        return False

    # Read the files in dir1 and the files in dir2
    list1 = sorted( glob.glob( dir1 + "/*.nc" ) )
    list2 = sorted( glob.glob( dir2 + "/*.nc" ) )

    # For now, we will not tolerate any extra files in our directories.
    # If we have the same number of files in each directory, we will
    # just compare them, in order.  (We may need to make this smarter later.)
    if len( list1 ) != len( list2 ):
        if ( verbose ):
            sys.stderr.write( "> " + this_program + ": Number of files in the directories differs.\n" )
        return False

    if 0 == len( list1 ):
        if ( verbose ):
            sys.stderr.write( "> " + this_program + ": Empty directories.\n" )
        return False

    all_files_OK = True
    for i in np.arange( len( list1 ) ):

        if not ( os.path.basename( list1[ i ] ) == os.path.basename( list2[ i ] ) ):
            if ( verbose ):
                sys.stderr.write( "> " + this_program + ": Filenames differ: " + list1[ i ] + " " + list2[ i ] + "\n" )
            return False
            
        if not compare_cetb_files( list1[ i ], list2[ i ], statistics=statistics, tolerance=tolerance, verbose=verbose ):
            sys.stderr.write( "\n" + this_program + ": Files differ:\n\t" + list1[ i ] + "\n\t" + list2[ i ] + "\n" )
            all_files_OK = False

    if verbose:
        if not all_files_OK:
            sys.stderr.write( "> " + this_program + ": Directories differ.\n" )
        else:
            sys.stderr.write( "> " + this_program + ": All files match.\n" )

    return all_files_OK
    
def compare_cetb_files( file1, file2, statistics=False, tolerance=0, verbose=False ):
    """
    Compares 2 CETB files.
    CETB files are netCDF files, with global attributes and a number of variable arrays.
    Eventually we will compare all the pieces of the two files, displaying any differences.
    For now, we will just compare the data array contained in variable a_image.
    Returns True when files compare as the "same", and False otherwise.
    """
    this_program = inspect.stack()[ 0 ][ 3 ]
    if ( verbose ):
        sys.stderr.write( "\n" )
        sys.stderr.write( "> " + this_program + ": file1: " + file1 + "\n" )
        sys.stderr.write( "> " + this_program + ": file2: " + file2 + "\n" )
        sys.stderr.write( "> " + this_program + ": tolerance: " + str( tolerance ) + "\n" )

    # Open and read the a_image data from both files
    # Note this cool netCDF4 trick: just printing the Dataset f
    # with "print( f )" will print out a bunch of stuff from the file
    f1 = Dataset( file1, 'r', 'NETCDF4' )
    image1 = f1.variables[ 'a_image' ][ : ]

    f2 = Dataset( file2, 'r', 'NETCDF4' )
    image2 = f2.variables[ 'a_image' ][ : ]
    
    diff = image2 - image1

    if statistics:
        sys.stderr.write( "\n" )
        dump_image_statistics( file1, image1 )
        dump_image_statistics( file2, image2 )
        dump_diff_statistics( diff, tolerance )
        
    # If the arrays are equal, we are done
    # If they are not, then check for differences less than |tolerance|
    if ( np.array_equal( image1, image2 ) ):
        if ( verbose ):
            sys.stderr.write( "> " + this_program + ": a_image data are identical.\n" )
        return True
    else:
        absdiff = abs( diff )
        if ( np.max( absdiff ) <= abs( tolerance ) ):
            if ( verbose ):
                sys.stderr.write( "> " + this_program + ": a_image data are within tolerance.\n" )
            return True
        else:
            sys.stderr.write( "\n" + this_program + ": a_image data differ.\n" )
            return False

def dump_image_statistics( filename, image ):
    """
    Dumps statistics on this image to stderr:
    filename: min, max, mean, stddev
    """
    sys.stderr.write( '{0}:\n\tmin={1:6.2f} max={2:6.2f} mean={3:6.2f} std={4:6.2f}\n'
                      .format( filename, np.min( image ), np.max( image ), np.mean( image ), np.std( image ) ) )
    
    return

def dump_diff_statistics( diff, tolerance ):
    """
    Dumps statistics on difference image to stderr:
    diff: min, max, mean, stddev
    """
    absdiff = abs( diff )
    num_diffs = len( absdiff[ absdiff > tolerance ] )

    sys.stderr.write( '{0}:\n\tmin={1:6.2f} max={2:6.2f} mean={3:6.2f} std={4:6.2f} num[|diff|>{5:.6f}]={6:8d}\n'
                      .format( "difference", np.min( diff ), np.max( diff ), np.mean( diff ), np.std( diff ), tolerance, num_diffs ) )
    
    return










