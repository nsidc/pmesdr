#========================================================================
# Makefile for meas_meta_make utility
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
CD = cd
INSTALL = cp -f -p
LS = ls -1
MKDIR = mkdir -p
RM = rm -fr
CP = cp
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CC = icc
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
MEAS_INCDIR = ../../sirlib/c/include
MEAS_LIBDIR = ../../sirlib/c
LIBS = -L$(MEAS_LIBDIR) -lcsir $(SYSLIBS)
CFLAGS = -I$(MEAS_INCDIR) -m64 $(CONFIG_CFLAGS)
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -D'JANUSicc=1' -I$(MEAS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -g -xHost -no-prec-div # -Zp8
endif
ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(MEAS_INCDIR) $(CONFIG_CFLAGS) -g
endif
DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_meta_make.c
OBJS = meas_meta_make.o
HDRS = $(MEAS_INCDIR)/sir3.h
BIN = meas_meta_make

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

install : 
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS) $(BINDIR)/$(BIN)

rss_ease :
	$(RM) $(TESTDIR)/rss_e2s
	$(MKDIR) $(TESTDIR)/rss_e2s
	$(RM) $(TESTDIR)/rss_e2n
	$(MKDIR) $(TESTDIR)/rss_e2n
	$(RM) $(TESTDIR)/rss_e2t
	$(MKDIR) $(TESTDIR)/rss_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2s/e2stest_RSS.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/RSS_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/RSS_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2t/e2ttest_RSS.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/RSS_regress_list

# the list of files to be processed (held in RSS_regress_list) must be relative to the directory
# from which the program is called - i.e. cd to the $(TOPDIR)/sample_data directory before invoking
# the application
rss_quick :
	$(RM) $(TESTDIR)/rss_e2n
	$(MKDIR) $(TESTDIR)/rss_e2n
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta 13 	\
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 61 61 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/RSS_regress_list		

csu_ease :
	$(RM) $(TESTDIR)/csu_e2n
	$(MKDIR) $(TESTDIR)/csu_e2n
	$(RM) $(TESTDIR)/csu_e2s
	$(MKDIR) $(TESTDIR)/csu_e2s
	$(RM) $(TESTDIR)/csu_e2t
	$(MKDIR) $(TESTDIR)/csu_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2s/e2stest_CSU.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2t/e2ttest_CSU.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/CSU_regress_list


