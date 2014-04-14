# Makefile for Fortran programs to manipulate SIR files
# written by R. Chen Feb. 2001  JPL PO.DAAC
#
# compile flags should allow for long source lines and initialized
# static variables
#

#*****************************************************************************
# Uncomment next 2 lines if using SUN solaris machine.  Also edit lib/SIR.inc
#*****************************************************************************
#FF             =       f77
#FFLAGS         =       -e -Ilib

#*****************************************************************************
# Uncomment next 2 lines if using SGI machine.  Also edit lib/SIR.inc
#*****************************************************************************
#FF             =       f77
#FFLAGS         =       -col120 -Ilib

#*****************************************************************************
# Uncomment next 2 lines if using HP machine.  Also edit lib/SIR.inc
#*****************************************************************************
#FF             =       f77
#FFLAGS         =       +es -Ilib

#*****************************************************************************
# Uncomment next 2 lines if using HP machine w/f90.  Also edit lib/SIR.inc
#*****************************************************************************
#FF             =       f90
#FFLAGS         =       -K +gformat77 +es -Ilib

#*****************************************************************************
# Uncomment next 2 lines if using AIX.  Also edit lib/SIR.inc
#*****************************************************************************
#FF             =       f77
#FFLAGS         =       -K -Ilib -qfixed=120

#*****************************************************************************
# Uncomment next 2 lines if using g77 on linux.  Also edit lib/SIR.inc
#*****************************************************************************
#FF             =       g77
#FFLAGS         =       -Ilib -ffixed-line-length-none 
#FFLAGS         =       -Ilib -ffixed-line-length-none -m64 -O3 -fomit-frame-pointer  -fpeel-loops -ftracer -funswitch-loops -fno-automatic

#*****************************************************************************
# Uncomment next 2 lines if using Lahery f95 on linux.  Also edit lib/SIR.inc
#*****************************************************************************
FF             =       lf95
##FFLAGS         =       --wide --mldefault cdecl --sav -Ilib
FFLAGS         =       --wide --ml cdecl --sav -Ilib

#*****************************************************************************
# Uncomment next 2 lines if using pathscale on linux.  Also edit lib/SIR.inc
#*****************************************************************************
#FF             =       pathf95
#FFLAGS         =       -Ilib -m64 -extend-source -static-data -O2

#*****************************************************************************
# Uncomment next 2 lines if using intel on linux.  Also edit lib/SIR.inc
#*****************************************************************************
#FF             =       ifort
#FFLAGS         =       -Ilib -m64 -extend-source -static -O2


EXES = fsirexample fsir_locmap sir2byte libfsirf.a viewsirhead fsir_dump \
       sir_extractregion sirdiff sirmask fsir_dump_small \
#       fsirexample_EZ fsir_locmap_EZ

LDFLAGS = -L. -lfsirf

# extra utility code for some programs -- see notes in extra/readme.txt
EXTRA = extra/ireadval1.f extra/readval1.f extra/numeric.f

all : $(EXES)

fsirexample : fsirexample.f libfsirf.a
	$(FF) $(FFLAGS) -ofsirexample fsirexample.f $(LDFLAGS)

fsir_locmap : fsir_locmap.f libfsirf.a
	$(FF) $(FFLAGS) -ofsir_locmap fsir_locmap.f $(LDFLAGS)

sir2byte : sir2byte.f libfsirf.a
	$(FF) $(FFLAGS) -osir2byte sir2byte.f $(LDFLAGS)

viewsirhead : viewsirhead.f libfsirf.a
	$(FF) $(FFLAGS) -oviewsirhead viewsirhead.f $(LDFLAGS)

fsir_dump : fsir_dump.f libfsirf.a
	$(FF) $(FFLAGS) -ofsir_dump fsir_dump.f $(LDFLAGS)

fsir_dump_small : fsir_dump_small.f libfsirf.a
	$(FF) $(FFLAGS) -ofsir_dump_small fsir_dump_small.f $(LDFLAGS)

# note: sir_extractregion, sirdiff, and sirmask use command line arguments.
# These are not supported under all compilers/operating systems.  The code
# can be edited to remove the command line arguments
sir_extractregion : sir_extractregion.f libfsirf.a
	$(FF) $(FFLAGS) -osir_extractregion sir_extractregion.f $(EXTRA) $(LDFLAGS)

sirdiff : sirdiff.f libfsirf.a
	$(FF) $(FFLAGS) -osirdiff sirdiff.f $(EXTRA) $(LDFLAGS)

sirmask : sirmask.f libfsirf.a
	$(FF) $(FFLAGS) -osirmask sirmask.f $(EXTRA) $(LDFLAGS)

# note: fsir_locmap_EZ.f and fsirexample_EZ.f use SIREZ.f routines.  
# Comment out from make script if SIREZ is not included in library.
#fsir_locmap_EZ : fsir_locmap_EZ.f libfsirf.a
#	$(FF) $(FFLAGS) -ofsir_locmap_EZ fsir_locmap_EZ.f $(LDFLAGS)

#fsirexample_EZ : fsirexample_EZ.f libfsirf.a
#	$(FF) $(FFLAGS) -ofsirexample_EZ fsirexample_EZ.f $(LDFLAGS)

# note: the SIREZ.f routines are based on extensions to fortran
# and do not work on many compilers.  Remove if compiler doesn't like.
# for g77 add extra/g77fix.f
LIBSRC = lib/arctand.f lib/easegrid.f lib/f2ipix.f lib/ieasegrid.f \
	lib/ilambert1.f lib/ipolster.f lib/lambert1.f lib/latlon2pix.f \
	lib/pix2latlon.f lib/polster.f lib/length.f lib/length1.f lib/printhead.f \
	lib/printhead3.f lib/readsirf.f lib/readsirhead.f lib/readsirhead3.f \
	lib/swapbuf.f lib/writesir.f lib/writesir3.f lib/sirupdate.f \
	lib/readsirf_block.f #extra/g77fix.f # lib/SIREZ.f

# create an archive library of compiled essential SIR routines
libfsirf.a : $(LIBSRC)
	rm -f *.o libfsirf.a
	$(FF) $(FFLAGS) -c $(LIBSRC)
	ar cr libfsirf.a *.o
	rm -f *.o *.B *.s

clean :
	rm -f $(EXES) *.o *.B *.s


