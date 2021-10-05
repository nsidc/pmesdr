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
ifeq ($(LOCALE),SUMMITicc)
  AMSRE_INPUT_FILES = AMSRE_regress_list_SUMMIT
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
ifeq ($(LOCALE),SUMMITicc)
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
ifeq ($(LOCALE),SUMMITicc)
  $(info Build location: $(LOCALE))
  CFLAGS = -D'SUMMITicc=1' -I$(INCDIR) -I$(MEAS_INCDIR) $(CONFIG_CFLAGS) -O3 -ipo -xHost -Wall -Wremarks -wd981 -no-prec-div -wd2415
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
	$(BINDIR)/$(BIN)  -r 0 $(TESTDIR)/csu_e2n/e2ntest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTDIR)/csu_e2s/e2stest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTDIR)/csu_e2t/e2ttest_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/CSU_regress_list

csu_ease_36 :
	$(RM) $(TESTDIR)/csu_e2n_36
	$(MKDIR) $(TESTDIR)/csu_e2n_36
	$(RM) $(TESTDIR)/csu_e2s_36
	$(MKDIR) $(TESTDIR)/csu_e2s_36
	$(RM) $(TESTDIR)/csu_e2t_36
	$(MKDIR) $(TESTDIR)/csu_e2t_36
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  -r 1 $(TESTDIR)/csu_e2n_36/e2ntest_36_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 1 $(TESTDIR)/csu_e2s_36/e2stest_36_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 1 $(TESTDIR)/csu_e2t_36/e2ttest_36_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/CSU_regress_list

csu_ease_24 :
	$(RM) $(TESTDIR)/csu_e2n_24
	$(MKDIR) $(TESTDIR)/csu_e2n_24
	$(RM) $(TESTDIR)/csu_e2s_24
	$(MKDIR) $(TESTDIR)/csu_e2s_24
	$(RM) $(TESTDIR)/csu_e2t_24
	$(MKDIR) $(TESTDIR)/csu_e2t_24
	$(CD) $(TOPDIR)/sample_data
	$(BINDIR)/$(BIN)  -r 2 $(TESTDIR)/csu_e2n_24/e2ntest_24_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 2 $(TESTDIR)/csu_e2s_24/e2stest_24_CSU.meta F13 \
	         61 61 1997 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 2 $(TESTDIR)/csu_e2t_24/e2ttest_24_CSU.meta F13 \
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
	$(BINDIR)/$(BIN)  -r 0 $(TESTDIR)/csu_e2n/e2ntest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTDIR)/csu_e2s/e2stest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 0 $(TESTDIR)/csu_e2t/e2ttest_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2T_test1ch.def $(TOPDIR)/testing/CSU_regress_list

csu_quick_36 :
	$(RM) $(TESTDIR)/csu_e2n_36
	$(MKDIR) $(TESTDIR)/csu_e2n_36
	$(RM) $(TESTDIR)/csu_e2s_36
	$(MKDIR) $(TESTDIR)/csu_e2s_36
	$(RM) $(TESTDIR)/csu_e2t_36
	$(MKDIR) $(TESTDIR)/csu_e2t_36
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  -r 1 $(TESTDIR)/csu_e2n_36/e2ntest_36_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 1 $(TESTDIR)/csu_e2s_36/e2stest_36_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 1 $(TESTDIR)/csu_e2t_36/e2ttest_36_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2T_test1ch.def $(TOPDIR)/testing/CSU_regress_list

csu_quick_24 :
	$(RM) $(TESTDIR)/csu_e2n_24
	$(MKDIR) $(TESTDIR)/csu_e2n_24
	$(RM) $(TESTDIR)/csu_e2s_24
	$(MKDIR) $(TESTDIR)/csu_e2s_24
	$(RM) $(TESTDIR)/csu_e2t_24
	$(MKDIR) $(TESTDIR)/csu_e2t_24
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  -r 2 $(TESTDIR)/csu_e2n_24/e2ntest_24_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 2 $(TESTDIR)/csu_e2s_24/e2stest_24_CSU.meta F13 	\
	         61 61 1997 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/CSU_regress_list
	$(BINDIR)/$(BIN)  -r 2 $(TESTDIR)/csu_e2t_24/e2ttest_24_CSU.meta F13 	\
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
	$(RM) $(TESTDIR)/amsre_e2s
	$(MKDIR) $(TESTDIR)/amsre_e2s
	$(RM) $(TESTDIR)/amsre_e2t
	$(MKDIR) $(TESTDIR)/amsre_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2n/e2ntest_AMSRE.meta AQUA \
	         35 35 2003 $(TOPDIR)/ref/E2N_amsre_quick.def $(TOPDIR)/testing/$(AMSRE_INPUT_FILES)
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2s/e2stest_AMSRE.meta AQUA \
	         35 35 2003 $(TOPDIR)/ref/E2S_amsre_quick.def $(TOPDIR)/testing/$(AMSRE_INPUT_FILES)
	$(BINDIR)/$(BIN)  $(TESTDIR)/amsre_e2t/e2ttest_AMSRE.meta AQUA \
	         35 35 2003 $(TOPDIR)/ref/E2T_amsre_quick.def $(TOPDIR)/testing/$(AMSRE_INPUT_FILES)

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

ssmis_cross :
	$(RM) $(TESTDIR)/ssmis_e2n
	$(MKDIR) $(TESTDIR)/ssmis_e2n
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_20051231.meta F16 	\
	         365 365 2005 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_20060101.meta F16 	\
	         1 1 2006 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/F16_list

ssmis_test :
	$(RM) $(TESTDIR)/ssmis_e2n
	$(MKDIR) $(TESTDIR)/ssmis_e2n
	$(RM) $(TESTDIR)/ssmis_e2s
	$(MKDIR) $(TESTDIR)/ssmis_e2s
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_2005.meta F16 	\
	         306 306 2005 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS_2005.meta F16 	\
	         306 306 2005 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_2008.meta F16 	\
	         2 2 2008 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS_2008.meta F16 	\
	         2 2 2008 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_2010.meta F16 	\
	         2 2 2010 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS_2010.meta F16 	\
	         2 2 2010 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_2012.meta F16 	\
	         2 2 2012 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS_2012.meta F16 	\
	         2 2 2012 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_2014.meta F16 	\
	         2 2 2014 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS_2014.meta F16 	\
	         2 2 2014 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_2015.meta F16 	\
	         2 2 2015 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/F16_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS_2015.meta F16 	\
	         2 2 2015 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/F16_list

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

smap_quick :
	$(RM) $(TESTDIR)/smap_e2n
	$(MKDIR) $(TESTDIR)/smap_e2n
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/smap_e2n/e2ntest_SMAP.meta SMAP 	\
	         204 208 2016 $(TOPDIR)/ref/E2N_smap_quick.def $(TOPDIR)/testing/SMAP_regress_list

ssmis_l1c_quick :
	$(RM) $(TESTDIR)/ssmis_e2n
	$(MKDIR) $(TESTDIR)/ssmis_e2n
	$(RM) $(TESTDIR)/ssmis_e2s
	$(MKDIR) $(TESTDIR)/ssmis_e2s
	$(RM) $(TESTDIR)/ssmis_e2t
	$(MKDIR) $(TESTDIR)/ssmis_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_L1C.meta F16 	\
	         81 81 2021 $(TOPDIR)/ref/EASE2N_test1ch.def $(TOPDIR)/testing/SSMIS_L1C_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS_L1C.meta F16 	\
	         81 81 2021 $(TOPDIR)/ref/EASE2S_test1ch.def $(TOPDIR)/testing/SSMIS_L1C_regress_list
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2t/e2ttest_SSMIS_L1C.meta F16 	\
	         81 81 2021 $(TOPDIR)/ref/EASE2T_test1ch.def $(TOPDIR)/testing/SSMIS_L1C_regress_list

ssmis_l1c_ease :
	$(RM) $(TESTDIR)/ssmis_e2n
	$(MKDIR) $(TESTDIR)/ssmis_e2n
	$(RM) $(TESTDIR)/ssmis_e2s
	$(MKDIR) $(TESTDIR)/ssmis_e2s
	$(RM) $(TESTDIR)/ssmis_e2t
	$(MKDIR) $(TESTDIR)/ssmis_e2t
	$(CD) $(TOPDIR)/sample_data		 
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2n/e2ntest_SSMIS_L1C.meta F16 	\
	         81 81 2021 $(TOPDIR)/ref/E2N_test.def $(TOPDIR)/testing/SSMIS_L1C_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2s/e2stest_SSMIS_L1C.meta F16 	\
	         81 81 2021 $(TOPDIR)/ref/E2S_test.def $(TOPDIR)/testing/SSMIS_L1C_regress_list_full
	$(BINDIR)/$(BIN)  $(TESTDIR)/ssmis_e2t/e2ttest_SSMIS_L1C.meta F16 	\
	         81 81 2021 $(TOPDIR)/ref/E2T_test.def $(TOPDIR)/testing/SSMIS_L1C_regress_list_full



