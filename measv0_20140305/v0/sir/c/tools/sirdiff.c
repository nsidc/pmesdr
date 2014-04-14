/*
   program to compute the difference between two BYU SIR file images

   Written by DGL 2 Feb. 2002
   Revised by DGL 25 Jul. 2005 + update EASE grid head computation

   This simple program reads two BYU sir-format input file using the
   easy sir routines.  The difference between the two images (where
   both are greater than the no-data value) is determined an stats
   computed.  The difference image is optionally written to a file

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
  int   lx, hx, ly, hy, i, j, ix, iy;
  float nodata1, nodata2;
  char  *in1 = NULL, *in2 = NULL, *out = NULL;
  int   ierr;
  float *stval1, *stval2, *stval;  /* pointer to image storage */
  float nodiffval = -1.e25;
  float rmin = -1.e25, rmax = 1.e25;
  int range_specify = 0;
  int region_select_latlon = 0, region_select_int=0;
  float LL_lat, LL_lon, UR_lat, UR_lon, x1, y1, delta;

  /* stats */

  int cnt = 0, cnt1 = 0, cnt2 = 0;
  int cnt_valid = 0, cnt1_valid = 0, cnt2_valid = 0;
  float ave = 0.0, ave1 = 0.0, ave2 = 0.0;
  float var = 0.0, var1 = 0.0, var2 = 0.0;
  float rms = 0.0, rms1 = 0.0, rms2 = 0.0;
  float std = 0.0, std1 = 0.0, std2 = 0.0;
  float smin = 1.e25, smin1 = 1.e25, smin2 = 1.e25;
  float smax = -1.e25, smax1 = -1.e25, smax2 = -1.e25;
  float corcoef = 0.0, covar = 0.0;

  sir_head head1, head2;   /* header storage structures */

  fprintf(stdout,"BYU SIR difference program\n") ;
  if(argc < 2) {
    fprintf(stdout,"\nusage: %s file <options> in1 in2\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   <options> (upper or lower case accepted) (optional)\n");
    fprintf(stdout,"    -l <value> =  min pixel value [def=all]\n");
    fprintf(stdout,"    -h <value> =  max pixel value [def=all]\n");
    fprintf(stdout,"    -o <name> =  output SIR file name for difference [def=none]\n");
    fprintf(stdout,"    -r <LL_lat LL_lon UR_lat UR_lon> = subregion [def=full image]\n");
    fprintf(stdout,"    -p <LL_x LL_y UR_x UR_y> = subregion [def=full image]\n");
    fprintf(stdout,"        (-r and -p are mutually exclusive)\n");
    fprintf(stdout,"   in1  =  input 1 SIR file (out=in1-in2)\n");
    fprintf(stdout,"   in2  =  input 2 SIR file\n");
    return(0);
  }

  /* read program arguments */

  j=0;
  for (i=1; i < argc; i++) {
    if (*argv[i] == '-') { /* optional arguments */

      if (*(argv[i]+1) == 'o' || *(argv[i]+1) == 'O')
	if (i+1 < argc)
	  out = argv[++i];

      if (*(argv[i]+1) == 'l' || *(argv[i]+1) == 'L')
	if (i+1 < argc) {
	  sscanf(argv[++i], "%f",&rmin);
	  range_specify = 1;
	}      

      if (*(argv[i]+1) == 'h' || *(argv[i]+1) == 'H')
	if (i+1 < argc) {
	  sscanf(argv[++i], "%f",&rmax);
	  range_specify = 1;
	}

      if (*(argv[i]+1) == 'r' || *(argv[i]+1) == 'R')
	if (i+4 < argc) {
	  region_select_latlon = 1;
	  sscanf(argv[++i], "%f", &LL_lat);
	  sscanf(argv[++i], "%f", &LL_lon);
	  sscanf(argv[++i], "%f", &UR_lat);
	  sscanf(argv[++i], "%f", &UR_lon);
	}
      
      if (*(argv[i]+1) == 'p' || *(argv[i]+1) == 'P')
	if (i+4 < argc) {
	  region_select_int=1;
	  sscanf(argv[++i], "%d", &lx);
	  sscanf(argv[++i], "%d", &ly);
	  sscanf(argv[++i], "%d", &hx);
	  sscanf(argv[++i], "%d", &hy);
	}

    } else {
      if (j == 0)
	in1 = argv[i];
      if (j == 1)
	in2 = argv[i];
      j++;
    }
  }


  /* read the two input SIR files into memory */

  sir_init_head(&head1);
  ierr = get_sir(in1, &head1, &stval1);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR file '%s'\n",in1);
     exit(-1);
  }
  sir_update_head(&head1, stval1);  

  sir_init_head(&head2);
  ierr = get_sir(in2, &head2, &stval2);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR file '%s'\n",in2);
     exit(-1);
  }
  sir_update_head(&head2, stval2);

  /* check compatibility of the images, i.e. same size and projection */

  if (!(head1.nsx    == head2.nsx    && head1.nsy    == head2.nsy    && 
	head1.iopt   == head2.iopt   && 
	head1.xdeg   == head2.xdeg   && head1.ydeg   == head2.ydeg   &&
	head1.ascale == head2.ascale && head1.bscale == head2.bscale &&
	head1.a0     == head2.a0     && head1.b0     == head2.b0) ) {
    fprintf(stdout,"*** Incompatible image size and/or projections\n");
    fprintf(stdout,"File 1 (Reference):  %s\n",in1);
    print_sir_head(stdout, &head1);
    fprintf(stdout,"File 2 (subtracted): %s\n",in2);
    print_sir_head(stdout, &head2);
    exit(-1);
  }
  
  nodata1 = head1.anodata + 0.0001;
  nodata2 = head2.anodata + 0.0001;

  if (region_select_int == 0) {
    lx = 1;
    ly = 1;
    hx = head1.nsx;
    hy = head1.nsy;
  }

  if (region_select_latlon == 1) {
    sir_latlon2pix(LL_lon, LL_lat, &x1, &y1, &head1);     /* (lat,lon) -> (x,y) */
    (void) sir_pix(x1, y1, &lx, &ly, &head1);             /* (x,y) -> (ix,iy) */
    /* fprintf(stdout,"LL Pixel (%d,%d) Lat,Lon (%3.3f,%7.3f)\n",lx,ly,LL_lat,LL_lon); */

    sir_latlon2pix(UR_lon, UR_lat, &x1, &y1, &head1);     /* (lat,lon) -> (x,y) */
    (void) sir_pix(x1, y1, &hx, &hy, &head1);             /* (x,y) -> (ix,iy) */
    /* fprintf(stdout,"UR Pixel (%d,%d) Lat,Lon (%3.3f,%7.3f)\n",hx,hy,UR_lat,UR_lon); */

    if (lx > hx) {
      ix = hx;
      hx = lx;
      lx = ix;
    }
    if (ly > hy) {
      iy = hy;
      hy = ly;
      ly = iy;
    }
    
  }  

  if (lx < 0) lx = 1; 
  if (lx > head1.nsx) lx = head1.nsx;
  if (ly < 0) ly = 1; 
  if (ly > head1.nsy) ly = head1.nsy;

  if (hx < lx) hx = lx; 
  if (hx > head1.nsx) hx = head1.nsx;
  if (hy < ly) hy = ly; 
  if (hy > head1.nsy) hy = head1.nsy;


  if (out != NULL) {
    /* output difference image desired.  for simplicity head1 is reused for
       the output image since head2 has the input file projection info  */

    if (region_select_latlon == 1 || region_select_int == 1) {

    /* modify projection information for subregion */

      head1.nsx = hx-lx+1;
      head1.nsy = hy-ly+1;

      switch(head1.iopt) {

      case -1: /* image only (no projection) */
      default:
	head1.a0 = (x1 - 1) * head2.xdeg / (float) head2.nsx + head2.a0;
	head1.b0 = (y1 - 1) * head2.ydeg / (float) head2.nsy + head2.b0;
	head1.xdeg = (float) head1.nsx * head2.xdeg / (float) head2.nsx;
	head1.ydeg = (float) head1.nsy * head2.ydeg / (float) head2.nsy;
	break;

      case 0: /* lat/lon grid */
	head1.a0 = (x1 - 1) * head2.xdeg / (float) head2.nsx + head2.a0;
	head1.b0 = (y1 - 1) * head2.ydeg / (float) head2.nsy + head2.b0;
	head1.xdeg = (float) head1.nsx * head2.xdeg / (float) head2.nsx;
	head1.ydeg = (float) head1.nsy * head2.ydeg / (float) head2.nsy;
	break;
	
      case 1: /* Lambert */
      case 2:
	head1.a0 = (lx - 1) / head2.ascale + head2.a0;
	head1.b0 = (ly - 1) / head2.bscale + head2.b0;
	break;

      case 5: /* polar stereographic */
	head1.a0 = (lx - 1) * head2.ascale + head2.a0;
	head1.b0 = (ly - 1) * head2.bscale + head2.b0;
	break;

      case 11: /* EASE grid*/
      case 12:
      case 13:
	head1.a0 = head2.a0 + (float) (lx - 1);
	head1.b0 = head2.b0 + (float) (ly - 1);
	break;
      }
    }
    
  /* allocate storage space for output difference image data */
  
    stval = (float *) malloc(sizeof(float) * head1.nsx * head1.nsy);
    if (stval == NULL) {
      fprintf(stderr,"*** ERROR: difference image memory allocation failure...\n");
      exit(-1);
    }
  }
  
  for (ix = lx; ix <= hx; ix++)
    for (iy = ly; iy <= hy; iy++) {
      i = sir_lex(ix, iy, &head2);  /* get pixel index  0 ... nsx*nxy-1 */

      /* compute differences only for no-data pixels, but compute
	 separate counts for valid pixels.  Statistics are computeed only
         for pixel values within specified range.*/

      cnt++;
      if (*(stval1+i) > nodata1) cnt1_valid++;
      if (*(stval2+i) > nodata2) cnt2_valid++;

      if (*(stval1+i) >  nodata1 && *(stval2+i) >  nodata2 &&
	  *(stval1+i) >= rmin    && *(stval2+i) <=  rmax  ) {

	cnt_valid++;
	if (*(stval1+i) < smin1) smin1 = *(stval1+i);
	if (*(stval1+i) > smax1) smax1 = *(stval1+i);
	if (*(stval2+i) < smin2) smin2 = *(stval2+i);
	if (*(stval2+i) > smax2) smax2 = *(stval2+i);

	ave1 = (cnt_valid-1) * ave1/(float)cnt_valid + *(stval1+i)/ (float)cnt_valid;
	ave2 = (cnt_valid-1) * ave2/(float)cnt_valid + *(stval2+i)/ (float) cnt_valid;
	var1 = (cnt_valid-1) * var1/(float)cnt_valid + *(stval1+i) * *(stval1+i)/(float)cnt_valid;
	var2 = (cnt_valid-1) * var2/(float)cnt_valid + *(stval2+i) * *(stval2+i)/(float)cnt_valid;
	covar = (cnt_valid-1) * covar/(float)cnt_valid + *(stval1+i) * *(stval2+i)/cnt_valid;

	delta = *(stval1+i) - *(stval2+i);

	if (delta < smin) smin = delta;
	if (delta > smax) smax = delta;

	ave = (float)(cnt_valid-1) * ave/(float)cnt_valid + delta / (float)cnt_valid;
	var = (float)(cnt_valid-1) * var/(float)cnt_valid + delta * delta / (float)cnt_valid;

	if (out != NULL) { 
	  j = sir_lex(ix-lx+1, iy-ly+1, &head1);
	  *(stval+j) = delta;
	}	

      } else {

	if (out != NULL) { 
	  j = sir_lex(ix-lx+1, iy-ly+1, &head1);
	  *(stval+j) = nodiffval;
	}
      }
      
    }

  rms=var;
  if (var > 0.0) { 
    rms=sqrt(rms);
    var=var-ave*ave;
    std=var;
    if (std > 0.0) std=sqrt(std);
  }

  rms1=var1;
  if (var1 > 0.0) { 
    rms1=sqrt(rms1);
    var1=var1-ave1*ave1;
    std1=var1;
    if (std1 > 0.0) std1=sqrt(std1);
  }

  rms2=var2;
  if (var2 > 0.0) { 
    rms2=sqrt(rms2);
    var2=var2-ave2*ave2;
    std2=var2;
    if (std2 > 0.0) std2=sqrt(std2);
  }

  covar = covar - ave1 * ave2;
  if (std1 > 0.0 && std2 > 0.0) corcoef=covar/(std1*std2);
  

  /* write out stats */

  if (range_specify == 1)
    fprintf(stdout,"Image range specified: %f %f\n",rmin, rmax);
  if (region_select_latlon)
    fprintf(stdout,"Subregion corners (lat,lon): LL=(%3.3f,%7.3f) UR=(%3.3f,%7.3f)\n",
	    LL_lat,LL_lon,UR_lat,UR_lon);
  if (region_select_latlon == 1 || region_select_int == 1)
    fprintf(stdout,"Subregion pixels (x,y):      LL=(%d,%d) UR=(%d,%d)\n",lx,ly,hx,hy);

  if (cnt_valid < 1) {
    fprintf(stdout," ** no valid pixels to compute difference\n");
    exit(-1);
  }
  fprintf(stdout,"Reference image '%s'\n",in1);
  fprintf(stdout,"  Pixels:  %d valid of %d\n",cnt1_valid,cnt);
  fprintf(stdout,"  No data value: %f\n",nodata1);
  fprintf(stdout,"  Ave/RMS: %f %f\n",ave1,var1);
  fprintf(stdout,"  var/std: %f %f\n",rms1,std1);
  fprintf(stdout,"  Min/max: %f %f\n",smin1,smax1);

  fprintf(stdout,"Subtracted image '%s'\n",in2);
  fprintf(stdout,"  No data value: %f\n",nodata2);
  fprintf(stdout,"  Pixels: %d valid of %d\n",cnt2_valid,cnt);
  fprintf(stdout,"  Ave/RMS: %f %f\n",ave2,rms2);
  fprintf(stdout,"  var/std: %f %f\n",var2,std2);
  fprintf(stdout,"  Min/max: %f %f\n",smin2,smax2);

  fprintf(stdout," Correlation coef, covariance: %f %f\n",corcoef,covar);
  fprintf(stdout,"Difference stats:  %d pixels of %d\n",cnt_valid, 
	  head1.nsx * head1.nsy);
  fprintf(stdout,"  Ave/RMS: %f %f\n",ave,rms);
  fprintf(stdout,"  var/std: %f %f\n",var,std);;
  fprintf(stdout,"  Min/max: %f %f\n",smin,smax);



  if (out == NULL) return(0);

  /* write out difference file

     first we modify the header information to store and display the results */

  delta = smax - smin;
  if (delta < 1.0) delta = 1.0;
  if (delta > 1.e6) 
    fprintf(stdout," *Advisory* large difference range may result in poor storage scaling \n");

  i = 0.1 * 32700 / delta;
  if (i > 10) i = 10;
  if (i < 1) i = 1;
  
  delta = delta + 3.0 * i;
  
  head1.v_min = smin;
  head1.v_max = smax;
  head1.anodata = (float) (((int) smin) - i);

  head1.idatatype = 2;  /* default 2 byte integer storage */
  head1.ioff = ((int) smin) - i * 2;
  head1.iscale = (int) (32700.0/delta);

  for (i=0; i< head1.nsx * head1.nsy; i++)
    if (*(stval+i) < head1.anodata) 
      *(stval+i) = head1.anodata;


/* write output difference SIR format file */

  fprintf(stdout,"Writing difference output SIR file '%s'\n", out);
  ierr = put_sir(out, &head1, stval);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output SIR file '%s'***\n",out);
     fflush(stderr);
  }

  return(0);
}









