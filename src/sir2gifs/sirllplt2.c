/*
   routines to enable conversion of SIR images to byte images with
   a corresponding color map and to modify and plot into the byte images

   Written by DGL March 11, 2000
   Revised by DGL Feb. 9, 2001 + channel color bars, 3 channel output, etc.
   Revised by DGL Mar. 1, 2003 + modified help text

*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* #define SIR2TIFF_BYTESHIFT   /* define for PC */

#include "preprocess.h"

/* define templates for routines from sir image plotting library */


/*  global variables from SIR file header used in location transformations*/

extern int nsx, nsy;
extern float xdeg, ydeg, ascale, bscale, a0, b0;  /* SIR header location info */
extern int iopt;

/* pixel plotting variables and their defaults */

float x1_opt=1.0,y1_opt=1.0,x2_opt=2.0,y2_opt=2.0,d_opt=1.0,a_opt=0.0,t_opt=10.0;
int z_opt=-1,s_opt=0,e_opt=255,i_opt=255,c_opt=255,w_opt=1,latlon_opt=0,
  subregion=0,Ii_opt=0,D_opt=5,n_opt=0,M_opt=0,nc_opt[OMAX];
char r_opt=255,g_opt=255,b_opt=255;
char m_opt[200]=" ", *f_opt;
int npix = 1, ncol, rot_opt = 0, chan_opt=0;

float smin[OMAX], smax[OMAX];    /* image scale range */
char rtab[256], gtab[256], btab[256]; /* color table */
int debug=0;

extern float *stval[OMAX];
extern int nchans;

extern struct { /* pixel plotting stuff */
  char  *a;
  int bytesperpixel;
  int color;
  int tab;
} fpimage_1;

int bpp;  /* bytes per pixel in "byte" image array */


#define NMAX 3

void line_plot_from_file(char *filename, int q, int Ii_opt, int latlon_opt, int c_opt, int w_opt);


/****************************************************************/

int get_input_files(char **argv, int narg, char *base, char *names[])
{
  FILE *fid;
  int i=1, n, j, ii, ncnt=0;
  char line[200], line2[200], optname[200], *c, *c2, *cp;

  if (base != NULL) {
    if (debug) printf(" debug: main input file %s\n",base);
    ncnt=1;
    if ((names[0]=(char *) malloc(sizeof(char)*(1+strlen(base)))) == NULL) {
      fprintf(stderr,"*** ERROR allocating input file name storage in get_input_files***\n");
      exit(-1);
    }
    strcpy(names[0], base);
  }
  
  while (*(argv[i]) == '-' && i <= narg) {   /* optional argument */
    /* printf("Checking input argument %d %s\n",i,argv[i]); */
    if (*(argv[i]+1) == 'f') {  /* check for file if optional argument is file input */
      sscanf(argv[i],"-f%s",optname);
      fprintf(stdout,"Opening options input file '%s'\n",optname);
      fid=fopen(optname,"r");
      if (fid == NULL) {
	fprintf(stderr,"*** Could not open options file '%s' ***\n",optname);
	exit(-1);
      }
      while (fscanf(fid,"%s",line2) > 0) {
	if (debug) printf(" debug: options file line: %s\n",line2);
	if (line2[0] == '-' && line2[1] == 'i') {
	  sscanf(line2,"-I%d=%s",&n,line);
	  if (strstr(line2,"=") == NULL) {
	    fprintf(stderr,"*** WARNING: no '=' in i command *** %s\n",line2);
	    sscanf(line2,"-I%d%s",&n,line);
	  }
	  n--;
	  if (n < 0) n=0;
	  if (n > NMAX) n=NMAX-1;
	  if ((names[n]=(char *) malloc(sizeof(char)*(1+strlen(line)))) == NULL) {
	    fprintf(stderr,"*** ERROR allocating input file name storage in get_input_files***\n");
	    exit(-1);
	  }
	  strcpy(names[n],line);
	  ncnt = (n+1 > ncnt? n+1 : ncnt);
	}
	if (line2[0] == '-' && line2[1] == 'I') {
	  sscanf(line2,"-I%d=%s",&n,line);
	  if (strstr(line2,"=") == NULL) {
	    fprintf(stderr,"*** WARNING: no '=' in I command *** %s\n",line2);
	    sscanf(line2,"-I%d%s",&n,line);
	  }
	  n--;
	  if (n < 0) n=0;
	  if (n > NMAX) n=NMAX-1;
	  if (base == NULL)
	    if (names[0] != NULL)
	      base = names[0];
	  if (base == NULL)
	    printf("*** ERROR: base input name not defined ***\n");
	  else {
	    if ((names[n]=(char *) malloc(sizeof(char) * (int) (1+strlen(base)))) == NULL) {
	      fprintf(stderr,"*** ERROR allocating input file name storage in get_input_files***\n");
	      exit(-1);
	    }
	    strcpy(names[n],base);
	    ncnt = (n+1 > ncnt? n+1 : ncnt);
	    
	    cp = line;
	    while ((c = strstr(cp,"S=")) != NULL) { /* start char */
	      sscanf(c+2,"%d",&ii);
	      cp=c+2;
	      j=1;
	      if ((c = strstr(cp,"C=")) != NULL) { /* replacement */
		sscanf(c+2,"%d",&j);
		cp=c+2;
	      }
	      if ((c = strstr(cp,"m=")) != NULL) { /* replacement */
		if (j==0) j=strlen((c+2));
		if ((c2=strchr(c+2,',')) != NULL) j=c2-(c+2);
		strncpy(names[n]+ii-1,(c+2),j);
		cp=c+2;
	      }
	    }
	  }
	}
      }
      if (debug) printf(" debug: end of input options file reached \n");
      fclose(fid);
    }
    
    if (*(argv[i]+1) == 'i') {  /* check for file if optional argument is file input */
      sscanf(argv[i],"-i%d=%s",&n,line);
      if (strstr(argv[i],"=") == NULL) {
	fprintf(stderr,"*** WARNING: no '=' in i command *** %s\n",argv[i]);
	sscanf(argv[i],"-i%d%s",&n,line);
      }
      n--;
      if (n < 0) n=0;
      if (n > NMAX) n=NMAX-1;
      if ((names[n]=(char *) malloc(sizeof(char)*(1+strlen(line)))) == NULL) {
	fprintf(stderr,"*** ERROR allocating input file name storage in get_input_files***\n");
	exit(-1);
      }
      strcpy(names[n],line);
      ncnt = (n+1 > ncnt? n+1 : ncnt);
    }
    if (*(argv[i]+1) == 'I') {  /* check for file if optional argument is file input */
      sscanf(argv[i],"-I%d=%s",&n,line);
      if (strstr(argv[i],"=") == NULL) {
	fprintf(stderr,"*** WARNING: no '=' in I command *** %s\n",argv[i]);
	sscanf(argv[i],"-I%d%s",&n,line);
      }
      n--;
      if (n < 0) n=0;
      if (n > NMAX) n=NMAX-1;
      if (base == NULL)
	if (names[0] != NULL)
	  base = names[0];
      if (base == NULL)
	printf("*** ERROR: base input name not defined ***\n");
      else {
	
	if ((names[n]=(char *) malloc(sizeof(char)*(1+strlen(base)))) == NULL) {
	  fprintf(stderr,"*** ERROR allocating input file name storage in get_input_files***\n");
	  exit(-1);
	}
	strcpy(names[n],base);
	ncnt = (n+1 > ncnt? n+1 : ncnt);
	
	cp = line;
	while ((c = strstr(cp,"S=")) != NULL) { /* start char */
	  sscanf(c+2,"%d",&ii);
	  cp=c+2;
	  j=1;
	  if ((c = strstr(cp,"C=")) != NULL) { /* replacement */
	    sscanf(c+2,"%d",&j);
	    cp=c+2;
	  }
	  if ((c = strstr(cp,"m=")) != NULL) { /* replacement */
	    if (j==0) j=strlen((c+2));
	    if ((c2=strchr(c+2,',')) != NULL) j=c2-(c+2);
	    strncpy(names[n]+ii-1,(c+2),j);
	    cp=c+2;
	  }
	}
      }
    }
    i++;
  }
  if (debug) {
    printf(" debug: program input files: %d \n",ncnt);
    for (i=0;i<ncnt;i++)
      if (names[i] != NULL) 
	printf("     %d: %s\n",i+1,names[i]);
  }
  return(ncnt);
}


#define char_clip(x) ((x) > 127 ? (x): (x)-256);


void plot_initialize(char *rtab, char *gtab, char *btab, int *ncol, int nchans)
{
  int i,j,k,l;
  
  *ncol = 256;  /* number of color table values */

  switch (nchans) {
  case 0:
  case 1:
  default:
    for (i=0; i<256; i++) {   /* default greyscale color table */
      rtab[i]=char_clip(i);
      gtab[i]=rtab[i];
      btab[i]=rtab[i];
    }  
    break;
    
  case 2:
    for (i=0; i<16; i++)   /* split color table */
      for (j=0; j<16; j++) {
	k=i*16 +j;
	rtab[k]=char_clip(i*16);
	gtab[k]=char_clip(j*16);
	/*	btab[k]=rtab[k]; */
      }
    break;
    
  case 3:
    for (i=0; i<6; i++)   /* three-way split table + upper gray*/
      for (j=0; j<6; j++)
	for (l=0; l<6; l++) {
	  k = (i * 6 + j) * 6 + l;
	  rtab[k] = char_clip(i*42);
	  gtab[k] = char_clip(j*42);
	  btab[k] = char_clip(l*42);
	}
    for (i=0; i < *ncol - 6*6*6 - 1; i++) {   /* upper gray*/
      k=i+6*6*6+1;
      rtab[k]=char_clip(i*5+21);
      gtab[k]=rtab[k];
      btab[k]=rtab[k];
    }
    break;
  }
  /*
  for (i=0; i<256; i++)
    printf("color table index %d: %d %d %d\n",i,rtab[i],gtab[i],btab[i]);
  */
}



int plot_pass_one(char **argv)
{
  int i;
  char optname[200], line[200];
  FILE *fid;
  
  /* initially process input options to 
     select scale parameters
     select pixel reduction factor
     extract subregion
     and read in color table information */

  if (debug) printf(" debug: first pass\n");
  i=1;
  while (*(argv[i]) == '-') { /* optional argument */
    if (*(argv[i]+1) == 'd') {  /* debug on */
      printf("Option debug ON\n");
      debug = 1;
    }
    if (debug) printf(" debug: option line: '%s' %c\n",argv[i],*(argv[i]+1));
    if (*(argv[i]+1) == 'p') {  /* pixel scale factor */
      sscanf(argv[i],"-p%d",&npix);
      if (npix == 0) npix = 1;
      if (npix > 16) npix = 16;
      if (npix < -16) npix = -16;
      if (debug) printf("  debug: pixel reduction factor: %d\n",npix);
    } 
    if (*(argv[i]+1) == 'f') {  /* optional argument is file input */
      sscanf(argv[i],"-f%s",optname);
      fid=fopen(optname,"r");
      if (fid == NULL) {
	fprintf(stderr,"*** Could not open options file '%s' ***\n",optname);
	return(-1);
      }
      while (fscanf(fid,"%s",line) > 0) {
	if (debug) printf(" debug: options file line: %s\n",line);
	if (line[0] == '-' && line[1] == 'p') {  /* pixel scale factor */
	  sscanf(line,"-p%d",&npix);
	  if (npix == 0) npix = 1;
	  if (npix > 16) npix = 16;
	  if (npix < -16) npix = -16;
	  if (debug) printf("  debug: pixel reduction factor: %d\n",npix);
	}
	if (line[0] == '-' && line[1] == 'C') {  /* binary color table file */
	  sscanf(line,"-C%s",line);
	  if (debug) printf("debug: binary read color table file: %s\n",line);
	  read_colortable(line,rtab,gtab,btab,0);
	} else if (line[0] == '-' && line[1] == 'A') {  /* ascii color table file */
	  sscanf(line,"-A%s",line);
	  if (debug) printf("debug: ascii read color table file: %s\n",line);
	  read_colortable(line,rtab,gtab,btab,1);
	} else
	  if (line[0] == '-' && !(line[1] == '%' || line[1] == '#') )
	    process_option(line+1,0);
      }
      fclose(fid);
    }
    if (*(argv[i]+1) == 'C') {  /* binary color table file */
      sscanf(argv[i],"-C%s",line);
      if (debug) printf("debug: read binary color table file: %s\n",line);
      read_colortable(line,rtab,gtab,btab,0);
    } else if (*(argv[i]+1) == 'A') {  /* ascii color table file */
      sscanf(argv[i],"-A%s",line);
      if (debug) printf("debug: read ascii color table file: %s\n",line);
      read_colortable(line,rtab,gtab,btab,1);
    } else 
      if (!(*(argv[i]+1) == '%' || *(argv[i]+1) == '#') )
	process_option(argv[i]+1,0);
    i++;
  }
  return(0);
}

int plot_region_resize(float anodata[], float *stval[], int nchans, int npix, 
		       int *nsx, int *nsy, float *ascale, float *bscale)
{
  int i,j,k,l,m,n,width = *nsx;
  float amag;
  float *stval2;

  if (nchans == 0)  nchans = 1;  

  if (npix > 0) {    /* reduce pixel size if selected */
    if (debug) printf("Pixel reduction %d\n",npix);
    
    *nsx = *nsx/npix;  /* note: if npix does not evenly divide nsx, nsy the number of output */
    *nsy = *nsy/npix;  /*       pixels in each dimension is truncated to an integer value */
                       /* this can mess up overplotting */

    for (m=0; m < nchans; m++)
      for (i=0; i < *nsy; i++)
	for (j=0; j < *nsx; j++) {
	  amag = 0;
	  n = 0;
	  for (k=0; k < npix; k++)
	    for (l=0; l < npix; l++)
	      if (*(stval[m]+(i * npix+k)*width+(j * npix+l)) != anodata[m]) {
		n++;
		amag = amag + *(stval[m]+(i * npix+k)*width+(j * npix+l));
	      };
	  if (n > 0) 
	    amag = amag / n;
	  else
	    amag = anodata[m];
	  *(stval[m]+i * *nsx+j) = amag;  /* works because of smaller array and processing order */
	}

    /* modify lat/lon transformation info */

    *ascale = *ascale * npix;
    *bscale = *bscale * npix;

  } else if (npix < 0) { /* increase pixel size if selected */
    npix = -npix;  /* switch sign for this computation*/
    if (debug) printf("Pixel replication %d (%d,%d) -> (%d,%d)\n",npix,*nsx,*nsy,*nsx*npix,*nsy*npix);

    for (m=0; m < nchans; m++) {
      stval2 = (float *) malloc(sizeof(float) * *nsx * npix * *nsy * npix);
      if (stval2 == NULL) {
	fprintf(stderr,"*** error allocating temporary buffer for pixel replication ***\n");
	return(-1);
      }

      for (i=0; i < *nsy; i++)
	for (j=0; j < *nsx; j++)
	  for (k=0; k< npix; k++)
	    for (l=0; l < npix; l++)
	      *(stval2+(i*npix+k)* *nsx*npix+(j*npix+l)) = *(stval[m]+i * *nsx+j);

      free(stval[m]);
      stval[m] = stval2;
    }
    
    *nsx = *nsx * npix;
    *nsy = *nsy * npix;

    /* modify lat/lon transformation info */

    *ascale = *ascale / (float) npix;
    *bscale = *bscale / (float) npix;
    printf("pixel replication completed\n");
    
  }
  return(0);
  
}



void make_byte_image(char *data, float *stval[], float smin[], float smax[], int nsx1, int nsy1, 
		     int nchans,
		     int s_opt, int e_opt, int nc_opt[], int z_opt, float anodata[], int debug)
{
  int i,j,k,t,u,v;
  float scale[OMAX], scaleoffset[OMAX], amag, am;

  nsx=nsx1;
  nsy=nsy1;

  /* byte scaling factors */

  if (nchans == 0) nchans = 1;

  fpimage_1.a = data;
  fpimage_1.bytesperpixel = 1;   /* set plotting array size*/
  fpimage_1.tab = 0;             /* use cclor table values for multi-byte images*/
  bpp = fpimage_1.bytesperpixel;

  for (k=0; k < nchans; k++) {
    if (smax[k] - smin[k] == 0.0) smax[k] = smin[k] + 1.0;
    scale[k] = (smax[k]-smin[k]);
    if (scale[k] > 0.) scale[k] = ((float) nc_opt[k]) / scale[k];
    scaleoffset[k] = smin[k];
  }

  if (debug) {
    if (nchans <= 1) {
      printf(" debug: single channel scaling options: no data=%d [%f,%f] to [%d,%d] %d\n",
	     z_opt,smin[0],smax[0],s_opt,e_opt,nchans);
      for (i=0; i < nchans; i++)
	printf("      Scale chan %d: [%f,%f] -> %d levels  sca=%f off=%f\n",i+1, 
	       smin[i], smax[i], nc_opt[i], scale[i], scaleoffset[i]);
    } else {
      printf(" debug: multi-channel scaling: no data=%d, start, end indices:  [%d,%d]  chans: %d\n",
	     z_opt,s_opt,e_opt,nchans);
      for (i=0; i < nchans; i++)
	printf("      Scale chan %d: [%f,%f] -> %d levels sc=%f off=%f\n",i+1, 
	       smin[i], smax[i], nc_opt[i], scale[i], scaleoffset[i]);
    }
  }
  
  /* generate byte-scaled image */

  for (i = 0; i < nsy; i++) 
    for (j = 0; j < nsx; j++) {
      t = 0.0;
      v = 0;
      for (k=0; k < nchans; k++) {
	amag = *(stval[k]+i*nsx+j);
	if (z_opt >= 0 && amag <= anodata[k]) v = 1;
	if (amag > smax[k]) amag = smax[k];
	if (amag < smin[k]) amag = smin[k];
	am = scale[k] * (amag - scaleoffset[k])+1;
	if (am < 0.0) am = 0.0;
	if (am-0.9999 > nc_opt[k]) am = (float) (nc_opt[k] - 1) - 0.001;
	u = (int) am;
	if (u > nc_opt[k] - 1) u = nc_opt[k]-1;
	if (k > 0) u = u * nc_opt[k-1];
	t = t + u;
      }
      t = t + s_opt;

      if (t > 255) t = 255;          /* check overflow */
      if (t < 0) t = 0;              /* check underflow */

      *(data+(nsy-i-1)*nsx+j) = (char) t;
      if (v == 1) *(data+(nsy-i-1)*nsx+j) = z_opt;      
    }
}


void make_byte_image2(char *data, float *stval[], float smin[], float smax[], int nsx1, int nsy1, 
		      int nchans,
		      int s_opt, int e_opt, int nc_opt[], int z_opt, float anodata[], int debug,
		      int SF, int dsize, char red[], char green[], char blue[])
{
  int i,j,k,t,u,v,ind,kk;
  float scale[OMAX], scaleoffset[OMAX], amag, am;

  nsx=nsx1;
  nsy=nsy1;

  /* byte scaling factors */
  /*
  printf("make_byte_image2:  no data=%d [%f,%f] to [%d,%d] %d\n",
	     z_opt,smin[0],smax[0],s_opt,e_opt,nchans);
  */
  if (nchans == 0) nchans = 1;

  for (k=0; k < nchans; k++) {
    if (smax[k] - smin[k] == 0.0) smax[k] = smin[k] + 1.0;
    scale[k] = (smax[k]-smin[k]);
    if (scale[k] > 0.) scale[k] = ((float) nc_opt[k]) / scale[k];
    scaleoffset[k] = smin[k];
  }

  if (debug) {
    if (nchans <= 1) {
      printf(" debug: single channel scaling options: no data=%d [%f,%f] to [%d,%d] %d\n",
	     z_opt,smin[0],smax[0],s_opt,e_opt,nchans);
      for (i=0; i < nchans; i++)
	printf("      Scale chan %d: [%f,%f] -> %d levels  sca=%f off=%f\n",i+1, 
	       smin[i], smax[i], nc_opt[i], scale[i], scaleoffset[i]);
    } else {
      printf(" debug: multi-channel scaling: no data=%d, start, end indices:  [%d,%d]  chans: %d\n",
	     z_opt,s_opt,e_opt,nchans);
      for (i=0; i < nchans; i++)
	printf("      Scale chan %d: [%f,%f] -> %d levels sc=%f off=%f\n",i+1, 
	       smin[i], smax[i], nc_opt[i], scale[i], scaleoffset[i]);
    }
  }
  
  /*
  for (i=0;i<256;i++)
    printf("rgb color table: %d %d %d\n",red[i],green[i],blue[i]);
  */

  /* generate byte-scaled image */

  fpimage_1.a = data;
  fpimage_1.bytesperpixel = dsize;   /* set plotting array size*/
  fpimage_1.tab = 0;             /* use cclor table values for multi-byte images*/
  bpp = fpimage_1.bytesperpixel;

  if (dsize == 4 && nchans > 1) { /* special multiple channel output */
  
    for (i = 0; i < nsy; i++) 
      for (j = 0; j < nsx; j++) {
	t = 0;
	v = 0;
#ifdef SIR2TIFF_BYTESHIFT
	kk = 256;   /* use kk=256 on PC, kk=1 for unix (sir2tiff2) */
#else
	kk = 1;
#endif
	for (k=0; k < nchans; k++) {
	  amag = *(stval[k]+i*nsx+j);
	  if (z_opt >= 0 && amag <= anodata[k]) v = 1;

	  if (amag > smax[k]) amag = smax[k];
	  if (amag < smin[k]) amag = smin[k];
	  am = scale[k] * (amag - scaleoffset[k])*256.0/nc_opt[k]+1;
	  if (am < 0.0) am = 0.0;
	  if (am > 255.0) am = 255.0;

	  u = (int) (am);
	  if (u > nc_opt[k] - 1) u = nc_opt[k]-1;
	  t = t + u * kk;
	  kk = kk * 256;
	  
	}
	
	/* if (v == 1) t = z_opt; */	
	if (v == 1) t = 0;
	ind = (nsy-i-1)*nsx+j;
	*((int *) (data+ind*dsize)) = t;
	  	
      }
    /*
    printf("view data %d\n",nchans);
    for (i=302401;i<(302401+512);i++) {
      printf("data %d %d %f %f %f\n",i	,*((int *) (data +ind*dsize)), *(stval[0]+i), *(stval[1]+i), *(stval[2]+i));
    }	
    */

  } else {  /* conventional single color table output */

    for (i = 0; i < nsy; i++) 
      for (j = 0; j < nsx; j++) {
 	t = 0;
	v = 0;
	for (k=0; k < nchans; k++) {
	  amag = *(stval[k]+i*nsx+j);
	  if (z_opt >= 0 && amag <= anodata[k]) v = 1;
	  if (amag > smax[k]) amag = smax[k];
	  if (amag < smin[k]) amag = smin[k];
	  am = scale[k] * (amag - scaleoffset[k])+1;
	  if (am < 0.0) am = 0.0;
	  if (am-0.9999 > nc_opt[k]) am = (float) (nc_opt[k] - 1)-0.001;
	  u = (int) am;
	  if (u > nc_opt[k] - 1) u = nc_opt[k]-1;
	  if (k > 0) u = u * nc_opt[k-1];
	  t = t + u;
	}
	t = t + s_opt;
	
	if (t > 255) t = 255;          /* check overflow */
	if (t < 0) t = 0;              /* check underflow */
	
	if (v == 1) t = z_opt;
	
	ind = (nsy-i-1)*nsx+j;
	
	switch(dsize) {
	case 1:  /* 8 bits */
	  *(data+ind) = t;
	  break;
	case 2:  /* 16 bits */
	  *((short int *) (data+ind*dsize)) = t*SF;
	  break;
	case 4:  /* 24 or 32 bits */
	  /* *((int *) (data+ind*dsize)) = t*SF; */
	  *((int *) (data+ind*dsize)) = 
	    /*	
		blue[t]*256*256*256 +
		green[t]*256*2	56 +
		red[t]*256; 	
	    */
	    blue[t]*256*256 +
	    green[t]*256 +
	    red[t]; 
	  break;
	}
	
      }
    
    /*	    printf("view data\n");
	    for (i=10240;i<(10240+1024);i++) {
	    amag = *(stval[0]+	i);
	    printf("data %d %d %f\n",i	,data[i],amag);
	    *(data+i)=128;	
	    }	
    */
  }
  
}



int plot_pass_two(char **argv, char *data, float *stval, int nsx, int nsy)
{
  FILE *fid;
  int i;
  float x1, y1;
  float zero=0.0;
  int c__4=4, c_999=9999;
  char optname[200], line[200];


  fpimage_1.a = data;   /* set plotting array pointer to byte data array*/

  /* initialize pixel plotting into byte array*/

  vinit_(&nsx,&nsy);

  x1=nsx;
  y1=nsy;
  plotvt_(&x1,&y1,&c__4);  /* set viewport */
  i=0;
  newvpen_(&i,&w_opt); /* set default line width */
  x1=c_opt;
  plotvt_(&x1,&zero,&i);   /* set default color */

  fpimage_color(c_opt, rtab, gtab, btab); /* override plotting array color*/
  
  /* process input options to (possibly) plot onto output image 
     options previously processed will be ignored */  

  x1_opt=1.0; y1_opt=1.0; x2_opt=1.0; y2_opt=1.0;  /* set default locations */
  if (debug) printf("\n debug: second pass\n");
  i=1;
  while (*(argv[i]) == '-') { /* optional argument */
    if (*(argv[i]+1) == 'f') {  /* optional argument is file input */
      sscanf(argv[i],"-f%s",optname);
      fid=fopen(optname,"r");
      if (fid == NULL) {
	fprintf(stderr,"*** Could not open options file '%s' ***\n",optname);
	return(-1);
      }
      while (fscanf(fid,"%s",line) > 0) {
	if (debug) printf(" debug: options file line: %s\n",line);
	if (line[0] == '-' && !(line[1] == '%' || line[1] == '#') )
	  process_option(line+1,1);
      }
      fclose(fid);
    }
    if (!(*(argv[i]+1) == '%' || *(argv[i]+1) == '#') )
      process_option((argv[i]+1),1);
    i++;
  }
  plotvt_(&zero,&zero,&c_999);  /* turn off pixel plotting into array */

  return(0);
  
}


void print_options_info(FILE *fid)
{
  fprintf(fid,"\nOptions:  (all but -p, -r, -D, or -E can occur multiple times)\n");

  fprintf(fid,"   -ANAME   = read ascii color table file NAME\n");
  fprintf(fid,"   -bstring = add rectangular box to image ([x1,y1],[x2,y2],w,c,I)\n");
  fprintf(fid,"   -cstring = add circle to image ([x,y],c,w,r)\n");
  fprintf(fid,"   -CNAME   = read binary color table file NAME\n");
  fprintf(fid,"   -d       = print debug output\n");
  fprintf(fid,"   -DNAME   = difference with SIR file NAME (sir_in - NAME)\n");
  fprintf(fid,"   -Estring = difference with SIR file (NAME from sir_in w/S,C,m)\n");
  fprintf(fid,"   -estring = color table options (i,r,g,b)\n");
  fprintf(fid,"   -fFNAME  = read options from file FNAME\n");
  fprintf(fid,"   -Gstring = add color bar to image ([x1,y1],[x2,y2],M,H)\n");
  fprintf(fid,"   -i#NAME  = read .sir file NAME into input buffer # (main=input buff 1)\n");
  fprintf(fid,"   -I#string= read edited file name into input buffer # (S,C,m)\n");
  fprintf(fid,"   -lstring = add line to image ([x1,y1],[x2,y2],w,c,I)\n");
  fprintf(fid,"   -Lstring = place logo/subimage ([x1,y1],c,X,Y,K,F) (def X,Y from file name)\n");
  fprintf(fid,"   -pvalue  = size/scale >0=shrink <0=expand (optional, def=1) [-16..16]\n");
  fprintf(fid,"   -Pstring = input file preprocessing command string\n");
  fprintf(fid,"   -rstring = extract image subregion ([x1,y1],[x2,y2])\n");
  fprintf(fid,"   -Rvalue  = rotate output image (def=0: 0=0, 1=90, 2=180, 3=280 deg CW)\n");
  fprintf(fid,"   -Sstring = symbol plot ([x1,y1],n,D,w,c)\n");
  fprintf(fid,"   -sstring = image scaling options (z,l,h,s,e)\n");
  fprintf(fid,"   -tstring = add text to image ([x,y],t,a,w,c,m)\n");
  fprintf(fid,"   -Tstring = add text from file name to image ([x,y],t,a,w,c,S,C)\n");
  fprintf(fid,"   -zstring = add lines specified in ascii input file (F,o,Z,w,c,I)\n");
  fprintf(fid,"   -%%string or -#string = comment (string ignored)\n");
  fprintf(fid,"\n The first letter after the dash is significant and is followed by a string\n");
  fprintf(fid," command parameters which have the form L=value.  Multiple parameters are\n");
  fprintf(fid," separated by commas. Possible parameters are: (units generally pixels)\n");
  fprintf(fid,"     a = angle (deg)\n");
  fprintf(fid,"     b = blue color table entry values [0..255]\n");
  fprintf(fid,"     c = color [0..255]\n");
  fprintf(fid,"     C = number of chacters\n");
  fprintf(fid,"     D = symbol dimension in pixels [>0]\n");
  fprintf(fid,"     e = end color table index [1..255, e > s]\n");
  fprintf(fid,"     F = file name\n");
  fprintf(fid,"     h = high image value\n");
  fprintf(fid,"     H = output channel (0=all/none) [0..3]\n");
  fprintf(fid,"     i = color table index [0..255] (if multibyte output, <0 switches on 24 bit color input)\n");
  fprintf(fid,"     g = green color table entry values [0..255]\n");
  fprintf(fid,"     I = number of lat/lon increments during line plotting (only valid for n,e) \n");
  fprintf(fid,"     K = logo fill option [-1,1] (1=fill with color [def] c, -1=fill w/logo value)\n");
  fprintf(fid,"     l = low image value\n");
  fprintf(fid,"     m = text\n");
  fprintf(fid,"     M = color bar direction [0..3] (0=l-r,1=r-l,2=b-t,3=t-b)\n");
  fprintf(fid,"     n = symbol id [0..2] (0=+,1=x,2=*)\n");
  fprintf(fid,"     o = line plot flag option (def=0 flag read but ignored, <0 flag not read, >0 match flag>\n");
  fprintf(fid,"     r = red color table entry values [0..255]\n");
  fprintf(fid,"     s = start color table index [0..255]\n");
  fprintf(fid,"     S = starting char number in input name, C=number of chars (=0 length)\n");
  fprintf(fid,"     t = char height\n");
  fprintf(fid,"     X = x dimension size (pixels)\n");
  fprintf(fid,"     Y = y dimension size (pixels)\n");
  fprintf(fid,"     w = line width [>0]\n");
  fprintf(fid,"     z = no data color index [0..255]\n");
  fprintf(fid,"     Z = line plot type Z=0: x,y (def) Z!=0: e,n\n");

  fprintf(fid,"\n  The [x,y] parameters are pixel coordinates but [e,n] (east,north)\n");
  fprintf(fid,"  can be used instead.  x1,y1,x2,y2->e1,n1,e2,n2  Mixing x,y and n,e in same\n");
  fprintf(fid,"  command string is NOT allowed. The processing sequence is: \n");
  fprintf(fid,"    1) difference, 2) subregion extract, 3) resize, 4) plot, 5) rotate\n\n");
  fprintf(fid,"Examples:\n -lx1=10,y1=4,x2=100,y2=300,c=255,w=2 creates a 2 pixel wide line from\n");
  fprintf(fid,"   (10,2) to (100,300) which uses color table entry 255.\n");
  fprintf(fid," -sl=-32.5,h=-5.0,s=1,e=254 sets the color table so that entry 1 corresponds\n");
  fprintf(fid,"    to an image value of -32.5 while table index 254 corresponds to -5.0\n");
  fprintf(fid," -ei=255,r=255,g=0,b=0 sets color table entry 255 to saturated red\n");
  fprintf(fid," -Tx=1,y=1,S=3,C=2 prints characters (3:4) from the main file name at (1,1)\n");
  fprintf(fid,"\nPreprocessing examples: (stack based) quotes may be needed\n");
  fprintf(fid,"  -P=I1,I2,- displays image i1 - image i2  w/nodata set to default\n");
  fprintf(fid,"  \"-P(-32)=I1,2,^,10,*\" displays 10*(I1^2) with nodata values set to -32\n");
  fprintf(fid,"\nValid preprocessing operators: + - * / ^ exp log abs mask > <\n");

  return;
}
  

 
void subregion_extract(float x1, float y1, float x2, float y2, float *stval);
void get_X_Y_from_logo_file_name(char *c, int *nx, int *ny);
void place_logo(char *d, char *c, float x1_opt, float y1_opt, int c_opt, int flag, int nx, int ny);
void line_plot_from_file(char *filename, int q, int Ii_opt, int latlon_opt, int c_opt, int w_opt);


int process_option(char *line, int flag)
{  /* process command line: flag=0 for first pass, flag=1 for second pass */
  char *c, *name, *c2, fname[180];
  int i, j, k, n, m, q, jj, kk;
  float x,y,x0,y0,dx,dy, f;
  int dsize, SF;

  int zeroi=0, one=1, two=2, three=3, five=5;
  float zerof=0.0, f360=360.0;
 
  /*  if (debug) printf(" debug: processing option: '%s'\n",line); */

  /* do general parameter extraction (assumes unique parameter names) */
  
  if (flag == 1) {
    if ((c = strstr(line+1,"c=")) != NULL) { /* plotting color */
      sscanf(c+2,"%d",&c_opt);
      if (c_opt > 255 && bpp == 1) c_opt=255;
      if (c_opt < 0) c_opt=0;
    }
    if ((c = strstr(line+1,"w=")) != NULL) { /* plotting width */
      sscanf(c+2,"%d",&w_opt);
      if (w_opt > 7) w_opt=7;
      if (w_opt < 1) w_opt=1;
    }
    if ((c = strstr(line+1,"I=")) != NULL) { /* plotting increment */
      sscanf(c+2,"%d",&Ii_opt);
      if (Ii_opt < 0) Ii_opt=0;
    }
  }
  
  latlon_opt=0;
  if ((c = strstr(line+1,"x=")) != NULL) { /* coordinates */
    sscanf(c+2,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"y=")) != NULL) {
    sscanf(c+2,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"n=")) != NULL) {
    latlon_opt=1;
    sscanf(c+2,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"e=")) != NULL) {
    latlon_opt=1;
    sscanf(c+2,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"N=")) != NULL) {
    latlon_opt=2;
    sscanf(c+2,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"E=")) != NULL) {
    latlon_opt=2;
    sscanf(c+2,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"x1=")) != NULL) {
    sscanf(c+3,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"y1=")) != NULL) {
    sscanf(c+3,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"n1=")) != NULL) {
    latlon_opt=1;
    sscanf(c+3,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"e1=")) != NULL) {
    latlon_opt=1;
    sscanf(c+3,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"N1=")) != NULL) {
    latlon_opt=2;
    sscanf(c+3,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"E1=")) != NULL) {
    latlon_opt=2;
    sscanf(c+3,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"x2=")) != NULL) {
    sscanf(c+3,"%f",&x2_opt);
  }
  if ((c = strstr(line+1,"y2=")) != NULL) {
    sscanf(c+3,"%f",&y2_opt);
  }
  if ((c = strstr(line+1,"n2=")) != NULL) {
    latlon_opt=1;
    sscanf(c+3,"%f",&y2_opt);
  }
  if ((c = strstr(line+1,"e2=")) != NULL) {
    latlon_opt=1;
    sscanf(c+3,"%f",&x2_opt);
  }
  if ((c = strstr(line+1,"N2=")) != NULL) {
    latlon_opt=2;
    sscanf(c+3,"%f",&y2_opt);
  }
  if ((c = strstr(line+1,"E2=")) != NULL) {
    latlon_opt=2;
    sscanf(c+3,"%f",&x2_opt);
  }
  if ((c = strstr(line+1,"H=")) != NULL) {
    chan_opt=0;
    sscanf(c+2,"%d",&chan_opt);
    if (chan_opt > nchans) chan_opt=nchans;
    if (chan_opt < 0) chan_opt=0;
  }
  
  switch(line[0])
    {
    case 'b':   /* box */
      if (flag == 1) {
	if (debug) printf(" debug: plot box f=%d (%f,%f)-(%f,%f) col=%d wide=%d\n",latlon_opt,x1_opt,y1_opt,x2_opt,y2_opt,c_opt,w_opt);
	
	newvpen_(&zeroi,&w_opt);
	x=c_opt;
	plotvt_(&x,&zerof,&zeroi);
	fpimage_color(c_opt, rtab, gtab, btab); /* override plotting array color*/

	x0 = x1_opt;
	y0 = y1_opt;
	x = x1_opt;
	y = y2_opt;
	plot_line(Ii_opt,latlon_opt,&x0,&y0,&x,&y);
	x0 = x1_opt;
	y0 = y2_opt;
	x = x2_opt;
	y = y2_opt;
	plot_line(Ii_opt,latlon_opt,&x0,&y0,&x,&y);
	x0 = x2_opt;
	y0 = y2_opt;
	x = x2_opt;
	y = y1_opt;
	plot_line(Ii_opt,latlon_opt,&x0,&y0,&x,&y);
	x0 = x2_opt;
	y0 = y1_opt;
	x = x1_opt;
	y = y1_opt;
	plot_line(Ii_opt,latlon_opt,&x0,&y0,&x,&y);

	if (latlon_opt != 0) {
	  latlon_trans(&x1_opt,&y1_opt);
	  latlon_trans(&x2_opt,&y2_opt);
	}

	/*
	rect_(&x1_opt,&y1_opt,&x2_opt,&y2_opt);
	plotvt_(&zerof,&zerof,&five);
	*/
      }
      break;

    case 'c':   /* circle */
      if (flag == 1) {
	if ((c = strstr(line+1,"d=")) != NULL) { /* circle radius */
	  sscanf(c+2,"%f",&d_opt);
	  if (d_opt < 0) d_opt=0.0;
	}

	if (debug) printf(" debug: plot circle f=%d (%f,%f) radius=%f color=%d width=%d\n",latlon_opt,x1_opt,y1_opt,d_opt,c_opt,w_opt);
	
	if (latlon_opt != 0) {
	  latlon_trans(&x1_opt,&y1_opt);
	}
	newvpen_(&zeroi,&w_opt);
	x=c_opt;
	plotvt_(&x,&zerof,&zeroi);
	fpimage_color(c_opt, rtab, gtab, btab); /* override plotting array color*/
	circle_(&x1_opt,&y1_opt,&zerof,&f360,&d_opt,&d_opt,&zerof);
	plotvt_(&zerof,&zerof,&five);
      }
      break;

    case 'e':   /* color table entry index */

      if ((c = strstr(line+1,"i")) != NULL) {  /* index */
	sscanf(c+2,"%d",&i_opt);
	if (i_opt > 255) i_opt=255;
	if (i_opt < 0) fpimage_1.tab=1;
	if (i_opt >= 0) fpimage_1.tab=0;
	if (i_opt < 0) i_opt=0;
      }
      
      if ((c = strstr(line+1,"r=")) != NULL) { /* red */
	sscanf(c+2,"%d",&i);
	if (i > 255) i=255;
	if (i < 0) i=0;
	if (i > 127)
	  r_opt=i-256;
	else
	  r_opt=i;
      }
      if ((c = strstr(line+1,"g=")) != NULL) { /* green */
	sscanf(c+2,"%d",&i);
	if (i > 255) i=255;
	if (i < 0) i=0;
	if (i > 127)
	  g_opt=i-256;
	else
	  g_opt=i;
      }
      if ((c = strstr(line+1,"b=")) != NULL) { /* blue */
	sscanf(c+2,"%d",&i);
	if (i > 255) i=255;
	if (i < 0) i=0;
	if (i > 127)
	  b_opt=i-256;
	else
	  b_opt=i;
      }

      if (i_opt > -1 && i_opt < 256) { /* set color table index */
	if (debug) printf(" debug: color table index %d to %d %d %d\n",i_opt,r_opt,g_opt,b_opt);
	rtab[i_opt] = r_opt;
	gtab[i_opt] = g_opt;
	btab[i_opt] = b_opt;
      }      
      break;

    case 'G':   /* add color bar */
      if (flag == 1) {
	if (debug) printf(" debug: add color bar f=%d (%f,%f)-(%f,%f) M=%d chan=%d\n",latlon_opt,x1_opt,y1_opt,x2_opt,y2_opt,M_opt,chan_opt);
	if (latlon_opt != 0) {
	  latlon_trans(&x1_opt,&y1_opt);
	  latlon_trans(&x2_opt,&y2_opt);
	  if (debug) printf(" debug: transformed location       (%f,%f)-(%f,%f)\n",x1_opt,y1_opt,x2_opt,y2_opt);
	}

	dsize = fpimage_1.bytesperpixel;
	SF=1;
	
	if ((c = strstr(line+1,"M=")) != NULL) {  /* index */
	  sscanf(c+2,"%d",&M_opt);
	  if (M_opt > 3) M_opt=3;
	  if (M_opt < 0) M_opt=0;
	}
	switch(M_opt) {
	case 0:
	  for (k=x1_opt;k<=x2_opt;k++) {

	    if (fpimage_1.bytesperpixel == 4 && nchans > 1 && chan_opt > 0) { /* special multiple channel output */
#ifdef SIR2TIFF_BYTESHIFT
	      kk = 256;   /* use kk=256 on PC, kk=1 for unix (sir2tiff2) */
#else	
	      kk = 1;
#endif
	      if (chan_opt > 1) kk=kk*256;
	      if (chan_opt > 2) kk=kk*256;  
	      i = nc_opt[chan_opt] * (k - x1_opt)/(x2_opt - x1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > nc_opt[chan_opt]-1) i=nc_opt[chan_opt]-1;
	      i = i * kk;
	      
	      for (j=y1_opt;j<y2_opt;j++)
		if (k > 0 && k <= nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		}

	    } else if (nchans > 1 && chan_opt > 0) { /* multi channel output into 8 bits*/

	      kk = 1;
	      if (chan_opt > 1) kk=kk*nc_opt[chan_opt-1];
	      if (chan_opt > 2) kk=kk*nc_opt[chan_opt-2];
	      i = nc_opt[chan_opt] * (k - x1_opt)/(x2_opt - x1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > nc_opt[chan_opt]-1) i=nc_opt[chan_opt]-1;
	      if (i < 0) i=0;
	      if (i > 255) i=255;
	      switch(dsize) {
	      case 1:  /* 8 bits */
		i = i * kk;
		break;
	      case 2:  /* 16 bits */
		i = i*SF;
		break;
	      case 4:  /* 24 or 32 bits */
		i = btab[i]*256*256 +
		  gtab[i]*256 +
		  rtab[i]; 
		break;
	      }

	      for (j=y1_opt;j<y2_opt;j++)
		if (k > 0 && k <= nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		}

	    } else {   /* single channel color bar */
	      i = (e_opt-s_opt)*(k - x1_opt)/(x2_opt - x1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > e_opt) i=e_opt;
	      i=fpimage_color(i, rtab, gtab, btab);
	      for (j=y1_opt;j<y2_opt;j++)
		if (k > 0 && k <= nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		  /* *(fpimage_1.a+(nsy-j-1)*nsx+k-1) = i; */
		}
	    }
	  }
	  break;
	  
	case 1:
	  for (k=x1_opt;k<=x2_opt;k++) {

	    if (fpimage_1.bytesperpixel == 4 && nchans > 1 && chan_opt > 0) { /* special multiple channel output */
#ifdef SIR2TIFF_BYTESHIFT
	      kk = 256;   /* use kk=256 on PC, kk=1 for unix (sir2tiff2) */
#else	
	      kk = 1;
#endif
	      if (chan_opt > 1) kk=kk*256;
	      if (chan_opt > 2) kk=kk*256;  
	      i = nc_opt[chan_opt] * (k - x1_opt)/(x2_opt - x1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > nc_opt[chan_opt]-1) i=nc_opt[chan_opt]-1;
	      i = i * kk;
	      
	      for (j=y1_opt;j<y2_opt;j++)
		if (k > 0 && k <= nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		}

	    } else if (nchans > 1 && chan_opt > 0) { /* multi channel output into 8 bits*/

	      kk = 1;
	      if (chan_opt > 1) kk=kk*nc_opt[chan_opt-1];
	      if (chan_opt > 2) kk=kk*nc_opt[chan_opt-2];
	      i = nc_opt[chan_opt] * (k - x1_opt)/(x2_opt - x1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > nc_opt[chan_opt]-1) i=nc_opt[chan_opt]-1;
	      if (i < 0) i=0;
	      if (i > 255) i=255;
	      switch(dsize) {
	      case 1:  /* 8 bits */
		i = i * kk;
		break;
	      case 2:  /* 16 bits */
		i = i*SF;
		break;
	      case 4:  /* 24 or 32 bits */
		i = btab[i]*256*256 +
		  gtab[i]*256 +
		  rtab[i]; 
		break;
	      }
	      for (j=y1_opt;j<y2_opt;j++)
		if (k > 0 && k <= nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		}

	    } else {   /* single channel color bar */

	      i = (s_opt-e_opt)*(k - x1_opt)/(x2_opt - x1_opt) + e_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > e_opt) i=e_opt;
	      i=fpimage_color(i, rtab, gtab, btab);
	      for (j=y1_opt;j<y2_opt;j++)
		if (k > 0 && k <= nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;		
		  fppix_(&k, &jj, &i, &one);	    
		  /* *(fpimage	_1.a+(nsy-j-1)*nsx+k-1) = i; */
		}
	    }
	    
	  }
	  break;

	case 2:
	  for (j=y1_opt;j<y2_opt;j++) {

	    if (fpimage_1.bytesperpixel == 4 && nchans > 1 && chan_opt > 0) { /* special multiple channel output */
#ifdef SIR2TIFF_BYTESHIFT
	      kk = 256;   /* use kk=256 on PC, kk=1 for unix (sir2tiff2) */
#else	
	      kk = 1;
#endif
	      if (chan_opt > 1) kk=kk*256;
	      if (chan_opt > 2) kk=kk*256;  
	      i = (nc_opt[chan_opt] * (j - y1_opt))/(y2_opt - y1_opt) + s_opt;
	      jj = i;
	      if (i < s_opt) i=s_opt;
	      if (i > nc_opt[chan_opt]-1) i=nc_opt[chan_opt]-1;
	      i = i * kk;

	      for (k=x1_opt;k<x2_opt;k++) 
		if (k >= 0 && k < nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		}

	    } else if (nchans > 1 && chan_opt > 0) { /* multi channel output into 8 bits*/

	      kk = 1;
	      if (chan_opt > 1) kk=kk*nc_opt[chan_opt-1];
	      if (chan_opt > 2) kk=kk*nc_opt[chan_opt-2];
	      i = nc_opt[chan_opt] * (j - y1_opt)/(y2_opt - y1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > nc_opt[chan_opt]-1) i=nc_opt[chan_opt]-1;
	      if (i < 0) i=0;
	      if (i > 255) i=255;
	      switch(dsize) {
	      case 1:  /* 8 bits */
		i = i * kk;
		break;
	      case 2:  /* 16 bits */
		i = i*SF;
		break;
	      case 4:  /* 24 or 32 bits */
		i = btab[i]*256*256 +
		  gtab[i]*256 +
		  rtab[i]; 
		break;
	      }
	      for (k=x1_opt;k<x2_opt;k++) 
		if (k >= 0 && k < nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		}

	    } else {   /* single channel color bar */

	      i = (e_opt-s_opt)*(j - y1_opt)/(y2_opt - y1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > e_opt) i=e_opt;
	      i=fpimage_color(i, rtab, gtab, btab);
	      for (k=x1_opt;k<x2_opt;k++) 
		if (k >= 0 && k < nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;		
		  fppix_(&k, &jj, &i, &one);
		  /* *(fpimage_1.a+(nsy-j-1)*nsx+k-1) = i; */
		}
	    }
	    
	  }
	  break;

	case 3:
	  for (j=y1_opt;j<y2_opt;j++) {

	    if (fpimage_1.bytesperpixel == 4 && nchans > 1 && chan_opt > 0) { /* special multiple channel output */
#ifdef SIR2TIFF_BYTESHIFT
	      kk = 256;   /* use kk=256 on PC, kk=1 for unix (sir2tiff2) */
#else	
	      kk = 1;
#endif
	      if (chan_opt > 1) kk=kk*256;
	      if (chan_opt > 2) kk=kk*256;  
	      i = nc_opt[chan_opt] * (j - y1_opt)/(y2_opt - y1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > nc_opt[chan_opt]-1) i=nc_opt[chan_opt]-1;
	      i = i * kk;

	      for (k=x1_opt;k<x2_opt;k++) 
		if (k >= 0 && k < nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		}

	    } else if (nchans > 1 && chan_opt > 0) { /* multi channel output into 8 bits*/

	      kk = 1;
	      if (chan_opt > 1) kk=kk*nc_opt[chan_opt-1];
	      if (chan_opt > 2) kk=kk*nc_opt[chan_opt-2];
	      i = nc_opt[chan_opt] * (j - y1_opt)/(y2_opt - y1_opt) + s_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > nc_opt[chan_opt]-1) i=nc_opt[chan_opt]-1;
	      if (i < 0) i=0;
	      if (i > 255) i=255;
	      switch(dsize) {
	      case 1:  /* 8 bits */
		i = i * kk;
		break;
	      case 2:  /* 16 bits */
		i = i*SF;
		break;
	      case 4:  /* 24 or 32 bits */
		i = btab[i]*256*256 +
		  gtab[i]*256 +
		  rtab[i]; 
		break;
	      }
	      for (k=x1_opt;k<x2_opt;k++) 
		if (k >= 0 && k < nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;
		  fppix_(&k, &jj, &i, &one);	    
		}

	    } else {   /* single channel color bar */

	      i = (s_opt-e_opt-1)*(j - y1_opt)/(y2_opt - y1_opt) + e_opt;
	      if (i < s_opt) i=s_opt;
	      if (i > e_opt) i=e_opt;
	      i=fpimage_color(i, rtab, gtab, btab);
	      for (k=x1_opt;k<=x2_opt;k++) 
		if (k >= 0 && k < nsx && j >=0 && j <  nsy) {
		  jj = nsy - j;		
		  fppix_(&k, &jj, &i, &one);
		  /* *(fpimage_1.a+(nsy-j-1)*nsx+k-1) = i; */
		}
	    }
	    
	  }
	  break;

	}
      }
      break;


    case 'I':   /* input file specification */
      break;
      

    case 'l':   /* line */
      if (flag == 1) {
	if (debug) printf(" debug: plot line f=%d (%f,%f)-(%f,%f) col=%d wide=%d\n",latlon_opt,x1_opt,y1_opt,x2_opt,y2_opt,c_opt,w_opt);

	newvpen_(&zeroi,&w_opt);
	x=c_opt;
	plotvt_(&x,&zerof,&zeroi);
	fpimage_color(c_opt, rtab, gtab, btab); /* override plotting array color*/

	plot_line(Ii_opt,latlon_opt,&x1_opt,&y1_opt,&x2_opt,&y2_opt);

      }
      break;

    case 'L':   /* logo/subimage */
      if (flag == 1) {
	k=1;
	if ((c = strstr(line+1,"F=")) != NULL) { /* file name */
	  name=c+2;
	  get_X_Y_from_logo_file_name(name,&i,&j);
	  if ((c = strstr(line+1,"X=")) != NULL) { /* specify x size */
	    sscanf(c+2,"%d",&i);
	  }
	  if ((c = strstr(line+1,"Y=")) != NULL) { /* specify y size */
	    sscanf(c+2,"%d",&j);
	  }
	  if ((c = strstr(line+1,"K=")) != NULL) { /* fill option option*/
	    sscanf(c+2,"%d",&k);
	    if (k>=0) k=1;
	    if (k<0) k=-1;
	  }
	  if (debug) printf(" debug: place logo f=%d (%f,%f) col=%d X,Y=%d,%d K=%d F='%s'\n",latlon_opt,x1_opt,y1_opt,c_opt,i,j,k,name);
	  if (latlon_opt != 0) {
	    latlon_trans(&x1_opt,&y1_opt);
	  } 
	  place_logo(fpimage_1.a,name,x1_opt,y1_opt,c_opt,k,i,j);
	} else {
	  if (debug) printf(" debug: ERROR no file name (F=) for place logo command\n");
	}
      }
      break;

    case 'P':   /* output channel option */
      break;

    case 'r':   /* extract subregion */
      if (flag == 0) {
	if (debug) printf(" debug: extract subregion inputs %d (%f,%f)-(%f,%f)\n",latlon_opt,x1_opt,y1_opt,x2_opt,y2_opt);
	if (latlon_opt != 0) {
	  latlon_trans(&x1_opt,&y1_opt);
	  latlon_trans(&x2_opt,&y2_opt);
	  if (debug) printf(" debug: transformed subregion       (%f,%f)-(%f,%f)\n",x1_opt,y1_opt,x2_opt,y2_opt);
	}
	if (nchans == 0) nchans++;
	for (m=0; m < nchans; m++)
	  subregion_extract(x1_opt, y1_opt, x2_opt, y2_opt, stval[m]);
      }
      break;

    case 'R':   /* rotate output image */
      if (flag == 1) {	
	sscanf(line+1,"%d",&rot_opt);
	if (rot_opt > 3) rot_opt = 0;
	if (rot_opt < 0) rot_opt = 0;      
	if (debug) printf(" debug: output image rotation set to %d\n",rot_opt);
      }
      break;
      

    case 's':   /* image scaling options */
      if (flag == 0) {
	if ((c = strstr(line+1,"z=")) != NULL) { /* no data color index value */
	  sscanf(c+2,"%d",&z_opt);
	  if (z_opt > 255) z_opt=255;
	  if (z_opt < 0) z_opt=0;
	}
	if ((c = strstr(line+1,"s=")) != NULL) { /* starting color index */
	  sscanf(c+2,"%d",&s_opt);
	  if (s_opt > 255) s_opt=255;
	  if (s_opt < 0) s_opt=0;
	}
	if ((c = strstr(line+1,"e=")) != NULL) { /* ending color index */
	  sscanf(c+2,"%d",&e_opt);
	  if (e_opt > 255) e_opt=255;
	  if (e_opt < 0) e_opt=0;
	  nc_opt[0] = e_opt - s_opt;
	}
	q=0;
	while ((c = strstr(line+1+q,"V")) != NULL) { /* number of entries */
	  if (*(c+1) == '=') {
	    sscanf(c+2,"%d",&m);
	    n=0;
	  } else {
	    sscanf(c,"V%d=%d",&n,&m);
	    if (n < 1) n=1;
	    if (n > OMAX) n=OMAX;
	    n--;
	  }
	  if (m > 255) m = 255;
	  if (m < 1) m = 1;
	  nc_opt[n] = m;
	  if (n == 0) e_opt=nc_opt[n]+s_opt-1;
	  q=q+3;
	}
	q=0;
	while ((c = strstr(line+1+q,"l")) != NULL) { /* low image value */
	  if (*(c+1) == '=') {
	    sscanf(c+2,"%f",&f);
	    n=0;
	  } else {
	    sscanf(c,"l%d=%f",&n,&f);
	    if (n < 1) n=1;
	    if (n > OMAX) n=OMAX;
	    n--;
	  }
	  smin[n] = f;
	  q=q+3;
	}
	q=0;
	while ((c = strstr(line+1+q,"h")) != NULL) { /* high image value */
	  if (*(c+1) == '=') {
	    sscanf(c+2,"%f",&f);
	    n=0;
	  } else {
	    sscanf(c,"h%d=%f",&n,&f);
	    if (n < 1) n=1;
	    if (n > OMAX) n=OMAX;
	    n--;
	  }
	  smax[n] = f;
	  q=q+3;
	}
	
	if (debug) {
	  if (nchans <= 1)
	    printf(" debug: input scaling options z=%d [%f,%f] to [%d,%d] % levels\n",z_opt,smin[0],smax[0],s_opt,e_opt,nc_opt[0]);
	  else
	    printf(" debug: input scaling options z=%d s=%d  e=%d  chans=%d\n",z_opt,s_opt,e_opt,nchans);
	  for (m=0; m < nchans; m++)
	  printf("        channel %d input range:  [%f,%f] %d levels\n",m,smin[m],smax[m],nc_opt[m]);
	}
	
      }
      break;

    case 'S':   /* symbol markder */
      if (flag == 1) {

	if ((c = strstr(line+1,"D=")) != NULL) {  /* symbol dimension */
	  sscanf(c+2,"%d",&D_opt);
	  if (D_opt < 1) D_opt=1;
	}
	if ((c = strstr(line+1,"n=")) != NULL) {  /* symbol id number */
	  sscanf(c+2,"%d",&n_opt);
	  if (n_opt < 0) n_opt=0;
	  if (n_opt > 2) n_opt=2;
	}

	if (debug) printf(" debug: plot symbol f=%d (%f,%f) n=%d D=%d col=%d wide=%d\n",latlon_opt,x1_opt,y1_opt,n_opt,D_opt,c_opt,w_opt);
	
	if (latlon_opt != 0)
	  latlon_trans(&x1_opt,&y1_opt);
	newvpen_(&zeroi,&w_opt);
	x=c_opt;
	plotvt_(&x,&zerof,&zeroi);
	fpimage_color(c_opt, rtab, gtab, btab); /* override plotting array color*/
	switch (n_opt) {  /* make symbol */
	case 2:  /* asterick */
	case 1:  /* X */
	  x=x1_opt - D_opt;
	  y=y1_opt - D_opt;
	  plotvt_(&x,&y,&three);
	  x=x1_opt + D_opt;
	  y=y1_opt + D_opt;
	  plotvt_(&x,&y,&two);
	  x=x1_opt + D_opt;
	  y=y1_opt - D_opt;
	  plotvt_(&x,&y,&three);
	  x=x1_opt - D_opt;
	  y=y1_opt + D_opt;
	  plotvt_(&x,&y,&two);
	  if (n_opt == 1) break;
	case 0:  /* + */
	default:
	  x=x1_opt - D_opt;
	  y=y1_opt;
	  plotvt_(&x,&y,&three);
	  x=x1_opt + D_opt;
	  plotvt_(&x,&y,&two);
	  x=x1_opt;
	  y=y1_opt - D_opt;
	  plotvt_(&x,&y,&three);
	  y=y1_opt + D_opt;
	  plotvt_(&x,&y,&two);
	}
	plotvt_(&zerof,&zerof,&five);
      }
      break;



    case 't':   /* add text */
      if (flag == 1) {
	if ((c = strstr(line+1,"t=")) != NULL) { /* text height */
	  sscanf(c+2,"%f",&t_opt);
	  if (t_opt < 0) t_opt=0.0;
	}
	if ((c = strstr(line+1,"a=")) != NULL) { /* text angle */
	  sscanf(c+2,"%f",&a_opt);
	}
	if ((c = strstr(line+1,"m=")) != NULL) { /* text message */
	  strcpy(m_opt,(c+2));
	  while ((c = strstr(m_opt,"`")) != NULL)
	    *c = ' ';   /* replace back quotes with spaces */
	  while ((c = strstr(m_opt,"~")) != NULL)
	    *c = ' ';   /* replace tildes with spaces */
	}
      
	if (debug) printf(" debug: plot text f=%d (%f,%f) text='%s' color=%d width=%d\n",latlon_opt,x1_opt,y1_opt,m_opt,c_opt,w_opt);
	
	if (latlon_opt != 0) {
	  latlon_trans(&x1_opt,&y1_opt);
	}
	newvpen_(&zeroi,&w_opt);
	x=c_opt;
	plotvt_(&x,&zerof,&zeroi);
	fpimage_color(c_opt, rtab, gtab, btab); /* override plotting array color*/
	j=strlen(m_opt);
	if ((c2=strchr(m_opt,',')) != NULL) j=c2-m_opt;
	if (debug) printf(" debug: character count in text line: %d\n",j);
	if (j > 0)
	  syms_(&x1_opt,&y1_opt,&t_opt,m_opt,&a_opt,&j,&one);
	plotvt_(&zerof,&zerof,&five);
      }
      break;

    case 'T':   /* add text extracted from path-stripped input file name*/
      if (flag == 1) {
	j=1; i=1;
	if ((c = strstr(line+1,"t=")) != NULL) { /* text height */
	  sscanf(c+2,"%f",&t_opt);
	  if (t_opt < 0) t_opt=0.0;
	}
	if ((c = strstr(line+1,"a=")) != NULL) { /* text angle */
	  sscanf(c+2,"%f",&a_opt);
	}
	if ((c = strstr(line+1,"S=")) != NULL) { /* start char */
	  sscanf(c+2,"%d",&i);
	}
	if ((c = strstr(line+1,"C=")) != NULL) { /* number of chars */
	  sscanf(c+2,"%d",&j);
	}
	if (strlen(f_opt) >= i) {
	  if (j==0) j=strlen((f_opt+i-1));
	  strncpy(m_opt,(f_opt+i-1),j);
	  *(m_opt+j)='\0';
	}
      
	if (debug) printf(" debug: plot fname text f=%d (%f,%f) Ext-text='%s' color=%d width=%d start=%d count=%d '%s' %d\n",latlon_opt,x1_opt,y1_opt,m_opt,c_opt,w_opt,i,j,f_opt,strlen(f_opt));
	
	if (latlon_opt != 0) {
	  latlon_trans(&x1_opt,&y1_opt);
	}
	newvpen_(&zeroi,&w_opt);
	x=c_opt;
	plotvt_(&x,&zerof,&zeroi);
	fpimage_color(c_opt, rtab, gtab, btab); /* override plotting array color*/
	j=strlen(m_opt);
	if (debug) printf(" debug: character count in text line: %d\n",j);
	if (j > 0)
	  syms_(&x1_opt,&y1_opt,&t_opt,m_opt,&a_opt,&j,&one);
	plotvt_(&zerof,&zerof,&five);
      }
      break;


    case 'z':   /* plot lines specified in external ascii file */
      if (flag == 1) {
	if (debug) printf(" debug: plot lines from file col=%d wide=%d\n",c_opt,w_opt);
	
	if ((c = strstr(line+1,"F=")) != NULL) { /* file name */
	  name=c+2;
	  strcpy(fname,name);
	  c=fname;
	  while (*c != ',' && *c != '\0') c++;
	  *c='\0';
	} else
	  printf("*** No input file specified, plot lines from file command ignored ***\n");
	if (debug) printf(" debug: input plot line file name '%s'\n",fname);
	
	latlon_opt=0;	
	if ((c = strstr(line+1,"Z=")) != NULL) { /* lat/lon vs x,y plotting */
	  sscanf(c+2,"%d",&latlon_opt);
	}

	q=0;	
	if ((c = strstr(line+1,"o=")) != NULL) { /* line plot flag option */
	  sscanf(c+2,"%d",&q);
	}
	if (debug) printf(" debug: plot lines from file latlon/xy: %d  flag option: %d\n",latlon_opt,q);

	line_plot_from_file(fname,q,Ii_opt,latlon_opt,c_opt,w_opt);
      }
      break;
      
    default:
      break;
    }

}





void latlon2pix(float alon, float alat, float *x, float *y, 
		int iopt, float xdeg, float ydeg,
		float ascale, float bscale, float a0, float b0);

int latlon_trans(float *x, float *y)
{
  float lon=*x,lat=*y;
  latlon2pix(lon, lat, x, y, iopt, xdeg, ydeg, ascale, bscale, a0, b0);
}

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

void subregion_extract(float x1, float y1, float x2, float y2, float *stval)
{
    int i__1, i__2, i__3;
    float r__1;
    float ebar, a02, b02, xdeg2, ydeg2, ascale2, bscale2;
    int nsx2, nsy2;
    int i1, i2, ix, iy, ix1, iy1, ix2, iy2;
    float *stval2;

    /* quantize locations to pixel values */

    ebar = .005f;                  /* 1/200 OF A PIXEL ROUNDING UP PERMITTED */
    r__1 = x1 + ebar;
    x1 = (float) floor((double) r__1);
    r__1 = y1 + ebar;
    y1 = (float) floor((double) r__1);
    r__1 = x2 + ebar;
    x2 = (float) floor((double) r__1);
    r__1 = y2 + ebar;
    y2 = (float) floor((double) r__1);

/* 	MAKE SURE PIXELS ARE WITHIN IMAGE (1..NSX,1..NSY) */

    i__2 = 1, i__3 = (int) x1;
    i__1 = max(i__2,i__3);
    ix1 = min(i__1,nsx);

    i__2 = 1, i__3 = (int) y1;
    i__1 = max(i__2,i__3);
    iy1 = min(i__1,nsy);

    i__2 = 1, i__3 = (int) x2;
    i__1 = max(i__2,i__3);
    ix2 = min(i__1,nsx);

    i__2 = 1, i__3 = (int) y2;
    i__1 = max(i__2,i__3);
    iy2 = min(i__1,nsy);

    ix = min(ix1,ix2);
    ix2 = max(ix1,ix2);
    ix1 = ix;
    iy = min(iy1,iy2);
    iy2 = max(iy1,iy2);
    iy1 = iy;

    nsx2 = ix2 - ix1 + 1;
    nsy2 = iy2 - iy1 + 1;

    if (debug) {
      printf(" SubImage size:            %d %d %d %d\n",nsx,nsy,nsx2,nsy2);
      printf(" Lower-Left  x,y location: %f,%f %d,%d\n",x1,y1,ix1,iy1);
      printf(" Upper-Right x,y location: %f,%f %d,%d\n",x2,y2,ix2,iy2);
    }
    
    if (nsx2 < 1 || nsy2 < 1) {
	fprintf(stderr,"*** Error Extracting SubRegion: ZERO SIZE %d %d\n",nsx2,nsy2);
	return;
    }

/* 	COMPUTE TRANSFORMATION INFORMATION FOR SUB IMAGE */

    if (iopt == 1 || iopt == 2) {         /* LAMBERT */
	x1 = (ix1 - 1) / ascale + a0;
	y1 = (iy1 - 1) / bscale + b0;
	a02 = x1;
	b02 = y1;
	xdeg2 = xdeg;
	ydeg2 = ydeg;
	ascale2 = ascale;
	bscale2 = bscale;
    } else if (iopt == 11 || iopt == 12 || iopt == 13) { /* EASE GRID */
	a02 = ix1 - 1.f - xdeg;
	b02 = iy1 - 1.f - ydeg;
	xdeg2 = xdeg;
	ydeg2 = ydeg;
	ascale2 = ascale;
	bscale2 = bscale;
    } else if (iopt == 5) {                        /* POLAR STEREOGRAPHIC */
	a02 = (ix1 - 1) * ascale + a0;
	b02 = (iy1 - 1) * bscale + b0;
	xdeg2 = xdeg;
	ydeg2 = ydeg;
	ascale2 = ascale;
	bscale2 = bscale;
    } else {                                       /* IMAGE ONLY AND LAT/LON */
	a02 = (ix1 - 1) / ascale + a0;
	b02 = (iy1 - 1) / bscale + b0;
	xdeg2 = (float) (nsx2) * xdeg / (float) nsx;
	ydeg2 = (float) (nsy2) * ydeg / (float) nsy;
	ascale2 = xdeg2 / (float) (nsx2);
	bscale2 = ydeg2 / (float) (nsy2);
    }

    stval2 = (float *) malloc(sizeof(float) * nsx2 * nsy2);
    if (stval2 == NULL) {
      fprintf(stderr,"*** error allocating temporary buffer for subregions ***\n");
      exit(-1);
    }

/* 	COPY DATA IN REGION INTO STORAGE AREA FOR NEW IMAGE */

    for (iy = 1; iy <= nsy2; ++iy)
      for (ix = 1; ix <= nsx2; ++ix) {
	i1 = (iy - 1) * nsx2 + ix;
	i2 = (iy - 1 + iy1 - 1) * nsx + (ix + ix1 - 1);
	stval2[i1-1] = stval[i2-1];
      }

    /* copy modified transformation back onto global variables*/

    a0 = a02;
    b0 = b02;
    xdeg = xdeg2;
    ydeg = ydeg2;
    ascale = ascale2;
    bscale = bscale2;
    nsx = nsx2;
    nsy = nsy2;

    for (iy = 0; iy < nsx * nsy; ++iy)
	stval[iy] = stval2[iy];

    free(stval2);
}


int read_colortable(char *name, char *rtab, char *gtab, char *btab, int flag)
{
  FILE *imf;
  int i,r,g,b;
  
  printf("Reading input %d color table file '%s'\n",flag,name);
  imf = fopen(name,"r"); 
  if (imf == NULL) {
    fprintf(stderr,"*** ERROR: cannot open color table file: %s\n",name);
    fflush(stderr);
    return(-1);
  } else {
    if (flag == 0) { /* binary color table file (1 byte/entry 256 red, then green, etc. */
      if (fread(rtab, sizeof(char), 256, imf) == 0) 
	fprintf(stderr," *** Error reading color table file (r)\n");
      if (fread(gtab, sizeof(char), 256, imf) == 0) 
	fprintf(stderr," *** Error reading color table file (g)\n");
      if (fread(btab, sizeof(char), 256, imf) == 0) 
	fprintf(stderr," *** Error reading color table file (b)\n");
    } else { /* ascii color table file 256 lines 1 r,g,b value per line */
      for (i=0; i< 256; i++)
	if (fscanf(imf,"%d %d %d",&r,&g,&b) == EOF) {
	  fprintf(stderr," *** Error reading color table file line %d\n",i);
	  fclose(imf);
	  return(-1);
	} else {	  
	  *(rtab+i) = r;
	  *(gtab+i) = g;
	  *(btab+i) = b;
	}
    }
    fclose(imf);
  }
  return(0);
}


void place_logo(char *d, char *c, float x1_opt, float y1_opt, int color, int flag, int nx, int ny)
{
  FILE *lmf;
  char *logo;
  int i,j,x=x1_opt,y=y1_opt;

  /* open logo image file and read into memory */
  
  lmf=fopen(c,"r");
  if (lmf == NULL) {
    fprintf(stderr,"*** Could not open logo file '%s' ***\n",c);
  } else {
    logo = (char *) malloc(sizeof(char) * nx * ny);
    if (logo == NULL) {
      fprintf(stderr,"*** ERROR: logo byte image memory allocation failure...\n");
      fclose(lmf);
      return;
    }
    if (fread(logo, sizeof(char), nx * ny, lmf) != nx * ny) {
      fprintf(stderr,"*** ERROR reading logo byte image...\n");
      fclose(lmf);
      free(logo);
      return;
    }
    fclose(lmf);

    fpimage_color(color, rtab, gtab, btab); /* override plotting array color*/

    /* copy logo image into output image */

    for (i=0; i < nx; i++)
      for (j=0; j < ny; j++)
	if (*(logo + (ny-j)*nx + i) != 0 )
	  if (x+i >= 0 && x+i < nsx && y+j >= 0 && y+j < nsy)
	    if (flag >= 0)
	      switch(bpp) {
	      case 1:
		*(d + (nsy-(y+j))*nsx + x+i) = color;
		break;
	      case 2:
		*((short int *) (d + (nsy-(y+j))*nsx + x+i)) = color;
		break;
	      case 4:
		*((int *) (d + (nsy-(y+j))*nsx + x+i)) = fpimage_1.color;
		break;
	      }
	    else {
	      switch(bpp) {
	      case 1:
		*(d + (nsy-(y+j))*nsx + x+i) = *(logo + (ny-j)*nx + i);
		break;
	      case 2:
		*((short int *) (d + (nsy-(y+j))*nsx + x+i)) = *(logo + (ny-j)*nx + i);  
		break;
	      case 4:
		fpimage_color( (int) *(logo + (ny-j)*nx + i), rtab, gtab, btab);
		*((int *) (d + (nsy-(y+j))*nsx + x+i)) = fpimage_1.color;
		break;
	      }
	    }
    
    free(logo);
  }
  
  return;
}


void get_X_Y_from_logo_file_name(char *f, int *nx, int *ny)
{
  char *c, ct[]="0123456789";

  /* finds the first two numerical values in the string f and returns them as nx and ny */
  
  *nx = *ny = 1; /* default values */

  if ((c = strrchr(f,'/')) != NULL) /* strip path off input file */
    f = c+1;

  c=strpbrk(f,ct);
  if (c != NULL) {
    c=c+sscanf(c,"%d",nx);
    c=strpbrk(c,ct);
    if (c != NULL)
      sscanf(c,"%d",ny);
  }
  if (debug) printf(" debug: get_X_Y_from_logo_file_name: from '%s' got %d %d\n",f,*nx,*ny);
  
  return;
}


void rotate_image(int rot, char *data, int *nsx, int *nsy)
{
  char *temp;
  int i, j, k, l, nsx2, nsy2, i1, j1, k1;
  
  switch(rot) {
  case 0:
  default:
    return;
  case 1:   /* 90 deg CW */
    nsx2 = *nsy;
    nsy2 = *nsx;
    i1 = nsx2;
    j1 = -1;
    k1 = *nsy - 1;
    break;
  case 2:   /* 180 deg CW */
    nsx2 = *nsx;
    nsy2 = *nsy;
    i1 = -1;
    j1 = -nsx2;
    k1 = (nsx2 - 1) + nsx2 * (nsy2 -1);
    break;
  case 3:   /* 270 deg CW */
    nsx2 = *nsy;
    nsy2 = *nsx;
    i1 = -nsx2;
    j1 = 1;
    k1 = nsx2 * (nsy2 -1);
    break;
  }
  
  temp = malloc(sizeof(char) * *nsx * *nsy * bpp);  
  if (temp == NULL) {
    fprintf(stderr,"*** ERROR allocating temporary space during image rotation ***\n");
    return;
  }

  printf("Begin rotation %d %d %d -> %d %d  %d %d %d\n",rot,*nsx,*nsy,nsx2,nsy2,i1,j1,k1);
  
  for (i=0; i< *nsx; i++)
    for (j=0; j< *nsy; j++) {
      k = i + j * *nsx;
      l = i * i1 + j * j1 + k1;
      switch(bpp) {
      case 1:
	temp[l]=data[k];
	break;
      case 2:
	*((short int *) (temp + l * bpp)) = *((short int *) (data + k * bpp));
	break;
      case 4:
	*((int *) (temp + l * bpp)) = *((int *) (data + k * bpp));
	break;
      }
    }

  for (i=0; i < *nsx * *nsy; i++)
      switch(bpp) {
      case 1:
	data[i]=temp[i];
	break;
      case 2:
	*((short int *) (data + i * bpp)) = *((short int *) (temp + i * bpp));
	break;
      case 4:
	*((int *) (data + i * bpp)) = *((int *) (temp + i * bpp));
	break;
      }

  *nsx = nsx2;
  *nsy = nsy2;

  free(temp);
  return;
}




void line_plot_from_file(char filename[], int q, int Ii_opt, int latlon_opt, int c_opt, int w_opt)
{     
  FILE *fid;
  int k,ip,opt;
  float x,y,x1,y1,x2,y2,dx,dy;
  int zeroi=0, one=1, two=2, three=3, five=5;
  float zerof=0.0, f360=360.0;
  
  
  fid=fopen(filename,"r");
  if (fid == NULL) {
    printf("*** error opening line plot file %s ***\n",filename);
    return;
  }
  if (debug) printf(" debug: reading from line plot file: %s\n",filename);

  newvpen_(&zeroi,&w_opt);
  x=c_opt;
  plotvt_(&x,&zerof,&zeroi);
  fpimage_color(c_opt, rtab, gtab, btab); /* override plotting array color*/
  if (debug) printf(" debug: color %d width %d  type %d Ii %d latlon_opt\n",c_opt,w_opt,q,Ii_opt,latlon_opt); 

  opt=0;
  while (1==1) {
    if (fscanf(fid,"%f",&y1) < 0) return;
    if (fscanf(fid,"%f",&x1) < 0) return;
    if (fscanf(fid,"%d",&ip) < 0) return;
    if (q >= 0) 
      if (fscanf(fid,"%d",&opt) < 0) return;

    /* (debug) printf("%f %f %d %d\n",x1,y1,ip,opt); */
    
    if (q <= 0 || q == opt) {

	if (ip == 3) {
	  x2 = x1;
	  y2 = y1;
	} else if (ip ==2) {
	  dx = x1;
	  dy = y1;
	  x = x2;
	  y = y2;
	  /* printf("plot_line %d %d %f %f %f %f\n",Ii_opt,latlon_opt,x,y,dx,dy); */
	  plot_line(Ii_opt,latlon_opt,&x,&y,&dx,&dy);
	  x2 = x1;
	  y2 = y1;
	} 
    }
  }
  plotvt_(&zerof,&zerof,&five);

}

	
plot_line(int Ii_opt, int latlon_opt, float *x1_opt, float *y1_opt, float *x2_opt, float *y2_opt)
{
  static float x, y, x0, y0, dx, dy, zerof = 0.0;
  static int k;
  static int two=2, three=3, five=5;
  
  if (Ii_opt > 1) {
    if (latlon_opt != 0) {
      x0 = *x1_opt;
      y0 = *y1_opt;
      dx = (*x2_opt - *x1_opt)/Ii_opt;
      dy = (*y2_opt - *y1_opt)/Ii_opt;

      for (k=0;k<Ii_opt+1;k++) {
	x=x0+dx*k;
	y=y0+dy*k;
	latlon_trans(&x,&y);
	if (k == 0)
	  plotvt_(&x,&y,&three);
	else {
	  plotvt_(&x,&y,&two);
	}
      }
      plotvt_(&zerof,&zerof,&five);
      
      latlon_trans(x1_opt,y1_opt);
      latlon_trans(x2_opt,y2_opt);
    } else {
      plotvt_(x1_opt,y1_opt,&three);
      plotvt_(x2_opt,y2_opt,&two);
      /* printf("line output 1: %f %f %f %f\n",*x1_opt,*y1_opt,*x2_opt,*y2_opt); */
      plotvt_(&zerof,&zerof,&five);
    }
  } else {
    if (latlon_opt != 0) {
      /* printf("line xform 2: %f %f %f %f\n",*x1_opt,*y1_opt,*x2_opt,*y2_opt); */
      latlon_trans(x1_opt,y1_opt);
      latlon_trans(x2_opt,y2_opt);
    } 
    plotvt_(x1_opt,y1_opt,&three);
    plotvt_(x2_opt,y2_opt,&two);
    /* printf("line output 2: %f %f %f %f\n",*x1_opt,*y1_opt,*x2_opt,*y2_opt); */
    plotvt_(&zerof,&zerof,&five);
  }
}
