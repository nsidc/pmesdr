#========================================================================
# Makefile for meas_meta_setup utility
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
INCDIR = $(TOPDIR)/include
LIBDIR = $(TOPDIR)/lib
BINDIR = $(TOPDIR)/bin

TESTINPUT = $(TOPDIR)/src/test_inputs
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
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CC = icc
endif
#
#       debug or optimization settings
#       these may be things that are machine-specific
#
CONFIG_CFLAGS = -O3 -m64


#
#	system libraries
#
SYSLIBS = -lm

#
#	system libraries and include directories
#
ifeq ($(LOCALE),int.nsidc.org)
  $(info Build location: $(LOCALE))
  SYSTEM_BASEDIR = /usr
  NETCDF4_INCDIR = $(SYSTEM_BASEDIR)/include
  NETCDF4_LIBDIR = $(SYSTEM_BASEDIR)/lib
  HDF5_INCDIR = $(SYSTEM_BASEDIR)/include
  HDF5_LIBDIR = $(SYSTEM_BASEDIR)/lib
  HDFLIB1 = hdf5_serial_hl
  HDFLIB2 = hdf5_serial
  UDUNITS_INCDIR  = $(SYSTEM_BASEDIR)/include
  UDUNITS_LIBDIR  = $(SYSTEM_BASEDIR)/lib/x86_64-linux-gnu
else ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  HDFLIB1 = hdf5_hl
  HDFLIB2 = hdf5
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
  UDUNITS_LIBDIR = ${CURC_UDUNITS_LIB}
else
  $(info Assuming default LOCALE, mileage may vary)
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib64
  UDUNITS_INCDIR = /usr/include
  UDUNITS_LIBDIR = /usr/lib64
endif

#
# end configuration section
#------------------------------------------------------------------------
MEAS_INCDIR = ../../sirlib/c/include
MEAS_LIBDIR = 
LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(UDUNITS_LIBDIR) \
	-lgsx -lutils -lcalcalcs -ludunits2 -lcsir \
	-lnetcdf -l$(HDFLIB1) -l$(HDFLIB2) -lz $(SYSLIBS)
CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(HDF5_INCDIR) -I$(UDUNITS_INCDIR) \
	-I$(MEAS_INCDIR) $(CONFIG_CFLAGS) -Wall -Wno-unused-variable
ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'ALPINEicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(HDF5_INCDIR) \
	-I$(UDUNITS_INCDIR) -I$(MEAS_INCDIR) -ipo -xHost -no-prec-div \
	-Wall -Wremarks -wd981 -wd2415 -inline-level=0
endif


DEPEND_LIBS = $(LIBDIR)/libutils.a \
	$(LIBDIR)/libcalcalcs.a \
	$(UDUNITS_LIBDIR)/libudunits2.a \
	$(LIBDIR)/libgsx.a $(LIBDIR)/libcsir.a

SRCS = meas_meta_setup.c
OBJS = meas_meta_setup.o
HDRS = $(INCDIR)/gsx.h $(INCDIR)/cetb.h $(MEAS_INCDIR)/sir_geom.h \
	$(INCDIR)/utCalendar2_cal.h
BIN = meas_meta_setup

all : $(BIN)

$(BIN) : $(OBJS) $(HDRS) $(DEPEND_LIBS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

meas_meta_setup.o : $(SRCS) $(HDRS)
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
# csu_quick or csu_ease (note that .meta files contain different information
# after meas_meta_make targets by matching names)
# does this mean we can eliminate one of these and just keep the other?
#   input gsx files are read with relative pathnames, so we need to cd
#   to the top input directory before running
#   input .meta files
#	$(TESTOUTPUT)/csu_e2n/e2ntest_CSU.meta      
#	$(TESTOUTPUT)/csu_e2n/e2stest_CSU.meta      
#	$(TESTOUTPUT)/csu_e2n/e2stest_CSU.meta
#   output to (these dirs are cleared out that the beginning of test):
#       $(TESTOUTPUT)/setupCSU/e2n/
#       $(TESTOUTPUT)/setupCSU/e2s/
#       $(TESTOUTPUT)/setupCSU/e2t/
#
#
csu_quick :
	$(RM) $(TESTOUTPUT)/setupCSU/e2n
	$(MKDIR) $(TESTOUTPUT)/setupCSU/e2n
	$(RM) $(TESTOUTPUT)/setupCSU/e2s
	$(MKDIR) $(TESTOUTPUT)/setupCSU/e2s
	$(RM) $(TESTOUTPUT)/setupCSU/e2t
	$(MKDIR) $(TESTOUTPUT)/setupCSU/e2t
	($(CD) $(TESTINPUT);		\
	$(BINDIR)/$(BIN)  $(TESTOUTPUT)/csu_e2n/e2ntest_CSU.meta  $(TESTOUTPUT)/setupCSU/e2n 	\
	)
	($(CD) $(TESTINPUT);		\
	$(BINDIR)/$(BIN)  $(TESTOUTPUT)/csu_e2s/e2stest_CSU.meta  $(TESTOUTPUT)/setupCSU/e2s 	\
	)
	($(CD) $(TESTINPUT);		\
	$(BINDIR)/$(BIN)  $(TESTOUTPUT)/csu_e2t/e2ttest_CSU.meta  $(TESTOUTPUT)/setupCSU/e2t 	\
	)

csu_ease :
	$(RM) $(TESTOUTPUT)/setupCSU/e2n
	$(MKDIR) $(TESTOUTPUT)/setupCSU/e2n
	($(CD) $(TESTINPUT);		\
	$(BINDIR)/$(BIN)  $(TESTOUTPUT)/csu_e2n/e2ntest_CSU.meta  $(TESTOUTPUT)/setupCSU/e2n 	\
	)
	$(RM) $(TESTOUTPUT)/setupCSU/e2s
	$(MKDIR) $(TESTOUTPUT)/setupCSU/e2s
	($(CD) $(TESTINPUT);		\
	$(BINDIR)/$(BIN)  $(TESTOUTPUT)/csu_e2s/e2stest_CSU.meta  $(TESTOUTPUT)/setupCSU/e2s 	\
	)
	$(RM) $(TESTOUTPUT)/setupCSU/e2t
	$(MKDIR) $(TESTOUTPUT)/setupCSU/e2t
	($(CD) $(TESTINPUT);		\
	$(BINDIR)/$(BIN)  $(TESTOUTPUT)/csu_e2t/e2ttest_CSU.meta  $(TESTOUTPUT)/setupCSU/e2t 	\
	)
