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
import re
import sys


def compare_cetb_directories( dir1, dir2,
                              exclude_out_of_range=False, statistics=False, tolerance=0,
                              max_diff_pixels=0, verbose=False ):
    """
    Compares 2 CETB output directories by matching up filenames in each one,
    and calling compare_cetb_files on the pairs.
    Returns 0 when dirs compare as the "same", and 1 otherwise.

    dir1: directory
    dir2: directory
    exclude_out_of_range : boolean, default is False
         when this keyword is set, any pixel locations outside the interval
         [ 50.0, 350.0 ] Kelvins are ignored
    statistics : boolean, default is False
         when this keyword is set, verbose image and difference statistics
         are dumped to stderr
    tolerance : numeric, default is 0
         differences smaller than abs(tolerance) will be ignored
    max_diff_pixels : default is 0
         the data will match as long as there are fewer
         than max_diff_pixels different pixels in each file in the directory
    verbose : boolean, default is False
         verbose output
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
    full_list2 = sorted( glob.glob( dir2 + "/*.nc" ) )

    list2 = full_list2

    if len( list1 ) != len( list2 ):
        sys.stderr.write( "\n" + this_program + ": Number of files in the directories differs.\n" )
        return False

    if 0 == len( list1 ):
        sys.stderr.write( "\n" + this_program + ": Empty directories.\n" )
        return False

    p = re.compile(r'(.+_[0-9]{8})(_[0-9]{10})?_v[0-9]+\.[0-9]+\.nc')
    
    all_files_OK = True
    for i in np.arange( len( list1 ) ):

        base1 = os.path.basename( list1[ i ] )
        base2 = os.path.basename( list2[ i ] )
        if not ( base1 == base2 ):
            
            # if filenames don't match exactly, try a relaxed test to
            # see if they only differ by version number and processing date
            # at end of filename
            m1 = p.match( base1 )
            m2 = p.match( base2 )
            if m1 and m2:
                if not ( m1.group(1) == m2.group(1) ):
                    sys.stderr.write(
                        "\n" + this_program + ": Filenames differ: " + \
                        list1[ i ] + " " + list2[ i ] + "\n" )
                    return False
                else:
                    if verbose:
                        sys.stderr.write(
                            "\n> " + this_program + \
                            ": Filenames differ by version number only: " + \
                            list1[ i ] + " " + list2[ i ] + "\n" )
            else: # There is no version number in the filename
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
                        statistics=False, tolerance=0, max_diff_pixels=0,
                        verbose=False ):
    """
    Compared 2 CETB files for pixelwise differences in TB arrays only. Returns
    True if files are identical (or similar within tolerances). Returns False
    if data are significantly different. Note that CETB files are netCDF files,
    with global attributes and several variables. No metadata or variables
    other variables are compared.

    file1: CETB .nc file
    file2: CETB .nc file
    exclude_out_of_range : boolean, default is False
         when this keyword is set, any pixel locations outside the interval
         [ 50.0, 350.0 ] Kelvins are ignored
    statistics : boolean, default is False
         when this keyword is set, verbose image and difference statistics
         are dumped to stderr
    tolerance : numeric, default is 0
         differences smaller than abs(tolerance) will be ignored
    max_diff_pixels : default is 0
         the data will match as long as there are fewer
         than max_diff_pixels different pixels in each file in the directory
    verbose : boolean, default is False
         verbose output
    
    """
    this_program = inspect.stack()[ 0 ][ 3 ]
    if ( verbose ):
        sys.stderr.write( "\n" )
        sys.stderr.write( "> " + this_program + ": file1: " + file1 + "\n" )
        sys.stderr.write( "> " + this_program + ": file2: " + file2 + "\n" )
        sys.stderr.write( "> " + this_program + ": tolerance: " +
                          str( tolerance ) + "\n" )
        sys.stderr.write( "> " + this_program + ": max_diff_pixels: " +
                          str( max_diff_pixels ) + "\n" )
        sys.stderr.write( "> " + this_program + ": exclude_out_of_range: " +
                          str( exclude_out_of_range ) + "\n" )

    # Old cetb nc files can either have "a_image" or "bgi_image" variables (but
    # not both).
    # New cetb cd files have a "TB" variable, which is scaled.
    # Figure out which variable name is in the first file, and
    # then always look for "TB" in the second one
    f1 = Dataset( file1, 'r', 'NETCDF4' )
    keys = list(f1.variables.keys())
    var_name = 'none'
    for key in keys:
        if ( 'a_image' == key ):
            var_name = 'a_image'
            break
        if ( 'bgi_image' == key ):
            var_name = 'bgi_image'
            break
        if ( 'TB' == key ):
            var_name = 'TB'
            break

    if ( 'none' == var_name ):
        sys.stderr.write( "\n" + this_program + ": " + file1 + ": " +
                          "contains neither a_image nor bgi_image.\n" )
        return False
    
    # Open and read the variable TB data from both files
    # Note this cool netCDF4 trick: just printing the Dataset f
    # with "print( f )" will print out a bunch of stuff from the file
    image1 = f1.variables[ var_name ][ : ]

    f2 = Dataset( file2, 'r', 'NETCDF4' )
    image2 = ( f2.variables[ "TB" ][ : ] )

    diff = image2 - image1

    if statistics:
        sys.stderr.write( "\n" )
        dump_image_statistics( file1, image1 )
        dump_image_statistics( file2, image2 )

    image1, image2, diff = filter_images( image1, image2, diff,
                                          exclude_out_of_range )

    if statistics:
        dump_diff_statistics( image1, image2, diff, tolerance,
                              exclude_out_of_range=exclude_out_of_range )

    # If there is an x dimension variable, compare first elements of it
    # FIXME on next regression data replacement:
    # The next time we make new regression data, we will need to change this to
    # compare x to x (instead of cols to x)
    if ('cols' in keys):
        sys.stderr.write( "> " + this_program + ": Comparing x dimension variables..." )
        if (f1.variables["cols"][0] != f2.variables["x"][0]):
            sys.stderr.write( "\n" + this_program + ": cols/x values differ\n" )
            return False
        sys.stderr.write( "OK\n" )

    # If the arrays are equal, we are done
    # If they are not, then check for differences less than |tolerance|
    # Finally check for fewer than max_diff_pixels different
    if ( np.array_equal( image1, image2 ) ):
        if ( verbose ):
            sys.stderr.write( "> " + this_program + ": " + var_name +
                              " data are identical.\n" )
        return True
    else:
        absdiff = abs( diff )
        if ( np.max( absdiff ) <= abs( tolerance ) ):
            if ( verbose ):
                sys.stderr.write( "> " + this_program + ": " + var_name +
                                  " data are within tolerance.\n" )
            return True
        elif ( (len( absdiff[ absdiff > abs( tolerance ) ] )) <= max_diff_pixels ):
            if ( verbose ):
                sys.stderr.write( "> " + this_program + ": " + var_name +
                                  " fewer than " + str( max_diff_pixels ) +
                                  " are different.\n")
            return True
        else:
            sys.stderr.write( "\n" + this_program + ": " + var_name +
                              " data differ.\n" )
            return False


def dump_image_statistics( filename, image ):
    """
    Dumps statistics on this image to stderr as a record of the form:
    filename: min=min, max=max, mean=mean, stddev=stddev

    filename : string filename to print
    image : ndarray to examine
    
    """
    try:
        sys.stderr.write('{0}:\n\tmin={1:8.4f} max={2:8.4f} mean={3:8.4f} std={4:8.4f}\n'
                         .format(filename, np.amin(image), np.amax(image), np.mean(image), np.std(image)))
    except ValueError as e:
        # I'll bet there's a better way to do this, but when the array is all fill-values,
        # the np.min/max etc functions return a string which makes the formatting {1:8.4f} to float
        # break...
        sys.stderr.write('{0}:\n\tmin={1:8} max={2:8} mean={3:8} std={4:8}\n'
                         .format(filename, np.amin(image), np.amax(image), np.mean(image), np.std(image)))

    return


def dump_diff_statistics( filtered_image1, filtered_image2, filtered_diff,
                          tolerance, exclude_out_of_range=False ):
    """
    Dumps statistics on difference image to stderr as a record of the form:
    difference: min=min, max=max, mean=mean, stddev=stddev

    filtered_image1 : ndarray of first image
    filtered_image2 : ndarray of second image
    filtered_diff : ndarray of difference
    tolerance : numeric, differences smaller than abs(tolerance) will be ignored
    exclude_out_of_range : boolean, default is False
         when this keyword is set, any pixel locations outside the interval
         [ 50.0, 350.0 ] Kelvins are ignored
    
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

    try:
        sys.stderr.write('{0}:\n\tmin={1:8.4f} max={2:8.4f} mean={3:8.4f} std={4:8.4f} num[|diff|>{5:.6f}]={6:8d} {7}\n'
                         .format("difference", np.amin( my_diff ), np.amax( my_diff ),
                                 np.mean( my_diff ), np.std( my_diff ), tolerance, num_diffs, label))
    except ValueError as e:
        sys.stderr.write('{0}:\n\tmin={1:8} max={2:8} mean={3:8} std={4:8} num[|diff|>{5:.6f}]={6:8d} {7}\n'
                         .format("difference", np.amin( my_diff ), np.amax( my_diff ),
                                 np.mean( my_diff ), np.std( my_diff ), tolerance, num_diffs, label))

    if ( 0 < num_diffs ):
        my_diff = my_diff[ absdiff > tolerance ]
        my_image1 = my_image1[ absdiff > tolerance ]
        my_image2 = my_image2[ absdiff > tolerance ]
        for i in np.arange( num_diffs ):
            try:
                sys.stderr.write( '{0:d}\tdiff={1:8.4f} img1={2:8.4f} img2={3:8.4f}\n'
                              .format( i, my_diff[ i ], my_image1[ i ], my_image2[ i ] ) )
            except ValueError as e:
                sys.stderr.write( '{0:d}\tdiff={1:8} img1={2:8} img2={3:8}\n'
                              .format( i, my_diff[ i ], my_image1[ i ], my_image2[ i ] ) )

    return


def filter_images( image1, image2, diff, exclude_out_of_range=False, verbose=False ):
    """
    If exclude_out_of_range is set, filters images and difference arrays to only
    the locations of pixels in the expected range and ignores any out of range
    values

    image1 : ndarray of first image
    image2 : ndarray of second image
    diff : ndarray of difference
    exclude_out_of_range : boolean, default is False
         when this keyword is set, any pixel locations outside the interval
         [ 50.0, 350.0 ] Kelvins will be ignored in all three images
    verbose : boolean, default is False

    """

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









