#!/bin/sh
OPTIND=1
usage() {
    echo "" 1>&2
    echo "Usage: $0 [-a -n -t top_level -s [AMSRE/AMSR2]"
    echo "  Rips through all a directory of input swath files for either"
    echo "  AMSRE or AMSR2 and creates a file to be run in gnu_parallel or via loadbalancer"
    echo "  and convert from .nc.partial from input swath to GSX format files."
    echo "Arguments:"
    echo "   -a - will run the archive combine strategy"
    echo "   -n - will run the near real time combine strategy"
    echo "   -t allows optional top_level directory specification"
    echo "   -s is AMSRE or AMSR2 sensor source of data"
    echo "   - only need the src arg for choosing the correct directories"
    echo "  top_level is optional"
    echo "" 1>&2
}

error_exit() {
    # Use for fatal program error
    # Argument:
    #   optional string containing descriptive error message
    #   if no error message, prints "Unknown Error"

    echo "${PROGNAME}: ERROR: ${1:-"Unknown Error"}" 1>&2
    return 1 
}


top_level=""
src=""
app=""

while getopts "ant:s:h" opt; do
    case $opt in
	a) app="combine_amsr_l1c_jaxa_archive";;
	n) app="combine_amsr_l1c_jaxa";;
	t) top_level=$OPTARG;;
	s) src=$OPTARG;;
	h) usage
	   return;;
	?) printf "Usage: %s: [-antsh] args\n" $0
           return;;
	esac
done

if [[ -n "$app" && -n "$src" ]]
then

  direc=/scratch/alpine/${USER}/${top_level}/
  for file in `find ${direc}/${src}-L1C_GSX -name "*.nc.partial"`
  do
      echo "${app} $file ${direc}/${src}-JAXA_GSX ${direc}/${src}_GSX" >> ${direc}/${src}_scripts/gsx_lb_list_alpine
  done

else 
    echo "Must specify either -a or -n for archive or near real time combo"
    echo "Must give specify src with -s argument to indicate output directory"
fi
