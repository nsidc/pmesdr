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
DEPEND_LIBS = 

SRCS = cetb_file.c
OBJS = cetb_file.o
HDRS = cetb_file.h

all : libcetb.a $(LIBDIR)/libcetb.a

$(LIBDIR)/libcetb.a : libcetb.a

libcetb.a : $(OBJS) $(DEPEND_LIBS) $(HDRS)
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
