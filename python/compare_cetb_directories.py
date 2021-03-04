'''
Command-line script to compare cetb files in two directories

2015-01-26 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
National Snow & Ice Data Center, University of Colorado, Boulder
Copyright (C) 2015 Regents of University of Colorado and Brigham-Young University

'''
from cetb_utilities import compare_cetb_directories
import sys
import getopt

def usage():
    print("compare_cetb_directories [-e ] [-h] [-s] [-t tolerance] [-m max_diff_pixels] [-v] dir1 dir2")
    print("compare_cetb_directories [--exclude_out_of_range] [--help] [--statistics] \
    [--tolerance=tolerance] [--max_diff_pixels=max_diff_pixels] [--verbose] dir1 dir2")
    print("  compares files in each directory")
    print("  -e : exclude out-of-range [50.,350.] temperature values")
    print("  -h : print usage message")
    print("  -s : print file/difference statistics to stderr")
    print("  -t tolerance : to within tolerance (default=0.0)")
    print("  -m max_diff_pixels : no more than max_diff_pixels different per file (default=0)")
    print("  -v : verbose output")

def main( argv ):

    exclude_out_of_range = False
    statistics = False
    tolerance = 0.0
    verbose = False
    max_diff_pixels = 0
    
    try:
        opts, args = getopt.getopt( argv, "ehst:m:v", [ "tolerance=", "max_diff_pixels=" ] )
    except getopt.GetoptError:
        usage()
        sys.exit( 2 )

    for opt, arg in opts:
        if opt in ( "-e", "--exclude_out_of_range" ):
            exclude_out_of_range = True
        elif opt in ( "-h", "--help"):
            usage()
            sys.exit( 2 )
        elif opt in ( "-s", "--statistics" ):
            statistics = True
        elif opt in ( "-t", "--tolerance" ):
            tolerance = arg
        elif opt in ( "-m", "--max_diff_pixels" ):
            max_diff_pixels = arg
        elif opt in ( "-v", "--verbose" ):
            verbose = True

    if 2 != len( args ):
        print("Requires 2 directories.")
        usage()
        sys.exit( 2 )

    if ( compare_cetb_directories( args[ 0 ], args[ 1 ],
                                   exclude_out_of_range=exclude_out_of_range, 
                                   verbose=verbose,
                                   statistics=statistics,
                                   tolerance=tolerance,
                                   max_diff_pixels=max_diff_pixels) ):
        sys.exit( 0 )
    else:
        sys.exit( 1 )
    

if __name__ == "__main__":
    main( sys.argv[ 1: ] )
    


    

    





