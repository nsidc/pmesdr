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
TESTDIR = $(TOPDIR)/NSIDCtest
ifeq ($(LOCALE),int.nsidc.org)
  TESTDIR = /projects/PMESDR/vagrant/NSIDCtest/$(shell hostname)
endif
AMSRE_INPUT_FILES = AMSRE_regress_list_VM
ifeq ($(LOCALE),JANUSicc)
  AMSRE_INPUT_FILES = AMSRE_regress_list_JANUS
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
CP = cp
ifeq ($(LOCALE),JANUSicc)
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
ifeq ($(LOCALE),JANUSicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'JANUSicc=1' -I$(INCDIR) -I$(MEAS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -g -xHost -Wall -Wremarks -wd981 -no-prec-div -wd2415
endif
ifeq ($(LOCALE),JANUSgcc)
  $(info Build location: $(LOCALE))
  CFLAGS = -I$(INCDIR) -I$(MEAS_INCDIR) $(CONFIG_CFLAGS) -g -Wall
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

rss_ease :
	$(RM) $(TESTDIR)/rss_e2s
	$(MKDIR) $(TESTDIR)/rss_e2s
	$(RM) $(TESTDIR)/rss_e2n
	$(MKDIR) $(TESTDIR)/rss_e2n
	$(RM) $(TESTDIR)/rss_e2t
	$(MKDIR) $(TESTDIR)/rss_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2s/e2stest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/RSS_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/RSS_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2t/e2ttest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/RSS_regress_list

rss_full :
	$(RM) $(TESTDIR)/rss_e2s
	$(MKDIR) $(TESTDIR)/rss_e2s
	$(RM) $(TESTDIR)/rss_e2n
	$(MKDIR) $(TESTDIR)/rss_e2n
	$(RM) $(TESTDIR)/rss_e2t
	$(MKDIR) $(TESTDIR)/rss_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2s/e2stest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/RSS_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/RSS_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2t/e2ttest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/RSS_regress_list_full

rss_ten :
	$(RM) $(TESTDIR)/rss_e2s
	$(MKDIR) $(TESTDIR)/rss_e2s
	$(RM) $(TESTDIR)/rss_e2n
	$(MKDIR) $(TESTDIR)/rss_e2n
	$(RM) $(TESTDIR)/rss_e2t
	$(MKDIR) $(TESTDIR)/rss_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2s/e2stest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/RSS_regress_list_ten
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/RSS_regress_list_ten
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2t/e2ttest_RSS.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/RSS_regress_list_ten

# the list of files to be processed (held in RSS_regress_list) must be relative to the directory
# from which the program is called - i.e. cd to the $(TOPDIR)/sample_data directory before invoking
# the application
rss_quick :
	$(RM) $(TESTDIR)/rss_e2n
	$(MKDIR) $(TESTDIR)/rss_e2n
	$(RM) $(TESTDIR)/rss_e2s
	$(MKDIR) $(TESTDIR)/rss_e2s
	$(RM) $(TESTDIR)/rss_e2t
	$(MKDIR) $(TESTDIR)/rss_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2n/e2ntest_RSS.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/RSS_regress_list		
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2s/e2stest_RSS.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/RSS_regress_list		
	$(BINDIR)/$(BIN)  $(TESTDIR)/rss_e2t/e2ttest_RSS.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2T_test1ch.def $(TOPDIR)/testing/RSS_regress_list		

csu_ease :
	$(RM) $(TESTDIR)/csu_e2n
	$(MKDIR) $(TESTDIR)/csu_e2n
	$(RM) $(TESTDIR)/csu_e2s
	$(MKDIR) $(TESTDIR)/csu_e2s
	$(RM) $(TESTDIR)/csu_e2t
	$(MKDIR) $(TESTDIR)/csu_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2s/e2stest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2t/e2ttest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/CSU_regress_list

csu_full :
	$(RM) $(TESTDIR)/csu_e2n
	$(MKDIR) $(TESTDIR)/csu_e2n
	$(RM) $(TESTDIR)/csu_e2s
	$(MKDIR) $(TESTDIR)/csu_e2s
	$(RM) $(TESTDIR)/csu_e2t
	$(MKDIR) $(TESTDIR)/csu_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/CSU_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2s/e2stest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/CSU_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2t/e2ttest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/CSU_regress_list_full

csu_ten :
	$(RM) $(TESTDIR)/csu_e2n
	$(MKDIR) $(TESTDIR)/csu_e2n
	$(RM) $(TESTDIR)/csu_e2s
	$(MKDIR) $(TESTDIR)/csu_e2s
	$(RM) $(TESTDIR)/csu_e2t
	$(MKDIR) $(TESTDIR)/csu_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/CSU_regress_list_ten
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2s/e2stest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/CSU_regress_list_ten
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2t/e2ttest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/CSU_regress_list_ten

csu_quick :
	$(RM) $(TESTDIR)/csu_e2n
	$(MKDIR) $(TESTDIR)/csu_e2n
	$(RM) $(TESTDIR)/csu_e2s
	$(MKDIR) $(TESTDIR)/csu_e2s
	$(RM) $(TESTDIR)/csu_e2t
	$(MKDIR) $(TESTDIR)/csu_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2n/e2ntest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/CSU_regress_list		
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2s/e2stest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/CSU_regress_list		
	$(BINDIR)/$(BIN)  $(TESTDIR)/csu_e2t/e2ttest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2T_test1ch.def $(TOPDIR)/testing/CSU_regress_list		

amsre_ease :
	$(RM) $(TESTDIR)/amsre_e2s
	$(MKDIR) $(TESTDIR)/amsre_e2s
	$(RM) $(TESTDIR)/amsre_e2n
	$(MKDIR) $(TESTDIR)/amsre_e2n
	$(RM) $(TESTDIR)/amsre_e2t
	$(MKDIR) $(TESTDIR)/amsre_e2t
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2s/e2stest_AMSRE.meta AQUA \
	         35 35 2003 $(TOPDIR)/ref/E2S_amsre_test.def $(TOPDIR)/testing/$(AMSRE_INPUT_FILES)
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2n/e2ntest_AMSRE.meta AQUA \
	         35 35 2003 $(TOPDIR)/ref/E2N_amsre_test.def $(TOPDIR)/testing/$(AMSRE_INPUT_FILES)
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2t/e2ttest_AMSRE.meta AQUA \
	         35 35 2003 $(TOPDIR)/ref/E2T_amsre_test.def $(TOPDIR)/testing/$(AMSRE_INPUT_FILES)

amsre_quick :
	$(RM) $(TESTDIR)/amsre_e2n
	$(MKDIR) $(TESTDIR)/amsre_e2n
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2n/e2ntest_AMSRE.meta AQUA \
	         35 35 2003 $(TOPDIR)/ref/E2N_amsre_quick.def $(TOPDIR)/testing/$(AMSRE_INPUT_FILES)

ssmis_quick :
	$(RM) $(TESTDIR)/ssmis_e2n
	$(MKDIR) $(TESTDIR)/ssmis_e2n
	$(RM) $(TESTDIR)/ssmis_e2s
	$(MKDIR) $(TESTDIR)/ssmis_e2s
	$(RM) $(TESTDIR)/ssmis_e2t
	$(MKDIR) $(TESTDIR)/ssmis_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS.meta F16 	\
	         81 81 2006 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/SSMIS_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS.meta F16 	\
	         81 81 2006 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/SSMIS_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2t/e2ttest_SSMIS.meta F16 	\
	         81 81 2006 $(TOPDIR)/ref/EASE2T_test1ch.def $(TOPDIR)/testing/SSMIS_regress_list

ssmis_ease :
	$(RM) $(TESTDIR)/ssmis_e2n
	$(MKDIR) $(TESTDIR)/ssmis_e2n
	$(RM) $(TESTDIR)/ssmis_e2s
	$(MKDIR) $(TESTDIR)/ssmis_e2s
	$(RM) $(TESTDIR)/ssmis_e2t
	$(MKDIR) $(TESTDIR)/ssmis_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS.meta F16 	\
	         81 81 2006 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/SSMIS_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS.meta F16 	\
	         81 81 2006 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/SSMIS_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2t/e2ttest_SSMIS.meta F16 	\
	         81 81 2006 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/SSMIS_regress_list_full

smmr_quick :
	$(RM) $(TESTDIR)/smmr_e2n
	$(MKDIR) $(TESTDIR)/smmr_e2n
	$(RM) $(TESTDIR)/smmr_e2s
	$(MKDIR) $(TESTDIR)/smmr_e2s
	$(RM) $(TESTDIR)/smmr_e2t
	$(MKDIR) $(TESTDIR)/smmr_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/smmr_e2n/e2ntest_SMMR.meta NIMBUS7 	\
	         360 360 1984 $(TOPDIR)/ref/E2N_smmr_quick.def $(TOPDIR)/testing/SMMR_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/smmr_e2s/e2stest_SMMR.meta NIMBUS7 	\
	         360 360 1984 $(TOPDIR)/ref/E2S_smmr_quick.def $(TOPDIR)/testing/SMMR_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/smmr_e2t/e2ttest_SMMR.meta NIMBUS7 	\
	         360 360 1984 $(TOPDIR)/ref/E2T_smmr_quick.def $(TOPDIR)/testing/SMMR_regress_list

smmr_ease :
	$(RM) $(TESTDIR)/smmr_e2n
	$(MKDIR) $(TESTDIR)/smmr_e2n
	$(RM) $(TESTDIR)/smmr_e2s
	$(MKDIR) $(TESTDIR)/smmr_e2s
	$(RM) $(TESTDIR)/smmr_e2t
	$(MKDIR) $(TESTDIR)/smmr_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/smmr_e2n/e2ntest_SMMR.meta NIMBUS7 	\
	         360 360 1984 $(TOPDIR)/ref/E2N_smmr_test.def $(TOPDIR)/testing/SMMR_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/smmr_e2s/e2stest_SMMR.meta NIMBUS7 	\
	         360 360 1984 $(TOPDIR)/ref/E2S_smmr_test.def $(TOPDIR)/testing/SMMR_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/smmr_e2t/e2ttest_SMMR.meta NIMBUS7 	\
	         360 360 1984 $(TOPDIR)/ref/E2T_smmr_test.def $(TOPDIR)/testing/SMMR_regress_list_full

