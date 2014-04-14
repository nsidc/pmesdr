/*
   program to read BYU SIR image files and create a bmp file

   Written DGL Aug. 8, 1997
   Revised by DGL Nov. 1, 2000   + version 3.0 header
   Revised by DGL Apr. 13, 2009  + WriteBMP routine

   link with sir_io.c

   note: SWAP is defined in sir3.h

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sir3.h"

/* function prototypes - also available in sir3.h */

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

void print_head(FILE *omf, int nhead, int ndes, int nhtype, int idatatype, int nsx,
		int nsy, float xdeg, float ydeg, float ascale,
		float bscale, float a0, float b0, 
		int ioff, int iscale, int iyear, int isday, int ismin,
		int ieday, int iemin, int iregion, int itype, int iopt,
		int ipol, int ifreqhm, int ispare1,
		float anodata, float v_min, float v_max,
		char *sensor, char *title, char *type, char *tag,
		char *crproc, char *crtime, 
		char *descrip, int ldes, short *iaopt, int nia);

int read_sir_data(FILE *imf, int nhead, int idatatype, 
		  int nsx, int nsy, int ioff, int iscale, float *stval);

int WriteBMP(char *fname, char *pic, int w, int h, char *rmap, char *gmap, char *bmap);


/* main routine */


int main(int argc, char **argv)
{
  FILE *imf;
  int i, j;
  int ix, iy, ix1, iy1;
  float x, y, x1, y1, thelon, thelat, alon, alat, am, amag;
  char outfname[80];

  char rtab[256], gtab[256], btab[256];
	
  float smin, smax;
  float s, soff;
  float scale, scaleoffset;

/* SIR file header information */

  float v_min, v_max, anodata;
  float xdeg, ydeg, ascale, bscale, a0, b0;  /* SIR header location info */
  int iopt;
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
  short *a;
  char *data;
  int nlines, width;

  union 
  {
    short i2[2];
    float f2;
  } un;

  fprintf(stdout,"BYU SIR to BMP conversion program\n") ;
  if (argc < 3) {
    fprintf(stdout,"\nusage: %s sir_in bmp_out <dmin> <dmax> <ctab>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   sir_in  =  input SIR file\n");
    fprintf(stdout,"   bmp_out =  output bmp file\n");
    fprintf(stdout,"   dmin    =  min saturation value (optional,def=vmin)\n");
    fprintf(stdout,"   dmax    =  max saturation value (optional,def=vmax)\n");
    fprintf(stdout,"   ctab    =  color table file (optional,def=greyscale)\n");
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
  print_head(stdout, nhead, ndes, nhtype, idatatype, nsx, nsy, 
	     xdeg, ydeg, ascale, bscale, a0, b0, 
	     ioff, iscale, iyear, isday, ismin, ieday, iemin, 
	     iregion, itype, iopt, ipol, ifreqhm, ispare1,
	     anodata, v_min, v_max,
	     sensor, title, type, tag, crproc, crtime, 
	     descrip, ldes, iaopt, nia);  

  rewind(imf);

  smin = v_min;
  smax = v_max;

  if (argc > 3) sscanf(argv[3],"%f",&smin); 
  if (argc > 4) sscanf(argv[4],"%f",&smax); 
  if (smax - smin == 0.0) smax = smin + 1.0;
  fprintf(stdout,"\nBMP Min, Max: %f , %f\n\n",smin,smax);
  fprintf(stdout,"Writing to output bmp file '%s'\n", argv[2]);

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
      am = scale * (*(stval+i*nsx+j) - scaleoffset);
      if (am > 255.) am = 255.;		/* check overflow */
      if (am <   0.) am = 0.;		/* check underflow */
      *(data+(nsy-i-1)*nsx+j) = (char)((int)(am));    /* convert to 8 bit value */
    }
  }

  fprintf(stdout,"SIR input file successfully processed\n");

  /* default greyscale 8-bit color table for bmp file*/

  j=255;
  for (i=0; i<256; i++) {
    if (i > 127)
      rtab[i]=i-256;
    else
      rtab[i]=i;
    gtab[i]=rtab[i];
    btab[i]=rtab[i];
  }
  
  if (argc > 5) { /* read user input color table file */
    fprintf(stdout,"Reading input 8-bit color table file '%s'\n",argv[5]);
    imf = fopen(argv[5],"rb"); 
    if (imf == NULL) {
      fprintf(stdout,"*** ERROR: cannot open color table file: %s\n",argv[5]);
      fprintf(stdout,"Using greyscale color table\n");
    } else {
      if (fread(rtab, sizeof(char), 256, imf) == 0) 
	fprintf(stdout," *** Error reading color table file\n");
      if (fread(gtab, sizeof(char), 256, imf) == 0) 
	fprintf(stdout," *** Error reading color table file\n");
      if (fread(btab, sizeof(char), 256, imf) == 0) 
	fprintf(stdout," *** Error reading color table file\n");
      fclose(imf);
    }
  } else
    fprintf(stdout,"Using greyscale color table\n");

  ierr=WriteBMP(argv[2], data, nsx, nsy, rtab, gtab, btab);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output file ***\n");
     fflush(stderr);
  }
  else
    fprintf(stdout,"Output file successfully written\n");

  return(0);
}


/**************************************************************************/
/* WriteBMP routines 
   this braindead bmp writing routine only writes 8 bit BMP image files 
   w/o compression */


static void putshort(FILE *fp, int i)
{
  int c, c1;
  c = ((unsigned int ) i) & 0xff;  c1 = (((unsigned int) i)>>8) & 0xff;
  putc(c, fp);   putc(c1,fp);
  return;  
}


/*******************************************/
static void putint(FILE *fp,int i)
{
  int c, c1, c2, c3;
  c  = ((unsigned int ) i)      & 0xff;  
  c1 = (((unsigned int) i)>>8)  & 0xff;
  c2 = (((unsigned int) i)>>16) & 0xff;
  c3 = (((unsigned int) i)>>24) & 0xff;
  putc(c, fp);   putc(c1,fp);  putc(c2,fp);  putc(c3,fp);
  return;  
}


/*******************************************/
void writeBMP8(FILE *fp, char *pic8, int w, int h)
{
  int   i,j,c,padw;
  char *pp;

  padw = ((w + 3)/4) * 4; /* 'w' padded to a multiple of 4pix (32 bits) */

  for (i=h-1; i>=0; i--) {
    pp = pic8 + (i * w);
    for (j=0; j<w; j++) putc(*pp++, fp);
    for ( ; j<padw; j++) putc(0, fp);
  }
  return;  
}  


/*******************************************/
int WriteBMP(char *fname, char *pic, int w, int h, char *rmap, char *gmap, char *bmap)
{
  int i, j, nc, nbits=8, bperlin;
  FILE *fp;
  
  fp=fopen(fname,"wb");
  if (fp==NULL) {
    fprintf(stdout,"*** error opening output file %s\n",fname);
    return(-1);
  }

  nc = 1<<nbits;                      /* # of entries in cmap */
  bperlin = ((w * nbits + 31) / 32) * 4;   /* # bytes written per line */

  putc('B', fp);  putc('M', fp);           /* BMP file magic number */

  /* compute filesize and write it */
  i = 14 +                /* size of bitmap file header */
      40 +                /* size of bitmap info header */
      (nc * 4) +          /* size of colormap */
      bperlin * h;        /* size of image data */

  putint(fp, i);
  putshort(fp, 0);        /* reserved1 */
  putshort(fp, 0);        /* reserved2 */
  putint(fp, 14 + 40 + (nc * 4));  /* offset from BOfile to BObitmap */

  putint(fp, 40);         /* biSize: size of bitmap info header */
  putint(fp, w);          /* biWidth */
  putint(fp, h);          /* biHeight */
  putshort(fp, 1);        /* biPlanes:  must be '1' */
  putshort(fp, nbits);    /* biBitCount: 1,4,8, or 24 */
  putint(fp, 0);          /* biCompression:  (none) */
  putint(fp, bperlin*h);  /* biSizeImage:  size of raw image data */
  putint(fp, 75 * 39);    /* biXPelsPerMeter: (75dpi * 39" per meter) */
  putint(fp, 75 * 39);    /* biYPelsPerMeter: (75dpi * 39" per meter) */
  putint(fp, nc);         /* biClrUsed: # of colors used in cmap */
  putint(fp, nc);         /* biClrImportant: same as above */

  /* write out the colormap */
  for (i=0; i<nc; i++) {
    putc(bmap[i],fp);
    putc(gmap[i],fp);
    putc(rmap[i],fp);
    putc(0,fp);
  }

  /* write out the image */
  writeBMP8 (fp, pic, w, h);

  if (ferror(fp) || feof(fp)) return -1;
  
  return 0;
}



