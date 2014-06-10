
#========================================================================
# Makefile for meas_combine_segs utility
#
# 2014-05-30 D.G. Long, adapted from code by
# M. J. Brodzik 303-492-8263 brodzik@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2014 Regents of University of Colorado and Brigham-Young University
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
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),NSIDCdev)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else
  $(warning Assuming default LOCALE=BYU)
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
endif

#
# end configuration section
#------------------------------------------------------------------------
MEAS_INCDIR = ../../sirlib/c/include
MEAS_LIBDIR = ../../sirlib/c
MEAS_DUMPDIR = ../meas_undump

LIBS = -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(ZLIB_LIBDIR) -L$(MEAS_LIBDIR) \
	-lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
CFLAGS =  -I$(MEAS_INCDIR) -I$(MEAS_DUMPDIR) -m64 $(CONFIG_CFLAGS) -I$(NETCDF4_INCDIR)
DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_combine_segs.c $(MEAS_DUMPDIR)/netcdf_dump.c
OBJS = meas_combine_segs.o $(MEAS_DUMPDIR)/netcdf_dump.o
HDRS = $(MEAS_INCDIR)/sir3.h $(MEAS_DUMPDIR)/ezdump.h
BIN = meas_combine_segs

RSSNPSFILES = $(shell find $(TESTDIR)/setupRSS/nps -type f -name "F*Arc*.setup")
RSSE2NFILES = $(shell find $(TESTDIR)/setupRSS/e2n -type f -name "F*E2N*.setup")
CSUNPSFILES = $(shell find $(TESTDIR)/setupCSU/nps -type f -name "F*Arc*.setup")
CSUE2NFILES = $(shell find $(TESTDIR)/setupCSU/e2n -type f -name "F*E2N*.setup")

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

meas_combine_segs.o : meas_combine_segs.c $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -c -o $@ meas_combine_segs.c

netcdf_dump.o : $(MEAS_DUMPDIR)/netcdf_dump.c $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -c -o $@ $(MEAS_DUMPDIR)/netcdf_dump.c

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

