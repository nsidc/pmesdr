/* (c) 1998, 1999, 2005, 2014 BYU MERS Laboratory

   standard C BYU SIR projection transformation routines

   DGL Dec. 19, 1998
   DGL Sept. 24, 1999 + fixed polster, latlon2pix, changed args to pixtolatlon
   DGL July 29, 2005 + modified and corrected EASE projection code
   DGL Feb 4, 2014 + added EASE2 grid projection
   DGL Feb 4, 2014 + added ease2sf
   DGL Aug 2, 2014 + modified EASE2T vertical dimension from 538 to 540

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int intfix(float r)
{
  int ret_val = r;
  if (ret_val - r > 0.5) ret_val--;
  if (r - ret_val > 0.5) ret_val++;
  return(ret_val);
}

int dceil(double r)
{
  int ret_val = r;
  if (ret_val - r > 0.0) ret_val++;
  return(ret_val);
}


/******* standard sir format geometric transformation routines *****/


void ilambert1(float*, float*, float, float, float, float, int);
void ipolster(float*, float*, float, float, float, float); 
void ieasegrid(int, float*, float*, float, float, float);
void iease2grid(int, float*, float*, float, float, float, float);

void pixtolatlon(float x, float y, float *alon, float *alat,
		 int iopt, float xdeg, float ydeg,
		 float ascale, float bscale, float a0, float b0)
{
   /* computes the lat, long position of the lower-left corner of the
      x,y-th pixel.  pixels indexed [1..nsx,1..nsy] */

   float thelon, thelat;
   
   switch(iopt) {
    case -1:
    case 0:
      *alon = (x-1.0)/ascale+a0;
      *alat = (y-1.0)/bscale+b0;
      break;
    case 1:
    case 2:
      thelon = (x-1.0)/ascale+a0;
      thelat = (y-1.0)/bscale+b0;
      ilambert1(alat,alon,thelon,thelat,ydeg,xdeg,iopt);
      break;
    case 5:
      thelon = (x-1.0)*ascale+a0;
      thelat = (y-1.0)*bscale+b0;
      ipolster(alon,alat,thelon,thelat,xdeg,ydeg);
      break;
    case 8:
    case 9:
    case 10:
      thelon = x - 1.0 + a0;
      thelat = y - 1.0 + b0;      
      iease2grid(iopt, alon, alat, thelon, thelat, ascale, bscale);
    case 11:
    case 12:
    case 13:
      thelon = x - 1.0 + a0;
      thelat = y - 1.0 + b0;
      ieasegrid(iopt, alon, alat, thelon, thelat, ascale);
      break;
    default:
      *alon=0.0;
      *alat=0.0;
   }
   return;
}

void lambert1(float, float, float*, float*, float, float, int);
void polster(float, float, float*, float*, float, float);
void easegrid(int, float, float, float*, float*, float);
void ease2grid(int, float, float, float*, float*, float, float);


void latlon2pix(float alon, float alat, float *x, float *y, 
		 int iopt, float xdeg, float ydeg,
		 float ascale, float bscale, float a0, float b0)
{
   /* computes the x,y pixel coordinates given the lat, lon position 
      x,y are fractional values not limited to within image */

   static float thelon, thelat;
   
   switch(iopt) {
    case -1:
    case 0:
      *x = ascale * (alon - a0)+1.0;
      *y = bscale * (alat - b0)+1.0;
      break;
    case 1:
    case 2:
      lambert1(alat, alon, &thelon, &thelat, ydeg, xdeg, iopt);
      *x = ascale * (thelon - a0)+1.0;
      *y = bscale * (thelat - b0)+1.0;
      break;
    case 5:
      polster(alon, alat, &thelon, &thelat, xdeg, ydeg);
      *x = (thelon - a0)/ascale+1.0;
      *y = (thelat - b0)/bscale+1.0;
      break;
    case 8:
    case 9:
    case 10:
      ease2grid(iopt, alon, alat, &thelon, &thelat, ascale, bscale);
      *x = thelon + 1.0 - a0;
      *y = thelat + 1.0 - b0;
      break;
    case 11:
    case 12:
    case 13:
      easegrid(iopt, alon, alat, &thelon, &thelat, ascale);
      *x = thelon + 1.0 - a0;
      *y = thelat + 1.0 - b0;
      break;
    default:
      *x=0.0;
      *y=0.0;
   }
   return;
}

/* floating point to integer pixel location quantization routine */

void f2ipix(float x, float y, int *ix, int *iy, int nsx, int nsy)
{
   /* quantizes the floating point pixel location to the actual pixel value
      returns a zero if location is outside of image limits
      a small amount (0.002 pixels) of rounding is permitted*/

  if (x+0.0002 >= 1.0 && x+0.0002 <= (float) (nsx+1))
    *ix = floor(x+0.0002);
  else
    *ix = 0;

  if (y+0.0002 >= 1.0 && y+0.0002 <= (float) (nsy+1))
    *iy = floor(y+0.0002);
  else
    *iy = 0;

  return;
}


#define DEG2RAD (double) 0.017453292
#define RAD2DEG (double) 57.295779

void lambert1(float lat, float lon, float *x, float *y, 
	       float orglat, float orglon, int iopt)
{
/*
	COMPUTES THE FORWARD TRANSFORMATION (FROM LAT/LON TO X/Y) FOR THE
	LAMBERT AZIMUTHAL EQUAL-AREA PROJECTION

	SEE "MAP PROJECTIONS USED BY THE U.S. GEOLOGICAL SURVEY"
	GEOLOGICAL SURVEY BULLETIN 1532, PGS 157-173

	FOR THIS ROUTINE, A SPHERICAL EARTH IS ASSUMED FOR THE PROJECTION.  THE
	ERROR WILL BE SMALL FOR SMALL-SCALE MAPS.  
	FOR IOPT=1 A FIXED, NOMINAL EARTH RADIUS IS USED.
	FOR IOPT=2 THE LOCAL RADIUS OF THE EARTH IS USED BASED ON
	THE 1972 WGS ELLIPSOID MODEL (BULLETIN PG 15).

	WRITTEN BY: DGL 20 MAY 1996 (based on fortran version)

	INPUTS:
	 LAT	(R): LATITUDE +90 TO -90 DEG WITH NORTH POSITIVE
	 LON	(R): LONGITUDE 0 TO +360 DEG WITH EAST POSITIVE
	 ORGLAT	(R): ORIGIN PARALLEL +90 TO -90 DEG WITH NORTH POSITIVE
	 ORGLON	(R): CENTRAL MERIDIAN (LONGITUDE) 0 TO +360 DEG
			OR -180 TO +180 WITH EAST MORE POSITIVE
	 IOPT	(I): EARTH RADIUS OPTION

	OUTPUTS:
	 X,Y	(R): RECTANGULAR COORDINATES IN KM
*/

   double orglon1, lon1, x1, y1, x2, y2, c, ak, denom, eradearth;
   double radearth=6378.135; /* equitorial radius of the earth */
   double f=298.26;          /* 1/f based on WGS 72 model */
   double era;

   orglon1 = fmod(orglon+720.0,360.0);
   lon1 = fmod(lon+720.0, 360.0);   
   x1 = cos(orglat*DEG2RAD);
   y1 = sin(orglat*DEG2RAD);
   x2 = cos(lat*DEG2RAD);
   y2 = sin(lat*DEG2RAD);
   c = cos((lon1-orglon1)*DEG2RAD);

/*	COMPUTE LOCAL RADIUS OF THE EARTH AT CENTER OF IMAGE */
   eradearth=6378.0;
   if (iopt == 2) { /* use the local radius */
      era = (1.-1./f);
      eradearth=radearth*era/sqrt(era*era*x1*x1+y1*y1);
   }
   
   denom = 1.0 + y1*y2 + x1*x2*c;
   if (denom > 0.0) 
      ak = sqrt(2.0/denom);
   else {
      fprintf(stderr,"*** Divsion error in lambert1 ***\n");
      ak = 1.0;
   }
   *x = (ak * x2 * sin((lon1-orglon1)*DEG2RAD) ) * eradearth;
   *y = (ak * (x1*y2-y1*x2*c) ) * eradearth;
   return;
}



void ilambert1(float *lat, float *lon, float x, float y, 
	       float orglat, float orglon, int iopt)
{
/*
	COMPUTES THE INVERSE TRANSFORMATION (FROM X/Y TO LAT/LON) FOR THE
	LAMBERT AZIMUTHAL EQUAL-AREA PROJECTION

	SEE "MAP PROJECTIONS USED BY THE U.S. GEOLOGICAL SURVEY"
	GEOLOGICAL SURVEY BULLETIN 1532, PGS 157-173

	FOR THIS ROUTINE, A SPHERICAL EARTH IS ASSUMED FOR THE PROJECTION.  THE
	ERROR WILL BE SMALL FOR SMALL-SCALE MAPS.  
	FOR IOPT=1 A FIXED, NOMINAL EARTH RADIUS IS USED.
	FOR IOPT=2 THE LOCAL RADIUS OF THE EARTH IS USED BASED ON
	THE 1972 WGS ELLIPSOID MODEL (BULLETIN PG 15).

	WRITTEN BY: DGL 20 MAY 1996 (based on fortran version)

	INPUTS:
	 X,Y	(R): RECTANGULAR COORDINATES IN KM
	 ORGLAT	(R): ORIGIN PARALLEL +90 TO -90 DEG WITH NORTH POSITIVE
	 ORGLON	(R): CENTRAL MERIDIAN (LONGITUDE) 0 TO +360 DEG
			OR -180 TO +180 WITH EAST MORE POSITIVE
	 IOPT	(I): EARTH RADIUS OPTION

	OUTPUTS:
	 LAT	(R): LATITUDE +90 TO -90 DEG WITH NORTH POSITIVE
	 LON	(R): LONGITUDE 0 TO +360 DEG WITH EAST POSITIVE
*/

   double orglon1, rho, c, t1, t2, x1, y1, eradearth;
   double radearth=6378.135; /* equitorial radius of the earth */
   double f=298.26;          /* 1/f based on WGS 72 model */
   double era;

   orglon1 = fmod(orglon+720.0,360.0);

/*	COMPUTE LOCAL RADIUS OF THE EARTH AT CENTER OF IMAGE */
   eradearth=6378.0;
   if (iopt == 2) { /* use the local radius */
      era = (1.-1./f);
      x1=cos(orglat*DEG2RAD);
      y1=sin(orglat*DEG2RAD);
      eradearth=radearth*era/sqrt(era*era*x1*x1+y1*y1);
   }
   
   x1=x/eradearth;
   y1=y/eradearth;
   rho=x1*x1+y1*y1;
   if (rho > 0.) {
      rho = sqrt(rho);
      c = 2.0 * asin(rho*0.5);
      *lat = asin(cos(c)*sin(orglat*DEG2RAD)+
		  y1*sin(c)*cos(orglat*DEG2RAD)/rho)*RAD2DEG;
   } else {
      c = 0.0;
      *lat = orglat;
   }
   
   if ( !((orglat < 0.0 ? - orglat : orglat) == 90.0)) {
      if (rho == 0.0)
	 *lon = orglon1;
      else {
	 t1=x1*sin(c);
	 t2=rho*cos(orglat*DEG2RAD)*cos(c)-y1*sin(orglat*DEG2RAD)*sin(c);
	 *lon = orglon1 + atan2(t1,t2)*RAD2DEG;
      }
   } else {
      if (orglat == 90.0)
	 *lon = orglon1 + atan2(x1,-y1)*RAD2DEG;
      else
	 *lon = orglon1 + atan2(x1,y1)*RAD2DEG;
   }
   *lon = fmod(*lon+720.0, 360.0);
   if (*lon > 180.0) *lon = *lon -360.0;
   return;
}


#define dtr (double) 0.017453292
#define abs(a) ((a) > 0.0 ? (a) : - (a))

double arctand(double y, double x);

void ipolster(float *alon, float *alat, float x, float y, 
	      float xlam, float slat)
{
/*
	COMPUTES THE INVERSE POLAR STEROGRAPHIC TRANSFORMATION FOR (X,Y)
	GIVEN IN KM WITH REFERENCES LON,LAT=(XLAM,SLAT).
	OUTPUT LON,LAT=ALON,ALAT

	ALGORITHM IS THE SAME AS USED FOR PROCESSING ERS-1 SAR IMAGES
	AS RECEIVED FROM M. DRINKWATER (1994).  UPDATED BY D. LONG TO
	IMPROVE ACCURACY USING ITERATION WITH FORWARD TRANSFORM.
*/
   int icnt;
   float xx,yy;
   double r,rr,rerr,a,aa,aerr,absaerr,sn1;
   double rho,e,e22,e23,cm,chi,t,x1,y1,sn,slat1;
   double e2=0.006693883;
   double re=6378.273;
   double pi2=1.570796327;

/* first use approximate inverse */

   e=sqrt(e2);
   e22=e2*e2;
   e23=e2*e2*e2;
   x1=x;
   y1=y;
   rho=x1*x1+y1*y1;
   if (rho > 0.0) rho = sqrt(rho);
   if (rho < 0.05) {
      *alon = xlam;
      *alat = (slat > 0.0 ? 90.0 : -90.0);
   } else {
      sn=1.0;
      slat1=slat;
      if (slat < 0.0) {
	 sn = -1.0;
	 slat1 = - slat;
      }
      cm=cos(slat1 * dtr)/sqrt(1.0-e2*sin(slat1 * dtr)*sin(slat1 * dtr));
      t=tan(dtr*(45.0-0.5*slat1))/
	    pow((1.-e*sin(slat1*dtr))/(1.+e*sin(slat1*dtr)),e*0.5);
      t=rho*t/(re*cm);
      chi=pi2-2.*atan(t);
      t=chi+(0.5*e2+5.*e22/24.+e23/12.0)*sin(2.*chi)+
	    (7.0*e22/48.0+28.0*e23/240.0)*sin(4.*chi)+
	       (7.0*e23/120.0)*sin(6.*chi);
      *alat=sn*(t*90.0/pi2);
      *alon=sn*atan2(sn*x1,-sn*y1)*RAD2DEG+xlam;
      if (*alon < -180.0) *alon = *alon+360.0;
      if (*alon > 180.0) *alon = *alon-360.0;
   }

/* using the approximate result as a starting point, iterate to improve
   the accuracy of the inverse solution */

   sn1 = 1.0;
   if (slat < 0.0) sn1=-1.0;
   a=arctand( (double) y, (double) x);
   r=sqrt((double) (x * x+ y * y));
   icnt=0;

 label10:
   polster(*alon,*alat,&xx,&yy,xlam,slat);
   rr=sqrt((double) (xx*xx+yy*yy));
   rerr=sn1*(rr-r)/180.0;
   aa=arctand((double) yy,(double) xx);
   aerr=aa-a;
   absaerr=abs(aerr);
   if (absaerr > 180.0) aerr=360.0-aerr;
   absaerr=abs(aerr);
   if ((abs(rerr) < 0.001 && absaerr < 0.001) || icnt > 9) goto label40;
   *alon = *alon +aerr;
   if (abs(*alon) > 360.0) *alon=fmod(*alon,360.0);
   if (*alat * slat < 0) {
      rerr = rerr * (1. - sin(dtr*(*alat>0.0?*alat:- *alat)));
      if ((rerr>0.0?rerr:-rerr)>2.0)
	 rerr=(rerr>0.0?2.0:-2.0)/(double) icnt;
   }
   *alat = *alat + rerr;
   if (abs(*alat) > 90.0) 
      *alat = (*alat > 0.0 ? 90.0 : -90.0);
   icnt++;
   goto label10;
 label40:
   if (abs(*alon) > 360.0) *alon = fmod(*alon + 360.0, 360.);
   return;
}

double arctand(double y, double x)
{
   if (y == 0.0 && x == 0.0)
      return(0.0);
   return(atan2(y,x)*RAD2DEG);
}

void polster( float alon, float alat, float *x1, float *y1, float xlam, float slat)
{
/*
	COMPUTES THE POLAR STEROGRAPHIC TRANSFORMATION FOR A LON,LAT
	INPUT OF (ALON,ALAT) WITH REFERENCE ORIGIN  LON,LAT=(XLAM,SLAT).
	OUTPUT IS (X1,Y1) IN KM

	ALGORITHM IS THE SAME AS USED FOR PROCESSING ERS-1 SAR IMAGES
	AS RECEIVED FROM M. DRINKWATER (1994) 
*/
   double e, t, tx, ty, cm, rho, rlat, sn;
   double e2=0.006693883;
   double re=6378.273;

   e=sqrt(e2);
   if (slat < 0.0) {
      sn = -1.0;
      rlat = -alat;
   } else {
      sn = 1.0;
      rlat = alat;
   }
   t=pow((1.0-e*sin(dtr*rlat))/(1.0+e*sin(dtr*rlat)),e*0.5); 
   ty=tan(dtr*(45.0-0.5*rlat))/t;
   rlat = abs(slat);
   t=pow((1.0-e*sin(dtr*rlat))/(1.0+e*sin(dtr*rlat)),e*0.5);
   tx=tan(dtr*(45.0-0.5*rlat))/t;
   cm=cos(dtr*rlat)/sqrt(1.0-e2*sin(dtr*rlat)*sin(dtr*rlat));
   rho=re*cm*ty/tx;
   *x1 = (sn*sin(dtr*(sn*alon-xlam)))*rho;
   *y1 =-(sn*cos(dtr*(sn*alon-xlam)))*rho;
   return;
}


void easegrid(int iopt, float alon, float alat, 
	       float *thelon, float *thelat, float ascale)
{
/*
	COMPUTE THE FORWARD "EASE" GRID TRANSFORM

	GIVEN THE IMAGE TRANSFORMATION COORDINATES (THELON,THELAT) AND
	THE SCALE (ASCALE) THE CORRESPONDING LON,LAT (ALON,ALAT) IS COMPUTED
	USING THE "EASE GRID" (VERSION 1.0) TRANSFORMATION GIVEN IN FORTRAN
	SOURCE CODE SUPPLIED BY NSIDC

	IOPT IS EASE TYPE: IOPT=11=NORTH, IOPT=12=SOUTH, IOPT=13=CYLINDRICAL

	THE RADIUS OF THE EARTH USED IN THIS PROJECTION IS EMBEDDED INTO
	ASCALE WHILE THE PIXEL DIMENSION IN KM IS EMBEDDED IN BSCALE
	THE BASE VALUES ARE: RADIUS EARTH= 6371.228 KM
			     PIXEL DIMEN =25.067525 KM
	THEN, BSCALE = BASE_PIXEL_DIMEN
	      ASCALE = RADIUS_EARTH/BASE_PIXEL_DIMEN

*/
   double pi2=1.57079633;

   switch (iopt) {
    case 11:   /* EASE grid north */
      *thelon = ascale*sin(alon*dtr)*sin((45.0-0.5*alat)*dtr);
      *thelat =-ascale*cos(alon*dtr)*sin((45.0-0.5*alat)*dtr);
      break;
    case 12:   /* EASE grid south */
      *thelon = ascale*sin(alon*dtr)*cos((45.0-0.5*alat)*dtr);
      *thelat = ascale*cos(alon*dtr)*cos((45.0-0.5*alat)*dtr);
      break;
    case 13:   /* EASE cylindrical */
      *thelon = ascale * pi2 * alon * cos(30.0 * dtr)/90.0;
      *thelat = ascale * sin(alat * dtr) / cos(30.0 * dtr);
   }
   return;
}

void ieasegrid(int iopt, float *alon, float *alat, 
	       float thelon, float thelat, float ascale)
{
/*
	COMPUTE THE INVERSE "EASE" GRID TRANSFORM

	GIVEN THE IMAGE TRANSFORMATION COORDINATES (THELON,THELAT) AND
	THE SCALE (ASCALE) THE CORRESPONDING LON,LAT (ALON,ALAT) IS COMPUTED
	USING THE "EASE GRID" (VERSION 1.0) TRANSFORMATION GIVEN IN FORTRAN
	SOURCE CODE SUPPLIED BY NSIDC

	IOPT IS EASE TYPE: IOPT=11=NORTH, IOPT=12=SOUTH, IOPT=13=CYLINDRICAL

	THE RADIUS OF THE EARTH USED IN THIS PROJECTION IS EMBEDDED INTO
	ASCALE WHILE THE PIXEL DIMENSION IN KM IS EMBEDDED IN BSCALE
	THE BASE VALUES ARE: RADIUS EARTH= 6371.228 KM
			     PIXEL DIMEN =25.067525 KM
	THEN, BSCALE = BASE_PIXEL_DIMEN
	      ASCALE = RADIUS_EARTH/BASE_PIXEL_DIMEN

*/
   double pi2=1.57079633;
   double x1, y1, temp;

   x1 = thelon;
   y1 = thelat;
   switch (iopt) {
    case 11:   /* EASE grid north */
      *alon = arctand( x1, -y1);
      if (abs(sin(*alon*dtr)) > abs(cos(*alon*dtr)))
	 temp = (x1/sin(dtr* *alon))/ ascale;
      else
	 temp = (-y1/cos(dtr* *alon))/ ascale;
      if (abs(temp) <= 1.0)
	 *alat = 90.0 - 2.0 * asin(temp)*RAD2DEG;
      else {
	 *alat = -90.0 + 2.0 * asin(1./temp)*RAD2DEG;
	 /*
	 fprintf(stderr,"*** ERROR in EASE inverse sine ***\n");
	 fflush(stderr);
	 */
	 *alat = (temp > 0.0 ? 90.0 : -90.0);
      }
      break;
    case 12:   /* EASE grid south */
      *alon = arctand( x1, y1);
      if (abs(cos(*alon*dtr)) > abs(sin(*alon*dtr)))
	 temp = (y1/cos(dtr* *alon))/ ascale;
      else
	 temp = (x1/sin(dtr* *alon))/ ascale;
      if (abs(temp) <= 1.0)
	 *alat = 90.0 - 2.0 * acos(temp)*RAD2DEG;
      else {
	/*
	 fprintf(stderr,"*** ERROR in EASE inverse cosine ***\n");
	 fflush(stderr);
	*/
	 *alat = 0.0;
      }
      break;
    case 13:   /* EASE cylindrical */
      *alon = ((x1/ ascale)/cos(dtr*30.0))*90.0/pi2;
      temp = (y1 * cos(dtr*30.0))/ ascale;
      if (abs(temp) <= 1.0)
	 *alat = asin(temp)*RAD2DEG;
      else {
	/*
	 fprintf(stderr,"*** ERROR in EASE inverse sine ***\n");
	 fflush(stderr);
	*/
	 *alat = (temp > 0.0 ? 90.0 : -90.0);
      }
   }
   return;
}

#define DTR 0.01745329241994
#define RTD 57.29577951308232
/* set base EASE2 map projection information */
void ease2_map_info(int iopt, int isc, int ind, 
		    double *map_equatorial_radius_m, double *map_eccentricity, 
		    double *e2, double *map_reference_latitude, 
		    double *map_reference_longitude, 
		    double *map_second_reference_latitude,double * sin_phi1, 
		    double *cos_phi1, double *kz,
		    double *map_scale, int *bcols, int *brows, 
		    double *r0, double *s0, double *epsilon)
{ /* define key EASE2 grid information 

  inputs
    iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
    isc:  scale factor 0..5 grid size is (basesize(ind))/2^isc
    ind:  base grid size index   (map units per cell in m
 
          NSIDC .grd file for isc=0
           project type    ind=0     ind=1         ind=3
	      N         EASE2_N25km EASE2_N30km EASE2_N36km  
              S         EASE2_S25km EASE2_S30km EASE2_S36km 
              T/M       EASE2_T25km EASE2_M25km EASE2_M36km 

          cell size (m) for isc=0 (scale is reduced by 2^isc)
           project type    ind=0     ind=1            ind=3
	      N          25000.0     30000.0         36000.0
              S          25000.0     30000.0         36000.0
              T/M       T25025.26   M25025.2600081  M36032.220840584
	      
	  for a given base cell size isc is related to NSIDC .grd file names
	     isc        N .grd name   S .grd name   T .grd name
	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  

  outputs
    map_equatorial_radius_m  EASE2 Earth equitorial radius (km) [WGS84]
    map_eccentricity         EASE2 Earth eccentricity [WGS84]
    map_reference_latitude   Reference latitude (deg) 
    map_reference_longitude  Reference longitude (deg)
    map_second_reference_latitude Secondary reference longitude* (deg)
    sin_phi1, cos_phi1 kz    EASE2 Cylin parameters*
    map_scale                EASE2 map projection pixel size (km)
    bcols, brows,            EASE2 grid size in pixels
    r0, s0                   EASE2 base projection size in pixels
    epsilon                  EASE2 near-polar test factor

    *these parameters only assigned values if projection is T

  */

  double base;  
  int m, nx, ny;

  *map_equatorial_radius_m = 6378137.0 ; /* WGS84 */
  *map_eccentricity = 0.081819190843 ;   /* WGS84 */
  *e2 = *map_eccentricity *  *map_eccentricity;
  *map_reference_longitude = 0.0;
  *epsilon = 1.e-6;

  /* map-specific parameters */
  switch (iopt) {
    case 8:   /* EASE2 grid north */
      *map_reference_latitude = 90.0;
      switch(ind) {
      case 1:  /* EASE2_N30km.gpd */
	base=30000.0;
	nx=600;
	ny=600;	
	break;
      case 2:  /* EASE2_N36km.gpd */
	base=36000.0;      
	nx=500;
	ny=500;	
	break;
      default: /* EASE2_N25km.gpd */
	base=25000.0;
	nx=720;
	ny=720;	
      }
      break;
    case 9:   /* EASE2 grid south */
      *map_reference_latitude = -90.0;
      switch(ind) {
      case 1:  /* EASE2_S30km.gpd */
	base=30000.0;
	nx=600;
	ny=600;	
	break;
      case 2:  /* EASE2_S36km.gpd */
	base=36000.0;      
	nx=500;
	ny=500;	
	break;
      default: /* EASE2_S25km.gpd */
	base=25000.0;
	nx=720;
	ny=720;	
      }
      break;
    case 10:  /* EASE2 cylindrical */
      *map_reference_latitude = 0.0;
      *map_second_reference_latitude = 30.0;
      *sin_phi1 = sin( DTR * *map_second_reference_latitude );
      *cos_phi1 = cos( DTR * *map_second_reference_latitude );
      *kz = *cos_phi1 / sqrt( 1.0 - *e2 * *sin_phi1 * *sin_phi1 );
      switch(ind) {
      case 1:  /* EASE2_M25km.gpd */
	base=25025.2600081;
	nx=1388;
	ny=584;	
	break;
      case 2:  /* EASE2_M36km.gpd */
	base=36032.220840584;
	nx=964;
	ny=406;	
	break;
      default: /* EASE2_T25km.gpd */
	base=25025.26000;
	nx=1388;
	ny=540;  /* originally was 538 */	
      }
   }

  
  /* grid info */
  if (isc>=0) {    
    for (m=1; isc>0; isc--) m *= 2;  /* compute power-law scale factor */
    *map_scale = base / (double) m;
    *bcols = nx * m;
    *brows = ny * m;
    *r0 = ((double) (*bcols - 1)) / 2.0;
    *s0 = ((double) (*brows - 1)) / 2.0;
  } else {
    for (m=1; isc<0; isc++) m *= 2;  /* compute power-law scale factor */
    *map_scale = base * (double) m;
    *bcols = dceil( nx / (double) m);    
    *brows = dceil( ny / (double) m);
    /* note: the following computation ensures that the lower-left corner
       remains at the same location even if ny/m is a non-integer. */
    *r0 = (nx / (double) m - 1.0) / 2.0;
    *s0 = (ny / (double) m - 1.0) / 2.0;    
  }
  
}

double easeconv_normalize_degrees(double b)
{ /* return -180 <= b <= 180.0 */
  while (b < -180.0)
    b = b + 360.0;
  while (b > 180.0)
    b = b - 360.0;
  return(b);
}

void ease2sf(int iopt, float ascale, float bscale, float *fmap_scale, 
	     int *bcols, int *brows, float *fr0, float *fs0)
{
  /* return selected EASE2 grid information (external interface)

  inputs
    iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
    (ascale and bscale should be integer valued)
    ascale: grid scale factor (0..5)  pixel size is (bscale/2^ascale)
    bscale: base grid scale index (ind=int(bscale))
 
          NSIDC .grd file for isc=0
           project type    ind=0     ind=1         ind=3
	      N         EASE2_N25km EASE2_N30km EASE2_N36km  
              S         EASE2_S25km EASE2_S30km EASE2_S36km 
              T/M       EASE2_T25km EASE2_M25km EASE2_M36km 

          cell size (m) for isc=0 (scale is reduced by 2^isc)
           project type    ind=0     ind=1            ind=3
	      N          25000.0     30000.0         36000.0
              S          25000.0     30000.0         36000.0
              T/M       T25025.26   M25025.2600081  M36032.220840584
	      
	  for a given base cell size isc is related to NSIDC .grd file names
	     isc        N .grd name   S .grd name   T .grd name
	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  

  outputs
    fmap_scale               EASE2 map projection pixel size (km)
    bcols, brows,            EASE2 grid size in pixels
    fr0, fs0                 EASE2 base projection size in pixels
  */

   double map_equatorial_radius_m,map_eccentricity, e2,
     map_reference_latitude, map_reference_longitude, 
     map_second_reference_latitude, sin_phi1, cos_phi1, kz,
     map_scale, r0, s0, epsilon;

   int ind = intfix(bscale);
   int isc = intfix(ascale);
    
   /* get base EASE2 map projection parameters */
   ease2_map_info(iopt, isc, ind, &map_equatorial_radius_m, &map_eccentricity, 
		  &e2, &map_reference_latitude, &map_reference_longitude, 
		  &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		  &map_scale, bcols, brows, &r0, &s0, &epsilon);
   *fmap_scale=map_scale;
   *fr0=r0;
   *fs0=s0;   
}

void ease2grid(int iopt, float alon, float alat, 
	       float *thelon, float *thelat, float ascale, float bscale)
{
/*
	COMPUTE THE FORWARD "EASE2" GRID TRANSFORMATION

	GIVEN THE IMAGE TRANSFORMATION COORDINATES (THELON,THELAT) AND
	THE CORRESPONDING LON,LAT (ALON,ALAT) IS COMPUTED
	USING THE "EASE2 GRID" (VERSION 2.0) TRANSFORMATION GIVEN IN IDL
	SOURCE CODE SUPPLIED BY MJ BRODZIK
	RADIUS EARTH=6378.137 KM (WGS 84)
	MAP ECCENTRICITY=0.081819190843 (WGS84)

	inputs:
	  iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
	  alon, alat: lon, lat (deg) to convert (can be outside of image)
          ascale and bscale should be integer valued)
	  ascale: grid scale factor (0..5)  pixel size is (bscale/2^ascale)
	  bscale: base grid scale index (ind=int(bscale))

          NSIDC .grd file for isc=0
           project type    ind=0     ind=1         ind=3
	      N         EASE2_N25km EASE2_N30km EASE2_N36km  
              S         EASE2_S25km EASE2_S30km EASE2_S36km 
              T/M       EASE2_T25km EASE2_M25km EASE2_M36km 

          cell size (m) for isc=0 (scale is reduced by 2^isc)
           project type    ind=0     ind=1            ind=3
	      N          25000.0     30000.0         36000.0
              S          25000.0     30000.0         36000.0
              T/M       T25025.26   M25025.2600081  M36032.220840584
	      
	  for a given base cell size isc is related to NSIDC .grd file names
	     isc        N .grd name   S .grd name   T .grd name
	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  

	outputs:
	  thelon: X coordinate in pixels (can be outside of image)
	  thelat: Y coordinate in pixels (can be outside of image)

*/
   double map_equatorial_radius_m,map_eccentricity, e2,
     map_reference_latitude, map_reference_longitude, 
     map_second_reference_latitude, sin_phi1, cos_phi1, kz,
     map_scale, r0, s0, epsilon;
   int bcols, brows;

   int ind = intfix(bscale);
   int isc = intfix(ascale);
   double dlon = alon;
   double phi = DTR * alat;
   double lam = dlon;

   double sin_phi, q, qp, rho, x, y;
    
   /* get base EASE2 map projection parameters */
   ease2_map_info(iopt, isc, ind, &map_equatorial_radius_m, &map_eccentricity, 
		  &e2, &map_reference_latitude, &map_reference_longitude, 
		  &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		  &map_scale, &bcols, &brows, &r0, &s0, &epsilon);

   dlon = dlon - map_reference_longitude;    
   dlon = easeconv_normalize_degrees( dlon );
   lam = DTR * dlon;
    
   sin_phi=sin(phi);
   q = ( 1.0 - e2 ) * ( ( sin_phi / ( 1.0 - e2 * sin_phi * sin_phi ) ) 
                        - ( 1.0 / ( 2.0 * map_eccentricity ) ) 
                        * log( ( 1.0 - map_eccentricity * sin_phi ) 
                                / ( 1.0 + map_eccentricity * sin_phi ) ) );

   switch (iopt) {
    case 8:   /* EASE2 grid north */
      qp = 1.0 - ( ( 1.0 - e2 ) / ( 2.0 * map_eccentricity ) 
		   * log( ( 1.0 - map_eccentricity ) 
			  / ( 1.0 + map_eccentricity ) ) );
      if ( abs( qp - q ) < epsilon ) 
	rho = 0.0;
      else
	rho = map_equatorial_radius_m * sqrt( qp - q );
      x =  rho * sin( lam );
      y = -rho * cos( lam );
      break;
    case 9:   /* EASE2 grid south */
      qp = 1.0 - ( ( 1.0 - e2 ) / ( 2.0 * map_eccentricity ) 
		   * log( ( 1.0 - map_eccentricity ) 
			  / ( 1.0 + map_eccentricity ) ) );
      if ( abs( qp + q ) < epsilon ) 
	rho = 0.0;
      else
	rho = map_equatorial_radius_m * sqrt( qp + q );
      x = rho * sin( lam );
      y = rho * cos( lam );
      break;
    case 10:   /* EASE2 cylindrical */
      x =   map_equatorial_radius_m * kz * lam;
      y = ( map_equatorial_radius_m * q ) / ( 2.0 * kz );    
      break;
    default:
      fprintf(stderr,"*** invalid EASE2 projection specificaion %d in ease2grid\n",iopt);      
      break;      
   }

   *thelon = (float) (r0 + ( x / map_scale ) + 0.5); 
   //*thelat = (float) (s0 - ( y / map_scale ) + 0.5);  /* 0 at top (NSIDC) */
   *thelat = (float) (s0 + ( y / map_scale ) + 0.5);  /* 0 at bottom (BYU SIR files) */

   return;
}

void iease2grid(int iopt, float *alon, float *alat, 
		float thelon, float thelat, float ascale, float bscale)
{
/*
	COMPUTE THE INVERSE "EASE2" GRID TRANSFORM

	GIVEN THE IMAGE TRANSFORMATION COORDINATES (THELON,THELAT) AND
	THE CORRESPONDING LON,LAT (ALON,ALAT) IS COMPUTED
	USING THE "EASE GRID" (VERSION 2.0) TRANSFORMATION GIVEN IN IDL
	SOURCE CODE SUPPLIED BY MJ BRODZIK

	inputs:
	  iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
	  thelon: X coordinate in pixels (can be outside of image)
	  thelat: Y coordinate in pixels (can be outside of image)
          ascale and bscale should be integer valued)
	  ascale: grid scale factor (0..5)  pixel size is (bscale/2^ascale)
	  bscale: base grid scale index (ind=int(bscale))

          NSIDC .grd file for isc=0
           project type    ind=0     ind=1         ind=3
	      N         EASE2_N25km EASE2_N30km EASE2_N36km  
              S         EASE2_S25km EASE2_S30km EASE2_S36km 
              T/M       EASE2_T25km EASE2_M25km EASE2_M36km 

          cell size (m) for isc=0 (scale is reduced by 2^isc)
           project type    ind=0     ind=1            ind=3
	      N          25000.0     30000.0         36000.0
              S          25000.0     30000.0         36000.0
              T/M       T25025.26   M25025.2600081  M36032.220840584
	      
	  for a given base cell size isc is related to NSIDC .grd file names
	     isc        N .grd name   S .grd name   T .grd name
	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  

	outputs:
	  alon, alat: lon, lat location in deg  (can be outside of image)
*/
   double map_equatorial_radius_m,map_eccentricity, e2,
     map_reference_latitude, map_reference_longitude, 
     map_second_reference_latitude, sin_phi1, cos_phi1, kz,
     map_scale, r0, s0, epsilon;
   int bcols, brows;

   int ind = intfix(bscale);
   int isc = intfix(ascale);

   double lam, arg, phi, beta, qp, rho2, x, y, e4, e6;
    
   /* get base EASE2 map projection parameters */
   ease2_map_info(iopt, isc, ind, &map_equatorial_radius_m, &map_eccentricity, 
		  &e2, &map_reference_latitude, &map_reference_longitude, 
		  &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		  &map_scale, &bcols, &brows, &r0, &s0, &epsilon);
   e4 = e2 * e2;
   e6 = e4 * e2;

   /* qp is the function q evaluated at phi = 90.0 deg */
   qp = ( 1.0 - e2 ) * ( ( 1.0 / ( 1.0 - e2 ) ) 
			 - ( 1.0 / ( 2.0 * map_eccentricity ) ) 
			 * log( ( 1.0 - map_eccentricity ) 
                                / ( 1.0 + map_eccentricity ) ) );

   x = ((double) thelon - r0 - 0.5) * map_scale;
   //y = (s0 - (double) thelat + 0.5) * map_scale;  /* 0 at top (NSIDC) */
   y = ((double) thelat - 0.5 - s0) * map_scale;  /* 0 at bottom (BYU SIR files) */

   switch (iopt) {
    case 8:   /* EASE2 grid north */
      rho2 = ( x * x ) + ( y * y );
      arg=1.0 - ( rho2 / ( map_equatorial_radius_m * map_equatorial_radius_m * qp ) );
      if (arg >  1.0) arg=1.0;      
      if (arg < -1.0) arg=-1.0;
      beta = asin( arg );
      lam = atan2( x, -y );
      break;
    case 9:   /* EASE2 grid south */
      rho2 = ( x * x ) + ( y * y );
      arg = 1.0 - ( rho2  / ( map_equatorial_radius_m * map_equatorial_radius_m * qp ) );
      if (arg >  1.0) arg=1.0;      
      if (arg < -1.0) arg=-1.0;
      beta = -asin( arg );
      lam = atan2( x, y );
      break;
    case 10:  /* EASE2 cylindrical */
      arg = 2.0 * y * kz / ( map_equatorial_radius_m * qp );
      if (arg >  1.0) arg=1.0;      
      if (arg < -1.0) arg=-1.0;
      beta = asin( arg );
      lam = x / ( map_equatorial_radius_m * kz );
      break;
    default:
      fprintf(stderr,"*** invalid EASE2 projection specificaion %d in iease2grid\n",iopt);      
      break;      
   }

   phi = beta 
     + ( ( ( e2 / 3.0 ) + ( ( 31.0 / 180.0 ) * e4 ) 
	   + ( ( 517.0 / 5040.0 ) * e6 ) ) * sin( 2.0 * beta ) ) 
     + ( ( ( ( 23.0 / 360.0 ) * e4) 
	   + ( ( 251.0 / 3780.0 ) * e6 ) ) * sin( 4.0 * beta ) ) 
     + ( ( ( 761.0 / 45360.0 ) * e6 ) * sin( 6.0 * beta ) );
   
   *alat = (float) (RTD * phi);
   *alon = (float) (easeconv_normalize_degrees( map_reference_longitude + ( RTD*lam ) ) );

   return;
}

