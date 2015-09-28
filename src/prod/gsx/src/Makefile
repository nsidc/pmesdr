#========================================================================
# Makefile for gsx utility library
#
# 2015-08-15 M. A. Hardman 303-492-2969 mhardman@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2015 Regents of University of Colorado and Brigham-Young University
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

#
#	commands
#
AR = ar
CC = gcc
CD = cd
INSTALL = cp -f -p
LS = ls -1
MKDIR = mkdir -p
RM = rm -fr
CP = cp
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CC = icc
  AR = xiar
endif

#
#       debug or optimization settings
#       these may be things that are machine-specific
#
CONFIG_CFLAGS =

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
else ifeq ($(LOCALE),int.nsidc.org)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib
  ZLIB_LIBDIR = /usr/lib
else ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = $(shell nc-config --includedir)
  NETCDF4_LIBDIR = $(shell nc-config --libs)
  HDF5_INCDIR = /curc/tools/x_86_64/rh6/hdf5/1.8.11/szip/2.1/zlib/1.2.78/jpeglib/8d/intel/13.0.0/include
  HDF5_LIBDIR = /curc/tools/x_86_64/rh6/hdf5/1.8.11/szip/2.1/zlib/1.2.78/jpeglib/8d/intel/13.0.0/lib
  ZLIB_LIBDIR = /curc/tools/x_86_64/rh6/zlib/1.2.7/lib
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = $(shell nc-config --includedir)
  NETCDF4_LIBDIR = $(shell nc-config --libs)
  HDF5_INCDIR = /curc/tools/x_86_64/rh6/hdf5/1.8.13/szip/2.1/zlib/1.2.8/jpeglib/9a/openmpi/1.8.2/gcc/4.9.1/include
  HDF5_LIBDIR = /curc/tools/x_86_64/rh6/hdf5/1.8.13/szip/2.1/zlib/1.2.8/jpeglib/9a/openmpi/1.8.2/gcc/4.9.1/lib
  ZLIB_LIBDIR = /curc/tools/x_86_64/rh6/zlib/1.2.8/gcc/4.9.1/lib
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
CFLAGS = -I$(INCDIR) -m64 $(CONFIG_CFLAGS)
ARFLAGS = ruv
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -D'JANUSicc=1' -I$(INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -g -xHost -no-prec-div # -Zp8
  ARFLAGS = crs
endif
ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) $(CONFIG_CFLAGS) -g
endif
#DEPEND_LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(ZLIB_LIBDIR) -L$(MEAS_LIBDIR) \
	-lgsx -lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
DEPEND_LIBS = 

SRCS = gsx.c
OBJS = gsx.o
HDRS = gsx.h

all : libgsx.a $(LIBDIR)/libgsx.a

$(LIBDIR)/libgsx.a : libgsx.a

libgsx.a : $(OBJS) $(DEPEND_LIBS)
	$(AR) $(ARFLAGS) libgsx.a $(OBJS)
	$(MKDIR) $(LIBDIR) $(INCDIR)
	$(INSTALL) libgsx.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

install : libgsx.a $(HDRS)
	$(MKDIR) $(LIBDIR) $(INCDIR)
	$(INSTALL) libgsx.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

clean :
	- $(RM) libgsx.a $(OBJS)
