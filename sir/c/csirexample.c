/*
   program to read BYU SIR image files and test transformation routines

   Written by DGL March 26, 1997
   Updated by DGL Nov. 1, 2000  + version 3 header
   Updated by DGL 14 Feb. 2002  + add call to update_sir_header

   Written in ANSI C. 

   This simple program reads a BYU sir-format input file, creates
   a scaled byte array image which is dumped to a file, and optionally
   writes out a copy of the sir file.  Several of the standard geometric
   transformation routines are also illustrated

   should be linked with sir_geom.c and sir_io.c

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


/* standard SIR routine function prototypes  - also in sir3.h*/

void pixtolatlon(float x, float y, float *alon, float *alat, 
		 int iopt, float xdeg, float ydeg,
		 float ascale, float bscale, float a0, float b0);

void latlon2pix(float alon, float alat, float *x, float *y, 
		int iopt, float xdeg, float ydeg,
		float ascale, float bscale, float a0, float b0);

void f2ipix(float x, float y, int *ix, int *iy, int nsx, int nsy);

int read_sir_header3(FILE *imf, int *nhead, int *ndes, int *nhtype,
		     int *idatatype, int *nsx, int *nsy, 
		     float *xdeg, float *ydeg, 
		     float *ascale, float *bscale, float *a0, float *b0, 
		     int *ixdeg_off, int *iydeg_off, int *idef_sc, int *iscale_sc,
		     int *ia0_off, int *ib0_off, int *i0_sc,
		     int *ioff, int *iscale, int *iyear, 
		     int *isday, int *ismin, int *ieday, int *iemin, 
		     int *iregion, int *itype, int *iopt,
		     int *ispare1, int *ispare2, int *ispare3,
		     float *anodata, float *v_min, float *v_max,
		     char *sensor, char *title, char *type, char *tag,
		     char *crproc, char *crtime, int maxdes, 
		     char *descrip, int *ldes, int maxi, short *iaopt,
		     int *nia);

int read_sir_data(FILE *imf, int nhead, int idatatype, int nsx, int nsy, 
		  int ioff, int iscale, float *stval);

void print_head3(FILE *omf, int nhead, int ndes, int nhtype, int idatatype,
		 int nsx, int nsy, float xdeg, float ydeg, float ascale,
		 float bscale, float a0, float b0, 
		 int ixdeg_off, int iydeg_off, int ideg_sc,
		 int iscale_sc, int ia0_off, int ib0_off, int i0_sc,
		 int ioff, int iscale, int iyear, int isday, int ismin,
		 int ieday, int iemin, int iregion, int itype, int iopt,
		 int ipol, int ifreqhm, int ispare1,
		 float anodata, float v_min, float v_max,
		 char *sensor, char *title, char *type, char *tag, char
		 *crproc, char *crtime, 
		 char *descrip, int ldes, short *iaopt, int nia);

int write_sir3(char *fname, float *stval, int *nhead, int nhtype,
	       int idatatype, int nsx, int nsy, float xdeg, float ydeg, 
	       float ascale, float bscale, float a0, float b0, 
	       int ixdeg_off, int iydeg_off, int idef_sc, int iscale_sc,
	       int ia0_off, int ib0_off, int i0_sc,
	       int ioff, int iscale, int iyear, 
	       int isday, int ismin, int ieday, int iemin, 
	       int iregion, int itype, int iopt,
	       int ipol, int ifreqhm, int ispare1,
	       float anodata, float v_min, float v_max,
	       char *sensor, char *title, char *type, char *tag,
	       char *crproc, char *crtime, 
	       char *descrip, int ldes, short *iaopt, int nia);


int main(int argc, char **argv)
{
  FILE  *imf, *omf;
  int   i, j, k;
  int   ix, iy, ix1, iy1;
  float x, y, x1, y1, alon, alat;
  char  outfname[80];
  int   ierr;
  float *stval;        /* pointer to image storage */
  char  *data;         /* pointer to byte array storage */
  int   nlines, width;
	
  float am, amag, smin, smax;
  float scale, scaleoffset;

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

  fprintf(stdout,"BYU SIR c example program\n") ;
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

  /* read sir image data */

  ierr = read_sir_data(imf, nhead, idatatype, nsx, nsy, ioff, iscale, stval);
  if (ierr < 0) exit(-1);
  fprintf(stdout,"SIR input file successfully read\n");

  /* fix old style header info if needed */

  update_sir_header(stval, nhtype, nsx, nsy, &anodata, &v_min, &v_max);

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

/* test transformations at image corners (may get errors for polar image) */
  
  printf("\nSIR geometric transformation test results at corners of image\n");
  for (ix = 1; ix <= nsx; ix = ix + nsx - 1)
     for (iy = 1; iy <= nsy; iy = iy + nsy - 1) {
	i = (iy-1) * nsx + ix;  /* word number (1..nsx*nsy) of pixel (ix,iy) 
                                   within the image, the row-scanned or
				   lexicographic index */ 
	pixtolatlon((float) ix, (float) iy, &alon, &alat,     /* (ix,iy) -> (lat,lon) */
		    iopt, xdeg, ydeg, ascale, bscale, a0, b0); 
	latlon2pix(alon, alat, &x1, &y1,      /* (lat,lon) -> (x,y) */
		   iopt, xdeg, ydeg, ascale, bscale, a0, b0);
	f2ipix(x1, y1, &ix1, &iy1, nsx, nsy); /* (x,y) -> (ix,iy) */

	printf(" Pixel (%d,%d) Lat,Lon (%3.3f,%7.3f) at\n       (%5.3f,%5.3f) (%d,%d) Value: %f\n",
	       ix,iy,alat,alon,x1,y1,ix1,iy1,*(stval+i-1)); 
     };

  if (argc < 3) return(0);
  
  omf = fopen(argv[2],"wb"); 
  if (omf == NULL) {
    fprintf(stderr,"ERROR: cannot open output byte file: '%s'\n",argv[2]); 
    return(-1);
  }
  fprintf(stdout,"\nWriting output byte file '%s'\n", argv[2]);

  smin = v_min;
  smax = v_max;

  if (argc > 4) sscanf(argv[4],"%f",&smin); 
  if (argc > 5) sscanf(argv[5],"%f",&smax); 
  if (smax - smin == 0.0) smax = smin + 1.0;
  fprintf(stdout,"Byte Min, Max: %f , %f\n",smin,smax);


  /* allocate space for byte array image and create it */

  width = nsx;
  nlines = nsy;
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
    if (*(stval+i)> anodata) {
      smin=(smin > *(stval+i) ? *(stval+i) : smin);
      smax=(smax < *(stval+i) ? *(stval+i) : smax);
    }
  fprintf(stdout,"Data Min, Max: %f , %f\n",smin,smax);

/* write a sir format file */

  strncpy(outfname,argv[3],80);
  fprintf(stdout,"\nWriting output SIR file '%s' using write_sir3\n", outfname);

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
     fprintf(stderr,"*** ERROR writing output file ***\n");
     fflush(stderr);
  }

  return(0);
}

