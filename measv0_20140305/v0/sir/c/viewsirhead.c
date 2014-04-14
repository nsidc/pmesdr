/*
   program to read BYU SIR image file header and print out

   This simple program reads the header of a BYU sir-format input file
   and writes out information to stdout.

   should be linked with sir_io.c

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* standard SIR routine function prototypes */

#include "sir3.h"



int main(int argc, char **argv)
{
  FILE  *imf;
  int   ierr;

/* SIR file header information */

  float xdeg, ydeg, ascale, bscale, a0, b0;  /* projection parameters */
  int   nsx, nsy, iopt;                      /* projection size, type */
  float v_min, v_max, anodata;
  int   ioff, iscale, iyear, isday, ismin, ieday, iemin;
  int   ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc;
  int   iregion, itype, nhead, ndes, nhtype, idatatype, ldes, nia;
  int   ipol, ifreqhm, ispare1;
  char  title[101], sensor[41], crproc[101], type[139], tag[101], crtime[29];

#define MAXDES 1024
  char  descrip[MAXDES+1];
#define MAXI 128
  short iaopt[MAXI];

  fprintf(stdout,"BYU SIR viewsirhead program\n") ;
  if(argc < 2) {
    fprintf(stdout,"\nusage: %s file \n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file  =  input SIR file name\n");
    return(0);
  }

  imf = fopen(argv[1],"rb"); 
  if (imf == NULL) {
     fprintf(stdout,"ERROR: cannot open input file: %s\n",argv[1]); 
     exit(-1);
  }

/* get SIR image header information */

  ierr = read_sir_header3(imf, &nhead, &ndes, &nhtype,
			  &idatatype, &nsx, &nsy, 
			  &xdeg, &ydeg, &ascale, &bscale, &a0, &b0, 
			  &ixdeg_off, &iydeg_off, &ideg_sc, &iscale_sc,
			  &ia0_off, &ib0_off, &i0_sc,
			  &ioff, &iscale, &iyear, &isday, &ismin, &ieday, &iemin, 
			  &iregion, &itype, &iopt, &ipol, &ifreqhm, &ispare1,
			  &anodata, &v_min, &v_max,
			  sensor, title, type, tag, crproc, crtime, 
			  MAXDES, descrip, &ldes, MAXI, iaopt, &nia);  
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR header from file '%s'\n",argv[1]);
     exit(-1);
  }

  fprintf(stdout,"\nSIR file header: '%s'\n",argv[1]);

  print_head3(stdout, nhead, ndes, nhtype, idatatype, nsx, nsy, 
	      xdeg, ydeg, ascale, bscale, a0, b0, 
	      ixdeg_off, iydeg_off, ideg_sc, iscale_sc,
	      ia0_off, ib0_off, i0_sc,
	      ioff, iscale, iyear, isday, ismin, ieday, iemin, 
	      iregion, itype, iopt, ipol, ifreqhm, ispare1,
	      anodata, v_min, v_max,
	      sensor, title, type, tag, crproc, crtime, 
	      descrip, ldes, iaopt, nia);  

  return(0);
}

