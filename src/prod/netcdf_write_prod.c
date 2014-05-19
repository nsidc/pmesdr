/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  netcdf_write_prod.c

  write standard MEASUReS NetCDF product files 
  this code defines the standard product

  includes routines:
    netcdf_write_single : writes float arrays as scaled short
    (TBD) netcdf_write_multi  : write multiple float and short arrays 
    (TBD) netcdf_write_add_short : add a short array to an existing netcdf file

  written by DGL at BYU 16 May 2014

  Note: Though written primarily in C, code includes some C++ constructs

 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <netcdf.h>  /* netCDF interface */

#include "sir3.h"    /* SIR geometry routine interface */

#define DTR (double) 0.017453292
#define RAD2DEG (double) 57.295779
#define abs(a) ((a) > 0.0 ? (a) : - (a))
#define rnd(a) ((a) >= 0 ? floor((a)+0.5L) : ceil((a)-0.5L))
#define FLOOR(x) (int) ((float) (floor((double)(x))))

#define INCLUDE_LATLON 0 /* compute and include lat/lon arrays if 1, do not include if 0 */

/********************************************************************/
/* error routines */
/********************************************************************/

void check_ncerr(const int stat, const int line, const char *file) {
  if (stat != NC_NOERR) {
    (void) fprintf(stderr, "*** NETCDF ERR: line %d of %s: %s\n", line, file, nc_strerror(stat));
    exit(1);
  }
}

/********************************************************************/
/* internal routines */
/********************************************************************/

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


/********************************************************************/
/* external routines */
/********************************************************************/

int netcdf_write_single(char *outpath, char *filename, float *val, char *varname,
			int idatatype, int nsx, int nsy, float xdeg, float ydeg, 
			float ascale, float bscale, float a0, float b0, 
			int ioff, int iscale, int iyear, int isday, int ismin, int ieday, int iemin, 
			int iregion, int itype, int iopt, int ipol, int ifreqhm, 
			float anodata, char *sensor, char *title, char *type, char *tag,
			char *crproc, char *crtime)
{

  /* write a netcdf file with a single array (and optionally lat/lon arrays) */

  int  stat;			/* return status */
  int  ncid;			/* netCDF id */

  /* dimension ids */
  int cols_dim;
  int rows_dim;
  int time_vertices_dim;
  int times_dim;

  /* dimension lengths */
  size_t cols_len = nsx;
  size_t rows_len = nsy;
  size_t time_vertices_len = 2;
  size_t times_len = 1;

  /* variable ids */
  int projection_id;
  int prod_var_id;
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
#  define RANK_TB 2
#  define RANK_prod_var 3
#  define RANK_time_vertices 2
#  define RANK_times 1
#  define RANK_latitude 2
#  define RANK_longitude 2
#  define RANK_rows 1
#  define RANK_cols 1

  /* variable shapes */
  short *prod_var;
  int prod_var_dims[RANK_prod_var];
  size_t prod_var_start[RANK_prod_var];
  size_t prod_var_count[RANK_prod_var];

  int times_dims[RANK_times];
  size_t times_start[RANK_times];
  size_t times_count[RANK_times];
  size_t time_vertices_start[RANK_time_vertices];
  size_t time_vertices_count[RANK_time_vertices];

  double *latitude;
  double *longitude;
  int lat_size;
  int latitude_dims[RANK_latitude];
  int longitude_dims[RANK_longitude];
  int rows_dims[RANK_rows];
  int cols_dims[RANK_cols];
  int time_vertices_dims[RANK_time_vertices];

  /* attribute scalars and vectors */
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
  short prod_var__FillValue[1];
  short prod_var__NoData[1];
  short prod_var_offset[1];
  short prod_var_scale[1];
  double latitude_valid_range[2];
  double latitude__FillValue[1];
  double longitude_valid_range[2];
  double longitude__FillValue[1];
  int rows_valid_range[2];
  int cols_valid_range[2];
  double xgrid_valid_range[2];
  double ygrid_valid_range[2];

  double times[1];    /* to store average of start and stop time */
  double times_2[2];  /* to store start and stop time */
   
  char out_name[256];
  char line[256], long_name[140], gstring[120],
    history[140], *ss, pline[512], proj4text[200];
  char units_description[]={"see documentation for this variable"};
  //char *string_2[2]={"yyyy-mm-ddThh:mm:ssZ","yyyy-mm-ddThh:mm:ssZ"};
  char *string_2[2];   
  char workstring[256];

  int ix, iy, ix1, iy1, i, j, ierr;
  float lonp,latp;
  float x1,y1,xp1,yp1,orglat,orglon;
  double xp, yp;
  int spatialalg=0;

  char crtime1[28];
  time_t tod;

  double gres, temp;
  short anodatas;

  char local[]="./";

  string_2[0]=(char *) malloc(22);
  string_2[1]=(char *) malloc(22);
  strcpy(string_2[0],"yyyy-mm-ddThh:mm:ssZ");
  strcpy(string_2[1],"yyyy-mm-ddThh:mm:ssZ");

  if (ipol==2)
    sprintf(long_name,"%s vertical polarization",title);
  else
    sprintf(long_name,"%s horizontal polarization", title);

  /* set times */
  i=julday(1, 1, iyear)+isday-1;
  j=julday(1, 1, iyear)+ieday-1;
  x1=i+((float) ismin)/(24.*60.);
  y1=j+((float) iemin)/(24.*60.);
  times[0]=0.5*(x1+y1)-julday(1,1,1601); // mean time
  times_2[0]=i+((float) ismin)/(24.*60.)-julday(1,1,1601);        // start time
  times_2[1]=j+((float) iemin)/(24.*60.)-julday(1,1,1601);        // end time
  //printf("Data ave=%f start=%lf stop=%lf time  (%d %d mins)\n",times[0],times_2[0],times_2[1],ismin,iemin);
    
  (void) time(&tod);
  (void) strftime(crtime1,28,"%c",localtime(&tod));
  //printf("Current time: '%s'\n",crtime1);
  sprintf(history,"Originally created: %s  netCDF File created %s",crtime,crtime1);

  /* determine the spatial processing algorithm from file name*/
  strcpy(line,filename);
  if (ss=strstr(line,".ave"))
    spatialalg=1;
  if (ss=strstr(line,".sir"))
    spatialalg=2;
  if (ss=strstr(line,".grd"))
    spatialalg=3;
  if (ss=strstr(line,".non"))
    spatialalg=4;
  //*ss='\0';	/* truncate off original file extension if desired (dangerous since could cause file overwrite) */
  sprintf(out_name,"%s/%s.nc",outpath,line);
  printf(" NetCDF output file: %s\n",out_name);

  /* set default transformation information for BYU SIR polarstereographic */
  scaling_factor[0] = 1.0;
  false_easting[0] = 0.;
  false_northing[0] = 0.;

  switch (iopt) {
  case 5:  // Polar stereographic
    gres=1.0/4450.0;   // pixels/m    
    semimajor_radius[0] = 6378273.0;
    //f[0]=298.780;   
    //semiminor_radius[0] = semimajor_radius[0] * (1.0 - 1.0/f[0]);
    f[0] = 2.0/0.006693883;
    semiminor_radius[0] = semimajor_radius[0] * sqrt(1.0 - 0.006693883);     
    /* use exact values rather than rounded values from header */
    if (iregion == 100) { // Antarctic
      latitude_of_projection_origin[0] = -90.0;
      latitude_of_true_scale[0] = -70.0;
      longitude_of_projection_origin[0] = 0.0;

      printf("*** CAUTION: Ant not fully tested yet!\n");
      xgrid_valid_range[0] = a0*1000.0;
      xgrid_valid_range[1] = round((a0+nsx*ascale)*1000.0);
      ygrid_valid_range[1] = b0*1000.0;
      ygrid_valid_range[0] = round((b0+nsy*bscale)*1000.0);

      sprintf(pline,"PROJCS[\"Stereographic_South_Pole\",GEOGCS[\"unnamed ellipse\",DATUM[\"D_unknown\",SPHEROID[\"Unknown\",%11.3lf,%11.7lf]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",%18.16lf]],PROJECTION[\"Stereographic_South_Pole\"],PARAMETER[\"standard_parallel_1\",%lf],PARAMETER[\"central_meridian\",%lf],PARAMETER[\"scale_factor\",%1lf],PARAMETER[\"false_easting\",%1lf],PARAMETER[\"false_northing\",%1lf],UNIT[\"Meter\",1, AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"3411\"]]",semimajor_radius[0],f[0],gres,latitude_of_true_scale[0],longitude_of_projection_origin[0],scaling_factor[0],false_easting[0],false_northing[0]);

    } else if (iregion == 110) { // Arctic
      latitude_of_projection_origin[0] = 90.0;
      latitude_of_true_scale[0] = 70.0;
      longitude_of_projection_origin[0] = -45.0;

      //printf("a0=%f b0=%f %f %f\n",a0*1000.,b0*1000.,(a0+nsx*ascale)*1000.0,(b0+nsy*bscale)*1000.0);
       
      xgrid_valid_range[0] = a0*1000.0;
      xgrid_valid_range[1] = round((a0+nsx*ascale)*1000.0);
      ygrid_valid_range[1] = b0*1000.;
      ygrid_valid_range[0] = round((b0+nsy*bscale)*1000.0);
      
      sprintf(pline,"PROJCS[\"Stereographic_North_Pole\",GEOGCS[\"unnamed ellipse\",DATUM[\"D_unknown\",SPHEROID[\"Unknown\",%11.3lf,%11.7lf]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",%18.16lf]],PROJECTION[\"Stereographic_North_Pole\"],PARAMETER[\"standard_parallel_1\",%8.2lf],PARAMETER[\"central_meridian\",%8.2lf],PARAMETER[\"scale_factor\",%2.0lf],PARAMETER[\"false_easting\",%2.0lf],PARAMETER[\"false_northing\",%2.0lf],UNIT[\"Meter\",1, AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"3411\"]]",semimajor_radius[0],f[0],gres,latitude_of_true_scale[0],longitude_of_projection_origin[0],scaling_factor[0],false_easting[0],false_northing[0]);

    } else if (iregion == 112) { // NHe
       latitude_of_projection_origin[0] = 90.0;
       latitude_of_true_scale[0] = 70.0;
       longitude_of_projection_origin[0] = -45.0;

       xgrid_valid_range[0] = a0*1000.0;
       xgrid_valid_range[1] = round((a0+nsx*ascale)*1000.0);
       ygrid_valid_range[1] = b0*1000.;
       ygrid_valid_range[0] = round((b0+nsy*bscale)*1000.0);
       sprintf(pline,"PROJCS[\"Stereographic_North_Pole\",GEOGCS[\"unnamed ellipse\",DATUM[\"D_unknown\",SPHEROID[\"Unknown\",%11.3lf,%11.7lf]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",%18.16lf]],PROJECTION[\"Stereographic_North_Pole\"],PARAMETER[\"standard_parallel_1\",%8.2lf],PARAMETER[\"central_meridian\",%8.2lf],PARAMETER[\"scale_factor\",%2.0lf],PARAMETER[\"false_easting\",%2.0lf],PARAMETER[\"false_northing\",%2.0lf],UNIT[\"Meter\",1, AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"3411\"]]",semimajor_radius[0],f[0],gres,latitude_of_true_scale[0],longitude_of_projection_origin[0],scaling_factor[0],false_easting[0],false_northing[0]);

       printf("*** CAUTION: NHe not fully tested yet!\n");

    } else {
       printf("*** This SIR polar stereographic projection region is not supported by this code %d\n",iregion);
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
    // fall through to next case intented
  case 2:   // Lambert equal area local radius
    if (iopt == 2) {
      semimajor_radius[0] = 6378135.0;     
      f[0]=298.260;   
      semiminor_radius[0] = semimajor_radius[0] * (1.0 - 1.0/f[0]);
    }

    latitude_of_projection_origin[0] = ydeg;
    latitude_of_true_scale[0] = ydeg;
    longitude_of_projection_origin[0] = xdeg;

    xgrid_valid_range[0] = a0*1000.0;
    xgrid_valid_range[1] = round((a0+nsx/ascale)*1000.0);
    ygrid_valid_range[0] = b0*1000.;
    ygrid_valid_range[1] = round((b0+nsy/bscale)*1000.0);

    sprintf(pline,"PROJCS[\"Lambert Equal Area\",GEOGCS[\"unnamed ellipse\",DATUM[\"D_unknown\",SPHEROID[\"Unknown\",%11.3lf,%7.3lf]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",%18.16lf]],PROJECTION[\"Lambert_Equal_Area\"],PARAMETER[\"central_longitude\",%8.2lf],PARAMETER[\"central_meridian\",%8.2lf],PARAMETER[\"scale_factor\",%2.0lf],PARAMETER[\"false_easting\",%2.0lf],PARAMETER[\"false_northing\",%2.0lf],UNIT[\"Meter\",1, AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"3411\"]]",semimajor_radius[0],f[0],gres,latitude_of_projection_origin[0],longitude_of_projection_origin[0],scaling_factor[0],false_easting[0],false_northing[0]);
    
    printf("*** CAUTION: Lambert images not fully tested yet!\n");

    sprintf(proj4text,"+proj=laea +lat_0=%lf +lon_0=%lf +k=1 +x_0=0 +y_0=0 +a=%11.3lf +rf=%11.3lf +units=m +no_defs -a_ullr %11.3lf %11.3lf %11.3lf %11.3lf",latitude_of_projection_origin[0],longitude_of_projection_origin[0],semimajor_radius[0],50000000.0,xgrid_valid_range[0],ygrid_valid_range[1],xgrid_valid_range[1],ygrid_valid_range[1]);
     
    break;

  case 8: // EASE2 grid north
  case 9: // EASE2 grid south
  case 10: // EASE2 grid cylindrical
    printf("*** EASE2 projection is not supported by this code %d\n",iopt);
    exit(-1);
    break;
     
  case 11: // EASE1 grid north
  case 12: // EASE1 grid south
  case 13: // EASE1 grid cylindrical
  default:
    printf("*** This SIR projection is not supported by this code %d\n",iopt);
    exit(-1);
  }
   
  /* set grid boundaries and GeoTransform */
  grid_boundary_top_projected_y[0] = ygrid_valid_range[0];
  grid_boundary_bottom_projected_y[0] = ygrid_valid_range[1];
  grid_boundary_right_projected_x[0]= xgrid_valid_range[1];   
  grid_boundary_left_projected_x[0] = xgrid_valid_range[0];

  sprintf(gstring,"%lf %lf 0 %lf 0 %lf",grid_boundary_left_projected_x[0],1./gres,grid_boundary_top_projected_y[0],-1./gres);
   
  /* output image size same as the input image size */
  cols_len = nsx;
  rows_len = nsy;

  double xgrid[cols_len];	
  double ygrid[rows_len];
  
  //printf("Output image size %d X %d\n",cols_len,rows_len);

  /* create output netcdf file and enter define mode to add header information */
  stat = nc_create(out_name, NC_CLOBBER, &ncid); check_ncerr(stat,__LINE__,__FILE__);

  /* define dimensions */
  stat = nc_def_dim(ncid, "xgrid", cols_len, &cols_dim); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_def_dim(ncid, "ygrid", rows_len, &rows_dim); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_def_dim(ncid, "time_vertices", time_vertices_len, &time_vertices_dim); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_def_dim(ncid, "time", times_len, &times_dim); check_ncerr(stat,__LINE__,__FILE__);

  /* define variables */
  stat = nc_def_var(ncid, "projection", NC_CHAR, RANK_projection, 0, &projection_id); check_ncerr(stat,__LINE__,__FILE__);
  
  prod_var_dims[0] = times_dim;
  prod_var_dims[1] = rows_dim;
  prod_var_dims[2] = cols_dim;
  stat = nc_def_var(ncid, varname, NC_SHORT, RANK_prod_var, prod_var_dims, &prod_var_id); check_ncerr(stat,__LINE__,__FILE__);
   
  times_dims[0] = times_dim;
  stat = nc_def_var(ncid, "time", NC_DOUBLE, RANK_times, times_dims, &times_id); check_ncerr(stat,__LINE__,__FILE__);

  time_vertices_dims[0] = times_dim;
  time_vertices_dims[1] = time_vertices_dim;
  stat = nc_def_var(ncid, "time_bounds", NC_DOUBLE, RANK_time_vertices, time_vertices_dims, &time_vertices_id); check_ncerr(stat,__LINE__,__FILE__);

  if (INCLUDE_LATLON) {
    latitude_dims[0] = rows_dim;
    latitude_dims[1] = cols_dim;
    stat = nc_def_var(ncid, "latitude", NC_DOUBLE, RANK_latitude, latitude_dims, &latitude_id); check_ncerr(stat,__LINE__,__FILE__);
    
    longitude_dims[0] = rows_dim;
    longitude_dims[1] = cols_dim;
    stat = nc_def_var(ncid, "longitude", NC_DOUBLE, RANK_longitude, longitude_dims, &longitude_id); check_ncerr(stat,__LINE__,__FILE__);
  }     
  rows_dims[0] = rows_dim;
  //stat = nc_def_var(ncid, "rows", NC_INT, RANK_rows, rows_dims, &rows_id); check_ncerr(stat,__LINE__,__FILE__);

  cols_dims[0] = cols_dim;
  //stat = nc_def_var(ncid, "cols", NC_INT, RANK_cols, cols_dims, &cols_id); check_ncerr(stat,__LINE__,__FILE__);

  stat = nc_def_var(ncid, "xgrid", NC_DOUBLE, RANK_cols, cols_dims, &xgrid_id); check_ncerr(stat,__LINE__,__FILE__);   
  stat = nc_def_var(ncid, "ygrid", NC_DOUBLE, RANK_rows, rows_dims, &ygrid_id); check_ncerr(stat,__LINE__,__FILE__);

  /* assign attributes to variables */

  /* projection variable */
  stat = nc_put_att_double(ncid, projection_id, "grid_boundary_top_projected_y", NC_DOUBLE, 1, grid_boundary_top_projected_y); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "grid_boundary_bottom_projected_y", NC_DOUBLE, 1, grid_boundary_bottom_projected_y); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "grid_boundary_right_projected_x", NC_DOUBLE, 1, grid_boundary_right_projected_x); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "grid_boundary_left_projected_x", NC_DOUBLE, 1, grid_boundary_left_projected_x); check_ncerr(stat,__LINE__,__FILE__);

  // printf("Line out=%s\n",pline);
  stat = nc_put_att_text(ncid, projection_id, "spatial_ref", strlen(pline), pline); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, projection_id, "proj4text", strlen(proj4text), proj4text); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, projection_id, "grid_mapping_name", 19, "polar_stereographic"); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "latitude_of_projection_origin", NC_DOUBLE, 1, latitude_of_projection_origin); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "latitude_of_true_scale", NC_DOUBLE, 1, latitude_of_true_scale); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "standard_parallel", NC_DOUBLE, 1, latitude_of_true_scale); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "longitude_of_projection_origin", NC_DOUBLE, 1, longitude_of_projection_origin); check_ncerr(stat,__LINE__,__FILE__);

  straight_vertical_longitude_from_pole[0]=longitude_of_projection_origin[0]+180.0; stat = nc_put_att_double(ncid, projection_id, "straight_vertical_longitude_from_pole", NC_DOUBLE, 1, straight_vertical_longitude_from_pole);
  check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "scaling_factor", NC_DOUBLE, 1,scaling_factor); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "false_easting", NC_DOUBLE, 1, false_easting); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "false_northing", NC_DOUBLE, 1, false_northing); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "semimajor_radius", NC_DOUBLE, 1, semimajor_radius); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_double(ncid, projection_id, "semiminor_radius", NC_DOUBLE, 1, semiminor_radius); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, projection_id, "units", 1, "m"); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, projection_id, "projection_x_coordinate", 5, "xgrid"); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, projection_id, "projection_y_coordinate", 5, "ygrid"); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, projection_id, "GeoTransform", strlen(gstring), gstring); check_ncerr(stat,__LINE__,__FILE__);

  /* leave define mode -- flushes define */
  stat = nc_enddef(ncid);  check_err(stat,__LINE__,__FILE__);
  /* re-enter define mode */
  stat = nc_redef(ncid);  check_err(stat,__LINE__,__FILE__);

  temp = rnd(((double) anodata - (double) ioff) * (double) iscale);
  anodatas = (short) temp;

  /* prod_var variable stored as a short ## */
  prod_var__FillValue[0] = -32767;
  stat = nc_put_att_short(ncid, prod_var_id, "_FillValue", NC_SHORT, 1, prod_var__FillValue); check_ncerr(stat,__LINE__,__FILE__);
  prod_var__NoData[0] = anodatas;
  stat = nc_put_att_short(ncid, prod_var_id, "_NoData", NC_SHORT, 1, prod_var__NoData); check_ncerr(stat,__LINE__,__FILE__);
  prod_var_scale[0] = iscale;
  stat = nc_put_att_short(ncid, prod_var_id, "scale_factor", NC_SHORT, 1, prod_var_scale); check_ncerr(stat,__LINE__,__FILE__);
  prod_var_offset[0] = ioff;
  stat = nc_put_att_short(ncid, prod_var_id, "offset", NC_SHORT, 1, prod_var_offset); check_ncerr(stat,__LINE__,__FILE__);

  stat = nc_put_att_text(ncid, prod_var_id, "long_name", strlen(long_name), long_name); check_ncerr(stat,__LINE__,__FILE__);
  //if (filename[1]=='u')
  stat = nc_put_att_text(ncid, prod_var_id, "cell_methods",  10, "time: mean"); check_ncerr(stat,__LINE__,__FILE__);
     //else 
     // stat = nc_put_att_text(ncid, prod_var_id, "cell_methods",  10, "time: mean local-time-of-day"); check_ncerr(stat,__LINE__,__FILE__);

  stat = nc_put_att_text(ncid, prod_var_id, "units", 1, "1"); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, prod_var_id, "units_description", strlen(units_description), units_description); check_ncerr(stat,__LINE__,__FILE__);

  stat = nc_put_att_text(ncid, prod_var_id, "coordinates", 18, "longitude latitude"); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, prod_var_id, "grid_mapping", 10, "projection"); check_ncerr(stat,__LINE__,__FILE__);

  /* add attributes to help describe data */
  if (ipol==2)
    sprintf(line,"Vertical");
  else
    sprintf(line,"Horizontal");
  stat = nc_put_att_text(ncid, prod_var_id, "polarization", strlen(line),line); sprintf(line,"%4.1f GHz",ifreqhm*0.1); 
  stat = nc_put_att_text(ncid, prod_var_id, "frequency", strlen(line),line); check_ncerr(stat,__LINE__,__FILE__);

  //if (filename[1]=='u')
  sprintf(line,"daily");   
     //else if (filename[1]=='m')
     // sprintf(line,"LTD morning");
     //else
     //sprintf(line,"LTD evening");
  stat = nc_put_att_text(ncid, prod_var_id, "dayspan", strlen(line),line); check_ncerr(stat,__LINE__,__FILE__);

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
  printf(" Spatial resolution enhancement algorithm: %s\n",line);   
  stat = nc_put_att_text(ncid, prod_var_id, "spatial_enhancement_algorithm", strlen(line), line); check_ncerr(stat,__LINE__,__FILE__);

  /* time variable */
  stat = nc_put_att_text(ncid, times_id, "long_name", 9, "ANSI date"); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, times_id, "bounds", 11, "time_bounds"); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, times_id, "axis", 1, "T"); check_ncerr(stat,__LINE__,__FILE__);
  //if (filename[1]=='u') { // daily
  if (1) {
    stat = nc_put_att_text(ncid, times_id, "units", 31, "days since 1601-01-01T00:00:00Z"); // UTC indicated with Z
    check_ncerr(stat,__LINE__,__FILE__);
    stat = nc_put_att_text(ncid, times_id, "comment", 8, "UTC time");  check_ncerr(stat,__LINE__,__FILE__);
    /* store UTC_bounds */
    stat = nc_put_att_double(ncid, times_id, "valid_range", NC_DOUBLE, 2, times_2);
    check_ncerr(stat,__LINE__,__FILE__);
    /* convert UTC_bounds to human readable strings */
    //printf("times: %lf = %s\n",times_2[0],string_2[0]);
    jdt2string(times_2[0]+julday(1,1,1601),string_2[0]);
    //printf("times: %lf = %s\n",times_2[0],string_2[0]);
    jdt2string(times_2[1]+julday(1,1,1601),string_2[1]); 
    //printf("times: %lf = %s\n",times_2[0],string_2[1]);
    sprintf(workstring,"%s\n%s",string_2[0],string_2[1]);
    //printf("utc_valid_range_days:\n%s\n",workstring);
    stat = nc_put_att_text(ncid, times_id, "utc_valid_range_days", strlen(workstring),workstring); check_ncerr(stat,__LINE__,__FILE__);
    
  } else if (filename[1]=='m') { // LTD morning
    stat = nc_put_att_text(ncid, times_id, "units", 30, "days since 1601-01-01T00:00:00");  // local time indicated w/o Z
    check_ncerr(stat,__LINE__,__FILE__);
    stat = nc_put_att_text(ncid, times_id, "comment", 25, "Local time-of-day (ltod)"); check_ncerr(stat,__LINE__,__FILE__);
    /* store UTC_bounds */
    stat = nc_put_att_double(ncid, times_id, "utc_valid_range", NC_DOUBLE, 2, times_2); check_ncerr(stat,__LINE__,__FILE__);
    
    /* convert UTC_bounds to human readable strings */
    //printf("times: %lf = %s\n",times_2[0],string_2[0]);
    jdt2string(times_2[0]+julday(1,1,1601),string_2[0]);
    //printf("times: %lf = %s\n",times_2[0],string_2[0]);
    jdt2string(times_2[1]+julday(1,1,1601),string_2[1]);
    //printf("times: %lf = %s\n",times_2[0],string_2[1]);
    sprintf(workstring,"%s\n%s",string_2[0],string_2[1]);
    printf("utc_valid_range_days:\n%s\n",workstring);
    stat = nc_put_att_text(ncid, times_id, "utc_valid_range_days", strlen(workstring), workstring);
    check_ncerr(stat,__LINE__,__FILE__);
    
    /* store local time-of-day_bounds */
    ismin=0*60;
    iemin=8*60;
    times_2[0]=i+((float) ismin)/(24.*60.)-julday(1,1,1601);        // start time
    times_2[1]=i+((float) iemin)/(24.*60.)-julday(1,1,1601);        // end time
    times[0]=0.5*(times_2[0]+times_2[1]); // mean time	
    printf("Morning LTOD start=%lf stop=%lf time  (%d %d mins)\n",times_2[0],times_2[1],ismin,iemin);
    stat = nc_put_att_double(ncid, times_id, "valid_range", NC_DOUBLE, 2, times_2); check_ncerr(stat,__LINE__,__FILE__);
    
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
    check_ncerr(stat,__LINE__,__FILE__);

  } else {// LTD evening
    stat = nc_put_att_text(ncid, times_id, "units", 30, "days since 1601-01-01T00:00:00");  // local time indicated w/o Z
    check_ncerr(stat,__LINE__,__FILE__);
    stat = nc_put_att_text(ncid, times_id, "comment", 25, "Local time-of-day (ltod)"); check_ncerr(stat,__LINE__,__FILE__);
    /* store UTC_bounds */
    stat = nc_put_att_double(ncid, times_id, "utc_valid_range", NC_DOUBLE, 2, times_2); check_ncerr(stat,__LINE__,__FILE__);

    /* convert UTC_bounds to human readable strings */
    //printf("times: %lf = %s\n",times_2[0],string_2[0]);
    jdt2string(times_2[0]+julday(1,1,1601),string_2[0]);
    //printf("times: %lf = %s\n",times_2[0],string_2[0]);
    jdt2string(times_2[1]+julday(1,1,1601),string_2[1]);
    //printf("times: %lf = %s\n",times_2[0],string_2[1]);
    sprintf(workstring,"%s\n%s",string_2[0],string_2[1]);
    printf("ltod_valid_range_days:\n%s\n",workstring);
    stat = nc_put_att_text(ncid, times_id, "utc_valid_range_days", strlen(workstring), workstring); check_ncerr(stat,__LINE__,__FILE__);
    
    /* store local time-of-day_bounds */
    ismin=16*60;
    iemin=24*60;
    times_2[0]=i+((float) ismin)/(24.*60.)-julday(1,1,1601);        // start time
    times_2[1]=i+((float) iemin)/(24.*60.)-julday(1,1,1601);        // end time
    times[0]=0.5*(times_2[0]+times_2[1]); // mean time	
    printf("Eve LTOD start=%lf stop=%lf time  (%d %d mins)\n",times_2[0],times_2[1],ismin,iemin);
    stat = nc_put_att_double(ncid, times_id, "valid_range", NC_DOUBLE, 2, times_2); check_ncerr(stat,__LINE__,__FILE__);

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
    stat = nc_put_att_text(ncid, times_id, "ltod_valid_range_days", strlen(workstring), workstring); check_ncerr(stat,__LINE__,__FILE__);
  }
   

  if (INCLUDE_LATLON) {
     /* latitude variable */
     stat = nc_put_att_text(ncid, latitude_id, "standard_name", 8, "latitude"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, latitude_id, "long_name", 8, "latitude"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, latitude_id, "units", 13, "degrees_north"); check_ncerr(stat,__LINE__,__FILE__);
     latitude_valid_range[0] = -90;
     latitude_valid_range[1] = 90;
     stat = nc_put_att_double(ncid, latitude_id, "valid_range", NC_DOUBLE, 2, latitude_valid_range); check_ncerr(stat,__LINE__,__FILE__);
     latitude__FillValue[0] = -999;
     stat = nc_put_att_double(ncid, latitude_id, "_FillValue", NC_DOUBLE, 1, latitude__FillValue); check_ncerr(stat,__LINE__,__FILE__);

     /* longitude variable */
     stat = nc_put_att_text(ncid, longitude_id, "standard_name", 9, "longitude"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, longitude_id, "long_name", 9, "longitude"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, longitude_id, "units", 12, "degrees_east"); check_ncerr(stat,__LINE__,__FILE__);
     longitude_valid_range[0] = -180;
     longitude_valid_range[1] = 180;
     stat = nc_put_att_double(ncid, longitude_id, "valid_range", NC_DOUBLE, 2, longitude_valid_range); check_ncerr(stat,__LINE__,__FILE__);
     longitude__FillValue[0] = -999;
     stat = nc_put_att_double(ncid, longitude_id, "_FillValue", NC_DOUBLE, 1, longitude__FillValue); check_ncerr(stat,__LINE__,__FILE__);

     /* rows and cols variables */
     if (0) {     
       rows_valid_range[0] = 0;
       rows_valid_range[1] = rows_len-1;
       stat = nc_put_att_int(ncid, rows_id, "valid_range", NC_INT, 2, rows_valid_range); check_ncerr(stat,__LINE__,__FILE__);
       stat = nc_put_att_text(ncid, rows_id, "units", 5, "count"); check_ncerr(stat,__LINE__,__FILE__);
       stat = nc_put_att_text(ncid, rows_id, "long_name", 17, "rows in the image"); check_ncerr(stat,__LINE__,__FILE__);
       cols_valid_range[0] = 0;
       cols_valid_range[1] = cols_len-1;
       stat = nc_put_att_int(ncid, cols_id, "valid_range", NC_INT, 2, cols_valid_range); check_ncerr(stat,__LINE__,__FILE__);
       stat = nc_put_att_text(ncid, cols_id, "units", 5, "count"); check_ncerr(stat,__LINE__,__FILE__);
       stat = nc_put_att_text(ncid, cols_id, "long_name", 20, "columns in the image"); check_ncerr(stat,__LINE__,__FILE__);
     }
   
     /* xgrid and ygrid variables */
     stat = nc_put_att_text(ncid, xgrid_id, "standard_name", 23, "projection_x_coordinate"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, xgrid_id, "long_name", 25, "projection_grid_x_centers"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_double(ncid, xgrid_id, "valid_range", NC_DOUBLE, 2, xgrid_valid_range); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, xgrid_id, "units", 6, "meters"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, xgrid_id, "axis", 1, "X"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, ygrid_id, "standard_name", 23, "projection_y_coordinate"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, ygrid_id, "long_name", 25, "projection_grid_y_centers"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_double(ncid, ygrid_id, "valid_range", NC_DOUBLE, 2, ygrid_valid_range); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, ygrid_id, "units", 6, "meters"); check_ncerr(stat,__LINE__,__FILE__);
     stat = nc_put_att_text(ncid, ygrid_id, "axis", 1, "Y"); check_ncerr(stat,__LINE__,__FILE__);
  }
   
  /* global file attributes */
  stat = nc_put_att_text(ncid, NC_GLOBAL, "Conventions", 6, "CF-1.4");  check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, NC_GLOBAL, "institution", 39, "Brigham Young University SCP, Provo, UT  "); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, NC_GLOBAL, "title", strlen(title), title); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, NC_GLOBAL, "source", 49, "See the title, original_file name, and references "); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, NC_GLOBAL, "original_file", strlen(filename), filename); check_ncerr(stat,__LINE__,__FILE__);
  strncpy(line,&type[0],139);line[139]='\0';
  stat = nc_put_att_text(ncid, NC_GLOBAL, "comment", strlen(line), line); check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, NC_GLOBAL, "references", 72, "Documentation available at: http://NSIDC/something.pdf                 	");  check_ncerr(stat,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, NC_GLOBAL, "history", strlen(history), history); check_ncerr(stat,__LINE__,__FILE__);

  /* leave define mode */
  stat = nc_enddef (ncid); check_ncerr(stat,__LINE__,__FILE__);

  { /* store projection infor */
    // char lambert_azimuthal_equal_area = '\t';
    // stat = nc_put_var_text(ncid, projection_id, &lambert_azimuthal_equal_area);
    // check_ncerr(stat,__LINE__,__FILE__);
  }

  /* create short version of float array ## */
  prod_var=malloc(sizeof(short)*nsx*nsy);
  if (prod_var==NULL)
    printf("*** error allocating prod_var memory\n");
  for (i = 0; i < nsx * nsy; i++) {
    temp = rnd(((double) val[i] - (double) ioff) * (double) iscale);
    if (temp >  32767.0) temp=32767.0;
    if (temp < -32767.0) temp=-32767.0;
    prod_var[i] = (short) ((int) temp);
  }

  /* store prod_var array */
  prod_var_start[0] = 0;
  prod_var_start[1] = 0;
  prod_var_start[2] = 0;
  prod_var_count[0] = 1;   //times_len;
  prod_var_count[1] = nsy; //rows_len;
  prod_var_count[2] = nsx; //cols_len;
  stat = nc_put_vara_short(ncid, prod_var_id, prod_var_start, prod_var_count, prod_var);
  check_ncerr(stat,__LINE__,__FILE__);
  free(prod_var);

  /* store average of start/stop time*/
  times_start[0] = 0;
  times_count[0] = times_len;
  stat = nc_put_vara_double(ncid, times_id, times_start, times_count, times);
  check_ncerr(stat,__LINE__,__FILE__);

  /* store time_bounds */
  time_vertices_start[0] = 0;
  time_vertices_start[1] = 0;
  time_vertices_count[0] = times_len;
  time_vertices_count[1] = time_vertices_len;
  stat = nc_put_vara_double(ncid, time_vertices_id, time_vertices_start, time_vertices_count, times_2);
  check_ncerr(stat,__LINE__,__FILE__);

  if (INCLUDE_LATLON) {
     /* latitude & longitude */
    lat_size = rows_len * cols_len;
    latitude=malloc(sizeof(double)*lat_size);
    if (latitude==NULL)
      printf("*** error allocating latitude memory\n");

    longitude=malloc(sizeof(double)*lat_size);
    if (longitude==NULL)
      printf("*** error allocating longitude memory\n");
       
    /* compute lat/lon and any necessary transformation */
    printf("start image mapping\n");
    for (iy = 0; iy < rows_len; iy++) {     
      if (iy % 100==0) printf(" row %d of %d\n",iy,rows_len);      
      for (ix = 0; ix < cols_len; ix++) {
	j=ix+iy*cols_len;  // conventional row-order
	i=ix+(rows_len-iy-1)*cols_len;  // flipped row-order

	// note that .SIR pixels are indexed 1..nsx,1..nsy, rather than 0..nsx-1
	pixtolatlon((float)ix+1.5,(float)iy+1.5, &lonp, &latp, iopt, xdeg, ydeg, ascale, bscale, a0, b0);
	//printf("%d %d  %f %f  %lf %lf\n",ix,iy,lonp,latp,xgrid[ix],ygrid[rows_len-iy-1]);

	longitude[i]=(double) lonp;
	latitude[i]=(double) latp;
	 
	//printf("%d,%d=%d  %f,%f  %f,%f  %f,%f=%d\n",ix,iy,i,xp,yp,lonp,latp,x1,y1,j);
      }
    }
    printf("done image mapping\n");
     
    /* store latitude array */
    stat = nc_put_var_double(ncid, latitude_id, latitude); check_ncerr(stat,__LINE__,__FILE__);

    /* store longitude array */
    stat = nc_put_var_double(ncid, longitude_id, longitude); check_ncerr(stat,__LINE__,__FILE__);

    if (0) {
      /* store rows array */
      int rows[rows_len];
      int i;
      for (i=0; i<rows_len; i++)
	rows[i]=i;
      stat = nc_put_var_int(ncid, rows_id, rows); check_ncerr(stat,__LINE__,__FILE__);

      /* store cols array */
      int cols[cols_len];	
      for (i=0; i<cols_len; i++)	
	cols[i]=i;	
      stat = nc_put_var_int(ncid, cols_id, cols); check_ncerr(stat,__LINE__,__FILE__);
    }
  }

  /* create arrays of x and y values */
  /* note: this code does not work for EASE2 */
  for (ix = 0; ix < cols_len; ix++){
    if (iopt==5)
      xp=round(1000.0*((ix+0.5)*ascale+a0)); // polar stereographic projection X axis
    else
      xp=1000.0*((ix+0.5)/ascale+a0); // lambert equal area projection X axis
    xgrid[ix]=xp;  // store in m
  }

  for (iy = 0; iy < rows_len; iy++){
    if (iopt==5) 
      yp=round(1000.0*((iy+0.5)*bscale+b0)); // polar stereographic projection X axis
    else
      yp=1000.0*((iy+0.5)/bscale+b0); // lambert equal area projection Y axis
    ygrid[rows_len-iy-1]=yp; // flipped, store in m
  }
  
  /* store xgrid array */
  stat = nc_put_var_double(ncid, xgrid_id, xgrid); check_ncerr(stat,__LINE__,__FILE__); 

  /* store ygrid array */
  stat = nc_put_var_double(ncid, ygrid_id, ygrid); check_ncerr(stat,__LINE__,__FILE__);
  
  /* close output netcdf file */
  stat = nc_close(ncid); check_ncerr(stat,__LINE__,__FILE__);
  
  return(stat);
}


