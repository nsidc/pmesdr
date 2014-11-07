#========================================================================
# Makefile for meas_meta_bgi utility
#
# 2014-04-14 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2014 Regents of University of Colorado and Brigham-Young University
#
# modified 2 Aug 2014 by DGL at BYU + netcdf_dump
#========================================================================

#------------------------------------------------------------------------
# configuration section
#
#	installation directories
#
TOPDIR = $(PMESDR_TOP_DIR)
BINDIR = $(TOPDIR)/bin
TESTDIR = $(TOPDIR)/NSIDCtest

#
#	commands
#
CC = gcc
INSTALL = cp -f -p
MKDIR = mkdir -p
RM = rm -fr
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CC = icc
endif

#
#       debug or optimization settings
#       these may be things that are machine-specific
#
CONFIG_CFLAGS = -m64 -O3
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CONFIG_FLAGS = 
endif
ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CC = gcc
endif

#
#	system libraries
#
SYSLIBS = -lm

#
#	system libraries and include directories
#
ifeq ($(LOCALE),NSIDCsnow)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),NSIDCdev)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = $(shell nc-config --libs)
  HDF5_LIBDIR = /curc/tools/x_86_64/rh6/hdf5/1.8.11/szip/2.1/zlib/1.2.78/jpeglib/8d/intel/13.0.0/lib
  ZLIB_LIBDIR = /curc/tools/x_86_64/rh6/zlib/1.2.7/lib
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = $(shell nc-config --libs)
  HDF5_LIBDIR = /curc/tools/x_86_64/rh6/hdf5/1.8.13/szip/2.1/zlib/1.2.8/jpeglib/9a/openmpi/1.8.2/gcc/4.9.1/lib
  ZLIB_LIBDIR = /curc/tools/x_86_64/rh6/zlib/1.2.8/gcc/4.9.1/lib
else
  $(warning Assuming default LOCALE=BYU)
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
endif

#
# end configuration section
#------------------------------------------------------------------------
MEAS_INCDIR = ../../sirlib/c/include
MEAS_LIBDIR = ../../sirlib/c
LIBS = -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(ZLIB_LIBDIR) -L$(MEAS_LIBDIR) \
	-lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
CFLAGS = -I$(MEAS_INCDIR) -I../meas_undump $(CONFIG_CFLAGS)
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(MEAS_INCDIR) -I../meas_undump -O3 -ipo -g # -xHost -no-prec-div -Zp8
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(MEAS_INCDIR) -I../meas_undump 
endif
DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_meta_bgi.c ../meas_undump/netcdf_dump.c
OBJS = meas_meta_bgi.o netcdf_dump.o
HDRS = nr.h nrutil.h $(MEAS_INCDIR)/sir3.h ../meas_undump/ezdump.h
BIN = meas_meta_bgi

RSSE2SFILES = $(shell find $(TESTDIR)/setupRSS/e2s -type f -name "F*E2S*.setup")
RSSE2NFILES = $(shell find $(TESTDIR)/setupRSS/e2n -type f -name "F*E2N*.setup")
RSSE2TFILES = $(shell find $(TESTDIR)/setupRSS/e2t -type f -name "F*E2T*.setup")
CSUE2SFILES = $(shell find $(TESTDIR)/setupCSU/e2s -type f -name "F*E2S*.setup")
CSUE2NFILES = $(shell find $(TESTDIR)/setupCSU/e2n -type f -name "F*E2N*.setup")
CSUE2TFILES = $(shell find $(TESTDIR)/setupCSU/e2t -type f -name "F*E2T*.setup")

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

netcdf_dump.o : ../meas_undump/netcdf_dump.c 
	$(CC) $(CFLAGS) -c -o $@ ../meas_undump/netcdf_dump.c

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS)

rss_ease :
	$(RM) $(TESTDIR)/bgiRSS/e2n
	$(MKDIR) $(TESTDIR)/bgiRSS/e2n
	for file in $(RSSE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiRSS/e2n; \
	done
	$(RM) $(TESTDIR)/bgiRSS/e2s
	$(MKDIR) $(TESTDIR)/bgiRSS/e2s
	for file in $(RSSE2SFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiRSS/e2s; \
	done
	$(RM) $(TESTDIR)/bgiRSS/e2t
	$(MKDIR) $(TESTDIR)/bgiRSS/e2t
	for file in $(RSSE2TFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiRSS/e2t; \
	done

csu_ease :
	$(RM) $(TESTDIR)/bgiCSU/e2n
	$(MKDIR) $(TESTDIR)/bgiCSU/e2n
	for file in $(CSUE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiCSU/e2n; \
	done
	$(RM) $(TESTDIR)/bgiCSU/e2s
	$(MKDIR) $(TESTDIR)/bgiCSU/e2s
	for file in $(CSUE2SFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiCSU/e2s; \
	done
	$(RM) $(TESTDIR)/bgiCSU/e2t
	$(MKDIR) $(TESTDIR)/bgiCSU/e2t
	for file in $(CSUE2TFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiCSU/e2t; \
	done



