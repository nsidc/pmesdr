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
    print "compare_cetb_directories [-h] [-s] [-t tolerance] [-v] dir1 dir2"
    print "compare_cetb_directories [--help] [--statistics] [--tolerance=tolerance] [--verbose] dir1 dir2"
    print "  compares files in each directory"
    print "  -h : print usage message"
    print "  -s : print file/difference statistics to stderr"
    print "  -t tolerance : to within tolerance (default=0.0)"
    print "  -v : verbose output"

def main( argv ):

    statistics = False
    tolerance = 0.0
    verbose = False
    
    try:
        opts, args = getopt.getopt( argv, "hst:v", [ "tolerance=" ] )
    except getopt.GetoptError:
        usage()
        sys.exit( 2 )

    for opt, arg in opts:
        if opt in ( "-h", "--help"):
            usage()
            sys.exit( 2 )
        elif opt in ( "-s", "--statistics" ):
            statistics = True
        elif opt in ( "-t", "--tolerance" ):
            tolerance = arg
        elif opt in ( "-v", "--verbose" ):
            verbose = True

    if 2 != len( args ):
        print "Requires 2 directories."
        usage()
        sys.exit( 2 )

    if ( compare_cetb_directories( args[ 0 ], args[ 1 ],
                                   verbose=verbose,
                                   statistics=statistics,
                                   tolerance=tolerance ) ):
        sys.exit( 0 )
    else:
        sys.exit( 1 )
    

if __name__ == "__main__":
    main( sys.argv[ 1: ] )
    


    

    





