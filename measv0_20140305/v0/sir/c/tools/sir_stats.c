/*
   program to read BYU a SIR image file and compute the pixel statistics
   of a specified sub region

   Written by DGL 13 Jul 2005 based on sir_extractregion.f

   Written in ANSI C.

   should be linked with 

   sir_ez.c
   sir_geom.c
   sir_io.c

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sir_ez.h"  /* get easy sir routine interface */


int main(int argc, char **argv)
{
  int   ierr, i, j, nsx2, nsy2, x1, y1, x2, y2, ix, iy;
  float rx1, ry1, rx2, ry2;
  float alon, alat, nodata, rms=0.0;
  float smin=1.e25, smax=-1.e25, ave=0.0, var=0.0, std=0.0;
  int cnt=0, cnt_valid=0;

  float *stval;        /* pointer to input image storage */

  sir_head head;       /* sir_ez defined structure to store SIR header info */
	
  fprintf(stdout,"BYU SIR subregion statistics program\n") ;
  if(argc < 7) {
    fprintf(stdout,"\nusage: %s file flag LLx LLy URx URy <out>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file    =  input SIR file\n");
    fprintf(stdout,"   flag    =  corner option flag\n");
    fprintf(stdout,"              P: pixel coordinates  L: lat/lon coordinates\n");
    fprintf(stdout,"   LLx,LLy =  lower-left corner coordinates\n");
    fprintf(stdout,"   URx,URy =  Upper-right corner coordinates\n");
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

  /* get region for computing statistics */

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

  /* compute statistics of data from subregion */

  nodata = head.anodata;
  for (ix = x1; ix <= x2; ix++)
     for (iy = y1; iy <= y2; iy++) {
       i = sir_lex(ix, iy, &head);  /* get pixel index  0 ... nsx*nxy-1 */
       cnt++;
       if (*(stval+i) > nodata+0.00001) { 
	 cnt_valid++;
	 if (*(stval+i) < smin) smin = *(stval+i);
	 if (*(stval+i) > smax) smax = *(stval+i);
	 ave = (cnt_valid-1) * ave/(float)cnt_valid + *(stval+i)/ (float)cnt_valid;
	 rms = (cnt_valid-1) * rms/(float)cnt_valid + *(stval+i) * *(stval+i)/ (float)cnt_valid;
       }
     }

  var=rms;
  if (var > 0.0) { 
    rms=sqrt((double)rms);
    var=var-ave*ave;
    std=var;
    if (std > 0.0) std=sqrt((double)std);
  }

  /* write out stats */

  if (cnt_valid < 1) {
    fprintf(stdout," ** no valid pixels to compute difference\n");
    exit(-1);
  }
  fprintf(stdout,"  Pixels:  %d valid of %d\n",cnt_valid,cnt);
  fprintf(stdout,"  Ave/RMS: %f %f\n",ave,rms);
  fprintf(stdout,"  var/std: %f %f\n",var,std);
  fprintf(stdout,"  Min/max: %f %f\n",smin,smax);

  return(0);
}

