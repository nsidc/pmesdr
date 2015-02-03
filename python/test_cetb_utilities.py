'''
Nose tests for cetb_utilities

To run tests : nosetests    test_cetb_utilities.py
Verbose (-v) : nosetests -v test_cetb_utilities.py

2015-01-16 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
National Snow & Ice Data Center, University of Colorado, Boulder
Copyright (C) 2015 Regents of University of Colorado and Brigham-Young University

'''
from nose.tools import assert_equals
from cetb_utilities import compare_cetb_files
from cetb_utilities import compare_cetb_directories

test_file = 'test_cetb_utilities_data/test_data/test.nc'
small_difference_file = 'test_cetb_utilities_data/test_data/test_small_diffs.nc'
big_difference_file = 'test_cetb_utilities_data/test_data/test_big_diffs.nc'
bgi_file = 'test_cetb_utilities_data/bgi_data/test_bgi.nc'
bgi_diffs_file = 'test_cetb_utilities_data/bgi_diff_data/test_bgi_diffs.nc'

dir_orig = 'test_cetb_utilities_data/test_data'
dir_copy = 'test_cetb_utilities_data/test_data_copy'
dir_one_file = 'test_cetb_utilities_data/test_data_2vars'
dir_small_diffs = 'test_cetb_utilities_data/test_data_small_diffs'
dir_big_diffs = 'test_cetb_utilities_data/test_data_big_diffs'
dir_3files_1diff = 'test_cetb_utilities_data/test_data_3files_1different'
dir_3files_1diff_name = 'test_cetb_utilities_data/test_data_3files_1different_name'

verbose = False

# Unit tests for compare_directories
# If compare_cetb_files returns True, it means the files are the "same"
# If compare_cetb_files returns False, it means the files are not the "same"
def test_same_file():
    assert_equals( compare_cetb_files( test_file, test_file,
                                       verbose=verbose ), 
                   True )

def test_big_difference_file():
    assert_equals( compare_cetb_files( test_file, big_difference_file,
                                       verbose=verbose ),
                   False )

def test_small_difference_file():
    assert_equals( compare_cetb_files( test_file, small_difference_file,
                                       verbose=verbose ),
                   False )

def test_small_difference_wtolerance_fail():
    assert_equals( compare_cetb_files( test_file, small_difference_file,
                                       verbose=verbose,
                                       tolerance=0.05 ), 
                   False )

def test_small_difference_wtolerance_pass():
    assert_equals( compare_cetb_files( test_file, small_difference_file,
                                       verbose=verbose,
                                       tolerance=0.1 ), 
                   True )

def test_statistics_output():
    assert_equals( compare_cetb_files( test_file, small_difference_file,
                                       statistics=True,
                                       tolerance=0.1 ), 
                   True )

def test_bgi_file():
    assert_equals( compare_cetb_files( bgi_file, bgi_file ),
                   True )

def test_bgi_diff_files():
    assert_equals( compare_cetb_files( bgi_file, bgi_diffs_file,
                                       exclude_out_of_range=True, 
                                       statistics=True ),
                   True )
    
# **************************************************************************************   
# Unit tests for compare_directories
#
def test_nonexistent_directories():
    assert_equals( compare_cetb_directories( "bogusdir1", "bogusdir2", 
                                             verbose=verbose ),
                   False )

def test_empty_directories():
    assert_equals( compare_cetb_directories( "test_cetb_utilities_data/test_empty1",
                                             "test_cetb_utilities_data/test_empty2",
                                             verbose=verbose ),
                   False )
    
def test_compare_directories():
    assert_equals( compare_cetb_directories( dir_orig, dir_copy,
                                             verbose=verbose ),
                   True )
                   
def test_compare_different_directories():
    assert_equals( compare_cetb_directories( dir_orig, dir_one_file,
                                             verbose=verbose ),
                   False )

def test_compare_dirs_wtolerance_pass():
    assert_equals( compare_cetb_directories( dir_one_file, dir_small_diffs,
                                             verbose=verbose,
                                             tolerance=0.1 ), 
                   True )

def test_compare_dirs_wtolerance_fail():
    assert_equals( compare_cetb_directories( dir_one_file, dir_small_diffs,
                                             verbose=verbose,
                                             statistics=True,
                                             tolerance=0.05 ), 
                   False )

def test_compare_dirs_3files_1different():
    assert_equals( compare_cetb_directories( dir_orig, dir_3files_1diff,
                                             statistics=True,
                                             verbose=verbose ),
                   False )

def test_compare_dirs_3files_1different_name():
    assert_equals( compare_cetb_directories( dir_orig, dir_3files_1diff_name,
                                             verbose=verbose ),
                   False )

def test_dir_statistics():
    assert_equals( compare_cetb_directories( dir_orig, dir_3files_1diff_name,
                                             statistics=True, 
                                             verbose=verbose ),
                   False )
    
    

    





