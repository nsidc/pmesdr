#========================================================================
# Makefile for calcalcs utility library
#
# Modified from original makefile in distribution, to create a linkable object
# with the .o file, and to install .h/.o files in PMESDR top directories
#
# 2018-02-21 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2018 Regents of University of Colorado and Brigham-Young University
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
CC = gcc -O3 -Wall
CD = cd
INSTALL = cp -f -p
LS = ls -1
MKDIR = mkdir -p
RM = rm -fr
CP = cp
ARFLAGS = ruv
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CC = icc
  AR = xiar
  ARFLAGS = crs
endif
ifeq ($(LOCALE),CUMULUSicc)
  $(info Build location: $(LOCALE))
  CC = icc
  AR = xiar
  ARFLAGS = crs
endif

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

.SUFFIXES:
.SUFFIXES: .o .c

################################################################################

.c.o:
	$(CC) -c -I$(UDUNITS_INCDIR) $< 


################################################################################
CFLAGS = -I$(INCDIR) -m64 $(CONFIG_CFLAGS) -Wall -Wno-unused-variable

# alpine or CUMULUS configuration
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'ALPINEicc=1' -I$(INCDIR) -I$(UDUNITS_INCDIR) \
	$(CONFIG_CFLAGS) -O3 -ipo -xHost -no-prec-div -Wall \
	-Wremarks -wd981 -wd869 -wd2415
endif
ifeq ($(LOCALE),CUMULUSicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'CUMULUSicc=1' -I$(INCDIR) -I$(UDUNITS_INCDIR) \
	$(CONFIG_CFLAGS) -O3 -ipo -xHost -no-prec-div -Wall \
	-Wremarks -wd981 -wd869 -wd2415
endif

SRCS = calcalcs.c utCalendar2_cal.c
OBJS = calcalcs.o utCalendar2_cal.o
HDRS = calcalcs.h utCalendar2_cal.h

all : libcalcalcs.a $(LIBDIR)/libcalcalcs.a

$(LIBDIR)/libcalcalcs.a : libcalcalcs.a

libcalcalcs.a : $(OBJS) $(HDRS)
	$(AR) $(ARFLAGS) libcalcalcs.a $(OBJS)
	$(MKDIR) $(LIBDIR) $(INCDIR)
	$(INSTALL) libcalcalcs.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

install : libcalcalcs.a $(HDRS)
	$(MKDIR) $(LIBDIR) $(INCDIR)
	$(INSTALL) libcalcalcs.a $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

clean :
	- $(RM) libcalcalcs.a $(OBJS)

