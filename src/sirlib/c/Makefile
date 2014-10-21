#
# generic makefile for SIR format C programs/library
# written by R. Chen Feb. 2001   JPL PO.DAAC
# modified by D. Long Mar. 2014
#
# be sure to edit include/sir3.h to reflect machine/compiler type

TOPDIR = $(PMESDR_TOP_DIR)
BINDIR = $(TOPDIR)/bin

EXES = csirexample viewsirhead csir_locmap csir_dump csir_dump_small \
	sir2byte sir2gif sir2gif_ez sir_ez_example sir2bmp \
	tools/sir_modhead3 tools/sir_remapper3 tools/sir_util tools/sir_util2 \
	tools/sirdiff tools/sirdiff_mask tools/sir_extractregion tools/sirmask tools/sir_stats
LIB = libcsir.a


AR = ar
CC = gcc
INSTALL = cp -f -p
MKDIR = mkdir -p
RM = rm -fr
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CC = icc
  AR = xiar
endif

CONFIG_CFLAGS = 
ARFLAGS = ruv
CFLAGS = -Iinclude -m64 -O3 $(CONFIG_CFLAGS) 
LDFLAGS = -L. -lcsir -lm
ifeq ($(LOCALE),JANUSicc)
  $(warning Build location: $(LOCALE))
  CFLAGS = -Iinclude -O3 -ipo -no-prec-div -xHost $(CONFIG_CFLAGS) 
  LDFLAGS = -L. -lcsir -lm 
  ARFLAGS = crs
endif
ifeq ($(LOCALE),JANUSdebug)
  $(warning Build location: $(LOCALE))
  CC = icc
  CFLAGS = -Iinclude $(CONFIG_CFLAGS) -g -debug all
endif


all : $(EXES) $(LIB)

install :
	$(MKDIR) $(BINDIR)
	$(INSTALL) $(EXES) $(BINDIR)

csirexample : csirexample.c libcsir.a
	$(CC) $(CFLAGS) -ocsirexample csirexample.c $(LDFLAGS)

viewsirhead : viewsirhead.c libcsir.a
	$(CC) $(CFLAGS) -oviewsirhead viewsirhead.c $(LDFLAGS)

csir_dump : csir_dump.c libcsir.a
	$(CC) $(CFLAGS) -ocsir_dump csir_dump.c $(LDFLAGS)

csir_dump_small : csir_dump_small.c libcsir.a
	$(CC) $(CFLAGS) -ocsir_dump_small csir_dump_small.c $(LDFLAGS)

csir_locmap : csir_locmap.c libcsir.a
	$(CC) $(CFLAGS) -ocsir_locmap csir_locmap.c $(LDFLAGS)

sir2byte : sir2byte.c libcsir.a
	$(CC) $(CFLAGS) -osir2byte sir2byte.c $(LDFLAGS)

sir2gif : sir2gif.c libcsir.a
	$(CC) $(CFLAGS) -osir2gif sir2gif.c $(LDFLAGS)

sir2gif_ez : sir2gif_ez.c libcsir.a
	$(CC) $(CFLAGS) -osir2gif_ez sir2gif_ez.c $(LDFLAGS)

sir2tiff_ez : sir2tiff_ez.c libcsir.a
	$(CC) $(CFLAGS) -osir2tiff_ez sir2tiff_ez.c $(LDFLAGS)

sir2bmp : sir2bmp.c libcsir.a
	$(CC) $(CFLAGS) -osir2bmp sir2bmp.c $(LDFLAGS)

sir_ez_example : sir_ez_example.c libcsir.a
	$(CC) $(CFLAGS) -osir_ez_example sir_ez_example.c $(LDFLAGS)

# create static c routines library
libcsir.a : lib/sir_io.c lib/sir_ez.c lib/sir_geom.c 
	rm -f *.o
	rm -f libcsir.a
	$(CC) $(CFLAGS) -c lib/sir_io.c
	$(CC) $(CFLAGS) -c lib/sir_ez.c
	$(CC) $(CFLAGS) -c lib/sir_geom.c
	$(AR) $(ARFLAGS) libcsir.a *.o
	$(RM) *.o

clean :
	$(RM) $(EXES) $(LIB) *.o

tools/sir_modhead3 : tools/sir_modhead3.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sir_modhead3 tools/sir_modhead3.c $(LDFLAGS)

tools/sir_remapper3 : tools/sir_remapper3.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sir_remapper3 tools/sir_remapper3.c $(LDFLAGS)

tools/sir_util : tools/sir_util.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sir_util tools/sir_util.c $(LDFLAGS)

tools/sir_util2 : tools/sir_util2b.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sir_util2 tools/sir_util2b.c $(LDFLAGS)

tools/sirdiff : tools/sirdiff.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sirdiff tools/sirdiff.c $(LDFLAGS)

tools/sirdiff_mask : tools/sirdiff_mask.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sirdiff_mask tools/sirdiff_mask.c $(LDFLAGS)

tools/sirmask : tools/sirmask.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sirmask tools/sirmask.c $(LDFLAGS)

tools/sir_extractregion : tools/sir_extractregion.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sir_extractregion tools/sir_extractregion.c $(LDFLAGS)

tools/sir_stats : tools/sir_stats.c libcsir.a
	$(CC) $(CFLAGS) -o tools/sir_stats tools/sir_stats.c $(LDFLAGS)


