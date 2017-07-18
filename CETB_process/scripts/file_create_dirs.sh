#!/bin/sh
# file_move.sh
# Created Apr 2017 by mhardman <mhardman@nsidc-mhardman.local>
# $Id$
# $Log$
#
# this little script will takes a source as argument and then creates the required 
# monthly directories
#
# edit as appropriate for the years in the dataset
#
#
SRC=$1
for year in  2008 2009 2010 2011 2012 2013 2014 2015 
do
    echo $year
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_01`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_02`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_03`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_04`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_05`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_06`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_07`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_08`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_09`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_10`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_11`
    `mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_12`

done

year=2016
echo $year
`mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_05`
`mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_06`
`mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_01`
`mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_02`
`mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_03`
`mkdir /scratch/summit/moha2290/${SRC}_sir/${year}_04`


