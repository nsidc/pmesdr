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
INCDIR = $(TOPDIR)/include

TESTINPUT = $(PMESDR_TOP_DIR)/src/test_inputs
TESTOUTPUT = $(PMESDR_TEST_OUT_DIR)

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
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
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
CFLAGS = -I$(INCDIR) -I$(MEAS_INCDIR) -m64 $(CONFIG_CFLAGS) -Wall -Wno-unused-variable
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'ALPINEicc=1' -I$(INCDIR) -I$(MEAS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -xHost -Wall -Wremarks -wd981 -no-prec-div -wd2415
endif

DEPEND_LIBS = $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_meta_make.c
OBJS = meas_meta_make.o
HDRS = $(MEAS_INCDIR)/sir_geom.h $(INCDIR)/cetb.h
BIN = meas_meta_make

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

install : 
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(BIN) $(BINDIR)

clean :
	$(RM) $(BIN) $(OBJS) $(BINDIR)/$(BIN)

#
#------------------------------------------------------------------------
# Begin test targets section
#
# csu_quick
#   cd to $(TESTINPUT) before anything is run, because list of input
#   files are defined with paths relative to this location
#   -r 0 tells it to use base resolution of 25 km
#   these region def files are 2 regions, M, E at 19H and 12.5 km
#   csu_full is exactly the same without the -r 0 option and a different gsx list file 
#   region def files:
#          $(TOPDIR)/ref/EASE2N_test1ch.def
#          $(TOPDIR)/ref/EASE2S_test1ch.def
#          $(TOPDIR)/ref/EASE2T_test1ch.def
#   gsx list files:
#          $(TESTINPUT)/CSU_regress_list
#   actual input files:
#          $(TESTINPUT)/CSU_JD061_1997/ (there are 2 files)
#   output to (these dirs are cleared out at beginning of test):
#          $(TESTOUTPUT)/csu_e2n/e2ntest_CSU.meta
#          $(TESTOUTPUT)/csu_e2s/e2stest_CSU.meta
#          $(TESTOUTPUT)/csu_e2t/e2ttest_CSU.meta
#
# csu_ease
#   csu_full is exactly the same without the -r 0 option (this is the default value)
#   these region def files are 14 regions, M, E (A/D for T),
#       all 7 channels, again at 12.5 km
#   same set of only 2 actual gsx files
#   region def files:
#          $(TOPDIR)/ref/E2N_test.def
#          $(TOPDIR)/ref/E2S_test.def
#          $(TOPDIR)/ref/E2T_test.def
#   gsx list files:
#          $(TESTINPUT)/CSU_regress_list
#   actual input files:
#          $(TESTINPUT)/CSU_JD061_1997/ (there are 2 files)
#   output to (these dirs are cleared out at beginning of test):
#          $(TESTOUTPUT)/csu_e2n/e2ntest_CSU.meta
#          $(TESTOUTPUT)/csu_e2s/e2stest_CSU.meta
#          $(TESTOUTPUT)/csu_e2t/e2ttest_CSU.meta

csu_ease :
	$(RM) $(TESTOUTPUT)/csu_e2n
	$(MKDIR) $(TESTOUTPUT)/csu_e2n
	$(RM) $(TESTOUTPUT)/csu_e2s
	$(MKDIR) $(TESTOUTPUT)/csu_e2s
	$(RM) $(TESTOUTPUT)/csu_e2t
	$(MKDIR) $(TESTOUTPUT)/csu_e2t
	$(CD) $(TESTINPUT)
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/csu_e2n/e2ntest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TESTINPUT)/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/csu_e2s/e2stest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TESTINPUT)/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/csu_e2t/e2ttest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TESTINPUT)/CSU_regress_list

csu_quick :
	$(RM) $(TESTOUTPUT)/csu_e2n
	$(MKDIR) $(TESTOUTPUT)/csu_e2n
	$(RM) $(TESTOUTPUT)/csu_e2s
	$(MKDIR) $(TESTOUTPUT)/csu_e2s
	$(RM) $(TESTOUTPUT)/csu_e2t
	$(MKDIR) $(TESTOUTPUT)/csu_e2t
	$(CD) $(TESTINPUT)
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/csu_e2n/e2ntest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TESTINPUT)/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/csu_e2s/e2stest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2S_test1ch.def $(TESTINPUT)/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/csu_e2t/e2ttest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2T_test1ch.def $(TESTINPUT)/CSU_regress_list
