#
# generic makefile for SIR format C programs/library
# written by R. Chen Feb. 2001   JPL PO.DAAC
# modified by D. Long Mar. 2014
#
# be sure to edit include/sir3.h to reflect machine/compiler type

TOPDIR = $(PMESDR_TOP_DIR)
LIBDIR = $(TOPDIR)/lib
INCDIR = $(TOPDIR)/include

EXES = csirexample 
LIB = libcsir.a
HDRS = lib/sir_geom.h

AR = ar
CC = gcc
INSTALL = cp -f -p
MKDIR = mkdir -p
RM = rm -fr

CONFIG_CFLAGS = 
ARFLAGS = ruv
CFLAGS = -Iinclude -m64 -O3 $(CONFIG_CFLAGS) 
LDFLAGS = -L. -lcsir -lm

ifeq ($(LOCALE),JANUSgcc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -Iinclude $(CONFIG_CFLAGS) -g
endif

ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CC = icc
  AR = xiar
  CFLAGS = -Iinclude  $(CONFIG_CFLAGS) -O3 -ipo -g -xHost -no-prec-div # -Zp8
  ARFLAGS = crs
endif


all :  $(LIB)

install :
	$(MKDIR) $(LIBDIR)
	$(INSTALL) $(LIB) $(LIBDIR)
	$(INSTALL) $(HDRS) $(INCDIR)

# create static c routines library
libcsir.a : lib/sir_io.c lib/sir_ez.c lib/sir_geom.c 
	rm -f *.o
	rm -f libcsir.a
	$(CC) $(CFLAGS) -c lib/sir_io.c
	$(CC) $(CFLAGS) -c lib/sir_ez.c
	$(CC) $(CFLAGS) -c lib/sir_geom.c
	$(AR) $(ARFLAGS) libcsir.a *.o
	$(RM) *.o
	$(MKDIR) $(LIBDIR)
	$(INSTALL) $(LIB) $(LIBDIR)

clean :
	$(RM) $(EXES) $(LIB) *.o $(BINDIR)/$(EXES) $(LIBDIR)/$(LIB)



