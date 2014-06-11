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
ifeq ($(LOCALE),NSIDCsnow)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else ifeq ($(LOCALE),NSIDCdev)
  $(warning Build location: $(LOCALE))
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
else
  $(warning Assuming default LOCALE=BYU)
  NETCDF4_INCDIR = /usr/include
  NETCDF4_LIBDIR = /usr/lib64
  HDF5_INCDIR = /usr/include
  HDF5_LIBDIR = /usr/lib64
  ZLIB_LIBDIR = /usr/lib64
  #NETCDF4_INCDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/include
  #NETCDF4_LIBDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/lib
  #HDF5_INCDIR = /auto/users/long/src/linux/netcdf/netcdf-4.0.1/include
  #HDF5_LIBDIR = /auto/users/long/src/linux/netcdf/hdf5-1.8.3/lib
  #ZLIB_LIBDIR = /auto/users/long/src/linux/netcdf/zlib-1.2.3/lib
endif

#
# end configuration section
#------------------------------------------------------------------------

SUBDIRS = ../sirlib/c meas_meta_make \
	meas_meta_setup/meas_meta_setup_RSS meas_meta_setup/meas_meta_setup_CSU \
	meas_meta_sir meas_meta_bgi meas_undump

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
	    'ZLIB_LIBDIR=$(ZLIB_LIBDIR)'	\
	  ) done

