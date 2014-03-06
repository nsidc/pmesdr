/* program to test the forward/reverse EASE2 grid transformation */

/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_meta_make.c

  generates SIR meta file for the MEaSUREs project

  translated to C by DGL at BYU 03/01/2014 from ssmi_meta_make3.f 
   note: while fortran was well-tested, not all C program options
   have been fully tested   

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <sir3.h>

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define mod(a,b) ((a) % (b))
#define abs(x) ((x) >= 0 ? (x) : -(x))

int nint(float r)
{
  int ret_val = r;
  if (ret_val - r > 0.5) ret_val--;
  if (r - ret_val > 0.5) ret_val++;
  return(ret_val);
}

extern void ease2_map_info(int iopt, int isc, int hem, 
			   double *map_equatorial_radius_m, 
			   double *map_eccentricity, double *e2,
			   double *map_reference_latitude, 
			   double *map_reference_longitude, 
			   double *map_second_reference_latitude,
			   double *sin_phi1, 
			   double *cos_phi1, double *kz,
			   double *map_scale, int *bcols, int *brows, 
			   double *r0, double *s0, double *epsilon);

/****************************************************************************/

int main(int argc,char *argv[])
{
  char name[20];  
  int projt, isc=25, nease=2, nsx, nsy, ix, iy;
  float a0, b0, ascale, bscale, xdeg, ydeg;
  float alat, alon, thelon, thelat, x, y, lon, lat;
  
  double map_equatorial_radius_m,map_eccentricity, e2,
    map_reference_latitude, map_reference_longitude, 
    map_second_reference_latitude, sin_phi1, cos_phi1, kz,
    map_scale, r0, s0, epsilon;
  int bcols, brows;

  for (projt=8; projt<11; projt++) {
 
    //printf("begin projt %d\n",projt);

    /* define projection parameters */
    ease2_map_info(projt, nease, isc, &map_equatorial_radius_m, 
		   &map_eccentricity, &e2,
		   &map_reference_latitude, &map_reference_longitude, 
		   &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		   &map_scale, &bcols, &brows, &r0, &s0, &epsilon);
    nsx=(int) s0;
    nsy=(int) r0;
    ascale=(float) nease;
    bscale=(float) isc;
    a0=0.;
    b0=0.;
    xdeg=(float) nsx/2.0;
    ydeg=(float) nsy/2.0; 

    /* test cases */
    switch(projt) {
    case 8:  /* ease2 grid north */
      strncpy(name,"EASE2 N",20);      
      alon=120.0;
      alat=75.0;
      break;

    case 9:  /* ease2 grid south */
      strncpy(name,"EASE2 S",20);      
      alon=120.0;
      alat=-75.0;
    break;
    
    case 10:  /* ease2 grid cylindrical */
      strncpy(name,"EASE2 M",20);      
      alon=0.0;
      alat=25.0;
    break;
    
    default: /* should not occur! */
      break;	
    }

    latlon2pix(alon, alat, &x, &y, projt, xdeg, ydeg,
	       ascale, bscale, a0, b0);
    f2ipix(x, y, &ix, &iy, nsx, nsy);    

    pixtolatlon(x,y, &lon, &lat, projt, xdeg, ydeg,
	       ascale, bscale, a0, b0);

    printf("\n%d %s ascale=%f bscale=%f a0=%f b0=%f\n nsx=%d nsy=%d xdeg=%f ydeg=%f\n",projt,name,ascale,bscale,a0,b0,nsx,nsy,xdeg,ydeg);
    
    printf(" (lon,lat) %f %f => %f %f  %d %d (x,y)\n  => %f %f\n",alon,alat,x,y,ix,iy,lon,lat);
    
  }
  return(0);  
}
