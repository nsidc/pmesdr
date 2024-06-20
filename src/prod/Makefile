#========================================================================
# make file for Passive Microwave Earth Science Data Record (PMESDR) production system
#
# 2014-04-14 M. J. Brodzik 303-492-8263 brodzik@nsidc.org
# National Snow & Ice Data Center, University of Colorado, Boulder
# Copyright (C) 2014 Regents of University of Colorado and Brigham-Young University
#========================================================================

#------------------------------------------------------------------------
# configuration section
#
# for compiling, installing and distributing you should be able
# to change these variables here and nowhere else and it should work
#
# PMESDR_TOPDIR should be the location you check the git repo out at
TOPDIR = $(PMESDR_TOP_DIR)
INCDIR = $(TOPDIR)/include
LIBDIR = $(TOPDIR)/lib
BINDIR = $(TOPDIR)/bin

#
#	commands
#
#SHELL = /bin/sh
#RDIST = rdist

#
#	C compiler options, debug or optimization settings
#
#
CONFIG_CFLAGS = 

#
#	system libraries and include directories
#
ifeq ($(LOCALE),int.nsidc.org)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib
  ZLIB_LIBDIR = /usr/lib
else ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
  UDUNITS_LIBDIR = ${CURC_UDUNITS_LIB}
else ifeq ($(LOCALE),ALPINEicc)
  $(info Build location: $(LOCALE))
  NETCDF4_INCDIR = ${CURC_NETCDF_INC}
  NETCDF4_LIBDIR = ${CURC_NETCDF_LIB}
  HDF5_INCDIR = ${CURC_HDF5_INC}
  HDF5_LIBDIR = ${CURC_HDF5_LIB}
  UDUNITS_INCDIR = ${CURC_UDUNITS_INC}
  UDUNITS_LIBDIR = ${CURC_UDUNITS_LIB}
else
  $(info Assuming default LOCALE=BYU)
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
  UDUNITS_INCDIR = /usr/include
  UDUNITS_LIBDIR = /usr/lib64
endif

#
# end configuration section
#------------------------------------------------------------------------

SUBDIRS = calcalcs/src utils/src cetb_file/src gsx/src \
	../sirlib/c meas_meta_make meas_meta_setup meas_meta_sir

all install clean:
	for i in $(SUBDIRS); do			\
	  (echo $$i; cd $$i;			\
	  $(MAKE) $@             		\
	    'CONFIG_CFLAGS=$(CONFIG_CFLAGS)'	\
	    'TOPDIR=$(TOPDIR)'	\
	    'BINDIR=$(BINDIR)'	\
	    'NETCDF4_INCDIR=$(NETCDF4_INCDIR)'	\
	    'NETCDF4_LIBDIR=$(NETCDF4_LIBDIR)'	\
	    'HDF5_INCDIR=$(HDF5_INCDIR)'	\
	    'HDF5_LIBDIR=$(HDF5_LIBDIR)'	\
	    'ZLIB_LIBDIR=$(ZLIB_LIBDIR)'        \
	  ) || exit 1; \
	done

regression-setup:
	cd ${PMESDR_TOP_DIR}/../
	git clone git@github.com:nsidc/pmesdr_regression_data.git
