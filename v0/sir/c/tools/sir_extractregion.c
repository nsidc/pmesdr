/*
   program to read BYU a SIR image file, extract a sub region, and
   wrie a new SIR format image file

   Written by DGL 13 Feb. 2002
   Modified by DGL 25 Jul. 2005 + update EASE grid header computation
   
   Written in ANSI C.

   should be linked with 

   sir_ez.c
   sir_geom.c
   sir_io.c

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "sir_ez.h"  /* get easy sir routine interface */


int main(int argc, char **argv)
{
  int   ierr, i, j, nsx2, nsy2, x1, y1, x2, y2, ix, iy;
  float rx1, ry1, rx2, ry2;
  float alon, alat, a02, b02, xdeg2, ydeg2;
  char  outfname[180], temp[180];

  float *stval;        /* pointer to input image storage */
  float *stval2;       /* pointer to output image storage */

  sir_head head;       /* sir_ez defined structure to store SIR header info */
	
  fprintf(stdout,"BYU SIR subregion extraction program\n") ;
  if(argc < 7) {
    fprintf(stdout,"\nusage: %s file flag LLx LLy URx URy <out>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file    =  input SIR file\n");
    fprintf(stdout,"   flag    =  corner option flag\n");
    fprintf(stdout,"              P: pixel coordinates  L: lat/lon coordinates\n");
    fprintf(stdout,"   LLx,LLy =  lower-left corner coordinates\n");
    fprintf(stdout,"   URx,URy =  Upper-right corner coordinates\n");
    fprintf(stdout,"   out     =  (optional) output SIR file.  Default=file.SUB\n");
    return(0);
  }

  /* initialize sir_ez structure and read in input SIR file */

  sir_init_head(&head);
  ierr = get_sir(argv[1], &head, &stval);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR header from file '%s'\n",argv[1]);
     exit(-1);
  }
  sir_update_head(&head, stval);

  fprintf(stdout,"\nSIR file header: '%s'\n",argv[1]);
  print_sir_head(stdout, &head);

/* test transformations at image corners (may get errors for polar image) */

  if (*(argv[2]) == 'L' || *(argv[2]) == 'l') { /* lat/lon coordinates */

    alon = atof(argv[3]);
    alat = atof(argv[4]);
    i = sir_latlon2pix(alon, alat, &rx1, &ry1, &head);  /* (lon,lat) -> (x, y) */
    j = sir_pix(rx1, ry1, &x1, &y1, &head);  /* (x,y) -> (ix,iy) */

    alon = atof(argv[5]);
    alat = atof(argv[6]);
    i = sir_latlon2pix(alon, alat, &rx2, &ry2, &head);
    j = sir_pix(rx2, ry2, &x2, &y2, &head);  /* (x,y) -> (ix,iy) */

  } else {                                      /* pixel coordinates */

    x1 = atoi(argv[3]);
    y1 = atoi(argv[4]);
    x2 = atoi(argv[5]);
    y2 = atoi(argv[6]);
    
  }

  /* ensure selected locations are completely contained within image and
     are in correct LL, UR arrangement */

  if (x1 < 1) x1 = 1;
  if (y1 < 1) y1 = 1;
  if (x2 < 1) x2 = 1;
  if (y2 < 1) y2 = 1;
  
  if (x1 > head.nsx) x1 = head.nsx;
  if (y1 > head.nsy) y1 = head.nsy;
  if (x2 > head.nsx) x2 = head.nsx;
  if (y2 > head.nsy) y2 = head.nsy;

  if (x2 < x1) {
    i = x1;
    x1 = x2;
    x2 = 1;
  }
  if (y2 < y1) {
    i = y1;
    y1 = y2;
    y2 = 1;
  }

  /* compute subregion size and echo final corners to user */
  
  nsx2 = x2 - x1 + 1;
  nsy2 = y2 - y1 + 1;
  
  printf("Image sizes: (in) %d x %d  (out) %d x %d\n",head.nsx,head.nsy,nsx2,nsy2);
  sir_pix2latlon((float) x1, (float) y1, &alon, &alat, &head);  /* (ix,iy) -> (lat,lon) */
  printf(" LL pixel location: %d, %d  Lon,Lat: %f, %f\n",x1,y1,alon,alat);
  sir_pix2latlon((float) x2, (float) y2, &alon, &alat, &head);  /* (ix,iy) -> (lat,lon) */
  printf(" UR pixel location: %d, %d  Lon,Lat: %f, %f\n",x2,y2,alon,alat);
  
  if (nsx2 < 1 || nsy2 < 1) {
    printf("*** zero size subregion, exiting...\n");
    exit(-1);
  }
  
  /* compute new image projection information */

  xdeg2 = head.xdeg;
  ydeg2 = head.ydeg;
  
  switch (head.iopt) {

  case 1:   /* Lambert */
  case 2:
    a02 = (x1 - 1) / head.ascale + head.a0;
    b02 = (y1 - 1) / head.bscale + head.b0;
    break;

  case 11:  /* EASE grid */
  case 12:
  case 13:
    a02 = head.a0 + (x1 - 1);
    b02 = head.b0 + (y1 - 1);
    break;
    
  case 5:   /* polar stereographic */
    a02 = (x1 - 1) * head.ascale + head.a0;
    b02 = (y1 - 1) * head.bscale + head.b0;
    break;

  case -1:  /* image only */
  case 0:   /* lat/lon */
  default:  /* unknown */
    a02 = (x1 - 1) * head.xdeg / (float) head.nsx + head.a0;
    b02 = (y1 - 1) * head.ydeg / (float) head.nsy + head.b0;
    xdeg2 = (float) nsx2 * head.xdeg / (float) head.nsx;
    ydeg2 = (float) nsy2 * head.ydeg / (float) head.nsy;
    break;
  }

  /* allocate new subregion image array */

  stval2 = (float *) malloc(sizeof(float) * nsx2 * nsy2);
  if (stval2 == NULL) {
     fprintf(stderr,"*** ERROR: subregion image memory allocation failure...\n");
     exit(-1);
  }

  /* copy data from subregion into new array */

  for (ix = x1; ix <= x2; ix++)
     for (iy = y1; iy <= y2; iy++) {
       i = (iy-1) * head.nsx + ix - 1;  /* word number (1..nsx*nsy) of pixel (ix,iy) */
       j = (iy-y1) * nsx2 + ix - x1;  /* word number (1..nsx2*nsy2) of pixel (ix,iy) */
       *(stval2 + j) = *(stval + i);
     }
					 
  /* generate new header values, reusing original header */

  head.nsx = nsx2;
  head.nsy = nsy2;
  head.a0 = a02; 
  head.b0 = b02;
  head.xdeg = xdeg2; 
  head.ydeg = ydeg2;

  sprintf(temp,"SubReg: %s",head.title);
  strncpy(head.title,temp,80);

  sprintf(temp,"BYU sir_extractregion - %s",head.crproc);
  strncpy(head.crproc,temp,100);

/* write a sir format file */

  if (argc > 7)
    strncpy(outfname,argv[7],180);
  else
    sprintf(outfname,"%s.SUB",argv[1]);

  fprintf(stdout,"\nWriting output SIR file '%s'\n", outfname);
  ierr = put_sir(outfname, &head, stval2);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output SIR file ***\n");
     fflush(stderr);
  }

  return(0);
}

