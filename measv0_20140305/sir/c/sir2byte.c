/*
   program to read BYU SIR image files and write out a byte file

   Written by DGL March 26, 1997
   Modified by DGL 9 Feb. 2002 + remove redundant code, version 3.0 header

   Written in ANSI C.

   link with sir_io.c
   uses sir3.h

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* function prototypes in sir3.h */

#include "sir3.h"


int main(int argc, char **argv)
{
  FILE *imf, *omf;
  int i, j;
  float smin, smax, am;
  float scale, scaleoffset;


/* SIR file header information */

  float xdeg, ydeg, ascale, bscale, a0, b0;
  int iopt;
  float v_min, v_max, anodata;
  int nsx, nsy, ioff, iscale, iyear, isday, ismin, ieday, iemin;
  int ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc;
  int iregion, itype, nhead, ndes, nhtype, idatatype, ldes, nia;
  int ipol, ifreqhm, ispare1;
  char title[101], sensor[41], crproc[101], type[139], tag[101], crtime[29];

#define MAXDES 1024
  char descrip[MAXDES+1];
#define MAXI 128
  short iaopt[MAXI];

  float *stval;
  int ierr;
  char *data;
  int nlines, width;


  fprintf(stdout,"BYU SIR sir2byte c program\n") ;
  if(argc < 3) {
    fprintf(stdout,"\nusage: %s file out1 <dmin> <dmax>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file  =  input SIR file\n");
    fprintf(stdout,"   out   =  output byte file\n");
    fprintf(stdout,"   dmin  =  min saturation value (optional)\n");
    fprintf(stdout,"   dmax  =  max saturation value (optional)\n"); 
    return(0);
  }

  imf = fopen(argv[1],"r"); 
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

  /* write out SIR file header information */

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

  smin = v_min;
  smax = v_max;

  if (argc > 3) sscanf(argv[3],"%f",&smin); 
  if (argc > 4) sscanf(argv[4],"%f",&smax); 
  if (smax - smin == 0.0) smax = smin + 1.0;
  fprintf(stdout,"\nByte Min, Max: %f , %f\n\n",smin,smax);

  width = nsx;
  nlines = nsy;

  /* declare image storage area */

  stval = (float *) malloc(sizeof(float)*nsx*nsy);
  data = (char *) malloc(sizeof(char)*width*nlines);
  if (stval == NULL || data == NULL) {
     fprintf(stderr,"*** ERROR: memory allocation failure...\n");
     exit(-1);
  }

  /* read image data from file */

  ierr=read_sir_data(imf, nhead, idatatype, nsx, nsy, ioff, iscale, stval);

  if (ierr < 0)
    fprintf(stdout,"ERROR reading SIR input file\n");  
  else
    fprintf(stdout,"SIR input file successfully read\n");  

  scale = (smax-smin);
  if (scale > 0.) 
    scale = 255./ scale;
  else
    scale = 1; 
  scaleoffset = smin;

  /* this could be done faster but shows how SIR image array is indexed */
  
  for (i = 0; i < nsy; i++) {
    for (j = 0; j < nsx; j++){          /* scale floating point to byte values */
      am = scale * (*(stval + i * nsx + j) - scaleoffset);
      if (am > 255.) am = 255.;		/* check overflow */
      if (am <   0.) am = 0.;		/* check underflow */
      *(data + i * width + j) = (char)((int)(am));    /* convert to 8 bit value */
    }
  }
  
  omf = fopen(argv[2],"w"); 
  if (omf == NULL) {
     fprintf(stderr,"ERROR: cannot open output byte file: '%s'\n",argv[2]); 
     exit(-1);
  }
  fprintf(stdout,"Writing output byte file '%s'\n", argv[2]);
  fwrite(data, sizeof(char), width*nlines, omf);
  fclose(omf);

  fprintf(stdout,"BYTE output file successfully written\n");

  return(0);
}

