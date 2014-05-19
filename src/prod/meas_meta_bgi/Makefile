#========================================================================
# Makefile for meas_meta_bgi utility
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

SRCS = meas_meta_bgi.c
OBJS = meas_meta_bgi.o
HDRS = nr.h nrutil.h $(MEAS_INCDIR)/sir3.h 
BIN = meas_meta_bgi

RSSNPSFILES = $(shell find $(TESTDIR)/setupRSS/nps -type f -name "F*Arc*.setup")
RSSE2NFILES = $(shell find $(TESTDIR)/setupRSS/e2n -type f -name "F*E2N*.setup")
CSUNPSFILES = $(shell find $(TESTDIR)/setupCSU/nps -type f -name "F*Arc*.setup")
CSUE2NFILES = $(shell find $(TESTDIR)/setupCSU/e2n -type f -name "F*E2N*.setup")

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS)

rss_nps :
	$(RM) $(TESTDIR)/bgiRSS/nps
	$(MKDIR) $(TESTDIR)/bgiRSS/nps
	for file in $(RSSNPSFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiRSS/nps; \
	done

rss_e2n :
	$(RM) $(TESTDIR)/bgiRSS/e2n
	$(MKDIR) $(TESTDIR)/bgiRSS/e2n
	for file in $(RSSE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiRSS/e2n; \
	done

csu_nps :
	$(RM) $(TESTDIR)/bgiCSU/nps
	$(MKDIR) $(TESTDIR)/bgiCSU/nps
	for file in $(CSUNPSFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiCSU/nps; \
	done

csu_e2n :
	$(RM) $(TESTDIR)/bgiCSU/e2n
	$(MKDIR) $(TESTDIR)/bgiCSU/e2n
	for file in $(CSUE2NFILES); do \
	   echo "running  $$file"; \
	   $(BINDIR)/$(BIN) $$file $(TESTDIR)/bgiCSU/e2n; \
	done


