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

test_file = 'test_data/test.nc'
small_difference_file = 'test_data/test_small_diffs.nc'
big_difference_file = 'test_data/test_big_diffs.nc'

# If compare_cetb_files returns 0, it means the files are the "same"
# If compare_cetb_files returns 1, it means the files are not the "same"
def test_same_file():
    assert_equals( compare_cetb_files( test_file, test_file,
                                       verbose=True ), 
                   0 )

def test_big_difference_file():
    assert_equals( compare_cetb_files( test_file, big_difference_file,
                                       verbose=True ),
                   1 )

def test_small_difference_file():
    assert_equals( compare_cetb_files( test_file, small_difference_file,
                                       verbose=True ),
                   1 )

def test_small_difference_wtolerance_fail():
    assert_equals( compare_cetb_files( test_file, small_difference_file,
                                       verbose=True,
                                       tolerance=0.05 ), 
                   1 )

def test_small_difference_wtolerance_pass():
    assert_equals( compare_cetb_files( test_file, small_difference_file,
                                       verbose=True,
                                       tolerance=0.1 ), 
                   0 )

    





