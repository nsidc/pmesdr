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

RSSE2SFILES = $(wildcard $(TESTDIR)/setupRSS/e2s/F*E2S*.setup)
RSSE2NFILES = $(wildcard $(TESTDIR)/setupRSS/e2n/F*E2N*.setup)
RSSE2TFILES = $(wildcard $(TESTDIR)/setupRSS/e2t/F*E2T*.setup)
CSUE2SFILES = $(wildcard $(TESTDIR)/setupCSU/e2s/F*E2S*.setup)
CSUE2NFILES = $(wildcard $(TESTDIR)/setupCSU/e2n/F*E2N*.setup)
CSUE2TFILES = $(wildcard $(TESTDIR)/setupCSU/e2t/F*E2T*.setup)

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
	$(RM) $(TESTDIR)/sirRSS/e2n
	$(MKDIR) $(TESTDIR)/sirRSS/e2n
ifeq ($(RSSE2NFILES),)
	$(error	No setup files found)
else
	for file in $(RSSE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS/e2n; \
	done
endif
	$(RM) $(TESTDIR)/sirRSS/e2s
	$(MKDIR) $(TESTDIR)/sirRSS/e2s
ifeq ($(RSSE2SFILES),)
	$(error	No setup files found)
else
	for file in $(RSSE2SFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS/e2s; \
	done
endif
	$(RM) $(TESTDIR)/sirRSS/e2t
	$(MKDIR) $(TESTDIR)/sirRSS/e2t
ifeq ($(RSSE2TFILES),)
	$(error	No setup files found)
else
	for file in $(RSSE2TFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS/e2t; \
	done
endif

rss_ease_validate :
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/daily/sirRSS/e2n $(TESTDIR)/sirRSS/e2n
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/daily/sirRSS/e2s $(TESTDIR)/sirRSS/e2s
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/daily/sirRSS/e2t $(TESTDIR)/sirRSS/e2t

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
	$(RM) $(TESTDIR)/sirCSU/e2n
	$(MKDIR) $(TESTDIR)/sirCSU/e2n
ifeq ($(CSUE2NFILES),)
	$(error	No setup files found)
else
	for file in $(CSUE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirCSU/e2n; \
	done
endif
	$(RM) $(TESTDIR)/sirCSU/e2s
	$(MKDIR) $(TESTDIR)/sirCSU/e2s
ifeq ($(CSUE2SFILES),)
	$(error	No setup files found)
else
	for file in $(CSUE2SFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirCSU/e2s; \
	done
endif
	$(RM) $(TESTDIR)/sirCSU/e2t
	$(MKDIR) $(TESTDIR)/sirCSU/e2t
ifeq ($(CSUE2TFILES),)
	$(error	No setup files found)
else
	for file in $(CSUE2TFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirCSU/e2t; \
	done
endif

csu_ease_validate :
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/daily/sirCSU/e2n $(TESTDIR)/sirCSU/e2n
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/daily/sirCSU/e2s $(TESTDIR)/sirCSU/e2s
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/daily/sirCSU/e2t $(TESTDIR)/sirCSU/e2t

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

