#
# generic makefile for SIR format C programs/library
# written by R. Chen Feb. 2001   JPL PO.DAAC
#
# be sure to edit include/sir3.h to reflect machine/compiler type

EXES = csirexample viewsirhead csir_locmap csir_dump csir_dump_small \
	sir2byte sir2gif sir2gif_ez sir_ez_example sir2bmp libcsir.a \
	tools/sir_modhead tools/sir_remapper tools/sir_remapper2 \
	tools/sirdiff tools/sir_extractregion tools/sirmask tools/sir_stats

CC = gcc
#CC = pathcc -m64 -O3
CFLAGS = -Iinclude
LDFLAGS = -L. -lcsir -lm

all : $(EXES)

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

sir_ez_example : sir_ez_example.c libcsir.a
	$(CC) $(CFLAGS) -osir_ez_example sir_ez_example.c $(LDFLAGS)

# create c routines library
libcsir.a : lib/sir_io.c lib/sir_ez.c lib/sir_geom.c
	rm -f *.o
	rm -f libcsir.a
	$(CC) $(CFLAGS) -c lib/sir_io.c
	$(CC) $(CFLAGS) -c lib/sir_ez.c
	$(CC) $(CFLAGS) -c lib/sir_geom.c
	ar cr libcsir.a *.o
	rm -f *.o

sir2bmp : sir2bmp.c libcsir.a
	$(CC) $(CFLAGS) -osir2bmp sir2bmp.c $(LDFLAGS)

clean :
	rm -f $(EXES) *.o

tools/sir_modhead : tools/sir_modhead.c libcsir.a
	$(CC) $(CFLAGS) -otools/sir_modhead tools/sir_modhead.c $(LDFLAGS)

tools/sir_remapper : tools/sir_remapper.c libcsir.a
	$(CC) $(CFLAGS) -otools/sir_remapper tools/sir_remapper.c $(LDFLAGS)

tools/sir_remapper2 : tools/sir_remapper2.c libcsir.a
	$(CC) $(CFLAGS) -otools/sir_remapper2 tools/sir_remapper2.c $(LDFLAGS)

tools/sirdiff : tools/sirdiff.c libcsir.a
	$(CC) $(CFLAGS) -otools/sirdiff tools/sirdiff.c $(LDFLAGS)

tools/sirmask : tools/sirmask.c libcsir.a
	$(CC) $(CFLAGS) -otools/sirmask tools/sirmask.c $(LDFLAGS)

tools/sir_extractregion : tools/sir_extractregion.c libcsir.a
	$(CC) $(CFLAGS) -otools/sir_extractregion tools/sir_extractregion.c $(LDFLAGS)

tools/sir_stats : tools/sir_stats.c libcsir.a
	$(CC) $(CFLAGS) -otools/sir_stats tools/sir_stats.c $(LDFLAGS)


