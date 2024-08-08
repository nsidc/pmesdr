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
LIBDIR = $(TOPDIR)/lib
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
LIBS = -L$(LIBDIR) -lutils $(SYSLIBS)
CFLAGS = -I$(INCDIR)  -m64 $(CONFIG_CFLAGS) -Wall -Wno-unused-variable
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'ALPINEicc=1' -I$(INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -xHost -Wall -Wremarks -wd981 -no-prec-div -wd2415
endif

DEPEND_LIBS = $(LIBDIR)/libutils.a

SRCS = meas_meta_make.c
OBJS = meas_meta_make.o
HDRS = $(INCDIR)/cetb.h $(INCDIR)/utils.h
BIN = meas_meta_make

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

meas_meta_make.o : $(SRCS) $(HDRS)
	$(CC) -c $(CFLAGS) -o $@ $(SRCS)

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
#
# csu_ease
#   these region def files are 14 regions, M, E (A/D for T),
#       all 7 channels, again at 12.5 km
#   same set of only 2 actual gsx files

csu_quick :
	$(RM) $(TESTOUTPUT)/makeCSU
	$(MKDIR) $(TESTOUTPUT)/makeCSU
	$(CD) $(TESTINPUT)
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/makeCSU/e2ntest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TESTINPUT)/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/makeCSU/e2stest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2S_test1ch.def $(TESTINPUT)/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/makeCSU/e2ttest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2T_test1ch.def $(TESTINPUT)/CSU_regress_list

csu_ease :
	$(RM) $(TESTOUTPUT)/makeCSU
	$(MKDIR) $(TESTOUTPUT)/makeCSU
	$(CD) $(TESTINPUT)
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/makeCSU/e2ntest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TESTINPUT)/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/makeCSU/e2stest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TESTINPUT)/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTOUTPUT)/makeCSU/e2ttest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TESTINPUT)/CSU_regress_list

