/*
   program to read BYU SIR image files and test transformation routines

   Written by DGL 4 Nov 2000
   Updated by DGL 14 Feb 2002 + added call to sir_update_head

   Written in ANSI C. 

   This simple program reads a BYU sir-format input file using the
   easy sir routines.  The program creates
   a scaled byte array image which is dumped to a file, and optionally
   writes out a copy of the sir file.  Several of the standard geometric
   transformation routines are illustrated

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
  FILE  *imf, *omf;
  int   i, j, k, nsx, nsy;
  int   ix, iy, ix1, iy1;
  float x, y, x1, y1, alon, alat;
  char  outfname[80];
  int   ierr;
  float *stval;        /* pointer to image storage */
  char  *data;         /* pointer to byte array storage */
  int   nlines, width;

  sir_head head;
	
  float am, amag, smin, smax;
  float scale, scaleoffset;


  fprintf(stdout,"BYU SIR easy interface c example program\n") ;
  if(argc < 2) {
    fprintf(stdout,"\nusage: %s file out1 out2 <dmin> <dmax>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file  =  input SIR file\n");
    fprintf(stdout,"   out1  =  output byte file\n");
    fprintf(stdout,"   out2  =  output SIR file\n");
    fprintf(stdout,"   dmin  =  min saturation value (optional)\n");
    fprintf(stdout,"   dmax  =  max saturation value (optional)\n"); 
    return(0);
  }

  /* there are several approaches to reading a SIR file supported
     by the easy SIR interface routines.  Two examples are shown below */

#define WAY1
#ifdef WAY1

  sir_init_head(&head);
  ierr = get_sir(argv[1], &head, &stval);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR header from file '%s'\n",argv[1]);
     exit(-1);
  }
  sir_update_head(&head, stval);  /* fix old-style header information */

#else  /* more explict method */

  imf = fopen(argv[1],"rb"); 
  if (imf == NULL) {
     fprintf(stdout,"ERROR: cannot open input file: %s\n",argv[1]); 
     exit(-1);
  }

/* get SIR image header information */

  ierr = get_sir_head_file(imf, &head);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR header from file '%s'\n",argv[1]);
     exit(-1);
  }


  /* allocate storage space for image data */
  
  stval = (float *) malloc(sizeof(float) * head.nsx * head.nsy);
  if (stval == NULL) {
     fprintf(stderr,"*** ERROR: image memory allocation failure...\n");
     exit(-1);
  }

  /* read sir image data */

  ierr = read_sir_data(imf, nhead, idatatype, nsx, nsy, ioff, iscale, stval);
  if (ierr < 0) {
    fprintf(stderr,"*** ERROR: error reading image...\n");
    exit(-1);
  }
  
  fprintf(stdout,"SIR input file successfully read\n");

  sir_update_head(&head, stval);  /* fix old-style header information */

#endif

  fprintf(stdout,"\nSIR file header: '%s'\n",argv[1]);
  print_sir_head(stdout, &head);

/* test transformations at image corners (may get errors for polar image) */
  
  nsx=head.nsx;
  nsy=head.nsy;

  printf("\nSIR geometric transformation test results at corners of image\n");
  for (ix = 1; ix <= nsx; ix = ix + nsx - 1)
    for (iy = 1; iy <= nsy; iy = iy + nsy - 1) {
      i = sir_lex(ix, iy, &head);  /* get image pixel index  0 ... nsx*nxy-1 */
      sir_pix2latlon((float) ix, (float) iy, &alon, &alat, &head);  /* (ix,iy) -> (lat,lon) */
      j = sir_latlon2pix(alon, alat, &x1, &y1, &head); /* (lat,lon) -> (x,y) */
      j = sir_pix(x1, y1, &ix1, &iy1, &head);  /* (x,y) -> (ix,iy) */
      printf(" Pixel (%d,%d) Lat,Lon (%3.3f,%7.3f) at\n       (%5.3f,%5.3f) (%d,%d) Value: %f\n",
	     ix,iy,alat,alon,x1,y1,ix1,iy1,*(stval+i)); 
     };

  if (argc < 3) return(0);
  
  omf = fopen(argv[2],"wb"); 
  if (omf == NULL) {
    fprintf(stderr,"ERROR: cannot open output byte file: '%s'\n",argv[2]); 
    return(-1);
  }
  fprintf(stdout,"\nWriting output byte file '%s'\n", argv[2]);

  smin = head.v_min;
  smax = head.v_max;

  if (argc > 4) sscanf(argv[4],"%f",&smin); 
  if (argc > 5) sscanf(argv[5],"%f",&smax); 
  if (smax - smin == 0.0) smax = smin + 1.0;
  fprintf(stdout,"Byte Min, Max: %f , %f\n",smin,smax);


  /* allocate space for byte array image and create it */

  width = head.nsx;
  nlines = head.nsy;
  data = (char *) malloc(sizeof(char) * width * nlines);
  if (data == NULL) {
    fprintf(stderr,"*** ERROR: byte memory allocation failure...\n");
    return(-1);
  }

  scale = (smax-smin);
  if (scale > 0.) scale = 255./ scale;
  scaleoffset = smin;

  for (i = 0; i < width*nlines; i++) {
    amag = *(stval+i);

    /* scale floating point to byte values */

    am = scale * (amag - scaleoffset);
    if (am > 255.) am = 255.;		/* check overflow */
    if (am < 0.) am = 0.;		/* check underflow */
    *(data+i) = (char)((int)(am));      /* byte array */
  }

  fwrite(data, sizeof(char), width*nlines, omf);
  fclose(omf);

  fprintf(stdout,"BYTE output file successfully written\n");

  if (argc < 4) return(0);

  smin=1.e25;
  smax=-1.e25;
  for (i=0; i< nsx * nsy; i++)
    if (*(stval+i)> head.anodata) {
      smin=(smin > *(stval+i) ? *(stval+i) : smin);
      smax=(smax < *(stval+i) ? *(stval+i) : smax);
    }
  fprintf(stdout,"Data Min, Max: %f , %f\n",smin,smax);

/* write a sir format file */

  strncpy(outfname,argv[3],80);
  fprintf(stdout,"\nWriting output SIR file '%s' using write_sir3\n", outfname);

  ierr = put_sir(outfname, &head, stval);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output SIR file ***\n");
     fflush(stderr);
  }

  return(0);
}

