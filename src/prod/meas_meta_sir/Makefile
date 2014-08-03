
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
BINDIR = $(TOPDIR)/bin
TESTDIR = $(TOPDIR)/NSIDCtest
#
#	commands
#
CC = gcc
INSTALL = cp -f -p
MKDIR = mkdir -p
RM = rm -fr

#
#       debug or optimization settings
#       these may be things that are machine-specific
#
CONFIG_CFLAGS = -O3

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
CFLAGS =  -I$(MEAS_INCDIR) -I../meas_undump -m64 $(CONFIG_CFLAGS)
DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_meta_sir.c ../meas_undump/netcdf_dump.c
OBJS = meas_meta_sir.o netcdf_dump.o
HDRS = $(MEAS_INCDIR)/sir3.h ../meas_undump/ezdump.h
BIN = meas_meta_sir

RSSNPSFILES = $(shell find $(TESTDIR)/setupRSS/nps -type f -name "F*Arc*.setup")
RSSE2NFILES = $(shell find $(TESTDIR)/setupRSS/e2n -type f -name "F*E2N*.setup")
CSUNPSFILES = $(shell find $(TESTDIR)/setupCSU/nps -type f -name "F*Arc*.setup")
CSUE2NFILES = $(shell find $(TESTDIR)/setupCSU/e2n -type f -name "F*E2N*.setup")

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

rss_nps :
	$(RM) $(TESTDIR)/sirRSS/nps
	$(MKDIR) $(TESTDIR)/sirRSS/nps
	for file in $(RSSNPSFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS/nps; \
	done

rss_e2n :
	$(RM) $(TESTDIR)/sirRSS/e2n
	$(MKDIR) $(TESTDIR)/sirRSS/e2n
	for file in $(RSSE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS/e2n; \
	done

csu_nps :
	$(RM) $(TESTDIR)/sirCSU/nps
	$(MKDIR) $(TESTDIR)/sirCSU/nps
	for file in $(CSUNPSFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirCSU/nps; \
	done

csu_e2n :
	$(RM) $(TESTDIR)/sirCSU/e2n
	$(MKDIR) $(TESTDIR)/sirCSU/e2n
	for file in $(CSUE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirCSU/e2n; \
	done

