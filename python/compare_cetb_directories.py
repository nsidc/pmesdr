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
    print "compare_cetb_directories [-h] [-t tolerance] dir1 dir2"
    print "compare_cetb_directories [--help] [--tolerance=tolerance] dir1 dir2"
    print "  compares files in each directory"
    print "  -h : print usage message"
    print "  -t tolerance : to within tolerance (default=0.0)"

def main( argv ):

    tolerance = 0.0
    
    try:
        opts, args = getopt.getopt( argv, "ht:", [ "tolerance="] )
    except getopt.GetoptError:
        usage()
        sys.exit( 2 )

    for opt, arg in opts:
        if opt in ( "-h", "--help"):
            usage()
            sys.exit( 2 )
        elif opt in ( "-t", "--tolerance" ):
            tolerance = arg

    if 2 != len( args ):
        print "Requires 2 directories."
        usage()
        sys.exit( 2 )

    if ( compare_cetb_directories( args[ 0 ], args[ 1 ],
                                   verbose=True,
                                   tolerance=tolerance ) ):
        sys.exit( 0 )
    else:
        sys.exit( 1 )
    

if __name__ == "__main__":
    main( sys.argv[ 1: ] )
    


    

    





