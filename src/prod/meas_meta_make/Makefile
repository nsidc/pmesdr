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
TESTDIR = $(TOPDIR)/test
#
#	commands
#
CC = gcc
INSTALL = cp -f -p
LS = ls -1
MKDIR = mkdir -p
RM = rm -fr

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
	$(RM) $(BIN) $(OBJS)

rss_nps :
	$(RM) $(TESTDIR)/rss_nps
	$(MKDIR) $(TESTDIR)/rss_nps
	$(LS)  $(TOPDIR)/sample_data/f13_r09999_tb_v7.dat > $(TESTDIR)/rss_nps/F1RSS.lis
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_nps/npstest_RSS.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 60 60 1997 $(TOPDIR)/ref/arctic.def $(TESTDIR)/rss_nps/F1RSS.lis

rss_e2n :
	$(RM) $(TESTDIR)/rss_e2n
	$(MKDIR) $(TESTDIR)/rss_e2n
	$(LS)  $(TOPDIR)/sample_data/f13_r09999_tb_v7.dat > $(TESTDIR)/rss_e2n/F1RSS.lis
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 60 60 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TESTDIR)/rss_e2n/F1RSS.lis

csu_nps :
	$(RM) $(TESTDIR)/csu_nps
	$(MKDIR) $(TESTDIR)/csu_nps
	$(LS)  $(TOPDIR)/sample_data/CSU_SSMI* > $(TESTDIR)/csu_nps/F1CSU.lis
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_nps/npstest_CSU.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 60 60 1997 $(TOPDIR)/ref/arctic.def $(TESTDIR)/csu_nps/F1CSU.lis

csu_e2n :
	$(RM) $(TESTDIR)/csu_e2n
	$(MKDIR) $(TESTDIR)/csu_e2n
	$(LS)  $(TOPDIR)/sample_data/CSU_SSMI* > $(TESTDIR)/csu_e2n/F1CSU.lis
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta 13 \
	         $(TOPDIR)/ref/s2g/ssmi_mN.s2g 60 60 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TESTDIR)/csu_e2n/F1CSU.lis


