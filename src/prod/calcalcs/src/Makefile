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
ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
  CC = icc
  AR = xiar
  ARFLAGS = crs
endif

.SUFFIXES:
.SUFFIXES: .o .c

UDUNITS2_BASE_DIR = /usr
UDUNITS2_INC_DIR  = $(UDUNITS2_BASE_DIR)/include
UDUNITS2_LIB_DIR  = $(UDUNITS2_BASE_DIR)/lib

################################################################################

.c.o:
	$(CC) -c -I$(UDUNITS2_INC_DIR) $< 


################################################################################
# summit configuration is TBD
ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'SUMMITicc=1' -I$(INCDIR) -I$(UDUNITS_INCDIR) \
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

