#========================================================================
# Makefile for utils utility library
#
# 2015-07-06 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
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
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),NSIDCdev)
  $(info Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),int.nsidc.org)
  $(info Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib
  HDF5_LIBDIR = /usr/lib
  ZLIB_LIBDIR = /usr/lib
else ifeq ($(LOCALE),JANUSicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_LIB}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else ifeq ($(LOCALE),JANUSgcc)
  $(info Build location: $(LOCALE))
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_LIB}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else ifeq ($(LOCALE),SUMMITgcc)
  $(info Build location: $(LOCALE))
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else
  $(info Assuming default LOCALE=BYU)
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
endif

#
# end configuration section
#------------------------------------------------------------------------
CFLAGS = -I$(INCDIR) -m64 $(CONFIG_CFLAGS) -Wall -Wno-unused-variable
ARFLAGS = ruv
ifeq ($(LOCALE),JANUSicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'JANUSicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(UDUNITS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -g -xHost -no-prec-div -Wall -Wremarks -wd981 -wd2415
  ARFLAGS = crs
endif
ifeq ($(LOCALE),JANUSgcc)
  $(info Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) $(NETCDF4_INCDIR) -I$(UDUNITS_INCDIR) $(CONFIG_CFLAGS) -g -Wall
endif
ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'SUMMITicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(UDUNITS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -g -xHost -no-prec-div -Wall -Wremarks -wd981 -wd2415
  ARFLAGS = crs
endif
ifeq ($(LOCALE),SUMMITgcc)
  $(info Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) $(NETCDF4_INCDIR) -I$(UDUNITS_INCDIR) $(CONFIG_CFLAGS) -g -Wall
endif

DEPEND_LIBS = 

SRCS = utils.c
OBJS = utils.o
HDRS = utils.h

all : libutils.a $(LIBDIR)/libutils.a

$(LIBDIR)/libutils.a : libutils.a

libutils.a : $(OBJS) $(DEPEND_LIBS) $(HDRS)
	$(AR) $(ARFLAGS) libutils.a $(OBJS)
	$(MKDIR) $(LIBDIR) $(INCDIR)
	$(INSTALL) libutils.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

install : libutils.a $(HDRS)
	$(MKDIR) $(LIBDIR) $(INCDIR)
	$(INSTALL) libutils.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

clean :
	- $(RM) libutils.a $(OBJS)
