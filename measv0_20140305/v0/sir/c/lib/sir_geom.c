/*
   standard C BYU SIR projection transformation routines

   DGL Dec. 19, 1998
   DGL Sept. 24, 1999 + fixed polster, latlon2pix, changed args to pixtolatlon
   DGL July 29, 2005 + modified and corrected EASE projection code
   
   Written in ANSI C.

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


/******* standard sir format geometric transformation routines *****/


void ilambert1(float*, float*, float, float, float, float, int);
void ipolster(float*, float*, float, float, float, float); 
void ieasegrid(int, float*, float*, float, float, float);

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
	SOURCE CODE SUPPLIED BY ...

	IOPT IS EASE TYPE: IOPT=11=NORTH, IOPT=12=SOUTH, IOPT=13=CYLINDRICAL

	THE RADIUS OF THE EARTH USED IN THIS PROJECTION IS IMBEDDED INTO
	ASCALE WHILE THE PIXEL DIMENSION IN KM IS IMBEDDED IN BSCALE
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
	SOURCE CODE SUPPLIED BY ...

	IOPT IS EASE TYPE: IOPT=11=NORTH, IOPT=12=SOUTH, IOPT=13=CYLINDRICAL

	THE RADIUS OF THE EARTH USED IN THIS PROJECTION IS IMBEDDED INTO
	ASCALE WHILE THE PIXEL DIMENSION IN KM IS IMBEDDED IN BSCALE
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
