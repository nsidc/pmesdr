/* (c) copyright 2014 David G. Long, Brigham Young University */
/*

   simplified program to compute Backus-Gilbert Inversion (bgi) images from
   simulated setup files generated

   Written by DGL 21 Jun 2014
   Revised by DGL 25 Jun 2014 + add output path

*/

/* define various sirf and program parameters */

float a_init=230.0;           /* initial image A value */
float meas_offset=0;          /* measurement offset */

/* both noisy and noise-free measurements are contained input file */

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "sir3.h"

#define NRANSI
#include "nr.h"
#include "nrutil.h"

#define REL_EOF   2           /* fseek relative to end of file */

/* the following BG parameters are subjectively set */
float bgi_gamma=0.01*3.141562654; /* def BGI gamma parameter */
float delta2=1.0;                 /* def BGI assumed noise variance */
float thres=0.125;                /* def minimum gain threshold */
float omega=0.001;                /* BGI scale factor */

/* function prototypes */

void count_hits(int count, int fill_array[], float response_array[], float thres, int cnt[], int *mdim, int nsx);

void make_indx(int nmax, int count, int fill_array[], float response_array[], float thres, char **indx, char * pointer);

void stat_updates(float tbval, float ang, int count, int *fill_array, float *response_array);

void filter(float *val, int size, int mode, int nsx, int nsy, float *temp, float thres);

char *addpath(char *outpath, char *name, char *temp);

/* global array variables used for storing images*/

float *a_val, *b_val, *a_temp, *sxy, *sx, *sx2, *sy, *tot;
int *cnts;

/* other global variables */

int NOISY=1;  /* use noisy measurements */

#define MAXFILES 2
FILE *imf, *jmf, *omf, *infiles[MAXFILES];
int nspace, nfiles=0, cur_file;
int head_len[MAXFILES];


/* main program */

int main(int argc, char **argv)
{

  char *file_in;
  char tstr[1000];  

  float latl, lonl, lath, lonh;
  char regname[11];
  int nls[MAXFILES], nrec, ncnt, nbyte, i, n, ii, iii, nsize;
  float ratio;
  char *space, *store, *store2, *last_store;
  float tbval, ang=0.0, azang;
  int count, ktime, iadd;
  int irecords;
  int non_size_x, non_size_y, nsx2, nsy2, ix, iy, nsize2;
  float xdeg2, ydeg2, ascale2, bscale2, a02, b02, gsize;
  char fname[180];
  char a_name[100], b_name[100];

/* SIR file header information */

  float v_min_A, v_max_A, anodata_A, anodata_B;
  int nsx, nsy, ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin;
  int iregion, itype_A, nhead, ndes, nhtype, idatatype, ldes, nia;
  char type_V[139];
  int ipol, ifreqhm, ispare1;
  char title[101], sensor[41], crproc[101], type_A[139], tag[101], crtime[29];
  float xdeg, ydeg, ascale, bscale, a0, b0;
  int iopt;
  int ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc;

#define MAXDES 1024
  char descrip[MAXDES+1];
#define MAXI 128
  short iaopt[MAXI];

  int its, irec, ierr, year, keep, tmax, ifile, j;
  float total;
  char pol;
  float amin, amax, bmin, bmax, temp, old_amin, old_amax,
    old_bmin, old_bmax, denom;

  int nmax, mdim, mdim2, k, dx, dy, i1, j1, m;
  int *ix0, *iy0, *ind;
  char **indx;
  float sum, *g, **z, **zc, *u, *v, *u1, *v1, *c, *work, *tb2, *ww, *patarr;
  float p,value1, value2;
  int *fill_array;
  float *weight_array;

  time_t tod;

  int storage = 2;
  int errors = 0;
  char title1[101];
  char outpath[100]="./";  

/* begin program */  

  printf("BYU simplified BGI program: C version 1.0\n");
  if (argc < 2) {
    printf("\nusage: %s setup_in noise_flag gamma delta2 thres outpath\n\n",argv[0]);
    printf(" input parameters:\n");
    printf("   setup_in    = input setup file\n");
    printf("   noise_flag  = use (1=noisy [def], 0=noise-free) measurements\n");
    printf("   gamma       = BGI gamma parameter\n");
    printf("   delta2      = BGI delta2 (noise variance)\n");
    printf("   thres       = gain threshold\n");
    printf("   outpath     = output path (def=./)\n");
   return(0);
  }

  file_in=argv[1];
  printf("Input file: '%s'\n",file_in);

  /* storage MUST be in ram for BGI processing */

  if (argc > 2) sscanf(argv[2],"%d",&NOISY);
  if (NOISY)
    printf(" Using noisy measurements\n");
  else
    printf(" Using noise-free measurements\n");
  
  if (argc > 3) sscanf(argv[3],"%f",&bgi_gamma);
  if (argc > 4) sscanf(argv[4],"%f",&delta2);
  if (argc > 5) sscanf(argv[5],"%f",&thres);
  if (argc > 6) sprintf(outpath,"%s/",argv[6]);
  printf(" Output path: %s\n",outpath);  

  printf("BGI options: omega=%f gamma=%f delta2=%f thres=%f\n",omega, bgi_gamma, delta2, thres);

  /* open input .setup file */

  imf = fopen(file_in,"r"); 
  if (imf == NULL) {
     fprintf(stdout,"ERROR: cannot open input file: %s\n",argv[1]); 
     exit(-1);
  }

/* get input file size */

  fseek(imf, 0L, REL_EOF);
  nls[nfiles]=ftell(imf);
  rewind(imf);

  printf("\n\nOpening setup file '%s' file# %d\n",file_in,nfiles+1);

  fread(&nsx, sizeof(int), 1, imf);
  fread(&nsy, sizeof(int), 1, imf);
  fread(&ascale, sizeof(float), 1, imf);
  fread(&gsize, sizeof(float), 1, imf);

  printf("Image size: %d %d %f grd size: %f\n",nsx,nsy,ascale,gsize);  

  head_len[nfiles] = 20 + nsx*nsy*4+8;
  
  a0=0.0;
  b0=0.0;
  xdeg=nsx*ascale;
  bscale=ascale;
  ydeg=nsy*bscale;
  iopt=-1;
  isday=0;
  ieday=0;
  ismin=0;
  iemin=0;
  iyear=2003;
  ipol=0;
  iregion=97;
  strcpy(regname,"Simulated Test");
  non_size_x=round(gsize/ascale);
  non_size_y=non_size_x;
  nsx2=nsx/non_size_x;
  nsy2=nsy/non_size_y;
  a02=0.0;
  b02=0.0;
  ascale2=ascale*non_size_x;
  bscale2=bscale*non_size_y;
  xdeg2=xdeg;
  ydeg2=ydeg;

  if (NOISY) {
    strcpy(a_name,"simA2.bgi");
  } else {
    strcpy(a_name,"simA.bgi");
  }  
  strcpy(b_name,"true.sir");
  
  printf("File size: %d %d\n",nls[nfiles],head_len[nfiles]);
  nls[nfiles] = nls[nfiles] - head_len[nfiles];
  nfiles++;

  printf("\n");

/* header read completed, now determine home much program memory to allocate */
  nspace = 0;
  for(i=0;i<nfiles;i++) {
    nspace = nspace + nls[i];
  }
  nspace=nspace*1.01;
  
  if (storage != 1) { /* allocate memory storage space for measurements */
    space = (char *) malloc(nspace);
    if (space == NULL) {
      fprintf(stdout,"*** Inadequate memory for data file storage\n");
      if (storage == 2) {
	fprintf(stdout,"*** Will use file instead (multiple file reads required)\n\n");
      } else
	exit(-1);
      storage = 1;  /* force use of file */
    }
  }

/* if program is to be run with file storage (storage=1), allocate working 
   buffer array for file reading */

  if (storage == 1) {
    nspace = 4096*64;  /* should be adequate for all fill_array sizes */
    space = (char *) malloc(nspace);
    if (space == NULL) {
      fprintf(stdout,"*** Inadequate memory for temp storage 1\n");
      exit(-1);
    }
    store2 = (char *) malloc(sizeof(int)*nspace/2);
    if (store2 == NULL) {
      fprintf(stdout,"*** Inadequate memory for temp storage 2\n");
      exit(-1);
    }
  }
  
/* allocate storage space for image and working array */

  nsize = nsx * nsy;
  
  a_val  = (float *) malloc(sizeof(float)*nsize);
  b_val  = (float *) malloc(sizeof(float)*nsize);
  a_temp = (float *) malloc(sizeof(float)*nsize);
  cnts   = (int *) a_temp;  /* share storage space */
  sxy    = (float *) malloc(sizeof(float)*nsize);
  sx     = (float *) malloc(sizeof(float)*nsize);
  sx2    = (float *) malloc(sizeof(float)*nsize);
  sy     = (float *) malloc(sizeof(float)*nsize);
  tot    = (float *) malloc(sizeof(float)*nsize);

  if (a_val == NULL || b_val == NULL || a_temp == NULL || sxy == NULL
      || sx == NULL || sx2 == NULL || sy == NULL || tot == NULL) {
    fprintf(stdout,"*** ERROR: inadequate memory for image working storage %d\n",nsize);
    exit(-1);
  }

  /*read true image */
  fread(b_val, sizeof(float), nsx*nsy, imf);

  /* initialize count array */
  for (i=0; i < nsize; i++)
    cnts[i]=0;
  mdim=0;

  /* with storage allocated, copy file into memory */

  if (storage != 1) {   /* read measurement file into memory, 
			   storing only essential information  */

    nrec = 0;         /* number of meaurements in file */
    ncnt = 0;         /* number of useable measurements */
    nbyte = 0;        /* file size in bytes */
    store=space;      /* storage pointer */
  
    printf("Begin file copy into memory\n");

    while (fread(store, sizeof(char), 16, imf) == 16) {

      iadd  = *((int *)    (store+0));
      count = *((int *)    (store+4));
      tbval = *((float *)  (store+8));
      azang = *((float *) (store+12));
      
      if (count > 5000)
	printf("*** Count error %d  record %d\n",count,nrec);

      /*
      printf("ncnt %d  %f %f  count %d %d\n",ncnt,tbval,azang,count,iadd); 	
      if (ncnt > 99) goto label;
      */

      /* if measurement is "valid" keep it by indexing counters 
	 if not, new values will be stored over old values */

      store2=store+4;
      nbyte=nbyte+16;
      store=store+16;
      ncnt++;
      keep=1;
      if (count < 1) keep=0;      

      /* read fill_array pixel indices */
      if (nbyte+count*4 < nspace) {
	if (fread(store, sizeof(char), 4*count, imf) != 4*count) {
	  fprintf(stdout," *** Error reading input file data at 111\n");
	  /* exit(-1); */
	  goto label;
	}

	if (keep == 1) {
	  last_store=store;
	  nbyte=nbyte+count*4;
	  store=store+count*4;
	}
      } else {
	fprintf(stdout," *** out of storage space 1 *** %d %d %d\n",ncnt,nbyte,nspace);
	exit(-1);
      }

      /* read response_array weights */
      if (nbyte+count*4 < nspace) {
	if (fread(store, sizeof(char), 4*count, imf) != 4*count) {
	  fprintf(stdout," *** Error reading input file data at 1111\n");
	  goto label;
	}
	if (keep == 1) {
	  count_hits(count, (int *) last_store, (float *) store, thres, cnts, &m, nsx);
	  if (m > mdim) mdim = m;
	  nbyte=nbyte+count*4;
	  store=store+count*4;
	}
      } else {
	fprintf(stdout," *** out of storage space 2 *** %d %d %d\n",ncnt,nbyte,nspace);
	exit(-1);
      }

      /*      
      printf("ncnt %d  %f %f  count %d %d  %d %f\n",ncnt,tbval,azang,count,iadd,
	     *(int *) store2, *(float *) (store2+4)); 
	     */
     
      nrec++;
    }
  label:
    fclose(imf);

/* print measurement file storage requirements */

    ratio=100.0 * (float) nbyte / (float) nspace;
    printf("  Total storage used: %d %d recs = %d of %d (%.1f%% %.1f%%)\n",
	   nrec,ncnt,nbyte,nspace,ratio,100.0);
  }

  /* generate SIR file header info */
  nhtype=31;		/* set header type */
  idatatype=2;		/* output image is in standard i*2 form */
  ifreqhm=31;
  nia=0;                /* no extra integers */
  ldes=0;               /* no extra text */
  ndes=0;
  ispare1=0;
  strncpy(tag,"(c) 2014 BYU MERS Laboratory",40);
  strncpy(sensor,"Simulation",40);
  regname[9]='\0';

  sprintf(title,"BGI image of %s",regname);

  (void) time(&tod);
  (void) strftime(crtime,28,"%X %x",localtime(&tod));
  printf("Current time: '%s'\n",crtime);

  /* set projection scale factors */

  switch (iopt){
  case -1: /* image only */
    ideg_sc=10;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 0: /* rectalinear lat/lon */
    ideg_sc=100;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=-100;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 1: /* lambert */
  case 2:
    ideg_sc=100;
    iscale_sc=100;
    i0_sc=1;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 5: /* polar stereographic */
    ideg_sc=100;
    iscale_sc=100;
    i0_sc=1;
    ixdeg_off=-100;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 11: /* EASE grid */
  case 12:
  case 13:
    ideg_sc=10;
    iscale_sc=1000;
    i0_sc=10;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  default: /* unknown */
    ideg_sc=100;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
  }

  /* image specific header info */

  ioff_A=140;
  iscale_A=200;
  itype_A=1;
  anodata_A=150.0;
  v_min_A=0.0;
  v_max_A=0.1;
  sprintf(type_A,"TB A image  (%s)",a_name);
  
  old_amin=1.e25;
  old_amax=-1.e25;
  for (i=0; i < nsize; i++) {
    old_amin=min(old_amin, *(b_val+i));
    old_amax=max(old_amax, *(b_val+i));
  }
  printf("True min/max %f %f\n",old_amin,old_amax);
  v_min_A=old_amin;
  v_max_A=old_amax;

  if (0) {      
    printf("Writing true A file '%s'\n", b_name);
    ierr = write_sir3(addpath(outpath,b_name, tstr), b_val, &nhead, nhtype, 
		      idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
		      ixdeg_off, iydeg_off, ideg_sc, iscale_sc, 
		      ia0_off, ib0_off, i0_sc,
		      ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
		      iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
		      anodata_A, v_min_A, v_max_A, sensor, title1, type_A, tag,
		      crproc, crtime, descrip, ldes, iaopt, nia);
    if (ierr < 0) {
      fprintf(stdout,"*** ERROR writing true file ***\n");
      errors++;
    }
  }
  
  printf("Begin BGI processing\n");  
  
  /* determine maximum hits per pixel */
  nmax=0;
  for (i=0; i< nsize; i++)
    if (cnts[i] > nmax) nmax=cnts[i];
  printf("\nMaximum hits above threshold: %d  max size: %d\n",nmax,mdim);
  mdim = mdim * 2+1;

  /* BG processing */

  /* allocate index array and BGI working arrays*/
  indx = (char **) malloc(sizeof(char *)*nsize*nmax);
  z = matrix(1,nmax,1,nmax);
  zc = matrix(1,nmax,1,nmax);
  u = vector(1,nmax);
  u1 = vector(1,nmax);
  v = vector(1,nmax); 
  v1 = vector(1,nmax); 
  ind = ivector(1,nmax);
  work = vector(1,nmax);
  c = vector(1,nmax);
  tb2 = vector(1,nmax);
  patarr = (float *) malloc(sizeof(float)*mdim*mdim*nmax);
  ix0 = (int *) malloc(sizeof(int)*(nmax+1));
  iy0 = (int *) malloc(sizeof(int)*(nmax+1));
  if (indx == NULL || z == NULL || u == NULL || v == NULL || c == NULL
      || zc == NULL || u1 == NULL || v1 == NULL || ind == NULL
      || work == NULL || tb2 == NULL || patarr == NULL || ix0 == NULL || iy0 == NULL) {
    printf("*** error allocating BGI work arrays \n");
    exit(-1);
  }


  /* create index array */
  for (i=0; i< nsize; i++)
    for (j=0; j< nmax; j++)
      indx[i*nmax+j] = NULL;

  store=space;
  for (irec = 0; irec < ncnt; irec++) {
    last_store=store;
    count = *((int *) (store+4));
    iadd = *((int *)  (store+0));
    store = store + 16;
    store2 = store + 4*count;
  
    make_indx(nmax, count, (int *) store, (float *) store2, thres, indx, last_store);
    store = store+4*count;
    store = store+4*count;
  }

  printf("Index array created %d %d\n",nsize,ncnt);
  
  
/* Begin BGI processing loop */

  mdim2=mdim/2+1;

  for (its=0; its < nsize; its++) {   /* for each pixel */
    *(a_val+its) = anodata_A;

    /* print progress */
    if ((its % nsx) == nsx/2  && (its/nsx) % 5 == 0) 
      printf("Processing row %d of %d  %f\n",its/nsx,nsy,amin);
    
    if (cnts[its] > 0) {
      m=0;  /* number of valid measurements */
      for (k=0; k < nmax; k++) {  /* for possible each measurement */
	store = indx[its*nmax + k];
	if (store != NULL) {        /* get measurement info from memory */
	  if (NOISY)
	    tbval = *((float *) (store+12));
	  else
	    tbval = *((float *) (store+8));
	  count = *((int *)   (store+4));
	  iadd  = *((int *)   (store+0));
	  //iadd = (iadd > 0 ? iadd : -iadd) -1;

	  store = store + 16;
	  fill_array = (int *) store;
	  store = store + 4*count;
	  weight_array = (float *) store;
	  /*
	  printf("at %d %d %d %d\n",its,m,k,count);
	  for (i=0; i < count; i++)
	    printf("  %d %d %d\n",i,fill_array[i],weight_array[i]);
	  */
	  m++;
	  v[m] = 0.0000001;  /* a very small, non-zero value */
	  sum = 0.0;
	  for (i=0; i < count; i++) 
	    if (fill_array[i]>0) {
	      if (fill_array[i]-1 == its) 
		v[m] = weight_array[i];
	      sum += weight_array[i];
	    }
	  tb2[m] = tbval;
	  if (sum > 0.0) 
	    v[m]=v[m]/sum;
	  u[m]=1.0;
	
	  ix0[m] = iadd % nsx;
	  iy0[m] = iadd / nsx;

	  for (i=0; i< mdim*mdim; i++)
	    patarr[i*nmax+m-1]=0.0;
	
	  for (i=0; i < count; i++) 
	    if (fill_array[i]>0) {
	      ix = (fill_array[i]-1) % nsx - ix0[m] + mdim2;
	      iy = (fill_array[i]-1) / nsx - iy0[m] + mdim2;
	      if (ix >= 0 && iy >= 0 && ix < mdim && iy < mdim)
		patarr[(iy*mdim+ix)*nmax+m-1]=weight_array[i];
	      else
		printf("*** patarr error %d %d %d  %d %f\n",i,ix,iy,its,weight_array[i]);
	    }
	}
      }

      if (m > 0) {

	/* compute z matrix */
	for (i=1; i <= m; i++)
	  for (j=1; j <= m; j++) {
	    dx = ix0[i] - ix0[j];
	    dy = iy0[i] - iy0[j];
	    z[i][j] = 0.0;
	    sum=0.0;
	    for (i1=1; i1 <= mdim; i1++)
	      for (j1=1; j1 <= mdim; j1++) {
		ix=i1-dx;
		iy=j1-dy;
		if (ix > 0 && iy > 0 && ix <= mdim && iy <= mdim)
		  sum = sum + patarr[((j1-1)*mdim+i1-1)*nmax+i-1]*patarr[((iy-1)*mdim+ix-1)*nmax+j-1];
	      } 
	    z[i][j] = sum * cos(bgi_gamma);

	    if (i == j) z[i][j] =  z[i][j] + omega * sin(bgi_gamma) * delta2;
	    z[j][i] = z[i][j];
	    /*	    printf("in z %d %d %d %d %f %f\n",i,j,dx,dy,sum*cos(bgi_gamma),z[dx,dy]); */
	  }
	/*
	printf("z is: %d %d %f\n",m,m,omega * sin(bgi_gamma) * delta2);
	for (i=1;i<=m;i++) {
	  for (j=1;j<=m;j++)
	    printf("%f ",z[i][j]);
	  printf("\n");
	}
	*/

	/* Do LU decomposition */
	ludcmp(z,m,ind,&p);

	for (i=1;i<=m;i++) {
	  u1[i]=u[i];
	  v1[i]=v[i];
	  for (j=1;j<=m;j++)
	    zc[i][j] = z[i][j];
	}
      
      /* solve linear system z x = u [compute z^-1 u]  (u and z destroyed in process) */
	lubksb(z,m,ind,u1);

      /* comute u^t z^-1 u */
	value2 = 0.0;
	for (i=1; i <= m; i++)
	  value2 = value2 + u[i] * u1[i];

	for (i=1;i<=m;i++)
	  for (j=1;j<=m;j++)
	    z[i][j] = zc[i][j];

      /* solve linear system z x = v [compute z^-1 v]  (v and z destroyed in process) */
	lubksb(z,m,ind,v1);

      /* comute u^t z^-1 v */
	value1 = 0.0;
	for (i=1; i <= m; i++)
	  value1 = value1 + u[i] * v1[i];

      /* compute work vector */
	for (i=1; i <= m; i++)
	  work[i] = cos(bgi_gamma)*v[i] + u[i] * (1.0 - cos(bgi_gamma) * value1)/value2;
      
      /* solve linear system z c = work [compute z^-1 work]  (destroyed in process) */
	lubksb(zc,m,ind,work);
      
      /* compute BG pixel estimate value */
	sum = 0.0;
	for (i=1; i <= m; i++)
	  sum = sum + work[i] * tb2[i];
	a_val[its] = sum;
	/*
	printf("one %d %d  %f %f %f\n",its,m,sum,value1,value2);
	for (i=1; i <= m; i++)
	  printf("%d %f %f %f %f %f %f\n",i,u[i],v[i],u1[i],v1[i],work[i],tb2[i]);
	*/
	amin=a_val[its];
	
      } else /* pixel not hit, set its value to the default nodata value */
	a_val[its] = anodata_A;
    } 
  }
  printf("\nMaximum hits above threshold: %d  max size: %d\n",nmax,mdim);


  /* output image file */
  sprintf(crproc,"BYU MERS:sim_bgi v1.0 g=%f d2=%f thres=%f", bgi_gamma, delta2, thres);

  printf("\n"); 
  printf("Writing A output BGI file '%s'\n", a_name);
  ierr = write_sir3(addpath(outpath,a_name, tstr), a_val, &nhead, nhtype, 
		    idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
		    ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
		    ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
		    iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
		    anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
		    crproc, crtime, descrip, ldes, iaopt, nia);
  if (ierr < 0) {
    printf("*** ERROR writing BGI A output file ***\n");
    errors++;
  } else
    printf("Successfully wrote file\n");

  /* apply median filter to image */
  filter(a_val, 3, 0, nsx, nsy, a_temp, anodata_A);  /* 3x3 median filter */

  if (NOISY) {
    strcpy(a_name,"simA2_median.bgi");
  } else {
    strcpy(a_name,"simA_median.bgi");
  }
  sprintf(title,"BGI w/median image of %s",regname);
  printf("\n"); 
  printf("Writing median-filtered A output BGI file'%s'\n", a_name);
  ierr = write_sir3(addpath(outpath, a_name, tstr), a_val, &nhead, nhtype, 
		    idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
		    ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
		    ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
		    iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
		    anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
		    crproc, crtime, descrip, ldes, iaopt, nia);
  if (ierr < 0) {
    printf("*** ERROR writing median-filtered BGI A output file ***\n");
    errors++;
  } else
    printf("Successfully wrote file\n");
  
  /* end of program */

  /* free malloc'ed memory (not strictly necessary, but good to be explicit) */
  free(space);
  free(a_val);
  free(b_val);
  free(a_temp);
  free(sxy);
  free(sx);
  free(sx2);
  free(sy);
  free(tot);
 
 return(errors);
}


/* ***************** support routines **************** */

void count_hits(int count, int fill_array[], float response_array[], float thres, int cnt[], int *mdim, int nsx)
{
  static int i,n,x,y,xx,yx,xn,yn;
  static float m, mpeak=-1.0;

  /* find peak response of measurement*/
  for (i=0; i < count; i++) {      
    m=response_array[i];
    if (m > mpeak) mpeak=m;
  }
  /* compute (local) threshold based on peak response */
  mpeak=thres*mpeak;

  yx=xx=-1.0;
  yn=xn=1.e6;
  for (i=0; i < count; i++) {
    n=fill_array[i]-1;
    m=response_array[i];
    if (m >= mpeak) { /* count all pixels hit by measuremen that are above threshold */ 
      (cnt[n])++;
      x = n % nsx;
      y = n / nsx;
      xx = max(x,xx);
      yx = max(y,yx);
      xn = min(x,xn);
      yn = min(y,yn);
    } else { /* mark as not to be used */
      fill_array[i]=0;
      response_array[i]=0.0;      
    }	
  }
  x=xx-xn+1;
  y=yx-yn+1;
  *mdim=max(x,y);
}

void make_indx(int nmax, int count, int fill_array[], float response_array[], float thres, char **indx, char *pointer)
{
  static int i,j,n;
  float m, mpeak=-1.0;  

  /* find peak response of measurement*/
  for (i=0; i < count; i++)
    if (fill_array[i] > 0) {
      m=response_array[i];
      if (m > mpeak) mpeak=m;
    }
  /* compute (local) threshold based on peak response */
  mpeak=thres*mpeak;
  
  for (i=0; i < count; i++) 
    if (fill_array[i] > 0) {
      n=fill_array[i]-1;
      m=response_array[i];
      if (m >= mpeak) {
	j = 0;
	while (j < nmax && indx[n*nmax+j] != NULL) j++;
	if (j < nmax) indx[n*nmax+j]=pointer;
      }
    }  
}



/* routine to compute variance and error from measurements */

void stat_updates(float tbval, float ang, int count, int fill_array[],
		  float response_array[])
{
  float ave, sigv;
  float total=0.0, num=0.0;
  int i, n;
  float m;

  /* compute forward projection of measurement */
  
  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    sigv = *(a_val+n-1);
    total = total + m * sigv;
    num = num + m;
  }
  if (num == 0) return;

  /*  ave = 10.0 * log10( (double) (total/num)); */
  ave =(double) (total/num);
  

  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    *(tot+n-1) = *(tot+n-1) + m;
    sigv = (tbval - ave);      /* difference */
    *(sx+n-1) = *(sx+n-1) + m * sigv;
    *(sy+n-1) = *(sy+n-1) + m * sigv * sigv;
  }

  return;
}


/* modified median or smoothing filter routine */

float median(float *array, int count);

void filter(float *val, int size, int mode, int nsx, int nsy, float *temp, float thres)
{
  float array[100], total;
  int i,j,x,y,size2,count,x1,x2,y1,y2;
  
  if ((size % 2) == 0) size=size+1;   /* force window size odd */
  size2 = (size-1)/2;                 /* window center */

  for (x=1; x<= nsx; x++)
    for (y=1; y<= nsy; y++) {
      count=0;
      y1=max(y-size2,1);
      y2=min(y+size2,nsy);
      x1=max(x-size2,1);
      x2=min(x+size2,nsx);
      for (i=x1; i <= x2; i++)
	for (j=y1; j <= y2; j++)
	  if (*(val+i+(j-1)*nsx-1) > thres) {
	    array[count]=*(val+i+(j-1)*nsx-1);
	    count++;
	  };
      
      if (count > 0 && *(val+x+(y-1)*nsx-1) > thres) {
	if (mode == 0)  /* Mode 0 = median */
	  *(temp+(y-1)*nsx+x-1) = median(array,count);
	else {		/* Mode 1 = average */
	  total=0.0;
	  for (i=0; i<count; i++)
	    total=total+array[i];
	  *(temp+(y-1)*nsx+x-1) = total/count;
	}
      } else
	*(temp+(y-1)*nsx+x-1) = thres;
    };
  
  for (i=0; i < nsx*nsy; i++) {
    *(val+i) = *(temp+i);
    *(temp+i)=0.0;
  }
  return;
}

/* compute modified median of an array of values */

float median(float array[], int count)
{
  int i,j;
  float temp;

  for (i=count; i >= 2; i--)
    for (j=1; j <= i-1; j++)
      if (array[i-1] < array[i-j-1]) {
	temp=array[i-1];
	array[i-1]=array[i-j-1];
	array[i-j-1]=temp;
      };
  
  temp=array[count/2];

  if (array[count-2]-array[1] < 0.25 && count > 5) {
    temp=0.0;
    for (i=count/2-1; i <= count/2+3; i++)
      temp=temp+array[i-1];
    temp=temp/5.0;
  }
  return(temp);
}

char *addpath(char *outpath, char *name, char *temp)
{ /* append path to name, return pointer to temp */
  sprintf(temp,"%s/%s",outpath,name);
  return(temp);  
}

/****************************************************************************/

/* Numerical Recipes routines */

void lubksb(float **a, int n, int *indx, float b[])
{
	int i,ii=0,ip,j;
	float sum;

	for (i=1;i<=n;i++) {
		ip=indx[i];
		sum=b[ip];
		b[ip]=b[i];
		if (ii)
			for (j=ii;j<=i-1;j++) sum -= a[i][j]*b[j];
		else if (sum) ii=i;
		b[i]=sum;
	}
	for (i=n;i>=1;i--) {
		sum=b[i];
		for (j=i+1;j<=n;j++) sum -= a[i][j]*b[j];
		b[i]=sum/a[i][i];
	}
}

#define TINY 1.0e-20;

void ludcmp(float **a, int n, int *indx, float *d)
{
	int i,imax,j,k;
	float big,dum,sum,temp;
	float *vv;

	vv=vector(1,n);
	*d=1.0;
	for (i=1;i<=n;i++) {
		big=0.0;
		for (j=1;j<=n;j++)
			if ((temp=fabs(a[i][j])) > big) big=temp;
		if (big == 0.0) nrerror("Singular matrix in routine ludcmp");
		vv[i]=1.0/big;
	}
	for (j=1;j<=n;j++) {
		for (i=1;i<j;i++) {
			sum=a[i][j];
			for (k=1;k<i;k++) sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
		}
		big=0.0;
		for (i=j;i<=n;i++) {
			sum=a[i][j];
			for (k=1;k<j;k++)
				sum -= a[i][k]*a[k][j];
			a[i][j]=sum;
			if ( (dum=vv[i]*fabs(sum)) >= big) {
				big=dum;
				imax=i;
			}
		}
		if (j != imax) {
			for (k=1;k<=n;k++) {
				dum=a[imax][k];
				a[imax][k]=a[j][k];
				a[j][k]=dum;
			}
			*d = -(*d);
			vv[imax]=vv[j];
		}
		indx[j]=imax;
		if (a[j][j] == 0.0) a[j][j]=TINY;
		if (j != n) {
			dum=1.0/(a[j][j]);
			for (i=j+1;i<=n;i++) a[i][j] *= dum;
		}
	}
	free_vector(vv,1,n);
}
#undef TINY

/* CAUTION: This is the ANSI C (only) version of the Numerical Recipes
   utility file nrutil.c.  Do not confuse this file with the same-named
   file nrutil.c that is supplied in the 'misc' subdirectory.
   *That* file is the one from the book, and contains both ANSI and
   traditional K&R versions, along with #ifdef macros to select the
   correct version.  *This* file contains only ANSI C.               */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#define NR_END 1
#define FREE_ARG char*

void nrerror(char error_text[])
/* Numerical Recipes standard error handler */
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

float *vector(long nl, long nh)
/* allocate a float vector with subscript range v[nl..nh] */
{
	float *v;

	v=(float *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(float)));
	if (!v) nrerror("allocation failure in vector()");
	return v-nl+NR_END;
}

int *ivector(long nl, long nh)
/* allocate an int vector with subscript range v[nl..nh] */
{
	int *v;

	v=(int *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(int)));
	if (!v) nrerror("allocation failure in ivector()");
	return v-nl+NR_END;
}

unsigned char *cvector(long nl, long nh)
/* allocate an unsigned char vector with subscript range v[nl..nh] */
{
	unsigned char *v;

	v=(unsigned char *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(unsigned char)));
	if (!v) nrerror("allocation failure in cvector()");
	return v-nl+NR_END;
}

unsigned long *lvector(long nl, long nh)
/* allocate an unsigned long vector with subscript range v[nl..nh] */
{
	unsigned long *v;

	v=(unsigned long *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(long)));
	if (!v) nrerror("allocation failure in lvector()");
	return v-nl+NR_END;
}

double *dvector(long nl, long nh)
/* allocate a double vector with subscript range v[nl..nh] */
{
	double *v;

	v=(double *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(double)));
	if (!v) nrerror("allocation failure in dvector()");
	return v-nl+NR_END;
}

float **matrix(long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	float **m;

	/* allocate pointers to rows */
	m=(float **) malloc((size_t)((nrow+NR_END)*sizeof(float*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(float *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

double **dmatrix(long nrl, long nrh, long ncl, long nch)
/* allocate a double matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	double **m;

	/* allocate pointers to rows */
	m=(double **) malloc((size_t)((nrow+NR_END)*sizeof(double*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;

	/* allocate rows and set pointers to them */
	m[nrl]=(double *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(double)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

int **imatrix(long nrl, long nrh, long ncl, long nch)
/* allocate a int matrix with subscript range m[nrl..nrh][ncl..nch] */
{
	long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
	int **m;

	/* allocate pointers to rows */
	m=(int **) malloc((size_t)((nrow+NR_END)*sizeof(int*)));
	if (!m) nrerror("allocation failure 1 in matrix()");
	m += NR_END;
	m -= nrl;


	/* allocate rows and set pointers to them */
	m[nrl]=(int *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(int)));
	if (!m[nrl]) nrerror("allocation failure 2 in matrix()");
	m[nrl] += NR_END;
	m[nrl] -= ncl;

	for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

float **submatrix(float **a, long oldrl, long oldrh, long oldcl, long oldch,
	long newrl, long newcl)
/* point a submatrix [newrl..][newcl..] to a[oldrl..oldrh][oldcl..oldch] */
{
	long i,j,nrow=oldrh-oldrl+1,ncol=oldcl-newcl;
	float **m;

	/* allocate array of pointers to rows */
	m=(float **) malloc((size_t) ((nrow+NR_END)*sizeof(float*)));
	if (!m) nrerror("allocation failure in submatrix()");
	m += NR_END;
	m -= newrl;

	/* set pointers to rows */
	for(i=oldrl,j=newrl;i<=oldrh;i++,j++) m[j]=a[i]+ncol;

	/* return pointer to array of pointers to rows */
	return m;
}

float **convert_matrix(float *a, long nrl, long nrh, long ncl, long nch)
/* allocate a float matrix m[nrl..nrh][ncl..nch] that points to the matrix
declared in the standard C manner as a[nrow][ncol], where nrow=nrh-nrl+1
and ncol=nch-ncl+1. The routine should be called with the address
&a[0][0] as the first argument. */
{
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1;
	float **m;

	/* allocate pointers to rows */
	m=(float **) malloc((size_t) ((nrow+NR_END)*sizeof(float*)));
	if (!m) nrerror("allocation failure in convert_matrix()");
	m += NR_END;
	m -= nrl;

	/* set pointers to rows */
	m[nrl]=a-ncl;
	for(i=1,j=nrl+1;i<nrow;i++,j++) m[j]=m[j-1]+ncol;
	/* return pointer to array of pointers to rows */
	return m;
}

float ***f3tensor(long nrl, long nrh, long ncl, long nch, long ndl, long ndh)
/* allocate a float 3tensor with range t[nrl..nrh][ncl..nch][ndl..ndh] */
{
	long i,j,nrow=nrh-nrl+1,ncol=nch-ncl+1,ndep=ndh-ndl+1;
	float ***t;

	/* allocate pointers to pointers to rows */
	t=(float ***) malloc((size_t)((nrow+NR_END)*sizeof(float**)));
	if (!t) nrerror("allocation failure 1 in f3tensor()");
	t += NR_END;
	t -= nrl;

	/* allocate pointers to rows and set pointers to them */
	t[nrl]=(float **) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float*)));
	if (!t[nrl]) nrerror("allocation failure 2 in f3tensor()");
	t[nrl] += NR_END;
	t[nrl] -= ncl;

	/* allocate rows and set pointers to them */
	t[nrl][ncl]=(float *) malloc((size_t)((nrow*ncol*ndep+NR_END)*sizeof(float)));
	if (!t[nrl][ncl]) nrerror("allocation failure 3 in f3tensor()");
	t[nrl][ncl] += NR_END;
	t[nrl][ncl] -= ndl;

	for(j=ncl+1;j<=nch;j++) t[nrl][j]=t[nrl][j-1]+ndep;
	for(i=nrl+1;i<=nrh;i++) {
		t[i]=t[i-1]+ncol;
		t[i][ncl]=t[i-1][ncl]+ncol*ndep;
		for(j=ncl+1;j<=nch;j++) t[i][j]=t[i][j-1]+ndep;
	}

	/* return pointer to array of pointers to rows */
	return t;
}

void free_vector(float *v, long nl, long nh)
/* free a float vector allocated with vector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_ivector(int *v, long nl, long nh)
/* free an int vector allocated with ivector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_cvector(unsigned char *v, long nl, long nh)
/* free an unsigned char vector allocated with cvector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_lvector(unsigned long *v, long nl, long nh)
/* free an unsigned long vector allocated with lvector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_dvector(double *v, long nl, long nh)
/* free a double vector allocated with dvector() */
{
	free((FREE_ARG) (v+nl-NR_END));
}

void free_matrix(float **m, long nrl, long nrh, long ncl, long nch)
/* free a float matrix allocated by matrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}

void free_dmatrix(double **m, long nrl, long nrh, long ncl, long nch)
/* free a double matrix allocated by dmatrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}

void free_imatrix(int **m, long nrl, long nrh, long ncl, long nch)
/* free an int matrix allocated by imatrix() */
{
	free((FREE_ARG) (m[nrl]+ncl-NR_END));
	free((FREE_ARG) (m+nrl-NR_END));
}

void free_submatrix(float **b, long nrl, long nrh, long ncl, long nch)
/* free a submatrix allocated by submatrix() */
{
	free((FREE_ARG) (b+nrl-NR_END));
}

void free_convert_matrix(float **b, long nrl, long nrh, long ncl, long nch)
/* free a matrix allocated by convert_matrix() */
{
	free((FREE_ARG) (b+nrl-NR_END));
}

void free_f3tensor(float ***t, long nrl, long nrh, long ncl, long nch,
	long ndl, long ndh)
/* free a float f3tensor allocated by f3tensor() */
{
	free((FREE_ARG) (t[nrl][ncl]+ndl-NR_END));
	free((FREE_ARG) (t[nrl]+ncl-NR_END));
	free((FREE_ARG) (t+nrl-NR_END));
}
