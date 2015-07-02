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

def compare_cetb_directories( dir1, dir2,
                              exclude_out_of_range=False, statistics=False, tolerance=0,
                              max_diff_pixels=0, verbose=False ):
    """
    Compares 2 CETB output directories, by matching up filenames in each one,
    and calling compare_cetb_files on the pairs.
    Returns 0 when dirs compare as the "same", and 1 otherwise.
    
    exclude_out_of_range : default is False
         when this keyword is set, any pixel locations outside the interval
         [ 50.0, 350.0 ] Kelvins are ignored.

    max_diff_pixels : default is 0
         when this keyword is set the data will match as long as there are fewer
         than max_diff_pixels different pixels in each file in the directory
    
    """
    this_program = inspect.stack()[ 0 ][ 3 ]
    if ( verbose ):
        sys.stderr.write( "\n" )
        sys.stderr.write( "> " + this_program + ": dir1: " + dir1 + "\n" )
        sys.stderr.write( "> " + this_program + ": dir2: " + dir2 + "\n" )
        sys.stderr.write( "> " + this_program + ": tolerance: " + str( tolerance ) + "\n" )
        sys.stderr.write( "> " + this_program + ": exclude_out_of_range: " + str( exclude_out_of_range ) + "\n" )
        sys.stderr.write( "> " + this_program + ": max_diff_pixels: " + str( max_diff_pixels ) + "\n" )

    tolerance = float( tolerance )
    max_diff_pixels = int( max_diff_pixels )

    if not os.path.isdir( dir1 ) or not os.path.isdir( dir2 ):
        sys.stderr.write( "\n" + this_program + ": One or both directories not found.\n" )
        return False

    # Read the files in dir1 and the files in dir2
    list1 = sorted( glob.glob( dir1 + "/*.nc" ) )
    list2 = sorted( glob.glob( dir2 + "/*.nc" ) )

    # For now, we will not tolerate any extra files in our directories.
    # If we have the same number of files in each directory, we will
    # just compare them, in order.  (We may need to make this smarter later.)
    if len( list1 ) != len( list2 ):
        sys.stderr.write( "\n" + this_program + ": Number of files in the directories differs.\n" )
        return False

    if 0 == len( list1 ):
        sys.stderr.write( "\n" + this_program + ": Empty directories.\n" )
        return False

    all_files_OK = True
    for i in np.arange( len( list1 ) ):

        if not ( os.path.basename( list1[ i ] ) == os.path.basename( list2[ i ] ) ):
            sys.stderr.write( "> " + this_program + ": Filenames differ: " + list1[ i ] + " " + list2[ i ] + "\n" )
            return False
            
        if not compare_cetb_files( list1[ i ], list2[ i ],
                                   exclude_out_of_range=exclude_out_of_range,
                                   statistics=statistics, tolerance=tolerance,
                                   max_diff_pixels=max_diff_pixels, verbose=verbose ):
            sys.stderr.write( "\n" + this_program + ": Files differ:\n\t" + list1[ i ] + "\n\t" + list2[ i ] + "\n" )
            all_files_OK = False

    if not all_files_OK:
        sys.stderr.write( "\n" + this_program + ": Directories differ.\n" )
    else:
        sys.stderr.write( "\n" + this_program + ": All files match.\n" )

    return all_files_OK
    
def compare_cetb_files( file1, file2, exclude_out_of_range=False,
                        statistics=False, tolerance=0, max_diff_pixels=0, verbose=False ):
    """
    status = compare_cetb_files( file1, file2,
                                 exclude_out_of_range=False,
                                 statistics=False,
                                 tolerance=0,
                                 max_diff_pixels=0,
                                 verbose=False )
    
    Compares 2 CETB files.
    
    CETB files are netCDF files, with global attributes and a number of variable arrays.
    Eventually we will compare all the pieces of the two files, displaying any differences.
    For now, we will just compare the data array contained in:
      variable a_image (used in sir files) or bgi_image (used in bgi files).
    Returns True when files compare as the "same", and False otherwise.

    exclude_out_of_range : default is False
         when this keyword is set, any pixel locations outside the interval
         [ 50.0, 350.0 ] Kelvins are ignored.
    
    max_diff_pixels : default is 0
         when this keyword is set the data will match as long as there are fewer
         than max_diff_pixels different pixels in each file in the directory
    
    """
    this_program = inspect.stack()[ 0 ][ 3 ]
    if ( verbose ):
        sys.stderr.write( "\n" )
        sys.stderr.write( "> " + this_program + ": file1: " + file1 + "\n" )
        sys.stderr.write( "> " + this_program + ": file2: " + file2 + "\n" )
        sys.stderr.write( "> " + this_program + ": tolerance: " + str( tolerance ) + "\n" )
        sys.stderr.write( "> " + this_program + ": max_diff_pixels: " + str( max_diff_pixels ) + "\n" )
        sys.stderr.write( "> " + this_program + ": exclude_out_of_range: " + str( exclude_out_of_range ) + "\n" )

    # cetb nc files can either have "a_image" or "bgi_image" variables (but
    # not both).  Figure out which variable name is in the first file, and
    # then only look for that variable in the 2nd file.
    f1 = Dataset( file1, 'r', 'NETCDF4' )
    keys = f1.variables.keys()
    var_name = 'none'
    for key in keys:
        if ( 'a_image' == key ):
            var_name = 'a_image'
            break
        if ( 'bgi_image' == key ):
            var_name = 'bgi_image'
            break

    if ( 'none' == var_name ):
        sys.stderr.write( "\n" + this_program + ": " + file1 + ": " + "contains neither a_image nor bgi_image.\n" )
        return False
    
    # Open and read the variable data from both files
    # Note this cool netCDF4 trick: just printing the Dataset f
    # with "print( f )" will print out a bunch of stuff from the file
    image1 = f1.variables[ var_name ][ : ]

    f2 = Dataset( file2, 'r', 'NETCDF4' )
    image2 = f2.variables[ var_name ][ : ]

    diff = image2 - image1

    if statistics:
        sys.stderr.write( "\n" )
        dump_image_statistics( file1, image1 )
        dump_image_statistics( file2, image2 )

    image1, image2, diff = filter_images( image1, image2, diff, exclude_out_of_range )

    if statistics:
        dump_diff_statistics( image1, image2, diff, tolerance, exclude_out_of_range=exclude_out_of_range )
        
    # If the arrays are equal, we are done
    # If they are not, then check for differences less than |tolerance|
    # Finally check for fewer than max_diff_pixels different
    if ( np.array_equal( image1, image2 ) ):
        if ( verbose ):
            sys.stderr.write( "> " + this_program + ": " + var_name + " data are identical.\n" )
        return True
    else:
        absdiff = abs( diff )
        if ( np.max( absdiff ) <= abs( tolerance ) ):
            if ( verbose ):
                sys.stderr.write( "> " + this_program + ": " + var_name + " data are within tolerance.\n" )
            return True
        elif ( (len( absdiff[ absdiff > abs( tolerance ) ] )) <= max_diff_pixels ):
            if ( verbose ):
                sys.stderr.write( "> " + this_program + ": " + var_name + " fewer than "
                                  + str( max_diff_pixels ) + " are different.\n")
            return True
        else:
            sys.stderr.write( "\n" + this_program + ": " + var_name + " data differ.\n" )
            return False

def dump_image_statistics( filename, image ):
    """
    Dumps statistics on this image to stderr:
    filename: min, max, mean, stddev
    """
    sys.stderr.write( '{0}:\n\tmin={1:8.4f} max={2:8.4f} mean={3:8.4f} std={4:8.4f}\n'
                      .format( filename, np.min( image ), np.max( image ), np.mean( image ), np.std( image ) ) )
    
    return

def dump_diff_statistics( filtered_image1, filtered_image2, filtered_diff, tolerance, exclude_out_of_range=False ):
    """
    Dumps statistics on difference image to stderr:
    diff: min, max, mean, stddev
    """
    my_image1 = filtered_image1
    my_image2 = filtered_image2
    my_diff = filtered_diff
    if ( exclude_out_of_range ):
        label = "(inside(50,350))"
    else:
        label = ""
        
    absdiff = abs( my_diff )
    num_diffs = len( absdiff[ absdiff > tolerance ] )

    sys.stderr.write( '{0}:\n\tmin={1:8.4f} max={2:8.4f} mean={3:8.4f} std={4:8.4f} num[|diff|>{5:.6f}]={6:8d} {7}\n'
                      .format( "difference", np.min( my_diff ), np.max( my_diff ),
                               np.mean( my_diff ), np.std( my_diff ), tolerance, num_diffs, label ) )

    if ( 0 < num_diffs ):
        my_diff = my_diff[ absdiff > tolerance ]
        my_image1 = my_image1[ absdiff > tolerance ]
        my_image2 = my_image2[ absdiff > tolerance ]
        for i in np.arange( num_diffs ):
            sys.stderr.write( '{0:d}\tdiff={1:8.4f} img1={2:8.4f} img2={3:8.4f}\n'
                              .format( i, my_diff[ i ], my_image1[ i ], my_image2[ i ] ) )
    
    return


def filter_images( image1, image2, diff, exclude_out_of_range=False, verbose=False ):

    if ( exclude_out_of_range ):
        if ( verbose ):
            sys.stderr.write( "> Excluding out of range values...\n" )
        min_TB = 50.0
        max_TB = 350.0
        idx = ( min_TB < image1 ) & ( image1 < max_TB ) & ( min_TB < image2 ) & ( image2 < max_TB )
        diff = diff[ idx ]
        image1 = image1[ idx ]
        image2 = image2[ idx ]

    return image1, image2, diff
    









