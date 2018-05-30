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
  $(info Build location: $(LOCALE))
  CC = icc
  AR = xiar
endif
ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
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
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),int.nsidc.org)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib
  ZLIB_LIBDIR = /usr/lib
else ifeq ($(LOCALE),JANUSicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
else ifeq ($(LOCALE),JANUSgcc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
else ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
else ifeq ($(LOCALE),SUMMITgcc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
else
  $(info Assuming default LOCALE=BYU)
  NETCDF4_INCDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/include
  NETCDF4_LIBDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/lib
  HDF5_INCDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/include
  HDF5_LIBDIR = /auto/users/long/src/linux/netcdf/hdf5-1.8.3/lib
  ZLIB_LIBDIR = /auto/users/long/src/linux/netcdf/zlib-1.2.3/lib
endif

#
# end configuration section
#------------------------------------------------------------------------
CFLAGS = -I$(INCDIR) -m64 $(CONFIG_CFLAGS) -Wall -Wno-unused-variable
ARFLAGS = ruv
ifeq ($(LOCALE),JANUSicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'JANUSicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -xHost -no-prec-div -Wall -Wremarks -wd981 -wd2415
  ARFLAGS = crs
endif
ifeq ($(LOCALE),JANUSgcc)
  $(info Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) $(CONFIG_CFLAGS) -Wall
endif
ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
#  CFLAGS = -D'SUMMITicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -xHost -no-prec-div -Wall -Wremarks -wd981 -wd2415
  CFLAGS = -D'SUMMITicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) $(CONFIG_CFLAGS) -g -ipo -xHost -no-prec-div -Wall -Wremarks -wd981 -wd2415
  ARFLAGS = crs
endif
ifeq ($(LOCALE),SUMMITgcc)
  $(info Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) $(CONFIG_CFLAGS) -Wall
endif


#DEPEND_LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(ZLIB_LIBDIR) -L$(MEAS_LIBDIR) \
	-lgsx -lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
DEPEND_LIBS = 

SRCS = gsx.c
OBJS = gsx.o
HDRS = gsx.h

all : libgsx.a $(LIBDIR)/libgsx.a

$(LIBDIR)/libgsx.a : libgsx.a

libgsx.a : $(OBJS) $(DEPEND_LIBS) $(HDRS) $(INCDIR)/cetb.h 
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
