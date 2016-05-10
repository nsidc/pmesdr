#========================================================================
# Makefile for cetb_file utility library
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
  $(warning Build location: $(LOCALE))
  CC = icc
  AR = xiar
endif

CALCALCS_DIR = $(TOPDIR)/src/prod/cetb_file/src/calcalcs_1.0
CALCALCS_INCDIR = $(CALCALCS_DIR)

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
  NETCDF4_INCDIR = ${CURC_NETCDF_LIB}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else
  $(warning Assuming default LOCALE=BYU)
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
endif

#
# end configuration section
#------------------------------------------------------------------------
CFLAGS = -I$(CALCALCS_INCDIR) -I$(INCDIR) -m64 $(CONFIG_CFLAGS) -Wall
ARFLAGS = ruv
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -D'JANUSicc=1' -I$(CALCALCS_INCDIR) -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(UDUNITS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -g -xHost -no-prec-div -Wall -Wremarks -wd981
  ARFLAGS = crs
endif
ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(CALCALCS_INCDIR) -I$(INCDIR) $(NETCDF4_INCDIR) -I$(UDUNITS_INCDIR) $(CONFIG_CFLAGS) -g -Wall
endif
DEPEND_LIBS = 

SRCS = cetb_file.c $(CALCALCS_DIR)/calcalcs.c
OBJS = cetb_file.o $(CALCALCS_DIR)/calcalcs.o
HDRS = cetb_file.h $(CALCALCS_DIR)/calcalcs.h

all : libcetb.a $(LIBDIR)/libcetb.a

$(LIBDIR)/libcetb.a : libcetb.a

libcetb.a : $(OBJS) $(DEPEND_LIBS) $(HDRS) $(INCDIR)/cetb.h 
	$(AR) $(ARFLAGS) libcetb.a $(OBJS)
	$(MKDIR) $(LIBDIR) $(INCDIR)
	$(INSTALL) libcetb.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

install : libcetb.a $(HDRS)
	$(MKDIR) $(LIBDIR) $(INCDIR)
	$(INSTALL) libcetb.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

clean :
	- $(RM) libcetb.a $(OBJS)
