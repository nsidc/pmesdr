/*
   program to read BYU SIR image file header and generate SIR "images"
   of the original SIR file lat/lon values

   Written by DGL Feb 9, 2002
   Revised by DGL Feb. 23, 2002 + fixed index error

   Written in ANSI C. 

   This simple program reads a BYU SIR-format input file header and creates
   arrays of the latitude and longitude of the file pixels.  These
   values are written to a SIR format file.

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
  FILE  *imf;
  int   i, ix, iy, ierr;
  float x, y, alon, alat, half;
  char  outfname[200];
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

  fprintf(stdout,"BYU SIR csir_locmap example program\n") ;
  if(argc < 2) {
    fprintf(stdout,"\nusage: %s file <ref>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file  =  input SIR file name\n");
    fprintf(stdout,"   <ref> =  (optional) pixel lat,lon reference location\n                L=lower-left corner (default), C=pixel center\n");
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

  /* print SIR file header information out */

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


  /* allocate storage space for image data */
  
  stval = (float *) malloc(sizeof(float) * nsx * nsy);
  if (stval == NULL) {
     fprintf(stderr,"*** ERROR: image memory allocation failure...\n");
     exit(-1);
  }

  /* Don't need to read the image data since it won't be used.  Instead
     fill the image array with the lat/lon data and write out. */

  /* By default, SIR geometric transformations refer to the lower-left corner
     of the image.  However, make it possible to output the center coordinates
     of the pixels. */

  half=0.0;
  if (argc > 2)
    if (*(argv[2]) == 'C' || *(argv[2]) == 'c') half=0.5;
  if (half > 0.1)
    printf(" Using pixel center lat,lon coordinates\n");
  else
    printf(" Using standard lower-left pixel corner lat,lon coordinates\n");
  
  /* The geometry routines compute both latitude and longitude at the same time
     However, to avoid having to have to create two large image arrays, we compute
     the geometry twice, once for the latitude and once for the longitude, storing
     the results in an image array and writing the output file before computing
     the next image */

  /* first do latitude */

  printf("Computing latitude values\n");  
  for (ix = 1; ix <= nsx; ix++)
     for (iy = 1; iy <= nsy; iy++) {
	i = (iy-1) * nsx + ix-1;  /* word number (0..nsx*nsy-1) of pixel (ix,iy) 
				     within the image, the row-scanned or
				     le	xicographic index */ 
	x = (float) ix + half;
	y = (float) iy + half;
	
	pixtolatlon(x, y, &alon, &alat,     /* (ix,iy) -> (lat,lon) */
		    iopt, xdeg, ydeg, ascale, bscale, a0, b0); 
	*(stval+i) = alat;
     }

  /* modify header to storage dynamic range of latitude */

  ioff = -91;
  iscale = 360;
  anodata = -91.0;
  v_min = -90.0;
  v_max = 90.0;
  itype = 31;     /* set type to latitude */
  idatatype = 2;  /* default two byte integer storage */

  sprintf(crproc,"BYU csir_locmap");
  sprintf(type,"Lat. of %s",argv[1]);
  sprintf(outfname,"%s_lat",argv[1]);


  /* write out latitude array as a SIR-format file */

  fprintf(stdout," Writing latitude to '%s'\n", outfname);

  ierr = write_sir3(outfname, stval, &nhead, nhtype, 
		    idatatype, nsx, nsy, 
		    xdeg, ydeg, ascale, bscale, a0, b0, 
		    ixdeg_off, iydeg_off, ideg_sc, iscale_sc,
		    ia0_off, ib0_off, i0_sc,
		    ioff, iscale, iyear, isday, ismin, ieday, iemin, 
		    iregion, itype, iopt, ipol, ifreqhm, ispare1,
		    anodata, v_min, v_max, sensor, title, type, tag,
		    crproc, crtime, descrip, ldes, iaopt, nia);

  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing latitude output file ***\n");
     fflush(stderr);
  }


  /* now do longitude */
  
  printf("Computing longitude values\n");
  for (ix = 1; ix <= nsx; ix++)
     for (iy = 1; iy <= nsy; iy++) {
	i = (iy-1) * nsx + ix-1;  /* word number (0..nsx*nsy-1) of pixel (ix,iy) 
				     within the image, the row-scanned or
				     le	xicographic index */ 
	x = (float) ix + half;
	y = (float) iy + half;
	
	pixtolatlon(x, y, &alon, &alat,     /* (ix,iy) -> (lat,lon) */
		    iopt, xdeg, ydeg, ascale, bscale, a0, b0);

	if (alon > 180.0) alon = alon - 360.0;
	if (alon < -180.0) alon = alon + 360.0;
	
	*(stval+i) = alon;
     }

  /* modify header to storage dynamic range of longitude */

  ioff = -181;
  iscale = 180;
  anodata = -181.0;
  v_min = -180.0;
  v_max = 180.0;
  itype = 30;     /* set type to longitude */

  sprintf(type,"Lon. of %s",argv[1]);
  sprintf(outfname,"%s_lon",argv[1]);
  

  /* write out longitude array as a SIR-format file */

  fprintf(stdout," Writing longitude to '%s'\n", outfname);

  ierr = write_sir3(outfname, stval, &nhead, nhtype, 
		    idatatype, nsx, nsy, 
		    xdeg, ydeg, ascale, bscale, a0, b0, 
		    ixdeg_off, iydeg_off, ideg_sc, iscale_sc,
		    ia0_off, ib0_off, i0_sc,
		    ioff, iscale, iyear, isday, ismin, ieday, iemin, 
		    iregion, itype, iopt, ipol, ifreqhm, ispare1,
		    anodata, v_min, v_max, sensor, title, type, tag,
		    crproc, crtime, descrip, ldes, iaopt, nia);

  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing latitude output file ***\n");
     fflush(stderr);
  }

  return(0);
}

