/*
   program to read BYU SIR image file header and dump to an ASCII file

   Written by DGL Feb 9, 2002
   Updated by DGL 14 Feb. 2002 + added call to update_sir_header

   Written in ANSI C. 

   This simple program reads a BYU SIR-format input file and
   dumps the pixel values, locations, and coordinates to an ASCII file
   with one line per pixel

   should be linked with sir_geom.c and sir_io.c
   uses sir3.h

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* standard SIR routine function prototypes in sir3.h */

#include "sir3.h"


int main(int argc, char **argv)
{
  FILE  *imf, *omf;
  int   i, ix, iy, ierr;
  float x, y, alon, alat, half;
  char  ch, outfname[200];
  float *stval;        /* pointer to image storage */
	
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

  fprintf(stdout,"BYU SIR csir_dump program - converts SIR to ASCII\n") ;
  if(argc < 2) {
    fprintf(stdout,"\nusage: %s file_in file_out <ref> <head>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file_in  =  input SIR file name\n");
    fprintf(stdout,"   file_out =  output ASCII file name\n");
    fprintf(stdout,"   <ref>    =  (optional) pixel lat,lon reference location\n                L=lower-left corner (default), C=pixel center\n");
    fprintf(stdout,"   <head>   =  (optional) head output flag\n                H=header included in output file; N=not included (default)\n");
    fprintf(stdout,"\n one pixel/line: x_index y_index image_value longitude latitude\n");

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

  /* allocate storage space for image data */
  
  stval = (float *) malloc(sizeof(float) * nsx * nsy);
  if (stval == NULL) {
     fprintf(stderr,"*** ERROR: image memory allocation failure...\n");
     exit(-1);
  }

  /* read in SIR image data from file*/

  ierr=read_sir_data(imf, nhead, idatatype, nsx, nsy, ioff, iscale, stval);

  /* fix old style header info if needed */

  update_sir_header(stval, nhtype, nsx, nsy, &anodata, &v_min, &v_max);

  /* print SIR file header information to stdout */

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

  /* By default, SIR geometric transformations refer to the lower-left corner
     of the image.  However, make it possible to output the center coordinates
     of the pixels. */

  half=0.0;
  if (argc > 3)
    if (*(argv[3]) == 'C' || *(argv[3]) == 'c') half=0.5;
  if (half > 0.1)
    printf(" Using pixel center lat,lon coordinates\n");
  else
    printf(" Using standard lower-left pixel corner lat,lon coordinates\n");
  printf(" Pixels: %d x %d = %d  No data value: %f\n",nsx,nsy,nsx*nsy,anodata);
  
  /* open output ASCII file */

  omf = fopen(argv[2],"w"); 
  if (omf == NULL) {
     fprintf(stderr,"ERROR: cannot open output file: '%s'\n",argv[2]); 
     exit(-1);
  }
  fprintf(stdout,"\nWriting output to '%s'\n", argv[2]);

  /* write header to output file if desired*/

  if (argc > 4)
    if ((*argv[4] == 'H') || (*argv[4] == 'h'))
      print_head3(omf, nhead, ndes, nhtype, idatatype, nsx, nsy, 
		  xdeg, ydeg, ascale, bscale, a0, b0, 
		  ixdeg_off, iydeg_off, ideg_sc, iscale_sc,
		  ia0_off, ib0_off, i0_sc,
		  ioff, iscale, iyear, isday, ismin, ieday, iemin, 
		  iregion, itype, iopt, ipol, ifreqhm, ispare1,
		  anodata, v_min, v_max,
		  sensor, title, type, tag, crproc, crtime, 
		  descrip, ldes, iaopt, nia);  

  for (iy = 1; iy <= nsy; iy++)  
    for (ix = 1; ix <= nsx; ix++) {

      i = (iy-1) * nsx + ix - 1; /*word number (0..nsx*nsy-1) of pixel (ix,iy) 
				   within the image, the row-scanned or
				   lexicographic index */ 
      x = (float) ix + half;
      y = (float) iy + half;
      
      pixtolatlon(x, y, &alon, &alat,     /* (ix,iy) -> (lat,lon) */
		  iopt, xdeg, ydeg, ascale, bscale, a0, b0); 
      
      fprintf(omf,"%d %d %f %f %f\n",ix,iy,*(stval+i),alon,alat);
    }

  fclose(omf);
  return(0);
}

