#========================================================================
# Makefile for meas_meta_setup_RSS utility
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
TESTDIR = $(TOPDIR)/NSIDCtest

#
#	commands
#
CC = gcc
INSTALL = cp -f -p
LS = ls -1
MKDIR = mkdir -p
RM = rm -fr
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CC = icc
endif
#
#       debug or optimization settings
#       these may be things that are machine-specific
#
CONFIG_CFLAGS = -O3 -m64
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CONFIG_CFLAGS = 
endif 

#
#	system libraries
#
SYSLIBS = -lm

#
# end configuration section
#------------------------------------------------------------------------
MEAS_INCDIR = ../../../sirlib/c/include
MEAS_LIBDIR = ../../../sirlib/c
LIBS = -L$(MEAS_LIBDIR) -lcsir $(SYSLIBS)
CFLAGS = -D'RSS=1' $(CONFIG_CFLAGS) -I$(MEAS_INCDIR) 
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -D'RSS=1' -D'JANUSicc=1' -I$(MEAS_INCDIR) -O3 -ipo -g -xHost -no-prec-div # -Zp8 # 
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -D'RSS=1' -I$(MEAS_INCDIR) -g
endif
DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = ../meas_meta_setup.c
OBJS = meas_meta_setup_RSS.o
HDRS = $(MEAS_INCDIR)/sir3.h
BIN = meas_meta_setup_RSS

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

meas_meta_setup_RSS.o : $(SRCS) $(HDRS)
	$(CC) -c $(CFLAGS) -o $@ $(SRCS)

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS) $(BINDIR)/$(BIN)

rss_ease :
	$(RM) $(TESTDIR)/setupRSS/e2n
	$(MKDIR) $(TESTDIR)/setupRSS/e2n
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta  $(TESTDIR)/setupRSS/e2n 0
	$(RM) $(TESTDIR)/setupRSS/e2s
	$(MKDIR) $(TESTDIR)/setupRSS/e2s
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2s/e2stest_RSS.meta  $(TESTDIR)/setupRSS/e2s 0
	$(RM) $(TESTDIR)/setupRSS/e2t
	$(MKDIR) $(TESTDIR)/setupRSS/e2t
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2t/e2ttest_RSS.meta  $(TESTDIR)/setupRSS/e2t 0


