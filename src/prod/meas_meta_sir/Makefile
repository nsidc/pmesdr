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
TESTDIR = $(PMESDR_TEST_OUT_DIR)

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
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CC = icc
endif

#
#       debug or optimization settings
#       these may be things that are machine-specific
#
CONFIG_CFLAGS = -m64 -O3
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CONFIG_FLAGS = 
endif

#
#	system libraries
#
SYSLIBS = -lm

#
#	system libraries and include directories
#
ifeq ($(LOCALE),int.nsidc.org)
  $(info Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib
  HDF5_LIBDIR = /usr/lib
  HDFLIB1 = hdf5_serial_hl
  HDFLIB2 = hdf5_serial
else ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  HDFLIB1 = hdf5_hl
  HDFLIB2 = hdf5
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
  UDUNITS_LIBDIR = ${CURC_UDUNITS_LIB}
else
  $(info Assuming default LOCALE, mileage may vary)
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
endif

#
# end configuration section
#------------------------------------------------------------------------
LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) \
	-lutils -lcetb -lcalcalcs -ludunits2 -lcsir -lnetcdf \
	-l$(HDFLIB1) -l$(HDFLIB2) -lz $(SYSLIBS) 
CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) $(CONFIG_CFLAGS) \
	-Wall -Wno-unused-variable -Wno-unused-but-set-variable
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(UDUNITS_LIBDIR) \
	-lutils -lcetb -lcalcalcs -ludunits2 -lcsir -lnetcdf \
	-lhdf5_hl -lhdf5 -lz $(SYSLIBS) 
  CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(UDUNITS_INCDIR) \
	-O3 -ipo -xHost -no-prec-div -Wall -Wremarks -wd981 -wd2415
endif
DEPEND_LIBS = $(LIBDIR)/libcetb.a $(LIBDIR)/libcalcalcs.a \
	$(LIBDIR)/libutils.a

SRCS = meas_meta_sir.c 
OBJS = meas_meta_sir.o 
HDRS = $(INCDIR)/cetb.h $(INCDIR)/cetb_file.h
BIN = meas_meta_sir

RSSFILES = $(wildcard $(TESTDIR)/setupRSS/e2*/F*E2*.setup)
RSSE2NFILES = $(wildcard $(TESTDIR)/setupRSS/e2n/F*E2N*.setup)
CSUFILES = $(wildcard $(TESTDIR)/setupCSU/e2*/F*E2*.setup)
CSUE2NFILES = $(wildcard $(TESTDIR)/setupCSU/e2n/F*E2N*.setup)
AMSREFILES = $(wildcard $(TESTDIR)/setupAMSRE/e2*/A*E2*.setup)
AMSRE2NFILES = $(wildcard $(TESTDIR)/setupAMSRE/e2n/A*E2N*.setup)
SSMISFILES = $(wildcard $(TESTDIR)/setupSSMIS/e2*/I*E2*.setup)
SSMISE2NFILES = $(wildcard $(TESTDIR)/setupSSMIS/e2n/I*E2N*.setup)
SMMRFILES = $(wildcard $(TESTDIR)/setupSMMR/e2*/R*E2*.setup)
SMMRE2NFILES = $(wildcard $(TESTDIR)/setupSMMR/e2n/R*E2N*.setup)
SMAPFILES = $(wildcard $(TESTDIR)/setupSMAP/e2*/S*E2*.setup)

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS) $(BINDIR)/$(BIN)

#
#------------------------------------------------------------------------
# Begin test targets section
#
# csu_quick is same as csu_ease, they end up running on whatever setup files
# they find in the input directory, so these targets don't need to be different
# 
# csu_ease:
#   input
#       $(TESTDIR)/setupCSU/e2*/F*E2*.setup
#   output to (this directory is wiped at beginning of test):
#	$(TESTDIR)/sirCSU
#	
# csu_quick_validate:
#   compares contents of $(TESTDIR)/sirCSU to
#   contents of $(COMPAREDIR)/quick/sirCSU
#
# csu__validate:
#   compares contents of $(TESTDIR)/sirCSU to
#   contents of $(COMPAREDIR)/daily/sirCSU
#
csu_quick :
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

csu_quick_validate :
	$(PYTHON) $(TOPDIR)/python/compare_cetb_directories.py \
		-t $(COMPARE_TOLERANCE) -m $(MAX_DIFF_PIXELS) \
                $(COMPAREDIR)/quick/sirCSU $(TESTDIR)/sirCSU

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
