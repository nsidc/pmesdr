#========================================================================
# Makefile for meas_undump utility
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
LIBDIR = $(TOPDIR)/lib

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
CONFIG_CFLAGS = -O3
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CONFIG_CFLAGS = 
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
else ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = $(CURC_NETCDF_INC)
  NETCDF4_LIBDIR = $(CURC_NETCDF_LIB)
  HDF5_INCDIR = $(CURC_HDF5_INC)
  HDF5_LIBDIR = $(CURC_HDF5_LIB)
else ifeq ($(LOCALE),JANUSgcc)
  NETCDF4_INCDIR = $(CURC_NETCDF_INC)
  NETCDF4_LIBDIR = $(CURC_NETCDF_LIB)
  HDF5_INCDIR = $(CURC_HDF5_INC)
  HDF5_LIBDIR = $(CURC_HDF5_LIB)
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
LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(ZLIB_LIBDIR) -L$(MEAS_LIBDIR) \
	-lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
CFLAGS =  -I$(MEAS_INCDIR) -m64 $(CONFIG_CFLAGS) -I$(NETCDF4_INCDIR) -I./
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(MEAS_INCDIR) -I$(NETCDF4_INCDIR) -I./ -O3 -ipo -g -xHost -no-prec-div # -Zp8
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(NETCDF4_INCDIR) -m64 -I$(MEAS_INCDIR) -I./ -g
endif
DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_undump.c netcdf_write_prod.c netcdf_dump.c
OBJS = meas_undump.o netcdf_write_prod.o netcdf_dump.o
HDRS = $(MEAS_INCDIR)/sir3.h
BIN = meas_undump

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

meas_undump.o : meas_undump.c $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -c -o $@ meas_undump.c

netcdf_dump.o : netcdf_dump.c $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -c -o $@ netcdf_dump.c

netcdf_write_prod.o : netcdf_write_prod.c $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -c -o $@ netcdf_write_prod.c

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS) $(BINDIR)/$(BIN)


