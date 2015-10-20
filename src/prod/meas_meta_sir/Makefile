#========================================================================
# Makefile for meas_meta_sir utility
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
INCDIR = $(TOPDIR)/include
LIBDIR = $(TOPDIR)/lib
BINDIR = $(TOPDIR)/bin
TESTDIR = $(TOPDIR)/NSIDCtest
ifeq ($(LOCALE),int.nsidc.org)
  TESTDIR = /projects/PMESDR/vagrant/NSIDCtest/$(shell hostname)
endif
COMPAREDIR = $(PMESDR_REGRESS_DIR)
COMPARE_TOLERANCE = $(PMESDR_COMPARE_TOLERANCE)
MAX_DIFF_PIXELS = $(PMESDR_MAX_DIFF_PIXELS)

#
#	commands
#
CC = gcc
INSTALL = cp -f -p
MKDIR = mkdir -p
PYTHON = python
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
else ifeq ($(LOCALE),int.nsidc.org)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib
  HDF5_LIBDIR = /usr/lib
  ZLIB_LIBDIR = /usr/lib
else ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
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
LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(ZLIB_LIBDIR) -L$(MEAS_LIBDIR) \
	-lcetb -lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
CFLAGS = -I$(INCDIR) -I$(MEAS_INCDIR) -I../meas_undump $(CONFIG_CFLAGS)
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) -I$(MEAS_INCDIR) -I../meas_undump -O3 -ipo -g -xHost -no-prec-div # -Zp8
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) -I$(MEAS_INCDIR) -I../meas_undump -g
endif
DEPEND_LIBS = $(LIBDIR)/libcetb.a $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_meta_sir.c ../meas_undump/netcdf_dump.c
OBJS = meas_meta_sir.o netcdf_dump.o
HDRS = $(INCDIR)/cetb_file.h $(MEAS_INCDIR)/sir3.h ../meas_undump/ezdump.h
BIN = meas_meta_sir

RSSFILES = $(wildcard $(TESTDIR)/setupRSS/e2*/F*E2*.setup)
RSSE2NFILES = $(wildcard $(TESTDIR)/setupRSS/e2n/F*E2N*.setup)
CSUFILES = $(wildcard $(TESTDIR)/setupCSU/e2*/F*E2*.setup)
CSUE2NFILES = $(wildcard $(TESTDIR)/setupCSU/e2n/F*E2N*.setup)

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

netcdf_dump.o : ../meas_undump/netcdf_dump.c 
	$(CC) $(CFLAGS) -c -o $@ ../meas_undump/netcdf_dump.c

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS) $(BINDIR)/$(BIN)

rss_ease :
	$(RM) $(TESTDIR)/sirRSS
	$(MKDIR) $(TESTDIR)/sirRSS
ifeq ($(RSSFILES),)
	$(error	No setup files found)
else
	for file in $(RSSFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS; \
	done
endif

rss_ease_validate :
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/daily/sirRSS $(TESTDIR)/sirRSS

rss_quick :
	$(RM) $(TESTDIR)/sirRSS
	$(MKDIR) $(TESTDIR)/sirRSS
ifeq ($(RSSE2NFILES),)
	$(error	No setup files found)
else
	for file in $(RSSE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS; \
	done
endif

rss_quick_validate :
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/quick/sirRSS $(TESTDIR)/sirRSS

csu_ease :
	$(RM) $(TESTDIR)/sirCSU
	$(MKDIR) $(TESTDIR)/sirCSU
ifeq ($(CSUFILES),)
	$(error	No setup files found)
else
	for file in $(CSUFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirCSU; \
	done
endif

csu_ease_validate :
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/daily/sirCSU $(TESTDIR)/sirCSU

csu_quick :
	$(RM) $(TESTDIR)/sirCSU
	$(MKDIR) $(TESTDIR)/sirCSU
ifeq ($(CSUE2NFILES),)
	$(error	No setup files found)
else
	for file in $(CSUE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirCSU; \
	done
endif

csu_quick_validate :
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/quick/sirCSU $(TESTDIR)/sirCSU



