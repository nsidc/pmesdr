/**************************************************************************

 Program to generate a netCDF-formatted file from a BYU .SIR file

 written by D.G. Long  05 Sep 2009 at BYU 
 revised by D.G. Long  03 Mar 2014 at BYU
 link to netCDF and SIR libraries

(it is recognized that the code is a bit awkwardly [i.e. poorly] 
 written in places in order to produce the NSIDC-desired netcdf header)

 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <netcdf.h>  /* netCDF interface */
#include "sir_ez.h"  /* easy sir routine interface */

#define dtr (double) 0.017453292
#define RAD2DEG (double) 57.295779
#define abs(a) ((a) > 0.0 ? (a) : - (a))

void polster1( float alon, float alat, float *x1, float *y1, 
	       float xlam, float slat)
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

double arctand(double y, double x);

void ipolster1(float *alon, float *alat, float x, float y, 
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
   static double e2=0.006693883;
   static double re=6378.273;
   static double pi2=1.570796327;

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
   polster1(*alon,*alat,&xx,&yy,xlam,slat);
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


#define FLOOR(x) (int) ((float) (floor((double)(x))))

void caldat(int julian, int *mm, int *id, int *iyyy)
{
  /* given julian day, returns output month, day, year */

  int IGREG=2299161;
  int jalpha, ja,jb,jc,jd,je;

  if (julian >= IGREG) {
    jalpha=FLOOR(((julian-1867216)-0.25)/36524.25);
    ja=julian+1+jalpha-FLOOR(0.25*jalpha);
  } else
    ja=julian;

  jb=ja+1524;
  jc=FLOOR(6680.+((jb-2439870)-122.1)/365.25);
  jd=365*jc+FLOOR(0.25*jc);
  je=FLOOR((jb-jd)/30.6001);
  *id=jb-jd-FLOOR(30.6001*je);
  *mm=je-1;
  if (*mm > 12)
    *mm=*mm-12;
  *iyyy=jc-4715;
  if (*mm > 2)
    *iyyy=*iyyy-1;
  if (*iyyy <= 0)
    *iyyy=*iyyy-1;

  return;
}

int julday(int mm, int id, int iyyy) {
  /* returns the Julian day number that begins on noon of the calendar                         
     date specifed by month mm, day id and year iyy */
  int julday, IGREG=15+31*(10+12*1582);
  int jy=iyyy, jm=1, ja;

  if (jy <0)
    jy=jy+1;
  if (mm > 2)
    jm=mm+1;
  else {
    jy=jy-1;
    jm=mm+13;
  }
  julday=(int) FLOOR(365.25*jy)+(int) FLOOR(30.6001*jm)+id+1720995;
  if (id+31*(mm+12*iyyy) >= IGREG) {
    ja=FLOOR(0.01*jy);
    julday=julday+2-ja+FLOOR(0.25*ja);
  }
  return(julday);
}

void jdt2string(double time, char *out) {
  int jd=(int) time, imm, id, iyy, imin, ihr, isec;
  
  caldat(jd, &imm, &id, &iyy);
  time=time-jd;
  ihr=time*24;
  imin=(time*24-ihr)*60;
  isec=(time*24*60-ihr*60)*60;  
  sprintf(out,"%4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2dZ",iyy,id,imm,ihr,imin,isec);
  
  return;  
}


void check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
	   (void) fprintf(stderr, "line %d of %s: %s\n", line, file, nc_strerror(stat));
        exit(1);
    }
}

int main(int argc, char **argv) {

   int  stat;			/* return status */
   int  ncid;			/* netCDF id */

   /* dimension ids */
   int cols_dim;
   int rows_dim;
   int time_vertices_dim;
   int times_dim;

   /* dimension lengths */
   size_t pfac = 6;   
   size_t cols_len = 244*pfac;
   size_t rows_len = 244*pfac;
   size_t time_vertices_len = 2;
   size_t times_len = 1;

   /* variable ids */
   int projection_id;
   int sigmao_id;
   int time_vertices_id;
   int times_id;
   int latitude_id;
   int longitude_id;
   int rows_id;
   int cols_id;
   int ygrid_id;
   int xgrid_id;

   /* rank (number of dimensions) for each variable */
#  define RANK_projection 0
   //#  define RANK_sigmao 2
#  define RANK_sigmao 3
#  define RANK_time_vertices 2
#  define RANK_times 1
#  define RANK_latitude 2
#  define RANK_longitude 2
#  define RANK_rows 1
#  define RANK_cols 1

   /* variable shapes */
   int sigmao_dims[RANK_sigmao];
   int times_dims[RANK_times];
   int latitude_dims[RANK_latitude];
   int longitude_dims[RANK_longitude];
   int rows_dims[RANK_rows];
   int cols_dims[RANK_cols];
   int time_vertices_dims[RANK_time_vertices];

   /* attribute vectors */
   double grid_boundary_top_projected_y[1];
   double grid_boundary_bottom_projected_y[1];
   double grid_boundary_right_projected_x[1];
   double grid_boundary_left_projected_x[1];
   double latitude_of_projection_origin[1];
   double latitude_of_true_scale[1];   
   double longitude_of_projection_origin[1];
   double straight_vertical_longitude_from_pole[1]; 
   double scaling_factor[1];
   double false_easting[1];
   double false_northing[1];
   double semimajor_radius[1];
   double f[1];
   double semiminor_radius[1];
   float sigmao__FillValue[1];
   double latitude_valid_range[2];
   double latitude__FillValue[1];
   double longitude_valid_range[2];
   double longitude__FillValue[1];
   int rows_valid_range[2];
   int cols_valid_range[2];
   double xgrid_valid_range[2];
   double ygrid_valid_range[2];
   int eggslice=0; /* slice if 1, egg if 0 */

   double times[1];  /* to store average of start and stop time */
   double times_2[2];  /* to store start and stop time */

   //char in_name[] = {"qusv-a-Arc07-152-152.sir"};
   //char in_name[] = {"quev-a-Arc07-052-052.sir"};
   
   char in_name[512];
   char out_name[256];
   char filename[256], line[256], long_name[140], gstring[120],
     title[140], history[140], *ss, pline[512], proj4text[200];
   char units_description[]={"unitless normalized radar cross-section stored as dB=10*log10(*)"};
   //char *string_2[2]={"yyyy-mm-ddThh:mm:ssZ","yyyy-mm-ddThh:mm:ssZ"};
   char *string_2[2];   
   char workstring[256];

   sir_head head;
   float *stval;
   int ix, iy, ix1, iy1, i, j, ierr, iopt=2;
   float ascale,bscale,a0,b0,lonp,latp;
   float x1,y1,xp1,yp1,orglat,orglon;
   double xp, yp;
   int spatialalg=0;
   double gres=0.0174532925199433/(double) pfac;

   char crtime[28];
   time_t tod;

   char local[]="./";
   char *outpath=NULL;

   printf("sir2cdf2hem\n");
   if (argc < 2) {
     printf("Usage: %s sir_file.sir outpath\n",argv[0]);
     printf(" converts a polar stereographic .sir file to NSIDC netdef form\n");
     printf(" output is name is outpath/sir_file.nc\n");
     exit(1);     
   }
   strcpy(in_name,argv[1]);

   if (argv[2] != NULL)
     outpath=argv[2];
   else
     outpath=local; 

   string_2[0]=(char *) malloc(22);
   string_2[1]=(char *) malloc(22);
   strcpy(string_2[0],"yyyy-mm-ddThh:mm:ssZ");
   strcpy(string_2[1],"yyyy-mm-ddThh:mm:ssZ");

  /* Read sir input file */
   sir_init_head(&head);
   printf("Reading SIR file %s\n",in_name);
   ierr = get_sir(in_name, &head, &stval);
   if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR header from file '%s'\n",in_name);
     exit(-1);
   }
   sir_update_head(&head, stval);  /* fix old-style header information */

   /* process full path input name */
   strcpy(filename,in_name);
   while (ss=strstr(filename,"/")) {  /* strip off path info from input*/
      strcpy(line,++ss);
      strcpy(filename,line);
    }

   /* based on input name create header lines */
   if (filename[0]='q') {     /* QuikSCAT */
     if (filename[1]=='u')
       strcpy(title,"QuikSCAT Scatterometer Climate Record Pathfinder Daily Backscatter Value");
     else if (filename[1]=='m')
       strcpy(title,"QuikSCAT Scatterometer Climate Record Pathfinder Local-Time-of-Day (Morning) Backscatter Value");
     else
       strcpy(title,"QuikSCAT Scatterometer Climate Record Pathfinder Local-Time-of-Day (Evening) Backscatter Value");
     if (filename[2]=='s')
       if (head.ipol==2)
	 strcpy(long_name,"QuikSCAT Ku-band normalized radar backscatter (slices) in dB, vertical polarization");
       else
	 strcpy(long_name,"QuikSCAT Ku-band normalized radar backscatter (slices) in dB, horizontal polarization");
     else
       if (head.ipol==2)
	 strcpy(long_name,"QuikSCAT Ku-band normalized radar backscatter (eggs) in dB, vertical polarization");
       else
	 strcpy(long_name,"QuikSCAT Ku-band normalized radar backscatter (eggs) in dB, horizontal polarization");
   } else if (filename[0]=='F') {  /* SSM/I */
     strcpy(title,"SSM/I Scatterometer Climate Record Pathfinder Daily TB");
   }   

   i=julday(1, 1, head.iyear)+head.isday-1;
   j=julday(1, 1, head.iyear)+head.ieday-1;
   x1=i+((float) head.ismin)/(24.*60.);
   y1=j+((float) head.iemin)/(24.*60.);
   times[0]=0.5*(x1+y1)-julday(1,1,1601); // mean time
   times_2[0]=i+((float) head.ismin)/(24.*60.)-julday(1,1,1601);        // start time
   times_2[1]=j+((float) head.iemin)/(24.*60.)-julday(1,1,1601);        // end time
   printf("Data ave=%f start=%lf stop=%lf time  (%d %d mins)\n",times[0],times_2[0],times_2[1],head.ismin,head.iemin);
    
   (void) time(&tod);
   (void) strftime(crtime,28,"%c",localtime(&tod));
   printf("Current time: '%s'\n",crtime);
   sprintf(history,"%s netCDF File created to NSIDC specifications.",crtime);


   /* generate output file name */
   strcpy(line,filename);
   if (ss=strstr(line,".sir")) {  /* strip off .sir from name */
     //     *ss='\0';
     spatialalg=2;
   }
   if (ss=strstr(line,".ave")) {  /* strip off .ave from name */
     // *ss='\0';
     spatialalg=1;
   }
   if (ss=strstr(line,".grd")) {  /* strip off .grd from name */
     //*ss='\0';
     spatialalg=3;
     printf("*** GRD images not supported by this code ***\n");
     exit(-1);     
   }
   if (ss=strstr(line,".non")) {  /* strip off .non from name */
     //*ss='\0';
     spatialalg=4;
   }
   sprintf(out_name,"%s/%s.nc",outpath,line);
   printf("Output file: %s\n",out_name);

   /* set egg or slice flag */
   if (filename[2]=='s') {     
     sprintf(line,"slices");
     eggslice=1;
   } else {
     sprintf(line,"eggs");
     eggslice=0;
   }

   /* set transformation information for BYU SIR polarstereographic */
   scaling_factor[0] = 1.0;
   false_easting[0] = 0.;
   false_northing[0] = 0.;

   switch (head.iopt) {
   case 5:  // Polar stereographic
     if (eggslice == 1) // QuikSCAT slices
       gres=1.0/2225.0;  // pixels/m
     else               // QuikSCAT eggs, ASCAT slices
       gres=1.0/4450.0;   // pixels/m    
     semimajor_radius[0] = 6378273.0;
     //f[0]=298.780;   
     //semiminor_radius[0] = semimajor_radius[0] * (1.0 - 1.0/f[0]);
     f[0] = 2.0/0.006693883;
     semiminor_radius[0] = semimajor_radius[0] * sqrt(1.0 - 0.006693883);     
     /* use exact values rather than rounded values from header */
     if (head.iregion == 100) { // Antarctic
       latitude_of_projection_origin[0] = -90.0;
       latitude_of_true_scale[0] = -70.0;
       longitude_of_projection_origin[0] = 0.0;

       printf("*** CAUTION: Ant not fully tested yet!\n");
       xgrid_valid_range[0] = head.a0*1000.0;
       xgrid_valid_range[1] = round((head.a0+head.nsx*head.ascale)*1000.0);
       ygrid_valid_range[1] = head.b0*1000.0;
       ygrid_valid_range[0] = round((head.b0+head.nsy*head.bscale)*1000.0);

       sprintf(pline,"PROJCS[\"Stereographic_South_Pole\",GEOGCS[\"unnamed ellipse\",DATUM[\"D_unknown\",SPHEROID[\"Unknown\",%11.3lf,%11.7lf]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",%18.16lf]],PROJECTION[\"Stereographic_South_Pole\"],PARAMETER[\"standard_parallel_1\",%lf],PARAMETER[\"central_meridian\",%lf],PARAMETER[\"scale_factor\",%1lf],PARAMETER[\"false_easting\",%1lf],PARAMETER[\"false_northing\",%1lf],UNIT[\"Meter\",1, AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"3411\"]]",semimajor_radius[0],f[0],gres,latitude_of_true_scale[0],longitude_of_projection_origin[0],scaling_factor[0],false_easting[0],false_northing[0]);

     } else if (head.iregion == 110) { // Arctic
       latitude_of_projection_origin[0] = 90.0;
       latitude_of_true_scale[0] = 70.0;
       longitude_of_projection_origin[0] = -45.0;

       //printf("a0=%f b0=%f %f %f\n",head.a0*1000.,head.b0*1000.,(head.a0+head.nsx*head.ascale)*1000.0,(head.b0+head.nsy*head.bscale)*1000.0);
       
       xgrid_valid_range[0] = head.a0*1000.0;
       xgrid_valid_range[1] = round((head.a0+head.nsx*head.ascale)*1000.0);
       ygrid_valid_range[1] = head.b0*1000.;
       ygrid_valid_range[0] = round((head.b0+head.nsy*head.bscale)*1000.0);
      
     //if (eggslice == 1) { // slices
     //	 xgrid_valid_range[0] = -3400000.000000;
     //	 xgrid_valid_range[1] = 3406275.000000+2225.0;
     //	 ygrid_valid_range[0] = -3400000.000000;
     //	 ygrid_valid_range[1] = 3406275.000000+2225.0;
     //} else { // eggs
     //	 xgrid_valid_range[0] = -3400000.000000;
     //	 xgrid_valid_range[1] = 3404049.750000+4450.0;
     //	 ygrid_valid_range[0] = -3400000.000000;
     //	 ygrid_valid_range[1] = 3404049.750000+4450.0;
     //}

       sprintf(pline,"PROJCS[\"Stereographic_North_Pole\",GEOGCS[\"unnamed ellipse\",DATUM[\"D_unknown\",SPHEROID[\"Unknown\",%11.3lf,%11.7lf]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",%18.16lf]],PROJECTION[\"Stereographic_North_Pole\"],PARAMETER[\"standard_parallel_1\",%8.2lf],PARAMETER[\"central_meridian\",%8.2lf],PARAMETER[\"scale_factor\",%2.0lf],PARAMETER[\"false_easting\",%2.0lf],PARAMETER[\"false_northing\",%2.0lf],UNIT[\"Meter\",1, AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"3411\"]]",semimajor_radius[0],f[0],gres,latitude_of_true_scale[0],longitude_of_projection_origin[0],scaling_factor[0],false_easting[0],false_northing[0]);

     } else if (head.iregion == 112) { // NHe
       latitude_of_projection_origin[0] = 90.0;
       latitude_of_true_scale[0] = 70.0;
       longitude_of_projection_origin[0] = -45.0;

       xgrid_valid_range[0] = head.a0*1000.0;
       xgrid_valid_range[1] = round((head.a0+head.nsx*head.ascale)*1000.0);
       ygrid_valid_range[1] = head.b0*1000.;
       ygrid_valid_range[0] = round((head.b0+head.nsy*head.bscale)*1000.0);
       sprintf(pline,"PROJCS[\"Stereographic_North_Pole\",GEOGCS[\"unnamed ellipse\",DATUM[\"D_unknown\",SPHEROID[\"Unknown\",%11.3lf,%11.7lf]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",%18.16lf]],PROJECTION[\"Stereographic_North_Pole\"],PARAMETER[\"standard_parallel_1\",%8.2lf],PARAMETER[\"central_meridian\",%8.2lf],PARAMETER[\"scale_factor\",%2.0lf],PARAMETER[\"false_easting\",%2.0lf],PARAMETER[\"false_northing\",%2.0lf],UNIT[\"Meter\",1, AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"3411\"]]",semimajor_radius[0],f[0],gres,latitude_of_true_scale[0],longitude_of_projection_origin[0],scaling_factor[0],false_easting[0],false_northing[0]);

       printf("*** CAUTION: NHe not fully tested yet!\n");

     } else {
       printf("*** This SIR polar stereographic projection region is not supported by this code %d\n",head.iregion);
       exit(-1);
     }

     //sprintf(proj4text,"+datum=wgs84 +proj=stere +lat_0=70.000000 +lon_0=-45.000000");
     sprintf(proj4text,"+proj=stere +lat_0=%lf +lat_ts=%lf +lon_0=%lf +k=1 +x_0=0 +y_0=0 +a=%11.3lf +b=%11.3lf +units=m +no_defs",latitude_of_projection_origin[0],latitude_of_true_scale[0],longitude_of_projection_origin[0],semimajor_radius[0],semiminor_radius[0]);


     break;

   case -1:  // no projection
   case 0:   // Lat/lon grid
   case 1:   // Lambert equal area fixed radius
     semimajor_radius[0] = 6378000.0;     
     f[0]=0.0;   
     semiminor_radius[0] = semimajor_radius[0];
     // fall through next case intented
   case 2:   // Lambert equal area local radius
     if (head.iopt == 2) {
       semimajor_radius[0] = 6378135.0;     
       f[0]=298.260;   
       semiminor_radius[0] = semimajor_radius[0] * (1.0 - 1.0/f[0]);
     }

     latitude_of_projection_origin[0] = head.ydeg;
     latitude_of_true_scale[0] = head.ydeg;
     longitude_of_projection_origin[0] = head.xdeg;

     xgrid_valid_range[0] = head.a0*1000.0;
     xgrid_valid_range[1] = round((head.a0+head.nsx/head.ascale)*1000.0);
     ygrid_valid_range[0] = head.b0*1000.;
     ygrid_valid_range[1] = round((head.b0+head.nsy/head.bscale)*1000.0);

     sprintf(pline,"PROJCS[\"Lambert Equal Area\",GEOGCS[\"unnamed ellipse\",DATUM[\"D_unknown\",SPHEROID[\"Unknown\",%11.3lf,%7.3lf]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",%18.16lf]],PROJECTION[\"Lambert_Equal_Area\"],PARAMETER[\"central_longitude\",%8.2lf],PARAMETER[\"central_meridian\",%8.2lf],PARAMETER[\"scale_factor\",%2.0lf],PARAMETER[\"false_easting\",%2.0lf],PARAMETER[\"false_northing\",%2.0lf],UNIT[\"Meter\",1, AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"3411\"]]",semimajor_radius[0],f[0],gres,latitude_of_projection_origin[0],longitude_of_projection_origin[0],scaling_factor[0],false_easting[0],false_northing[0]);

     printf("*** CAUTION: Lambert images not fully tested yet!\n");

     sprintf(proj4text,"+proj=laea +lat_0=%lf +lon_0=%lf +k=1 +x_0=0 +y_0=0 +a=%11.3lf +rf=%11.3lf +units=m +no_defs -a_ullr %11.3lf %11.3lf %11.3lf %11.3lf",latitude_of_projection_origin[0],longitude_of_projection_origin[0],semimajor_radius[0],50000000.0,xgrid_valid_range[0],ygrid_valid_range[1],xgrid_valid_range[1],ygrid_valid_range[1]);
     
     break;

   case 11: // EASE grid north
   case 12: // EASE grid south
   case 13: // EASE grid cylindrical
   default:
     printf("*** This SIR projection is not supported by this code %d\n",head.iopt);
     exit(-1);
   }
   
   /* set grid boundaries and GeoTransform */
   grid_boundary_top_projected_y[0] = ygrid_valid_range[0];
   grid_boundary_bottom_projected_y[0] = ygrid_valid_range[1];
   grid_boundary_right_projected_x[0]= xgrid_valid_range[1];   
   grid_boundary_left_projected_x[0] = xgrid_valid_range[0];

   sprintf(gstring,"%lf %lf 0 %lf 0 %lf",grid_boundary_left_projected_x[0],1./gres,grid_boundary_top_projected_y[0],-1./gres);

   
   /* output image size same as the input image size */
   cols_len = head.nsx;
   rows_len = head.nsy;

   printf("Output image size %d X %d\n",cols_len,rows_len);   

   double xgrid[cols_len];	
   double ygrid[rows_len];

   printf("From %s create %s\n",in_name,out_name);
   printf("Output image size %d X %d\n",cols_len,rows_len);

   /* create output netcdf file and enter define mode to add header information */
   stat = nc_create(out_name, NC_CLOBBER, &ncid);
   check_err(stat,__LINE__,__FILE__);

   /* define dimensions */
   stat = nc_def_dim(ncid, "xgrid", cols_len, &cols_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "ygrid", rows_len, &rows_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "time_vertices", time_vertices_len, &time_vertices_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "time", times_len, &times_dim);
   check_err(stat,__LINE__,__FILE__);

   /* define variables */

   stat = nc_def_var(ncid, "projection", NC_CHAR, RANK_projection, 0, &projection_id);
   check_err(stat,__LINE__,__FILE__);

   //sigmao_dims[0] = rows_dim;
   //sigmao_dims[1] = cols_dim;
   sigmao_dims[0] = times_dim;
   sigmao_dims[1] = rows_dim;
   sigmao_dims[2] = cols_dim;
   if (filename[0]='q')
     stat = nc_def_var(ncid, "sigmao", NC_FLOAT, RANK_sigmao, sigmao_dims, &sigmao_id);
   else
     stat = nc_def_var(ncid, "TB", NC_FLOAT, RANK_sigmao, sigmao_dims, &sigmao_id);
   check_err(stat,__LINE__,__FILE__);

   times_dims[0] = times_dim;
   stat = nc_def_var(ncid, "time", NC_DOUBLE, RANK_times, times_dims, &times_id);
   check_err(stat,__LINE__,__FILE__);

   time_vertices_dims[0] = times_dim;
   time_vertices_dims[1] = time_vertices_dim;
   stat = nc_def_var(ncid, "time_bounds", NC_DOUBLE, RANK_time_vertices, time_vertices_dims, &time_vertices_id);
   check_err(stat,__LINE__,__FILE__);

   latitude_dims[0] = rows_dim;
   latitude_dims[1] = cols_dim;
   stat = nc_def_var(ncid, "latitude", NC_DOUBLE, RANK_latitude, latitude_dims, &latitude_id);
   check_err(stat,__LINE__,__FILE__);

   longitude_dims[0] = rows_dim;
   longitude_dims[1] = cols_dim;
   stat = nc_def_var(ncid, "longitude", NC_DOUBLE, RANK_longitude, longitude_dims, &longitude_id);
   check_err(stat,__LINE__,__FILE__);

   rows_dims[0] = rows_dim;
   //stat = nc_def_var(ncid, "rows", NC_INT, RANK_rows, rows_dims, &rows_id);
   //check_err(stat,__LINE__,__FILE__);

   cols_dims[0] = cols_dim;
   //stat = nc_def_var(ncid, "cols", NC_INT, RANK_cols, cols_dims, &cols_id);
   //check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "xgrid", NC_DOUBLE, RANK_cols, cols_dims, &xgrid_id);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_def_var(ncid, "ygrid", NC_DOUBLE, RANK_rows, rows_dims, &ygrid_id);
   check_err(stat,__LINE__,__FILE__);


   /* assign attributes to variables */

   /* projection variable */
   stat = nc_put_att_double(ncid, projection_id, "grid_boundary_top_projected_y", NC_DOUBLE, 1, grid_boundary_top_projected_y);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "grid_boundary_bottom_projected_y", NC_DOUBLE, 1, grid_boundary_bottom_projected_y);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "grid_boundary_right_projected_x", NC_DOUBLE, 1, grid_boundary_right_projected_x);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "grid_boundary_left_projected_x", NC_DOUBLE, 1, grid_boundary_left_projected_x);
   check_err(stat,__LINE__,__FILE__);

   // printf("Line out=%s\n",pline);
   stat = nc_put_att_text(ncid, projection_id, "spatial_ref", strlen(pline), pline);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, projection_id, "proj4text", strlen(proj4text), proj4text);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, projection_id, "grid_mapping_name", 19, "polar_stereographic");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "latitude_of_projection_origin", NC_DOUBLE, 1, latitude_of_projection_origin);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "latitude_of_true_scale", NC_DOUBLE, 1, latitude_of_true_scale);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "standard_parallel", NC_DOUBLE, 1, latitude_of_true_scale);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "longitude_of_projection_origin", NC_DOUBLE, 1, longitude_of_projection_origin);
   check_err(stat,__LINE__,__FILE__);

   straight_vertical_longitude_from_pole[0]=longitude_of_projection_origin[0]+180.0;
   stat = nc_put_att_double(ncid, projection_id, "straight_vertical_longitude_from_pole", NC_DOUBLE, 1, straight_vertical_longitude_from_pole);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "scaling_factor", NC_DOUBLE, 1,scaling_factor );
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "false_easting", NC_DOUBLE, 1, false_easting);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "false_northing", NC_DOUBLE, 1, false_northing);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "semimajor_radius", NC_DOUBLE, 1, semimajor_radius);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, projection_id, "semiminor_radius", NC_DOUBLE, 1, semiminor_radius);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, projection_id, "units", 1, "m");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, projection_id, "projection_x_coordinate", 5, "xgrid");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, projection_id, "projection_y_coordinate", 5, "ygrid");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, projection_id, "GeoTransform", strlen(gstring), gstring);
   check_err(stat,__LINE__,__FILE__);


   /* sigma0 variable */
   sigmao__FillValue[0] = -999.0;
   stat = nc_put_att_float(ncid, sigmao_id, "_FillValue", NC_FLOAT, 1, sigmao__FillValue);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sigmao_id, "long_name", strlen(long_name), long_name);
   check_err(stat,__LINE__,__FILE__);
   if (filename[1]=='u')
     stat = nc_put_att_text(ncid, sigmao_id, "cell_methods",  10, "time: mean");
   else if (filename[1]=='m')
     stat = nc_put_att_text(ncid, sigmao_id, "cell_methods",  10, "time: mean local-time-of-day");
   else
     stat = nc_put_att_text(ncid, sigmao_id, "cell_methods",  10, "time: mean local-time-of-day");
   check_err(stat,__LINE__,__FILE__);

   stat = nc_put_att_text(ncid, sigmao_id, "units", 1, "1");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sigmao_id, "units_description", strlen(units_description), units_description);
   check_err(stat,__LINE__,__FILE__);

   stat = nc_put_att_text(ncid, sigmao_id, "coordinates", 18, "longitude latitude");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, sigmao_id, "grid_mapping", 10, "projection");
   check_err(stat,__LINE__,__FILE__);

   /* for computer convenience, add attributes to backscatter help describe data */
   if (head.ipol==2)
     sprintf(line,"Vertical");
   else
     sprintf(line,"Horizontal");
   stat = nc_put_att_text(ncid, sigmao_id, "polarization", strlen(line),line);
   sprintf(line,"%4.1f GHz",head.ifreqhm*0.1); 
   stat = nc_put_att_text(ncid, sigmao_id, "frequency", strlen(line),line);
   check_err(stat,__LINE__,__FILE__);
   if (filename[1]=='u')
     sprintf(line,"daily");   
   else if (filename[1]=='m')
     sprintf(line,"LTD morning");
   else
     sprintf(line,"LTD evening");
   stat = nc_put_att_text(ncid, sigmao_id, "dayspan", strlen(line),line);
   check_err(stat,__LINE__,__FILE__);
   if (eggslice==1)
     sprintf(line,"slices");
   else
     sprintf(line,"eggs");
   stat = nc_put_att_text(ncid, sigmao_id, "eggslice", strlen(line),line);
   check_err(stat,__LINE__,__FILE__);

   if (spatialalg==0)
     sprintf(line,"N/A");   
   else if (spatialalg==1)
     sprintf(line,"AVE"); 
   else if (spatialalg==3)  
     sprintf(line,"grd");   
   else if (spatialalg==2)
     sprintf(line,"SIR");
   else if (spatialalg==4)
     sprintf(line,"non");
   printf("Spatial resolution enhancement algorithm: %s\n",line);   
   stat = nc_put_att_text(ncid, sigmao_id, "spatial_enhancement_algorithm", strlen(line), line);
   check_err(stat,__LINE__,__FILE__);

   /* time variable */
   stat = nc_put_att_text(ncid, times_id, "long_name", 9, "ANSI date");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, times_id, "bounds", 11, "time_bounds");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, times_id, "axis", 1, "T");
   check_err(stat,__LINE__,__FILE__);
   if (filename[1]=='u') { // daily
     stat = nc_put_att_text(ncid, times_id, "units", 31, "days since 1601-01-01T00:00:00Z"); // UTC indicated with Z
     check_err(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, times_id, "comment", 8, "UTC time");
     check_err(stat,__LINE__,__FILE__);
     /* store UTC_bounds */
     stat = nc_put_att_double(ncid, times_id, "valid_range", NC_DOUBLE, 2, times_2);
     check_err(stat,__LINE__,__FILE__);
     /* convert UTC_bounds to human readable strings */
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[0]+julday(1,1,1601),string_2[0]);
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[1]+julday(1,1,1601),string_2[1]); 
     //printf("times: %lf = %s\n",times_2[0],string_2[1]);
     sprintf(workstring,"%s\n%s",string_2[0],string_2[1]);
     printf("utc_valid_range_days:\n%s\n",workstring);
     stat = nc_put_att_text(ncid, times_id, "utc_valid_range_days", strlen(workstring),workstring);
     check_err(stat,__LINE__,__FILE__);
   } else if (filename[1]=='m') { // LTD morning
     stat = nc_put_att_text(ncid, times_id, "units", 30, "days since 1601-01-01T00:00:00");  // local time indicated w/o Z
     check_err(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, times_id, "comment", 25, "Local time-of-day (ltod)");
     check_err(stat,__LINE__,__FILE__);
     /* store UTC_bounds */
     stat = nc_put_att_double(ncid, times_id, "utc_valid_range", NC_DOUBLE, 2, times_2);
     check_err(stat,__LINE__,__FILE__);
     
     /* convert UTC_bounds to human readable strings */
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[0]+julday(1,1,1601),string_2[0]);
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[1]+julday(1,1,1601),string_2[1]);
     //printf("times: %lf = %s\n",times_2[0],string_2[1]);
     sprintf(workstring,"%s\n%s",string_2[0],string_2[1]);
     printf("utc_valid_range_days:\n%s\n",workstring);
     stat = nc_put_att_text(ncid, times_id, "utc_valid_range_days", strlen(workstring), workstring);
     check_err(stat,__LINE__,__FILE__);

     /* store local time-of-day_bounds */
     head.ismin=0*60;
     head.iemin=8*60;
     times_2[0]=i+((float) head.ismin)/(24.*60.)-julday(1,1,1601);        // start time
     times_2[1]=i+((float) head.iemin)/(24.*60.)-julday(1,1,1601);        // end time
     times[0]=0.5*(times_2[0]+times_2[1]); // mean time	
     printf("Morning LTOD start=%lf stop=%lf time  (%d %d mins)\n",times_2[0],times_2[1],head.ismin,head.iemin);
     stat = nc_put_att_double(ncid, times_id, "valid_range", NC_DOUBLE, 2, times_2);
     check_err(stat,__LINE__,__FILE__);

     /* convert ltotd_bounds to human readable strings */
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[0]+julday(1,1,1601),string_2[0]);
     *(string_2[0]+19)='\0';     
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[1]+julday(1,1,1601),string_2[1]);
     *(string_2[1]+19)='\0';
     //printf("times: %lf = %s\n",times_2[0],string_2[1]);
     sprintf(workstring,"%s\n%s",string_2[0],string_2[1]);
     printf("ltod_valid_range_days:\n%s\n",workstring);
     stat = nc_put_att_text(ncid, times_id, "ltod_valid_range_days", strlen(workstring), workstring);
     check_err(stat,__LINE__,__FILE__);

   } else {// LTD evening
     stat = nc_put_att_text(ncid, times_id, "units", 30, "days since 1601-01-01T00:00:00");  // local time indicated w/o Z
     check_err(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, times_id, "comment", 25, "Local time-of-day (ltod)");
     check_err(stat,__LINE__,__FILE__);
     /* store UTC_bounds */
     stat = nc_put_att_double(ncid, times_id, "utc_valid_range", NC_DOUBLE, 2, times_2);
     check_err(stat,__LINE__,__FILE__);

     /* convert UTC_bounds to human readable strings */
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[0]+julday(1,1,1601),string_2[0]);
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[1]+julday(1,1,1601),string_2[1]);
     //printf("times: %lf = %s\n",times_2[0],string_2[1]);
     sprintf(workstring,"%s\n%s",string_2[0],string_2[1]);
     printf("ltod_valid_range_days:\n%s\n",workstring);
     stat = nc_put_att_text(ncid, times_id, "utc_valid_range_days", strlen(workstring), workstring);
     check_err(stat,__LINE__,__FILE__);

     /* store local time-of-day_bounds */
     head.ismin=16*60;
     head.iemin=24*60;
     times_2[0]=i+((float) head.ismin)/(24.*60.)-julday(1,1,1601);        // start time
     times_2[1]=i+((float) head.iemin)/(24.*60.)-julday(1,1,1601);        // end time
     times[0]=0.5*(times_2[0]+times_2[1]); // mean time	
     printf("Eve LTOD start=%lf stop=%lf time  (%d %d mins)\n",times_2[0],times_2[1],head.ismin,head.iemin);
     stat = nc_put_att_double(ncid, times_id, "valid_range", NC_DOUBLE, 2, times_2);
     check_err(stat,__LINE__,__FILE__);

     /* convert ltod_bounds to human readable strings */
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[0]+julday(1,1,1601),string_2[0]);
     *(string_2[0]+19)='\0';
     //printf("times: %lf = %s\n",times_2[0],string_2[0]);
     jdt2string(times_2[1]+julday(1,1,1601),string_2[1]);
     *(string_2[1]+19)='\0';
     //printf("times: %lf = %s\n",times_2[0],string_2[1]);
     sprintf(workstring,"%s\n%s",string_2[0],string_2[1]);
     printf("ltod_valid_range_days:\n%s\n",workstring);
     stat = nc_put_att_text(ncid, times_id, "ltod_valid_range_days", strlen(workstring), workstring);
     check_err(stat,__LINE__,__FILE__);
   }
   
   /* latitude variable */
   stat = nc_put_att_text(ncid, latitude_id, "standard_name", 8, "latitude");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, latitude_id, "long_name", 8, "latitude");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, latitude_id, "units", 13, "degrees_north");
   check_err(stat,__LINE__,__FILE__);
   latitude_valid_range[0] = -90;
   latitude_valid_range[1] = 90;
   stat = nc_put_att_double(ncid, latitude_id, "valid_range", NC_DOUBLE, 2, latitude_valid_range);
   check_err(stat,__LINE__,__FILE__);
   latitude__FillValue[0] = -999;
   stat = nc_put_att_double(ncid, latitude_id, "_FillValue", NC_DOUBLE, 1, latitude__FillValue);
   check_err(stat,__LINE__,__FILE__);

   /* longitude variable */
   stat = nc_put_att_text(ncid, longitude_id, "standard_name", 9, "longitude");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, longitude_id, "long_name", 9, "longitude");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, longitude_id, "units", 12, "degrees_east");
   check_err(stat,__LINE__,__FILE__);
   longitude_valid_range[0] = -180;
   longitude_valid_range[1] = 180;
   stat = nc_put_att_double(ncid, longitude_id, "valid_range", NC_DOUBLE, 2, longitude_valid_range);
   check_err(stat,__LINE__,__FILE__);
   longitude__FillValue[0] = -999;
   stat = nc_put_att_double(ncid, longitude_id, "_FillValue", NC_DOUBLE, 1, longitude__FillValue);
   check_err(stat,__LINE__,__FILE__);

   /* rows and cols variables */
   if (0) {     
     rows_valid_range[0] = 0;
     rows_valid_range[1] = rows_len-1;
     stat = nc_put_att_int(ncid, rows_id, "valid_range", NC_INT, 2, rows_valid_range);
     check_err(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, rows_id, "units", 5, "count");
     check_err(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, rows_id, "long_name", 17, "rows in the image");
     check_err(stat,__LINE__,__FILE__);
     cols_valid_range[0] = 0;
     cols_valid_range[1] = cols_len-1;
     stat = nc_put_att_int(ncid, cols_id, "valid_range", NC_INT, 2, cols_valid_range);
     check_err(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, cols_id, "units", 5, "count");
     check_err(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, cols_id, "long_name", 20, "columns in the image");
     check_err(stat,__LINE__,__FILE__);
   }
   
   /* xgrid and ygrid variables */
   stat = nc_put_att_text(ncid, xgrid_id, "standard_name", 23, "projection_x_coordinate");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, xgrid_id, "long_name", 25, "projection_grid_x_centers");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, xgrid_id, "valid_range", NC_DOUBLE, 2, xgrid_valid_range);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, xgrid_id, "units", 6, "meters");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, xgrid_id, "axis", 1, "X");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, ygrid_id, "standard_name", 23, "projection_y_coordinate");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, ygrid_id, "long_name", 25, "projection_grid_y_centers");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_double(ncid, ygrid_id, "valid_range", NC_DOUBLE, 2, ygrid_valid_range);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, ygrid_id, "units", 6, "meters");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, ygrid_id, "axis", 1, "Y");
   check_err(stat,__LINE__,__FILE__);

   /* global attributes */
   stat = nc_put_att_text(ncid, NC_GLOBAL, "Conventions", 6, "CF-1.4");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "institution", 39, "Brigham Young University SCP, Provo, UT  ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "title", strlen(title), title);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "source", 49, "See the title, original_file name, and references ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "original_file", strlen(filename), filename);
   check_err(stat,__LINE__,__FILE__);
   strncpy(line,&head.type[0],139);line[139]='\0';
   stat = nc_put_att_text(ncid, NC_GLOBAL, "comment", strlen(line), line);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "references", 72, "Documentation available at: http://www.scp.byu.edu/docs/pdf/MERS0504.pdf ");
   check_err(stat,__LINE__,__FILE__);
   stat = nc_put_att_text(ncid, NC_GLOBAL, "history", strlen(history), history);
   check_err(stat,__LINE__,__FILE__);

   /* leave define mode */
   stat = nc_enddef (ncid);
   check_err(stat,__LINE__,__FILE__);

   { /* store projection */
     // static char lambert_azimuthal_equal_area = '\t';
     // stat = nc_put_var_text(ncid, projection_id, &lambert_azimuthal_equal_area);
     // check_err(stat,__LINE__,__FILE__);
   }

   /* sigmao */
   static size_t sigmao_start[RANK_sigmao];
   static size_t sigmao_count[RANK_sigmao];
   size_t sigmao_size = rows_len * cols_len;
   float *sigmao;
   sigmao=malloc(sizeof(float)*sigmao_size);
   if (sigmao==NULL)
     printf("*** error allocating sigmao memory\n");


   /* time_bounds */

   /* latitude & longitude */
   size_t lat_size = rows_len * cols_len;
   double *latitude;
   latitude=malloc(sizeof(double)*lat_size);
   if (latitude==NULL)
     printf("*** error allocating latitude memory\n");
   double *longitude;
   longitude=malloc(sizeof(double)*lat_size);
   if (longitude==NULL)
     printf("*** error allocating longitude memory\n");

   /* print sir file header */
   print_sir_head(stdout, &head);
   
   a0=head.a0;
   b0=head.b0;
   ascale=head.ascale;
   bscale=head.bscale;

   /* create arragys of x and y values */
   for (ix = 0; ix < cols_len; ix++){
     if (head.iopt==5)
       xp=round(1000.0*((ix+0.5)*ascale+a0)); // polar stereographic projection X axis
     else
       xp=1000.0*((ix+0.5)/ascale+a0); // lambert equal area projection X axis
     xgrid[ix]=xp;  // store in m
   }

   for (iy = 0; iy < rows_len; iy++){
     if (head.iopt==5) 
       yp=round(1000.0*((iy+0.5)*bscale+b0)); // polar stereographic projection X axis
     else
       yp=1000.0*((iy+0.5)/bscale+b0); // lambert equal area projection Y axis
     ygrid[rows_len-iy-1]=yp; // flipped, store in m
   }
      
   /* compute lat/lon and any necessary transformation */
   printf("start image mapping\n");
   sigmao[0]=sigmao__FillValue[0];
   for (iy = 0; iy < rows_len; iy++) {     
     if (iy % 100==0) printf(" row %d of %d\n",iy,rows_len);      
     for (ix = 0; ix < cols_len; ix++) {
       j=ix+iy*cols_len;  // conventional row-order
       i=ix+(rows_len-iy-1)*cols_len;  // flipped row-order

       // use pixel corner (note that pixels are indexed 1..nsx,1..nsy, rather than 0..nsx-1)

       if (head.iopt == 5)
	 ipolster1(&lonp, &latp, (float) (xgrid[ix]*0.001), (float) (ygrid[rows_len-iy-1]*0.001), head.xdeg, head.ydeg);
       else
	 sir_pix2latlon((float)ix+1.5,(float)iy+1.5,&lonp,&latp,&head);	
       //printf("%d %d  %f %f  %lf %lf\n",ix,iy,lonp,latp,xgrid[ix],ygrid[rows_len-iy-1]);

       longitude[i]=(double) lonp;
       latitude[i]=(double) latp;

       //polster1(lonp, latp, &xp1, &yp1, head.xdeg, head.ydeg);
       //printf("%d %d  %f %f  %f %f  %lf %lf\n",ix,iy,lonp,latp,xp1,yp1,xgrid[ix],ygrid[rows_len-iy-1]);
       //ipolster1(&lonp, &latp, (float) (xgrid[ix]*0.001), (float) (ygrid[rows_len-iy-1]*0.001), head.xdeg, head.ydeg);
       //ipolster1(&lonp, &latp, xp1, yp1, head.xdeg, head.ydeg);
       //printf("%d %d  %f %f  %lf %lf\n",ix,iy,lonp,latp,xgrid[ix],ygrid[rows_len-iy-1]);

       // remapping code
       //j = sir_latlon2pix(lonp, latp, &x1, &y1, &head); /* (lat,lon) -> (x,y) */
       if (j>0 && j<head.nsx*head.nsy) { /* copy pixel value */
	 sigmao[i]=stval[j];
         if (sigmao[i]<-31.95)  /* set underflows as fill value */
	   sigmao[i]=sigmao__FillValue[0];
       } else
	 sigmao[i]=sigmao__FillValue[0];
       if (iy==0 && ix==0 || iy+1==rows_len && ix+1==cols_len) {
	 if (head.iopt == 5) {	   
	   polster1(lonp,latp,&x1,&y1,head.xdeg,head.ydeg);
	   printf(" Corner: %4d %4d %f %f %f %f %lf %lf\n",ix+1,iy+1,lonp,latp,x1*1000.,y1*1000.,xgrid[ix],ygrid[rows_len-iy-1]);
	 }
       }       

       //printf("%d,%d=%d  %f,%f  %f,%f  %f,%f=%d\n",ix,iy,i,xp,yp,lonp,latp,x1,y1,j);
     }
   }
   printf("done image mapping\n");
   
   /* store sigmao array */
   sigmao_start[0] = 0;
   sigmao_start[1] = 0;
   sigmao_start[2] = 0;
   sigmao_count[0] = times_len;
   sigmao_count[1] = rows_len;
   sigmao_count[2] = cols_len;
   stat = nc_put_vara_float(ncid, sigmao_id, sigmao_start, sigmao_count, sigmao);
   check_err(stat,__LINE__,__FILE__);
   free(sigmao);

   { /* store average of start/stop time*/
    static size_t times_start[RANK_times];
    static size_t times_count[RANK_times];

    times_start[0] = 0;
    times_count[0] = times_len;
    stat = nc_put_vara_double(ncid, times_id, times_start, times_count, times);
    check_err(stat,__LINE__,__FILE__);
   }

   { /* store time_bounds */
    static size_t time_vertices_start[RANK_time_vertices];
    static size_t time_vertices_count[RANK_time_vertices];

    time_vertices_start[0] = 0;
    time_vertices_start[1] = 0;
    time_vertices_count[0] = times_len;
    time_vertices_count[1] = time_vertices_len;
    stat = nc_put_vara_double(ncid, time_vertices_id, time_vertices_start, time_vertices_count, times_2);
    check_err(stat,__LINE__,__FILE__);
   }

   /* store latitude array */
   stat = nc_put_var_double(ncid, latitude_id, latitude);
   check_err(stat,__LINE__,__FILE__);

   /* store longitude array */
   stat = nc_put_var_double(ncid, longitude_id, longitude);
   check_err(stat,__LINE__,__FILE__);

   if (0) {
     /*	 store rows array */
     int rows[rows_len];
     int i;
     for (i=0; i<rows_len; i++)
       rows[i]=i;
     stat = nc_put_var_int(ncid, rows_id, rows);
     check_err(stat,__LINE__,__FILE__);

     /* store cols array */
     int cols[cols_len];	
     for (i=0; i<cols_len; i++)	
       cols[i]=i;	
     stat = nc_put_var_int	(ncid, cols_id, cols);
     check_err(stat,__LINE__,__FILE__);
   }
   
   /* store xgrid array */
   stat = nc_put_var_double(ncid, xgrid_id, xgrid);
   check_err(stat,__LINE__,__FILE__);

   /* store ygrid array */
   stat = nc_put_var_double(ncid, ygrid_id, ygrid);
   check_err(stat,__LINE__,__FILE__);

   /* close output netcdf file */
   stat = nc_close(ncid);
   check_err(stat,__LINE__,__FILE__);

   printf("All done\n");
   return 0;
}


