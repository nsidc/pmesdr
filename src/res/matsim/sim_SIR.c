/* (c) copyright 2003 David G. Long, Brigham Young University */
/*

   simplified SIR/SIRF program to compute SIR/SIRF image from simulated data

   Written by DGL Feb. 22, 2003
    + modified from sea_meta_sir_egg4c.c
*/

/* define various sirf and program parameters */

float a_init=150.0;           /* initial image A value */
float meas_offset=0;          /* measurement offset */
int   nits=30;                /* maximum number of SIR iterations */

/* both noisy and noise-free measurements are contained input file */



#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define rnd(a) ((a) >= 0 ? floor((a)+0.5L) : ceil((a)-0.5L))

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "sir3.h"

#define REL_EOF   2           /* fseek relative to end of file */


/* function prototypes */

void Ferror(int i);

int get_measurements(char *store, char *store2, float *pow, float *ang, int *count,
		     int *ktime, int *iadd, int *nrec);

void get_updates(float pow, float ang, int count, int *fill_array,
		 float *response_array, int rec);

void stat_updates(float pow, float ang, int count, int *fill_array,
		  float *response_array);

void compute_ave(float pow, int count, int fill_array[],
		 float response_array[]);

void filter(float *val, int size, int opt, int nsx, int nsy, float
	    *temp, float thres);


/* global array variables used for storing images*/

float *a_val, *b_val, *a_temp, *sxy, *sx, *sx2, *sy, *tot;


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

  float latl, lonl, lath, lonh;
  char regname[11];
  int nls[MAXFILES], nrec, ncnt, nbyte, i, n, ii, iii, nsize;
  float ratio;
  char *space, *store, *store2;
  float pow, ang=0.0, azang;
  int count, ktime, iadd;
  int irecords;
  int non_size_x, non_size_y, nsx2, nsy2, ix, iy, nsize2;
  float xdeg2, ydeg2, ascale2, bscale2, a02, b02, gsize;
  char fname[180];

  char a_name[100], b_name[100], v_name[100], a_name_ave[100];
  char grd_aname[100], grd_vname[100], non_aname[100];

/* SIR file header information */

  float v_min_A, v_max_A, anodata_A, anodata_B;
  int nsx, nsy, ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin;
  int iregion, itype_A, nhead, ndes, nhtype, idatatype, ldes, nia;
  int ioff_V, iscale_V, itype_V;
  float anodata_V, v_min_V, v_max_V;
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

  time_t tod;

  int storage = 2;
  int errors = 0;
  char title1[101];

  int median_flag = 0;  /* default: no median filter in SIRF algorithm */

/* begin program */  

  printf("BYU simplified SIR/SIRF program: C version 1.0\n");
  if (argc < 2) {
    printf("\nusage: %s setup_in storage_option noise_flag\n\n",argv[0]);
    printf(" input parameters:\n");
    printf("   setup_in        = input setup file\n");
    printf("   storage_option  = (0=mem only [def], 1=file only, 2=mem then file\n");
    printf("   noise_flag      = use (1=noisy [def], 0=noise-free) measurements\n");
    return(0);
  }

  file_in=argv[1];
  printf("Input file: '%s'\n",file_in);

  if (argc > 2) sscanf(argv[2],"%d",&storage);
  printf("Storage option %d: ",storage);
  if (storage == 1)
    printf(" File only\n");
  else if (storage == 2)
    printf(" Memory then File\n");
  else
    printf(" Memory only\n");

  if (argc > 3) sscanf(argv[3],"%d",&NOISY);
  if (NOISY)
    printf(" Using noisy measurements\n");
  else
    printf(" Using noise-free measurements\n");
  
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
    strcpy(a_name,"simA2.sir");
    strcpy(grd_aname,"simA2.grd");
    strcpy(grd_vname,"simV2.grd");
    strcpy(non_aname,"simA2.non");
    strcpy(a_name_ave,"simA2.ave");
    strcpy(v_name,"simV2.sir");
  } else {
    strcpy(a_name,"simA.sir");
    strcpy(grd_aname,"simA.grd");
    strcpy(grd_vname,"simV.grd");
    strcpy(non_aname,"simA.non");
    strcpy(a_name_ave,"simA.ave");
    strcpy(v_name,"simV.sir");
  }  
  strcpy(b_name,"true.sir");
  
  median_flag=0;
  
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

  /* with storage allocated, copy file into memory if selected */

  if (storage != 1) {   /* read measurement file into memory, 
			   storing only essential information  */

    nrec = 0;         /* number of meaurements in file */
    ncnt = 0;         /* number of useable measurements */
    nbyte = 0;        /* file size in bytes */
    store=space;      /* storage pointer */
  
    printf("Begin file copy into memory\n");

    while (fread(store, sizeof(char), 16, imf) == 16) {

      iadd  = *((int *)   (store+0));
      count = *((int *)   (store+4));
      pow   = *((float *) (store+8));
      azang = *((float *) (store+12));
      
      if (count > 5000)
	printf("*** Count error %d  record %d\n",count,nrec);

      /*
      printf("ncnt %d  %f %f  count %d %d\n",ncnt,pow,azang,count,iadd); 	
      if (ncnt > 99) goto label;
      */

      /* if measurement is "valid" keep it by indexing counters 
	 if not, new values will be stored over old values */

      store2=store+4;
      nbyte=nbyte+16;
      store=store+16;
      ncnt++;
      keep=1;

      /* read fill_array pixel indices */
      if (nbyte+count*4 < nspace) {
	if (fread(store, sizeof(char), 4*count, imf) != 4*count) {
	  fprintf(stdout," *** Error reading input file data at 111\n");
	  /* exit(-1); */
	  goto label;
	}

	if (keep == 1) {
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
	  nbyte=nbyte+count*4;
	  store=store+count*4;
	}
      } else {
	fprintf(stdout," *** out of storage space 2 *** %d %d %d\n",ncnt,nbyte,nspace);
	exit(-1);
      }

      /*      
      printf("ncnt %d  %f %f  count %d %d  %d %f\n",ncnt,pow,azang,count,iadd,
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
  strncpy(tag,"(c) 2003 BYU MERS Laboratory",40);
  strncpy(sensor,"Simulation",40);
  regname[9]='\0';

  if (median_flag == 1) 
    sprintf(title,"SIRF image of %s",regname);
  else
    sprintf(title,"SIR image of %s",regname);

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
  
  ioff_V=-1;
  iscale_V=500;
  itype_V=22;
  anodata_V=-1.00;
  v_min_V=0.0;
  v_max_V=10.0;
  sprintf(type_V,"TB STD  (%s)",a_name);

  old_amin=1.e25;
  old_amax=-1.e25;
  for (i=0; i < nsize; i++) {
    old_amin=min(old_amin, *(b_val+i));
    old_amax=max(old_amax, *(b_val+i));
  }
  printf("True min/max %f %f\n",old_amin,old_amax);
  v_min_A=old_amin;
  v_max_A=old_amax;

  printf("Writing true A file '%s'\n", b_name);
  ierr = write_sir3(b_name, b_val, &nhead, nhtype, 
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

/* Begin SIR/SIRF processing.  First initialize working arrays. */

  for (i=0; i < nsize; i++) {
    *(a_val+i) = a_init;
    *(b_val+i) = 0.0;
    *(a_temp+i) = 0.0;
    *(sx+i)  = 0.0;
    *(sy+i)  = 0.0;
    *(sxy+i) = 0.0;
    *(sx2+i) = 0.0;
    *(tot+i) = 0.0;
  }
  old_amin=a_init;
  old_amax=a_init;

  if (storage == 1) {  /* for file storage */
    for (ifile=0; ifile < nfiles; ifile++)
      fseek(infiles[ifile], head_len[ifile], REL_BEGIN);
    cur_file=0;
  }

  printf("SIRF parameters: A_init=%f N=%d\n",a_init,nits);
  printf("                 Offset=%f\n\n",meas_offset);

  /* for each iteration of SIRF */

  for (its=0; its < nits; its++) {

    printf("\nSIRF iteration %d %d\n",its+1,ncnt);

    if (storage == 1) {  /* file storage: rewind file and skip head */
      for (ifile=0; ifile < nfiles; ifile++)
	fseek(infiles[ifile], head_len[ifile], REL_BEGIN);
      cur_file=0;
    }

    /* for each measurement, accumulate results */

    store=space;
    for (irec = 0; irec < ncnt; irec++) {
    
      if (storage == 1) { /* get measurement info from file */
	store=space;
	if (get_measurements(store, store2, &pow, &ang, &count,
			     &ktime, &iadd, &nrec)) {
	  if (irec == 0) ncnt = nrec;  /* set count value */
	  goto done;
	}

      } else {  /* get measurement info from memory */

	if (NOISY)
	  pow   = *((float *) (store+12));
	else
	  pow   = *((float *) (store+8));

	count = *((int *)   (store+4));
	if (its == 0) iadd = *((int *) (store+0));
	
	store = store+16;
	store2 = store + 4*count;
      }
      /*      
      printf("%d %f %d  %d %f\n",irec,pow,count,
             *((int *)(store)),*((float *)(store2)) );
	     */
	/* for (j=0;j<count;j++)
	printf("%d %f\n",*((int *)(store+4*j)),*((int *)(store2+4*j)) );
	*/

      if (meas_offset != 0.0) pow = pow + meas_offset;

      /* printf("get_updates\n"); */
      
      get_updates(pow, ang, count, (int *) store,
		  (float *) store2, irec);

      /* compute AVE image during first iteration */

      if (its == 0) 
	compute_ave(pow, count,(int *) store, (float *) store2);

      store = store+4*count;
      store = store+4*count;

    }

done:
    /* after processing all measurements for this iteration and
       updating the A image, clear arrays */

    amin =  320.0;            /* for user stats */
    amax =  -320.0;
    tmax = -1;
    total = 0.0;
    
    for (i=0; i<nsize; i++){
      if (*(tot+i) > 0) {    /* update only hit pixels */
	total = total + *(tot+i);
	*(a_val+i) = *(a_temp+i);

	if (its+1 != nits) {          /* clean up */
	  *(a_temp+i) = 0.0;
	  *(sx+i) = 0.0;
	  *(sx2+i) = 0.0;
	  *(tot+i) = 0.0; 
	} else {                      /* last iteration */
	  *(sx2+i) = *(sx2+i) - *(sx+i) * *(sx+i);
	  if (*(sx2+i) > 0.0) *(sx2+i) = sqrt((double) *(sx2+i));
	  *(sxy+i) = *(tot+i);
	  tmax = max(tmax, *(tot+i));
	}
	if (its == 0) {        /* first iteration */
	  if (*(sy+i) > 0) 
	    *(b_val+i) = *(b_val+i) / *(sy+i);
	  else
	    *(b_val+i) = anodata_A;
	}
	amin = min(amin, *(a_val+i));
	amax = max(amax, *(a_val+i));
	
      } else {
	*(a_val+i) = anodata_A;
	*(b_val+i) = anodata_A;
	if (its+1 == nits) {
	  *(sx2+i) = -1;
	  *(sxy+i) = -1;
	}
      }	
    }

    if (its == 0) printf("Average hits: %.4f\n",total/nsize);
    printf(" A min   max --> %f %f %d\n",amin,amax,its+1);
    printf(" A change    --> %f %f\n",amin-old_amin,amax-old_amax);

    old_amin=amin;
    old_amax=amax;


    if (median_flag == 1)   /* apply modified median filtering */
      filter(a_val, 3, 0, nsx, nsy, a_temp, anodata_A);  /* 3x3 modified median filter */

    if (its == 0) {  /* output AVE image */
      
      sprintf(title1, "AVE image of %s",regname);
      sprintf(crproc,"BYU MERS:sea_meta_sirf_egg v4.1 AVE image");

      if (meas_offset != 0.0) {   /* shift A image before save */
	for (i=0; i<nsize; i++)
	  if (*(b_val+i) > anodata_A)    /* update only hit pixels */
	    *(b_val+i) = *(b_val+i) - meas_offset;
      }

      printf("	Writing A output AVE file '%s'\n", a_name_ave);
      ierr = write_sir3(a_name_ave, b_val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, 
			ia0_off, ib0_off, i0_sc,
			ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
			anodata_A, v_min_A, v_max_A, sensor, title1, type_A, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
      if (ierr < 0) {
	fprintf(stdout,"*** ERROR writing A AVE output file ***\n");
	errors++;
      }
    }

    /* output A files during iterations */

    if ( ((its+1)% 5) == 0 || its+1 == nits) {

      if (meas_offset != 0.0) {   /* shift A image before save */
	for (i=0; i<nsize; i++)
	  if (*(a_val+i) > anodata_A)    /* update only hit pixels */
	    *(a_val+i) = *(a_val+i) - meas_offset;
      }

      sprintf(crproc,"BYU MERS:meta_sirf_sea_egg v4.1 Ai=%6.2f It=%d",a_init,its+1);

      printf("\n");      
      printf("Writing A output SIR file '%s'\n", a_name);
      ierr = write_sir3(a_name, a_val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, 
			ia0_off, ib0_off, i0_sc,
			ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
			anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
      if (ierr < 0) {
	fprintf(stdout,"*** ERROR writing A output file ***\n");
	errors++;
      }

      if (meas_offset != 0.0 && its+1 != nits) {   /* shift A image back */
	for (i=0; i<nsize; i++)
	  if (*(a_val+i) > anodata_A)    /* update only hit pixels */
	    *(a_val+i) = *(a_val+i) + meas_offset;

      }
    }
  }


/* create STD and Err images */

  printf("\nBegin creation of STD images\n");  

  /* initialize arrays */

  for (i=0; i < nsize; i++) {
    *(a_temp+i) = 0.0;
    *(sx+i)  = 0.0;
    *(sy+i)  = 0.0;
    *(sxy+i) = 0.0;
    *(sx2+i) = 0.0;
    *(tot+i) = 0.0;
  }

  if (storage == 1) {  /* file storage: rewind file and skip head */
    for (ifile=0; ifile < nfiles; ifile++)
      fseek(infiles[ifile], head_len[ifile], REL_BEGIN);
    cur_file=0;
  }

  store=space;
  for (irec = 0; irec < ncnt; irec++) {

    if (storage == 1) { /* get measurement info from file */

      store=space;
      if (get_measurements(store, store2, &pow, &ang, &count,
			   &ktime, &iadd, &nrec)) goto done1;
      
    } else {  /* get measurement info from memory */

      if (NOISY)
	pow   = *((float *) (store+12));
      else
	pow   = *((float *) (store+8));

      count = *((int *)   (store+4));
      if (its == 0) iadd = *((int *) (store+0));
      
      store = store+16;
      store2 = store+4*count;
    }

    stat_updates(pow, ang, count, (int *) store, (float *) store2);
    store = store+4*count;
    store = store+4*count;

  }
done1:

  amin =  32000.0;            /* for user stats */
  amax = -32000.0;
  bmin =  300.0;
  bmax = -300.0;
  total = 0.0;
    
  for (i=0; i<nsize; i++){
    if (*(tot+i) > 0) {    /* update only hit pixels */
      total = total + *(tot+i);
      *(sx+i) = *(sx+i) / *(tot+i);
      *(sy+i) = *(sy+i) / *(tot+i) - *(sx+i) * *(sx+i);
      
      if (*(sy+i) > 0.0)
	*(sxy+i) = sqrt( (double) *(sy+i));
      else
	*(sxy+i) = 0.0;
		
      amin = min(amin, *(sxy+i));
      amax = max(amax, *(sxy+i));
	
      } else {
	*(sxy+i) = anodata_V;
	*(sx+i) = 0.0;
      }
  }
    
  printf(" TB STD min   max --> %f %f\n",amin,amax);

  sprintf(title,"TB STD image of %s",regname);
  printf("Writing STD (V) output SIR file '%s'\n", v_name);
  ierr = write_sir3(v_name, sxy, &nhead, nhtype, 
		    idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
		    ixdeg_off, iydeg_off, ideg_sc, iscale_sc, 
		    ia0_off, ib0_off, i0_sc,
		    ioff_V, iscale_V, iyear, isday, ismin, ieday, iemin, 
		    iregion, itype_V, iopt, ipol, ifreqhm, ispare1,
		    anodata_V, v_min_V, v_max_V, sensor, title, type_V, tag,
		    crproc, crtime, descrip, ldes, iaopt, nia);
  if (ierr < 0) {
    fprintf(stdout,"*** ERROR writing STD (V) output file ***\n");
    fprintf(stderr,"*** ERROR writing STD (V) output file ***\n");
    errors++;
  }


/* create non-enhanced images */

  printf("\nBegin creation of non-enhanced GRD images\n");

  /* note that this should be the case: 
     nsx2 = nsx/non_size_x;
     nsy2 = nsy/non_size_y; */

  nsize2=nsx2*nsy2;
  printf("Non-enhanced sizes: %d %d %d\n",nsx2,nsy2,nsize2);
  
  /* initialize arrays */

  for (i=0; i < nsize2; i++) {
    *(a_val+i) = 0.0;
    *(a_temp+i) = 0.0;
    *(tot+i) = 0;
    *(sx+i)  = 0.0;
    *(sy+i)  = 0.0;
    *(sxy+i) = 0.0;
    *(sx2+i) = 0.0;
  }

  if (storage == 1) {  /* file storage: rewind file and skip head */
    for (ifile=0; ifile < nfiles; ifile++)
      fseek(infiles[ifile], head_len[ifile], REL_BEGIN);
    cur_file=0;
  }

  store=space;
  for (irec = 0; irec < ncnt; irec++) {

    if (storage == 1) { /* get measurement info from file */

      store=space;
      if (get_measurements(store, store2, &pow, &ang, &count,
			   &ktime, &iadd, &nrec)) goto done3;
      
    } else {  /* get measurement info from memory */

      if (NOISY)
	pow   = *((float *) (store+12));
      else
	pow   = *((float *) (store+8));

      ang   = *((float *) (store+12));
      count = *((int *)   (store+4));
      iadd  = *((int *)   (store+0));
      store = store+16;
    }
    store = store+4*count;
    store = store+4*count;

    /* compute address in shrunk workspace */

    if (iadd < 0) iadd=-iadd;  /* remove asc/des flag */
    if (iadd > 0) {            /* skip out-of-area measurements */

      /* compute location of measurement within lo-res grid */

      ix = iadd % nsx;
      iy = iadd / nsx;
      ix = floor((float) ix / (float) non_size_x);
      iy = floor((float) iy / (float) non_size_y);
      iadd = nsx2*iy+ix; 
      /*
      printf("Non-enhanced: %f %d  %d %d %d %d %d\n",pow, irec,
	     iadd,ix,iy,non_size_x,non_size_y);
      */
    /* compute average and variance */

      if (iadd >= nsx2*nsy2 || iadd < 0) {  /* keep only in-image
					      measurements */
	/*
	printf("*** Non-enhanced address error: %d %d %d %d %d\n",
	       iadd,ix,iy,non_size_x,non_size_y);
	*/
      } else {
	n = *(tot + iadd);
	*(tot +  iadd) = *(tot +   iadd) + 1;
	*(sy +   iadd) = (*(sy +   iadd) * n + pow)/(n+1);	
	*(a_val+ iadd) = (*(a_val+ iadd) * n + pow*pow)/(n+1);	

      }
    }
  }
done3:

  amin =  32000.0;            /* for user stats */
  amax = -32000.0;
  bmin =  32000.0;
  bmax = -32000.0;
  old_bmax = -3200.0;
  old_bmin =  3200.0;
  tmax = -1;
  total = 0.0;
    
  for (i=0; i<nsize2; i++){
    if (*(tot+i) > 0) {    /* update only hit pixels */
      total = total + *(tot+i);

      temp =  *(a_val + i) - *(sy + i) * *(sy + i);
      if (temp > 0.0) {
	temp = sqrt((double) temp);
	old_bmin = min(old_bmin, temp);
	old_bmax = max(old_bmax, temp);
      } else
	temp = anodata_V;

      *(a_val+i) = *(sy+i);
      if (*(tot+i) > 1) {
	denom = *(sx2+i) - (*(sx+i) * *(sx+i));
	if (denom > 0.0) {
	  *(sx2+i) = sqrt(denom);
	}
      } else {
	  *(sx2+i) = 0;
      }

      if (*(a_val+i) >  300.0) *(a_val+i) =  300.0;
      if (*(a_val+i) < 150.0) *(a_val+i) = 150.0;

      amin = min(amin, *(a_val+i));
      amax = max(amax, *(a_val+i));

      *(sxy+i) = temp;
      tmax = max(tmax, *(tot+i));
      *(sy+i) = *(tot+i);
	
    } else {
      *(a_val+i) = anodata_A;
      *(sxy+i) = anodata_V;
    }
  }

  printf(" Non-enhanced/Grid A  min   max --> %f %f\n",amin,amax);
  printf(" Non-enhanced/Grid V  min   max --> %f %f\n",old_bmin,old_bmax);
  printf(" Non-enhanced/Grid C  min   max --> %f %f\n",0.,tmax);

  if (old_bmax < 1.0)
    v_max_V = 1;

  sprintf(title,"Grid image of %s",regname);
  printf("Writing grid A output SIR file '%s'\n", grd_aname);
  ierr = write_sir3(grd_aname, a_val, &nhead, nhtype, 
		    idatatype, nsx2, nsy2, xdeg2, ydeg2, 
		    ascale2, bscale2, a02, b02, 
		    ixdeg_off, iydeg_off, ideg_sc, iscale_sc, 
		    ia0_off, ib0_off, i0_sc,
		    ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
		    iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
		    anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
		    crproc, crtime, descrip, ldes, iaopt, nia);
  if (ierr < 0) {
    fprintf(stdout,"*** ERROR writing grid output A file ***\n");
    errors++;
  }

  sprintf(title,"Grid TB STD image of %s",regname);
  printf("Writing Grid STD (V) output SIR file '%s'\n", grd_vname);
  ierr = write_sir3(grd_vname, sxy, &nhead, nhtype, 
		    idatatype, nsx2, nsy2, xdeg2, ydeg2, 
		    ascale2, bscale2, a02, b02, 
		    ixdeg_off, iydeg_off, ideg_sc, iscale_sc, 
		    ia0_off, ib0_off, i0_sc,
		    ioff_V, iscale_V, iyear, isday, ismin, ieday, iemin, 
		    iregion, itype_V, iopt, ipol, ifreqhm, ispare1,
		    anodata_V, v_min_V, v_max_V, sensor, title, type_V, tag,
		    crproc, crtime, descrip, ldes, iaopt, nia);
  if (ierr < 0) {
    fprintf(stdout,"*** ERROR writing Grid STD (V) STD output file ***\n");
    errors++;
  }

  /* create images with same pixel sizes as enhanced resolution images
     using grid image data */


  for (i=0; i < nsize; i++) {
    *(sx + i) = anodata_A;
    *(sy + i) = anodata_V;
  }
  
  for (i=0; i < nsize2; i++) {
    ix = (i % nsx2) * non_size_x;
    iy = (i / nsx2) * non_size_y;

    for (ii=0; ii < non_size_y; ii++)
      for (iii=0; iii < non_size_x; iii++) {
	iadd = nsx*(iy+ii)+ix+iii;
	*(sx + iadd) = *(a_val + i);
	*(sy + iadd) = *(sxy + i);
      }
  }
  
  sprintf(title,"Non-enhanced image of %s",regname);
  printf("Writing Non-enhanced A output SIR file '%s'\n", non_aname);
  ierr = write_sir3(non_aname, sx, &nhead, nhtype, 
		    idatatype, nsx, nsy, xdeg, ydeg, 
		    ascale, bscale, a0, b0, 
		    ixdeg_off, iydeg_off, ideg_sc, iscale_sc, 
		    ia0_off, ib0_off, i0_sc,
		    ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
		    iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
		    anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
		    crproc, crtime, descrip, ldes, iaopt, nia);
  if (ierr < 0) {
    fprintf(stdout,"*** ERROR writing nonenhanced output A file ***\n");
    errors++;
  }


  
/* end of program */

  return(errors);
}
      
	

/* SIR algorithm update step */

void get_updates(float power, float ang, int count, int fill_array[],
		 float response_array[], int rec)
{
  float total = 0.0, num=0.0;
  float ave, scale, update, test;
  int i, n;
  float m;

  /* compute back projection */ 
  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    num=num+m;
    total=total + m * *(a_val+n-1);
  }
  if (num > 0)
    ave=(total/num);
  else
    return;

  if (ave == 0.0) {
    printf("No update\n");
    return;
  }
  
  /* for each measurement hitting a pixel calculate updates */

  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    scale = power / ave;
    if (scale > 0.0)       /* damp scale factor */
      scale=(float) sqrt((double) scale);
    else                   /* unless scale is negative */
      scale=1.0;
    if (scale > 1.0)       /* constraining function */
      update = 1.0/((0.5/ave)*(1.0-1.0/scale)+1.0/(*(a_val+n-1) * scale));
    else
      update = 0.5 * ave * (1.0 - scale) + *(a_val+n-1) * scale;

    (*(tot+n-1)) = (*(tot+n-1)) + m;
    test=(*(a_temp+n-1) * ( *(tot+n-1) - m) + update * m) / *(tot+n-1);
    *(a_temp+n-1) = test; 
    
    *(sx +n-1) =( *(sx +n-1) * ( *(tot+n-1) - m)+m*ang)    / *(tot+n-1); 
    *(sx2+n-1) =( *(sx2+n-1) * ( *(tot+n-1) - m)+m*ang*ang)/ *(tot+n-1);
  }
  return;
}


/* compute contribution of measurement to AVE image */ 

void compute_ave(float pow, int count, int fill_array[],
		 float response_array[])
{
  int i, n;
  float m;

  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    *(b_val+n-1) = *(b_val+n-1) + m * pow;
    *(sy+n-1) = *(sy+n-1) + m;
  }
  return;
}
 


/* modified median or smoothing filter routine */

float median(float *array, int count);

void filter(float *val, int size, int mode, int nsx, int nsy, 
	    float *temp, float thres)
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



/* routine to compute variance and error from measurements */

void stat_updates(float power, float ang, int count, int fill_array[],
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
    /* total = total + m * (float) pow( (double) 10.0, (double) (sigv * 0.1)); */
    total = total + m * sigv;
    num = num + m;
  }
  if (num == 0) return;

  ave = 10.0 * (total/num);

  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    *(tot+n-1) = *(tot+n-1) + m;
    sigv = (power - ave);      /* difference */
    *(sx+n-1) = *(sx+n-1) + m * sigv;
    *(sy+n-1) = *(sy+n-1) + m * sigv * sigv;
  }

  return;
}


void Ferror(int i)
{
  fprintf(stdout,"*** Error reading input file at %d ***\n",i);
  fflush(stdout);
  return;
  /*  
  exit(-1);
  */
}

void Ferror2(int *error, int i)
{
  fprintf(stdout,"*** Error reading input file at %d ***\n",i);
  fflush(stdout);

  *error = -1;  
  return;
}


int get_measurements(char *store, char *store2, float *pow, float *ang, int *count,
		     int *ktime, int *iadd, int *nrec)
{  /* returns the next set of measurement from the file */
  int flag=1, err;

  while (flag == 1) {
    if (fread(store, sizeof(char), 20,imf) != 20) {	
      Ferror2(&err,200);
      return(1);      
    } else {

      if (NOISY)
	*pow   = *((float *) (store+12));
      else
	*pow   = *((float *) (store+8));

      *count = *((int *)   (store+4));
      *iadd  = *((int *)   (store+0));

      /*
      printf("record %d %f %f %d %d %d\n",*nrec,*pow,*ang,*count,*ktime,*iadd);
      */

      /* read fill_array pixel indices */
      if (*count * 4 < nspace) {
	if (fread(store, sizeof(char), 4 * *count, imf) != 4 * *count) Ferror2(&err,211);
      } else {
	fprintf(stdout," *** fill_array storage error 3 *** %d %d\n",*nrec,nspace);
	exit(-1);
      }

      /* read response_array values */
      if (*count * 4 < nspace) {
	if (fread(store2, sizeof(char), 4 * *count, imf) != 4 * *count) Ferror2(&err,2111);
      } else {
	fprintf(stdout," *** fill_array storage error 4 *** %d %d\n",*nrec,nspace);
	exit(-1);
      }
      (*nrec)++;
	
       return(0);
    }
  }  
  return(-1);
}

