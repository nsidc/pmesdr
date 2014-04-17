/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_sim_setup.c    MEaSURES project

  program to generate synthetic data in simulated .setup file 

  Written by DGL at BYU 03/24/2014 

******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "sir3.h"
#include "sir_ez.h" 

#define VERSION 0.0

#define file_savings 1.00     /* measurement file savings ratio */
#define REL_EOF   2           /* fseek relative to end of file */

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define rnd(a) ((a) >= 0 ? floor((a)+0.5L) : ceil((a)-0.5L))

/****************************************************************************/

/* some variables and their default values */

char  sensor_in[40];          /* sensor description string */
int   MAXFILL=1000;           /* maximum number of pixels in response */
int   HASAZANG=0;             /* azimuth angle data not included */
int   HS=20;                  /* measurement headersize in bytes */

/****************************************************************************/

void eprintf(char *s)
{ /* print to both stdout and stderr to catch errors */
  fprintf(stdout,s);
  fflush(stdout);
  fprintf(stderr,s);
  fflush(stderr);
}

void eprintfi(char *s, int a)
{ /* print to both stdout and stderr to catch errors */
  fprintf(stdout,s,a);
  fflush(stdout);
  fprintf(stderr,s,a);
  fflush(stderr);
}
void eprintfc(char *s, char *a)
{ /* print to both stdout and stderr to catch errors */
  fprintf(stdout,s,a);
  fflush(stdout);
  fprintf(stderr,s,a);
  fflush(stderr);
}


/* function prototypes */

void Ferror(int i);
void dFerror(int i);
void rFerror(int i, int j);

float gauss(void);

int get_measurements(char *store, int *locs, short int *resp, float *tbval, float *ang, float *azang, int *count,
		     int *ktime, int *iadd, int *nrec, FILE *imf);

int put_measurements(char *store, int *locs, short int *resp, float tbval, float ang, float azang, int count,
		     int ktime, int iadd, FILE *omf);

void no_trailing_blanks(char *s);

char *addpath(char *outpath, char *name, char *temp);

/****************************************************************************/

/* main program */

int main(int argc, char **argv)
{

  char *sir_in, *file_in, *outfile;
  sir_head head;       /* sir_ez defined structure to store SIR header info */
  float *a_val;  
  int option, ierr, k, n; 
  int ADDNOISE=0;
  float noiseSTD=1.0;  
  int nsx, nsy, isday, ieday, ismin, iemin, iyear, iregion, iopt, ipol;
  char store[100], line[101], *x;  
  FILE *imf, *omf;
  float latl, lonl, lath, lonh;
  char regname[11];
  int dumb1, dumb2, nrec, ncnt, nsize;
  short int resp[4096], thres=200;
  int locs[4096];  
  float tbval, ang, azang=0.0;
  int count, ktime, iadd, end_flag;
  int irecords;
  int non_size_x, non_size_y, nsx2, nsy2, ix, iy;
  float ascale, bscale, a0, b0, xdeg, ydeg;
  float xdeg2, ydeg2, ascale2, bscale2, a02, b02;
  double sum, w;

  /* begin program */

  printf("BYU meta_sim_setup program: C version %f\n",VERSION);

  if (argc < 2) {
    printf("\nusage: %s opt setup_in true_image outfile\n\n",argv[0]);
    printf(" input parameters:\n");
    printf("   setup_in        = input setup file\n");
    printf("   true_image      = input sir file\n");
    printf("   outfile         = output file\n");
    return(0);
  }

  /* program option */
  sscanf(argv[1],"%d",&option);

  /* file names */
  file_in=argv[2];
  sir_in=argv[3];
  outfile=argv[4];

  printf("Input setup file %s:\n",file_in);
  printf("Input truth file %s:\n",sir_in);
  printf("Output file %s:     \n",outfile);


  /* open all input files */

 /* initialize sir_ez structure */
  sir_init_head(&head);
  
  /* read in input SIR file */
  ierr = get_sir(sir_in, &head, &a_val);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR header from file '%s'\n",sir_in);
     exit(-1);
  }
  /* for compatibility with old versions of sir files, update header */
  sir_update_head(&head, a_val);


  /* open input setup file */
  imf = fopen(file_in,"r");
  if (imf == NULL) {
     eprintfc("ERROR: cannot open input setup file: %s\n",file_in); 
     exit(-1);
  }

  /* open input setup file */
  omf = fopen(outfile,"w");
  if (omf == NULL) {
     eprintfc("ERROR: cannot open output setup file: %s\n",outfile); 
     exit(-1);
  }

  /* copy input file header to output file header */
  
  /* read setup file header info */
   if (fread(&dumb1,   sizeof(int)  , 1, imf) == 0) Ferror(0);  /* record header */
   if (fread(&irecords,sizeof(int)  , 1, imf) == 0) Ferror(1);  /* number of measurement records in file */
   if (fread(&dumb2,   sizeof(int)  , 1, imf) == 0) Ferror(2);  /* record trailer */

   if (fwrite(&dumb1,   sizeof(int)  , 1, omf) == 0) dFerror(0);  /* record header */
   if (fwrite(&irecords,sizeof(int)  , 1, omf) == 0) dFerror(1);  /* number of measurement records in file */
   if (fwrite(&dumb2,   sizeof(int)  , 1, omf) == 0) dFerror(2);  /* record trailer */


   if (fread(&dumb1, sizeof(int)  , 1, imf) == 0) Ferror(3);  /* record header */
   if (fread(&nsx,   sizeof(int)  , 1, imf) == 0) Ferror(4);
   if (fread(&nsy,   sizeof(int)  , 1, imf) == 0) Ferror(5);
   if (fread(&ascale,sizeof(float), 1, imf) == 0) Ferror(6);
   if (fread(&bscale,sizeof(float), 1, imf) == 0) Ferror(7);
   if (fread(&a0,    sizeof(float), 1, imf) == 0) Ferror(8);
   if (fread(&b0,    sizeof(float), 1, imf) == 0) Ferror(9);
   if (fread(&xdeg,  sizeof(float), 1, imf) == 0) Ferror(10);
   if (fread(&ydeg,  sizeof(float), 1, imf) == 0) Ferror(11);
   if (fread(&dumb2, sizeof(int)  , 1, imf) == 0) Ferror(12); /* record trailer */

   if (fwrite(&dumb1, sizeof(int)  , 1, omf) == 0) dFerror(3);  /* record header */
   if (fwrite(&nsx,   sizeof(int)  , 1, omf) == 0) dFerror(4);
   if (fwrite(&nsy,   sizeof(int)  , 1, omf) == 0) dFerror(5);
   if (fwrite(&ascale,sizeof(float), 1, omf) == 0) dFerror(6);
   if (fwrite(&bscale,sizeof(float), 1, omf) == 0) dFerror(7);
   if (fwrite(&a0,    sizeof(float), 1, omf) == 0) dFerror(8);
   if (fwrite(&b0,    sizeof(float), 1, omf) == 0) dFerror(9);
   if (fwrite(&xdeg,  sizeof(float), 1, omf) == 0) dFerror(10);
   if (fwrite(&ydeg,  sizeof(float), 1, omf) == 0) dFerror(11);
   if (fwrite(&dumb2, sizeof(int)  , 1, omf) == 0) dFerror(12); /* record trailer */


   if (fread(&dumb1,  sizeof(int),   1, imf) == 0) Ferror(21);
   if (fread(&isday,  sizeof(int),   1, imf) == 0) Ferror(22);
   if (fread(&ieday,  sizeof(int),   1, imf) == 0) Ferror(23);
   if (fread(&ismin,  sizeof(int),   1, imf) == 0) Ferror(24);
   if (fread(&iemin,  sizeof(int),   1, imf) == 0) Ferror(25);
   if (fread(&iyear,  sizeof(int),   1, imf) == 0) Ferror(26);
   if (fread(&iregion,sizeof(int),   1, imf) == 0) Ferror(27);
   if (fread(&iopt ,  sizeof(int),   1, imf) == 0) Ferror(28);
   if (fread(&ipol,   sizeof(int),   1, imf) == 0) Ferror(29);
   if (fread(&latl,   sizeof(float), 1, imf) == 0) Ferror(30);
   if (fread(&lonl,   sizeof(float), 1, imf) == 0) Ferror(31);
   if (fread(&lath,   sizeof(float), 1, imf) == 0) Ferror(32);
   if (fread(&lonh,   sizeof(float), 1, imf) == 0) Ferror(33);
   if (fread(regname, sizeof(char), 10, imf) == 0) Ferror(34);
   regname[10]='\0';
   if (fread(&dumb2,  sizeof(int),   1, imf) == 0) Ferror(35);

   if (fwrite(&dumb1,  sizeof(int),   1, omf) == 0) dFerror(21);
   if (fwrite(&isday,  sizeof(int),   1, omf) == 0) dFerror(22);
   if (fwrite(&ieday,  sizeof(int),   1, omf) == 0) dFerror(23);
   if (fwrite(&ismin,  sizeof(int),   1, omf) == 0) dFerror(24);
   if (fwrite(&iemin,  sizeof(int),   1, omf) == 0) dFerror(25);
   if (fwrite(&iyear,  sizeof(int),   1, omf) == 0) dFerror(26);
   if (fwrite(&iregion,sizeof(int),   1, omf) == 0) dFerror(27);
   if (fwrite(&iopt ,  sizeof(int),   1, omf) == 0) dFerror(28);
   if (fwrite(&ipol,   sizeof(int),   1, omf) == 0) dFerror(29);
   if (fwrite(&latl,   sizeof(float), 1, omf) == 0) dFerror(30);
   if (fwrite(&lonl,   sizeof(float), 1, omf) == 0) dFerror(31);
   if (fwrite(&lath,   sizeof(float), 1, omf) == 0) dFerror(32);
   if (fwrite(&lonh,   sizeof(float), 1, omf) == 0) dFerror(33);
   if (fwrite(regname, sizeof(char), 10, omf) == 0) dFerror(34);
   regname[10]='\0';
   if (fwrite(&dumb2,  sizeof(int),   1, omf) == 0) dFerror(35);


   if (fread(&dumb1,  sizeof(int)  , 1, imf) == 0) Ferror(43);/* record header */
   if (fread(&nsx2,   sizeof(int)  , 1, imf) == 0) Ferror(44);
   if (fread(&nsy2,   sizeof(int)  , 1, imf) == 0) Ferror(45);
   if (fread(&non_size_x,sizeof(int)  , 1, imf) == 0) Ferror(46);
   if (fread(&non_size_y,sizeof(int)  , 1, imf) == 0) Ferror(47);
   if (fread(&ascale2,sizeof(float), 1, imf) == 0) Ferror(48);
   if (fread(&bscale2,sizeof(float), 1, imf) == 0) Ferror(49);
   if (fread(&a02,    sizeof(float), 1, imf) == 0) Ferror(50);
   if (fread(&b02,    sizeof(float), 1, imf) == 0) Ferror(51);
   if (fread(&xdeg2,  sizeof(float), 1, imf) == 0) Ferror(52);
   if (fread(&ydeg2,  sizeof(float), 1, imf) == 0) Ferror(53);
   if (fread(&dumb2,  sizeof(int)  , 1, imf) == 0) Ferror(54);/* record trailer */

   if (fwrite(&dumb1,  sizeof(int)  , 1, omf) == 0) dFerror(43);/* record header */
   if (fwrite(&nsx2,   sizeof(int)  , 1, omf) == 0) dFerror(44);
   if (fwrite(&nsy2,   sizeof(int)  , 1, omf) == 0) dFerror(45);
   if (fwrite(&non_size_x,sizeof(int)  , 1, omf) == 0) dFerror(46);
   if (fwrite(&non_size_y,sizeof(int)  , 1, omf) == 0) dFerror(47);
   if (fwrite(&ascale2,sizeof(float), 1, omf) == 0) dFerror(48);
   if (fwrite(&bscale2,sizeof(float), 1, omf) == 0) dFerror(49);
   if (fwrite(&a02,    sizeof(float), 1, omf) == 0) dFerror(50);
   if (fwrite(&b02,    sizeof(float), 1, omf) == 0) dFerror(51);
   if (fwrite(&xdeg2,  sizeof(float), 1, omf) == 0) dFerror(52);
   if (fwrite(&ydeg2,  sizeof(float), 1, omf) == 0) dFerror(53);
   if (fwrite(&dumb2,  sizeof(int)  , 1, omf) == 0) dFerror(54);/* record trailer */

   /* file header read completed, summarize */
   printf("\nInput file header info: '%s'\n",file_in);
   printf("  Year, day range: %d %d - %d\n",iyear,isday,ieday);
   printf("  Image size: %d x %d = %d   Projection: %d\n",nsx,nsy,nsx*nsy,iopt);
   printf("  Origin: %f,%f  Span: %f,%f\n",a0,b0,xdeg,ydeg);
   printf("  Scales: %f,%f  Pol: %d  Reg: %d\n",ascale,bscale,ipol,iregion);
   printf("  Region: '%s'   Records: %d\n",regname,irecords);
   printf("  Corners: LL %f,%f UR %f,%f\n",latl,lonl,lath,lonh);
   printf("  Grid size: %d x %d = %d  Scales %d %d\n",nsx2,nsy2,nsx2*nsy2,non_size_x,non_size_y);
   printf("  Grid Origin: %f,%f  Grid Span: %f,%f\n",a02,b02,xdeg2,ydeg2);
   printf("  Grid Scales: %f,%f\n",ascale2,bscale2);
   printf("\n");
   fflush(stdout);

   /* now read and copy output file names and misc variables */

   end_flag = 0;
   do {     
     if (fread(&dumb1,  sizeof(int),   1, imf) == 0) Ferror(70);
     if (fread(line,   sizeof(char), 100, imf) == 0) Ferror(71);
     if (fread(&dumb2,  sizeof(int),   1, imf) == 0) Ferror(72);
     /* printf("line read '%s'\n",line); */

     if (fwrite(&dumb1,  sizeof(int),   1, omf) == 0) dFerror(70);
     if (fwrite(line,   sizeof(char), 100, omf) == 0) dFerror(71);
     if (fwrite(&dumb2,  sizeof(int),   1, omf) == 0) dFerror(72);

     if (strstr(line,"Has_Azimuth_Angle") != NULL) {
       x = strchr(line,'=')+1;
       if (strstr(x,"T") != NULL || strstr(x,"t") != NULL) {
	 HASAZANG=1;
	 HS += 4;  /* increase read buffer size */
       }
       if (strstr(x,"F") != NULL || strstr(x,"f") != NULL)
	 HASAZANG=0;
       /* printf("Has azimuth angle: %d\n",HASAZANG); */
     }
      
     if (strstr(line,"End_header") != NULL) {
       end_flag = 1;
     }
   } while (end_flag == 0);

   printf("\n");

   nsize = nsx * nsy;
   nrec = 0;         /* number of meaurements in file */
   ncnt = 0;         /* number of useable measurements */
  
   printf("Begin processing setup file\n");
   while (fread(&dumb1, sizeof(int), 1, imf) != 0) {
     if (get_measurements(store, locs, resp, &tbval, &ang, &azang, &count,
			  &ktime, &iadd, &nrec, imf)) goto label;
     nrec++;

     if (option == -1) { /* straight pass-thru for debugging */
       printf("measurement %d: %f %f %d %d %d\n", nrec, tbval, ang, count, ktime, iadd);
     } else { /* use input true image to generate synthetic measurements */
       if (option == 0) { /* use response function unmodified */
	 ;  /* do nothing */
       } else if (option == 1) { /* use response function unmodified plus noise */
	 ADDNOISE=1;	 
       } else if (option == 2) { /* threshold response function */
	 for (k=0; k<count; k++)
	   if (resp[k] < thres)
	     resp[k]=0;
       } else if (option == 3) { /* etc */
       } else
	 printf("*** Invalid option \n");


       /* generate synthetic measurement from truth image and the locations and response arrays 
	  Note that the response array values are short ints in the range  0..1000*/
       if (iadd > 0 && iadd <= nsize) {
	 sum=0.0; w=0.0; n=0;	 
	 for (k=0; k<count; k++)
	   if (a_val[locs[k]] > head.anodata+0.001 && resp[k] > 0) {	       
	     sum = sum + a_val[locs[k]] * (double) resp[k];
	     w = w + (double) resp[k];
	     n++;	     
	   }
	 if (w>0.0) {	     
	   sum=sum/w;
	   /* optionally add noise */
	   if (ADDNOISE)
	     sum=sum+gauss()*noiseSTD;
	 }
	 tbval=sum;
	 if (n != count) count=n;	 
       }       

       /* save simulated measurements */
       if (count > 0) {	   
	 put_measurements(store, locs, resp, tbval, ang, azang, count, ktime, iadd, omf);
	 ncnt++;
       }
     }
   }

label:
   fclose(imf);
   fclose(omf);

   printf("Input measurements %d\n",nrec);
   printf("Output measurements %d\n",ncnt);   

  /* end of program */
  printf("All done\n");  
}



void Ferror(int i)
{
  fprintf(stdout,"*** Error reading input file at %d ***\n",i);
  fflush(stdout);
  return;
}

void dFerror(int i)
{
  fprintf(stdout,"*** Error writing output file at %d ***\n",i);
  fflush(stdout);
  exit(-1);
  return;
}

void rFerror(int i, int r)
{
  fprintf(stdout,"*** Error %d reading input file at record %d ***\n",i,r);
  fflush(stdout);
  exit(-1);
  return;
}



int get_measurements(char *store, int *locs, short int *resp, float *tbval, float *ang, float *azang, int *count,
		     int *ktime, int *iadd, int *nrec, FILE *imf)
{  /* returns the next set of measurement from the file */
  int dumb;

  /* read values */
  if (fread(store, sizeof(char), HS, imf) != HS) rFerror(200,*nrec);
  if (fread(&dumb, sizeof(int),   1, imf) == 0 ) rFerror(201,*nrec);

  *tbval = *((float *) (store+0));
  *ang   = *((float *) (store+4));
  *count = *((int *)   (store+8));
  *ktime = *((int *)   (store+12));
  *iadd  = *((int *)   (store+16));
  if (HASAZANG)
    *azang = *((float *) (store+20));

  /* printf("count=%f %f %f %d %d %d\n", *tbval,*ang,*count,*ktime,dumb); */
  
  /* read fill_array pixel indices */
  if (fread(&dumb, sizeof(int), 1, imf) == 0) rFerror(210,*nrec); 
  if (fread(locs, sizeof(int), *count, imf) != *count) rFerror(211,*nrec);
  if (fread(&dumb, sizeof(int), 1, imf) == 0) rFerror(212,*nrec);

  /* read response_array values */
  if (fread(&dumb, sizeof(int), 1, imf) == 0) rFerror(213,*nrec); 
  if (fread(resp, sizeof(short int), *count, imf) != *count) rFerror(214,*nrec);
  if (fread(&dumb, sizeof(int), 1, imf) == 0) rFerror(215,*nrec);
  
  return(0);
}


int put_measurements(char *store, int *locs, short int *resp, float tbval, float ang, float azang, int count,
		     int ktime, int iadd, FILE *omf)
{  /* writes a set of measurement to the outupt file */
  int dumb;

  *((float *) (store+0)) = tbval;
  *((float *) (store+4)) = ang;
  *((int *)   (store+8)) = count;
  *((int *)   (store+12)) = ktime;
  *((int *)   (store+16)) = iadd;
  if (HASAZANG)
    *((float *) (store+20)) = azang;
  
  /* write values */
  dumb=HS;  
  fwrite(&dumb, sizeof(int),   1, omf);
  fwrite(store, sizeof(char), HS, omf);
  fwrite(&dumb, sizeof(int),   1, omf);

  /* write fill_array pixel indices */
  dumb=count*4;  
  fwrite(&dumb, sizeof(int), 1, omf); 
  fwrite(locs, sizeof(int), count, omf);
  fwrite(&dumb, sizeof(int), 1, omf);
  
  /* write response_array values */
  dumb=count*2;  
  fwrite(&dumb, sizeof(int), 1, omf);
  fwrite(resp, sizeof(short int), count, omf);
  fwrite(&dumb, sizeof(int), 1, omf);
}


void no_trailing_blanks(char *s)
{  /* remove trailing blanks (spaces) from string */
  int n=strlen(s);
  
  while (n > 0) {
    if (s[n] != ' ' && s[n] != '\0') return;
    if (s[n] == ' ') s[n] = '\0';
    n--;
  }
  return;
}


char *addpath(char *outpath, char *name, char *temp)
{ /* append path to name, return pointer to temp */
  sprintf(temp,"%s/%s",outpath,name);
  return(temp);  
}

  
float gauss(void)
{ /* generate a random normal deviate
     method due to G. MARSAGLIA AND T.A. BRAY,
     SIAM REVIEW, VOL. 6, NO. 3, JULY 1964. 260-264 */

  float rx, ry;
  double r=2.0;

  while (r>=1.0) {
    rx=2.0*(float) rand()/ (float) RAND_MAX - 1.0;
    ry=2.0*(float) rand()/ (float) RAND_MAX - 1.0;
    r = rx*rx + ry*ry;
  }
  r = (float) sqrt( -2.0 * log(r)/r);
  return((float) rx*r);

}

