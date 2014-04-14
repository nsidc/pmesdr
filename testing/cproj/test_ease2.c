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

extern void ease2_map_info(int iopt, int isc, int ind, 
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
  int projt, ind=0, nease=0, nsx, nsy, ix, iy, ipix, k, kk, n, nerr;
  float a0, b0, ascale, bscale, xdeg, ydeg;
  float alat, alon, thelon, thelat, x, y, lon, lat;

  double *lats, *lons;  
  float x1, y1;  
  
  double map_equatorial_radius_m,map_eccentricity, e2,
    map_reference_latitude, map_reference_longitude, 
    map_second_reference_latitude, sin_phi1, cos_phi1, kz,
    map_scale, r0, s0, epsilon;
  int bcols, brows;

  char latname[200], lonname[200];

  FILE *fid;

  for (projt=8; projt<11; projt++) {
 
    /* define projection parameters for particular EASE2 case */
    ease2_map_info(projt, nease, ind, &map_equatorial_radius_m, 
		   &map_eccentricity, &e2,
		   &map_reference_latitude, &map_reference_longitude, 
		   &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		   &map_scale, &bcols, &brows, &r0, &s0, &epsilon);
    nsx=bcols;             /* X dim (horizontal=cols) pixels */
    nsy=brows;             /* Y dim (vertical=rows) pixels */
    ascale=(float) nease;  /* base grid scale factor (0..5) */
    bscale=(float) ind;    /* base grid scale index (0..2) */
    a0=0.0;                /* X origin pixel - 1 */
    b0=0.0;                /* Y origin pixel - 1 */
    xdeg=(float) (nsx/2);  /* map center X pixel */
    ydeg=(float) (nsy/2);  /* map center Y pixel */

    for (k=0; k<2; k++) {      
      /* test cases */
      switch(projt) {
      case 8:  /* ease2 grid north */
	strncpy(name,"EASE2 N",20);      
	alon=0.0;
	alat=90.0-k*20;
	break;

      case 9:  /* ease2 grid south */
	strncpy(name,"EASE2 S",20);      
	alon=120.0;
	alat=-90.0+k*20;
	break;
    
      case 10:  /* ease2 grid cylindrical */
	strncpy(name,"EASE2 T",20);      
	alon=0.0;
	alat=25.0-k*10;
	break;
      }

      latlon2pix(alon, alat, &x, &y, projt, xdeg, ydeg, ascale, bscale, a0, b0);
      (void) f2ipix(x, y, &ix, &iy, nsx, nsy);
      pixtolatlon(x, y, &lon, &lat, projt, xdeg, ydeg, ascale, bscale, a0, b0);

      if (k==0) {	
	printf("\n%d %s ascale=%f bscale=%f a0=%f b0=%f\n           nsx=%d nsy=%d xdeg=%f ydeg=%f\n\n",projt,name,ascale,bscale,a0,b0,nsx,nsy,xdeg,ydeg);
	printf("Forward check:\n");
      }
          
      printf(" BYU:  (lon,lat) %f %f => %f %f  %d %d ( x,y)\n              => %f %f\n",alon,alat,x,y,ix,iy,lon,lat);
      x=x-1.5;
      y=nsy-y+0.5;
      printf(" NSIDC:(lon,lat) %f %f => %f %f\n",alon,alat,x,y);
    }
    
    printf("\nInverse check:\n");
    for (k=0; k<3; k++) {      
      /* test cases */
      switch(projt) {
      case 8:  /* ease2 grid north */
	strncpy(name,"EASE2 N",20);      
	alon=0.0;
	alat=90.0-k*20;
	break;

      case 9:  /* ease2 grid south */
	strncpy(name,"EASE2 S",20);      
	alon=120.0;
	alat=-90.0+k*20;
	break;
    
      case 10:  /* ease2 grid cylindrical */
	strncpy(name,"EASE2 T",20);      
	alon=0.0;
	alat=25.0-k*10;
	break;
      }
      for (kk=0; kk<3; kk++) {	  
	x=1.0+k*(nsx/2)-0.5;
	if (k==2) x=x-1.0;	
	if (kk>0) x=x+0.5;
	y=1.0+k*(nsy/2)-0.5;
	if (k==2) y=y-1.0;
	if (kk>1) y=y+0.5;

	pixtolatlon(x, y, &lon, &lat, projt, xdeg, ydeg, ascale, bscale, a0, b0);
	latlon2pix(lon, lat, &x1, &y1, projt, xdeg, ydeg, ascale, bscale, a0, b0);
	(void) f2ipix(x, y, &ix, &iy, nsx, nsy);

	printf(" BYU:  (x,y) %6.2f %6.2f => %f %f => %f %f => %d %d\n",x,y,lon,lat,x1,y1,ix,iy);

	x=x-1.5;      /* compute NSIDC locations from BYU locations */
	y=nsy-y+0.5;
	x1=x1-1.5;
	y1=nsy-y1+0.5;
	printf(" NSIDC:(x,y) %6.2f %6.2f => %f %f => %f %f\n",x,y,lon,lat,x1,y1);
      }
    }    
  }

  //exit(0);  


  /* test over all image pixels and compare to NSIDC-supplied arrays */
   for (projt=8; projt<11; projt++) {
     
    /* define projection parameters for particular EASE2 case */
    ease2_map_info(projt, nease, ind, &map_equatorial_radius_m, 
		   &map_eccentricity, &e2,
		   &map_reference_latitude, &map_reference_longitude, 
		   &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		   &map_scale, &bcols, &brows, &r0, &s0, &epsilon);
    nsx=(int) bcols;       /* X dim (horizontal=cols) pixels */
    nsy=(int) brows;       /* Y dim (vertical=rows) pixels */
    ascale=(float) nease;  /* base grid scale factor (0..5) */
    bscale=(float) ind;    /* base grid scale index (0..2) */
    a0=0.0;                /* X origin pixel - 1 */
    b0=0.0;                /* Y origin pixel - 1 */
    xdeg=(float) (nsx/2);  /* map center X pixel */
    ydeg=(float) (nsy/2);  /* map center Y pixel */

    switch(projt) {
    case 8:  /* ease2 grid north */
      strncpy(name,"N",20);      
      strncpy(latname,"../../ease2/pixcenter/EASE2_N25km.lats.720x720x1.double",200);
      strncpy(lonname,"../../ease2/pixcenter/EASE2_N25km.lons.720x720x1.double",200);
      break;
      
    case 9:  /* ease2 grid south */
      strncpy(name,"S",20);      
      strncpy(latname,"../../ease2/pixcenter/EASE2_S25km.lats.720x720x1.double",200);
      strncpy(lonname,"../../ease2/pixcenter/EASE2_S25km.lons.720x720x1.double",200);
      break;
      
    case 10:  /* ease2 grid cylindrical */
      strncpy(name,"T",20);      
      strncpy(latname,"../../ease2/pixcenter/EASE2_T25km.lats.1388x538x1.double",200);
      strncpy(lonname,"../../ease2/pixcenter/EASE2_T25km.lons.1388x538x1.double",200); 
      break;
    }

    printf("\nChecking %s projection\n",name);
 
    lats=(double *) malloc(sizeof(double)*nsx*nsy);
    lons=(double *) malloc(sizeof(double)*nsx*nsy);

    //printf(" read two files\n");
    fid=fopen(latname,"r");
    if (fid==NULL)
      printf("*** error opening file %s\n",latname);      
    n=fread(lats,sizeof(double),nsx*nsy,fid);
    fclose(fid);
    fid=fopen(lonname,"r");
    if (fid==NULL)
      printf("*** error opening file %s\n",lonname);      
    n=fread(lons,sizeof(double),nsx*nsy,fid);
    fclose(fid);

    // printf(" both files read\n");    
    nerr=0;    

    /* note that BYU SIR files use 1-based indexing */

    for (ix=0; ix<nsx; ix+=1)
      for (iy=0; iy<nsy; iy+=1) {
	x=ix+1.5; /* zero-based to one-based location PLUS 1/2 pixel for corner to center */
	y=iy+1.5; /* BYU pixel locations */

	ipix=iy * nsx + ix;         /* BYU SIR pixel order (0 at bottom) */
	ipix=(nsy-iy-1) * nsx + ix; /* NSIDC pixel order (0 at top) */	

	pixtolatlon(x, y, &lon, &lat, projt, xdeg, ydeg, ascale, bscale, a0, b0);
	latlon2pix(lon, lat, &x1, &y1, projt, xdeg, ydeg, ascale, bscale, a0, b0);

	/* check results by comparison with NSIDC computation */
	if (lats[ipix]>-990.0 && lons[ipix]>-990.0 &&
	    (abs(x-x1)>0.0001 || abs(y-y1)>0.0001)) {	  
	  printf("A: %d %d %f %f  %f %f %f %f\n",ix+1,iy+1,x1,y1,lon,lat,lons[ipix],lats[ipix]);
	  nerr++;  
	}
	
	if (lats[ipix]>-990.0 && lons[ipix]>-990.0 &&
	    (abs(lon-lons[ipix])>0.0001 || abs(lat-lats[ipix])>0.0001)) {
	  printf("B: %d %d %f %f  %f %f %f %f\n",ix+1,iy+1,x1,y1,lon,lat,lons[ipix],lats[ipix]);
	  nerr++;
	}

	x1=x-1.5;     /* NSIDC location */
	y1=nsy-y+0.5; /* NSIDC location */

	/* print selected locations */
	if (ix==0 & iy==0) printf(" center of origin pixel: %d %d  %.2f,%.2f (%.2f,%.2f) => %f.%f\n",ix+1,iy+1,x,y,x1,y1,lon,lat);
	if (ix==nsx/2 && iy==nsy/2) printf(" center of center pixel: %d %d  %.2f,%.2f (%.2f,%.2f) => %f,%f\n",ix+1,iy+1,x,y,x1,y1,lon,lat);
	
      }

    free(lats);
    free(lons);
    
    printf(" Pixel location computation errors: %d\n\n",nerr);    
    
  }

  return(0);  
}
