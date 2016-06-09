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
else ifeq ($(LOCALE),NSIDCdev)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),int.nsidc.org)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib
  HDF5_LIBDIR = /usr/lib
else ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
else
  $(warning Assuming default LOCALE=BYU)
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
endif

#
# end configuration section
#------------------------------------------------------------------------
LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) \
	-lutils -lcetb -lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) $(CONFIG_CFLAGS) -Wall -Wno-unused-variable -Wno-unused-but-set-variable
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) -O3 -ipo -g -xHost -no-prec-div -Wall -Wremarks -wd981
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) -g
endif
DEPEND_LIBS = $(LIBDIR)/libcetb.a 

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

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

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

amsre_ease :
	$(RM) $(TESTDIR)/sirAMSRE
	$(MKDIR) $(TESTDIR)/sirAMSRE
ifeq ($(AMSREFILES),)
	$(error	No setup files found)
else
	for file in $(AMSREFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirAMSRE; \
	done
endif

amsre_quick :
	$(RM) $(TESTDIR)/sirAMSRE
	$(MKDIR) $(TESTDIR)/sirAMSRE
ifeq ($(AMSRE2NFILES),)
	$(error	No setup files found)
else
	for file in $(AMSRE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirAMSRE; \
	done
endif


