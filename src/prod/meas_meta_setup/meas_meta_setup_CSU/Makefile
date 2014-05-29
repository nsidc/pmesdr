#========================================================================
# Makefile for meas_meta_setup_CSU utility
#
# 2014-04-14 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
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
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else
  $(warning Assuming default LOCALE=BYU)
  NETCDF4_INCDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/include
  NETCDF4_LIBDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/lib
  HDF5_INCDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/include
  HDF5_LIBDIR = /auto/users/long/src/linux/netcdf/hdf5-1.8.3/lib
  ZLIB_LIBDIR = /auto/users/long/src/linux/netcdf/zlib-1.2.3/lib
endif

#
# end configuration section
#------------------------------------------------------------------------
MEAS_INCDIR = ../../../sirlib/c/include
MEAS_LIBDIR = ../../../sirlib/c
LIBS = -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(ZLIB_LIBDIR) -L$(MEAS_LIBDIR) \
	-lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
CFLAGS = -D'CSU=1' -I$(NETCDF4_INCDIR) -I$(HDF5_INCDIR) -I$(MEAS_INCDIR) -m64 $(CONFIG_CFLAGS)
DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = ../meas_meta_setup.c
OBJS = meas_meta_setup_CSU.o
HDRS = $(MEAS_INCDIR)/sir3.h
BIN = meas_meta_setup_CSU

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

meas_meta_setup_CSU.o : $(SRCS) $(HDRS)
	$(CC) -c $(CFLAGS) -o $@ $(SRCS)

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS)

csu_nps :
	$(RM) $(TESTDIR)/setupCSU/nps
	$(MKDIR) $(TESTDIR)/setupCSU/nps
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_nps/npstest_CSU.meta  $(TESTDIR)/setupCSU/nps 0

csu_e2n :
	$(RM) $(TESTDIR)/setupCSU/e2n
	$(MKDIR) $(TESTDIR)/setupCSU/e2n
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta  $(TESTDIR)/setupCSU/e2n 0


