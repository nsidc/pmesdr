#!/bin/sh
# F08_make.sh
# Created Jan 30 2016 by Molly Hardman <mhardman@nsidc-driftice.ad.int.nsidc.org>
# $Id$
# $Log$
#
# Lines starting with #PBS are treated by bash as comments, but interpreted by qsub
# as arguments.  

#
# Set the name of the job
#SBATCH -J F08_make

#
# Set a walltime for the job. The time format is HH:MM:SS - this is set via an environment variable in
# the calling script, based on the channel number being processed


# Set the output file and embed the job number in the filename
#SBATCH -o output/amsre_make-%j.out

# Select the janus QOS 
#SBATCH --qos=janus-debug
#SBATCH --time=10:00
#
# Set the system up to notify upon completion
#SBATCH --mail-type=ALL
#SBATCH --mail-user=mhardman@nsidc.org
YEAR=$1
SRC=F08
#source /projects/moha2290/summit/measures-byu/src/prod/summit_set_pmesdr_environment.sh
TOPDIR=$PMESDR_TOP_DIR
BINDIR=$TOPDIR/bin
OUTDIR=/scratch/summit/moha2290/F08_make/
FINDIR=/scratch/summit/moha2290/prototype-output
#
#
# run meas_meta_make with specific parameters
#
date
echo $YEAR
echo $2
echo $3
for DOY in `seq $2 $3`
do
    DOYM1=$(( $DOY - 1 ))
#    echo $DOYM1
    DAY=`date -d "$YEAR-01-01 + $DOYM1 days" +%d`
#    echo $DAY
#    echo `$DOY - 1`
    MONTH=`date -d "$YEAR-01-01 + $DOYM1 days" +%m`
#    echo $MONTH
    DOYA=$( printf "%03g" $DOY )
#    echo $DOYA
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.19-22.NS.meta F08 $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi19-22hv_NS.def \
	    /scratch/summit/moha2290/F08_lists/F08.$YEAR$MONTH$DAY.NS" >> F08_make_list_19-22
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.19-22.T.meta F08 $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi19-22hv_T.def \
	    /scratch/summit/moha2290/F08_lists/F08.$YEAR$MONTH$DAY" >> F08_make_list_19-22
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37-85.NS.meta F08 $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi37-85hv_NS.def \
	    /scratch/summit/moha2290/F08_lists/F08.$YEAR$MONTH$DAY.NS" >> F08_make_list_37-85
    echo "$BINDIR/meas_meta_make $OUTDIR/$SRC.$DOYA.${YEAR}.37-85.T.meta F08 $DOYA $DOYA $YEAR $TOPDIR/ref/ssmi37-85hv_T.def \
	    /scratch/summit/moha2290/F08_lists/F08.$YEAR$MONTH$DAY" >> F08_make_list_37-85
#    echo " done with day ${DOY} "
done
#
date
