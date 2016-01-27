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
TESTDIR = $(TOPDIR)/NSIDCtest
ifeq ($(LOCALE),int.nsidc.org)
  TESTDIR = /projects/PMESDR/vagrant/NSIDCtest/$(shell hostname)
endif
#
#	commands
#
CC = gcc
CD = cd
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


#
#	system libraries
#
SYSLIBS = -lm

#
#	system libraries and include directories
#
ifeq ($(LOCALE),NSIDCsnow)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),int.nsidc.org)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib
  ZLIB_LIBDIR = /usr/lib
else ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
else
  $(warning Assuming default LOCALE=BYU)
  NETCDF4_INCDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/include
  NETCDF4_LIBDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/lib
  HDF5_INCDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/include
  HDF5_LIBDIR = /auto/users/long/src/linux/netcdf/hdf5-1.8.3/lib
  ZLIB_LIBDIR = /auto/users/long/src/linux/netcdf/zlib-1.2.3/lib
endif

#
# end configuration section
#------------------------------------------------------------------------
MEAS_INCDIR = ../../sirlib/c/include
MEAS_LIBDIR = ../../sirlib/c
LIBS = -L$(LIBDIR) -L$(NETCDF4_LIBDIR) -L$(HDF5_LIBDIR) -L$(ZLIB_LIBDIR) -L$(MEAS_LIBDIR) \
	-lgsx -lcsir -lnetcdf -lhdf5_hl -lhdf5 -lz $(SYSLIBS)
CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(HDF5_INCDIR) -I$(MEAS_INCDIR) $(CONFIG_CFLAGS) -g
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -D'JANUSicc=1' -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(HDF5_INCDIR) -I$(UDUNITS_INCDIR) -I$(MEAS_INCDIR) -O3 -ipo -g -xHost -no-prec-div -Wall # -Zp8 # 
else ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) -I$(NETCDF4_INCDIR) -I$(HDF5_INCDIR) -I$(UDUNITS_INCDIR) -I$(MEAS_INCDIR) -g
endif
DEPEND_LIBS = $(LIBDIR)/libgsx.a $(MEAS_LIBDIR)/libcsir.a

SRCS = meas_meta_setup.c
OBJS = meas_meta_setup.o
HDRS = $(INCDIR)/gsx.h $(MEAS_INCDIR)/sir3.h
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

csu_ease :
	$(RM) $(TESTDIR)/setupCSU/e2n
	$(MKDIR) $(TESTDIR)/setupCSU/e2n
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta  $(TESTDIR)/setupCSU/e2n 	\
	)
	$(RM) $(TESTDIR)/setupCSU/e2s
	$(MKDIR) $(TESTDIR)/setupCSU/e2s
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2s/e2stest_CSU.meta  $(TESTDIR)/setupCSU/e2s 	\
	)
	$(RM) $(TESTDIR)/setupCSU/e2t
	$(MKDIR) $(TESTDIR)/setupCSU/e2t
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2t/e2ttest_CSU.meta  $(TESTDIR)/setupCSU/e2t 	\
	)

csu_quick :
	$(RM) $(TESTDIR)/setupCSU/e2n
	$(MKDIR) $(TESTDIR)/setupCSU/e2n
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta  $(TESTDIR)/setupCSU/e2n 	\
	)

rss_ease :
	$(RM) $(TESTDIR)/setupRSS/e2n
	$(MKDIR) $(TESTDIR)/setupRSS/e2n
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta  $(TESTDIR)/setupRSS/e2n 	\
	)
	$(RM) $(TESTDIR)/setupRSS/e2s
	$(MKDIR) $(TESTDIR)/setupRSS/e2s
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2s/e2stest_RSS.meta  $(TESTDIR)/setupRSS/e2s 	\
	)
	$(RM) $(TESTDIR)/setupRSS/e2t
	$(MKDIR) $(TESTDIR)/setupRSS/e2t
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2t/e2ttest_RSS.meta  $(TESTDIR)/setupRSS/e2t 	\
	)

# the input file list in the .meta file has pathnames relative to the directory
# from which the program is called - i.e. cd to the $(TOPDIR)/sample_data directory before invoking
# the application
rss_quick :
	$(RM) $(TESTDIR)/setupRSS/e2n
	$(MKDIR) $(TESTDIR)/setupRSS/e2n
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta  $(TESTDIR)/setupRSS/e2n 	\
	)

amsre_ease :
	$(RM) $(TESTDIR)/setupAMSRE/e2n
	$(MKDIR) $(TESTDIR)/setupAMSRE/e2n
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2n/e2ntest_AMSRE.meta  $(TESTDIR)/setupAMSRE/e2n 	\
	)
	$(RM) $(TESTDIR)/setupAMSRE/e2s
	$(MKDIR) $(TESTDIR)/setupAMSRE/e2s
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2s/e2stest_AMSRE.meta  $(TESTDIR)/setupAMSRE/e2s 	\
	)
	$(RM) $(TESTDIR)/setupAMSRE/e2t
	$(MKDIR) $(TESTDIR)/setupAMSRE/e2t
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2t/e2ttest_AMSRE.meta  $(TESTDIR)/setupAMSRE/e2t 	\
	)

amsre_quick :
	$(RM) $(TESTDIR)/setupAMSRE/e2n
	$(MKDIR) $(TESTDIR)/setupAMSRE/e2n
	($(CD) $(TOPDIR)/sample_data;		\
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2n/e2ntest_AMSRE.meta  $(TESTDIR)/setupAMSRE/e2n 	\
	)


