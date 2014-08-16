# 
#########################################################
#  simple makefile for sirtool
#
#  usage:  make -- builds sirtool
#
#  requires prior installation of the fltk library
#
#  written D.G. Long 3/13/2014
#  updated D.G. Long 8/16/2014 + remove duplicate files
#
#########################################################

# local platform definitions for cc and c++ compilers

CC = gcc
CXX = g++
CFLAGS =
CXXFLAGS =
FLTK_INC = 

# location of fltk_config script from fltk installation
FLTK_CONFIG = /auto/users/long/src/linux/fltk/bin/fltk-config
FLTK_LDFLAGS  = -L/usr/X11R6/lib64 `$(FLTK_CONFIG) --ldflags`
FLTK_CFLAGS   = `$(FLTK_CONFIG) --cflags`
FLTK_CXXFLAGS = `$(FLTK_CONFIG) --cxxflags`

#FLTK_CONFIG = /auto/users/long/src/linux/fltk-1.3.2/bin/fltk-config
#FLTK_INC      = `$(FLTK_CONFIG) --includedir`

INC   = -I./ -I../sirlib/c/include/

OBJS  = libsirtool.o \
	sir_io.o \
	sir_geom.o \
	sir_ez.o \
	sir_plot.o \
	ll_nr_fp4.o \
	ll_fpio3.o \
	syms.o \
	ll_util.o \
	ll_ramreplace.o \
	circle.o \
	export.o


INCS  = libsirtool.h \
	../sirlib/c/include/sir3.h \
	export.h \
        ../sirlib/c/include/sir_ez.h

all:	sirtool

sirtool:  sirtool.o $(OBJS)
	$(CXX) $(CXXFLAGS) sirtool.o $(OBJS) $(FLTK_LDFLAGS) -o $@

sirtool.o:  sirtool.cpp $(INCS)
	$(CXX) -c $(CXXFLAGS) sirtool.cpp $(INC) $(FLTK_INC) $(FLTK_CXXFLAGS) -o $@

libsirtool.o:  libsirtool.cpp $(INCS)
	$(CXX) -c $(CXXFLAGS) libsirtool.cpp $(INC) $(FLTK_CXXFLAGS) -o $@

export.o:  export.c
	$(CC) -c $(CFLAGS) export.c $(INC) $(FLTK_CFLAGS) -o $@

sir_io.o:  ../sirlib/c/lib/sir_io.c $(INCS)
	$(CC) -c $(CFLAGS) ../sirlib/c/lib/sir_io.c $(INC) $(FLTK_CFLAGS) -o $@

sir_geom.o:  ../sirlib/c/lib/sir_geom.c $(INCS)
	$(CC) -c $(CFLAGS) ../sirlib/c/lib/sir_geom.c $(INC) $(FLTK_CFLAGS) -o $@

sir_ez.o:  ../sirlib/c/lib/sir_ez.c $(INCS)
	$(CC) -c $(CFLAGS) ../sirlib/c/lib/sir_ez.c $(INC) $(FLTK_CFLAGS) -o $@

sir_plot.o:  sir_plot.c $(INCS)
	$(CC) -c $(CFLAGS) sir_plot.c $(INC) $(FLTK_CFLAGS) -o $@

ll_nr_fp4.o:  ll_nr_fp4.c $(INCS)
	$(CC) -c $(CFLAGS) ll_nr_fp4.c $(INC) $(FLTK_CFLAGS) -o $@

ll_fpio3.o:  ll_fpio3.c $(INCS)
	$(CC) -c $(CFLAGS) ll_fpio3.c $(INC) $(FLTK_CFLAGS) -o $@

ll_util.o:  ll_util.c $(INCS)
	$(CC) -c $(CFLAGS) ll_util.c $(INC) $(FLTK_CFLAGS) -o $@

ll_ramreplace.o:  ll_ramreplace.c $(INCS)
	$(CC) -c $(CFLAGS) ll_ramreplace.c $(INC) $(FLTK_CFLAGS) -o $@

syms.o:  syms.c $(INCS)
	$(CC) -c $(CFLAGS) syms.c $(INC) $(FLTK_CFLAGS) -o $@

circle.o:  circle.c $(INCS)
	$(CC) -c $(CFLAGS) circle.c $(INC) $(FLTK_CFLAGS) -o $@

clean:
	rm -f sirtool.o $(OBJS)

distribution:
	rm -f sirtool sirtool.o $(OBJS)

