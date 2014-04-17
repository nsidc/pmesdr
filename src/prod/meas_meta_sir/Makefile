#========================================================================
# Makefile for meas_meta_sir utility
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
TESTDIR = $(TOPDIR)/test
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
# end configuration section
#------------------------------------------------------------------------
MEAS_INCDIR = ../../sirlib/c/include
MEAS_LIBDIR = ../../sirlib/c
LIBS = -L$(MEAS_LIBDIR) -lcsir $(SYSLIBS)
CFLAGS = -I$(MEAS_INCDIR) -m64 $(CONFIG_CFLAGS)
DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_meta_sir.c
OBJS = meas_meta_sir.o
HDRS = $(MEAS_INCDIR)/sir3.h
BIN = meas_meta_sir

NPSFILES = $(shell find $(TESTDIR)/setupRSS/nps -type f -name "F*Arc*.setup")
E2NFILES = $(shell find $(TESTDIR)/setupRSS/e2n -type f -name "F*E2N*.setup")

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS)

test_nps :
	$(RM) $(TESTDIR)/sirRSS/nps
	$(MKDIR) $(TESTDIR)/sirRSS/nps
	for file in $(NPSFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS/nps; \
	done

test_e2n :
	$(RM) $(TESTDIR)/sirRSS/e2n
	$(MKDIR) $(TESTDIR)/sirRSS/e2n
	for file in $(E2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/sirRSS/e2n; \
	done

