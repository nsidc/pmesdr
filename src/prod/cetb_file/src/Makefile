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
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CC = icc
  AR = xiar
endif
ifeq ($(LOCALE),CUMULUSicc)
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
ifeq ($(LOCALE),int.nsidc.org)
  $(info Build location: $(LOCALE))
  NETCDF4_LIBDIR = /usr/lib
  HDF5_LIBDIR = /usr/lib
  ZLIB_LIBDIR = /usr/lib
  UDUNITS_INCDIR = /usr/include
else ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else ifeq ($(LOCALE),CUMULUSicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib/x86_64-linux-gnu
  HDF5_INCDIR = /usr/include/hdf5/serial
  HDF5_LIBDIR = /usr/lib/x86_64-linux-gnu
  UDUNITS_INCDIR = /usr/local/include
  UDUNITS_LIBDIR = /usr/local/lib
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
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'ALPINEicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) \
	-I$(UDUNITS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -xHost \
	-no-prec-div -Wall -Wremarks -wd981 -wd869 -wd2415
  ARFLAGS = crs
endif
ifeq ($(LOCALE),CUMULUSicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'CUMULUSicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) \
	-I$(UDUNITS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -xHost \
	-no-prec-div -Wall -Wremarks -wd981 -wd869 -wd2415
  ARFLAGS = crs
endif
DEPEND_HDRS = $(INCDIR)/calcalcs.h $(INCDIR)/utCalendar2_cal.h $(INCDIR)/cetb.h 
DEPEND_LIBS = $(LIBDIR)/libcalcalcs.a

SRCS = cetb_file.c
OBJS = cetb_file.o
HDRS = cetb_file.h

all : libcetb.a $(LIBDIR)/libcetb.a

$(LIBDIR)/libcetb.a : libcetb.a

libcetb.a : $(OBJS) $(DEPEND_LIBS) $(HDRS) $(DEPEND_HDRS)
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
