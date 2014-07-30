/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  netcdf_dump.c

  read and write intermediate dump files

  written by DGL at BYU 15 May 2014

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <netcdf.h>  /* netCDF interface */

# define RANK_0 0	
# define RANK_1 1
# define RANK_2 2
# define RANK_3 3

/* global variables */

/********************************************************************/
/* error routines */
/********************************************************************/

void check_err(const int stat, const int line, const char *file) {
  if (stat != NC_NOERR) {
    (void) fprintf(stderr, "*** NETCDF ERR: line %d of %s: %s\n", line, file, nc_strerror(stat));
    exit(1);
  }
}

void check_err1(const int stat, char *id, const int line, const char *file) {
  if (stat != NC_NOERR) {
    (void) fprintf(stderr, "*** NETCDF VAR %s ERR: line %d of %s: %s\n", id, line, file, nc_strerror(stat));
    exit(1);
  }
}

void check_cerr(int stat, char *name, const int line, const char *file) {
  if (stat != NC_NOERR && stat != NC_ENOTATT ) {
    (void) fprintf(stderr, "*** NETCDF (copy) %s ERR: line %d of %s: %s\n", name, line, file, nc_strerror(stat));
    /* exit(1); */
  }
  if (stat == NC_ENOTATT )
    (void) fprintf(stderr, "* Attribute copy '%s': %s\n", name, nc_strerror(stat));
}

void check_rerr(int stat, char *name, const int line, const char *file) {
  if (stat != NC_NOERR) {
    (void) fprintf(stderr, "*** NETCDF (read) %s ERR: line %d of %s: %s\n", name, line, file, nc_strerror(stat));
    /* exit(1); */
  }
}

void check_werr(int stat, char *name, const int line, const char *file) {
  if (stat != NC_NOERR) {
    (void) fprintf(stderr, "*** NETCDF (write) %s ERR: line %d of %s: %s\n", name, line, file, nc_strerror(stat));
    /* exit(1); */
  }
}

/********************************************************************/
/* internal routines */
/********************************************************************/

int define_var_short(int ncid, char *short_name, char *long_name, int rank, int *dims, float scale, short offset, char *units, short *nodata)
{
  int var_id, stat;
  /* printf("define_var_short %d name=%s dims=%d %d %d rank=%d\n",ncid,short_name,dims[0],dims[1],dims[2],rank); */
  stat = nc_def_var(ncid, short_name, NC_SHORT, rank, dims, &var_id); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, var_id, "long_name", strlen(long_name), long_name); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_float(ncid, var_id, "scale_factor", NC_FLOAT, 1, &scale); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_short(ncid, var_id, "offset", NC_SHORT, 1, &offset); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, var_id, "units", strlen(units), units); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_short(ncid, var_id, "_NoData", NC_SHORT, 1, nodata); check_err1(stat,short_name,__LINE__,__FILE__);
  /*FillValue = -32767;
    stat = nc_put_att_short(ncid, var_id, "_FillValue", NC_SHORT, 1, &FillValue); check_err(stat,__LINE__,__FILE__);*/
  return(var_id);      
}


int define_var_int(int ncid, char *short_name, char *long_name, int rank, int *dims, float scale, int offset, char *units, int *nodata)
{
  int var_id, stat;
  /* printf("name=%s dims=%d %d %d\n",short_name,dims[0],dims[1],dims[2]); */  
  stat = nc_def_var(ncid, short_name, NC_INT, rank, dims, &var_id);  check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, var_id, "long_name", strlen(long_name), long_name); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_float(ncid, var_id, "scale_factor", NC_FLOAT, 1, &scale); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_int(ncid, var_id, "offset", NC_INT, 1, &offset); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, var_id, "units", strlen(units), units); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_int(ncid, var_id, "_NoData", NC_FLOAT, 1, nodata); check_err1(stat,short_name,__LINE__,__FILE__);
  /*FillValue = -32767;
    stat = nc_put_att_int(ncid, var_id, "_FillValue", NC_INT, 1, &FillValue); check_err(stat,__LINE__,__FILE__);*/
  return(var_id);      
}


int define_var_float(int ncid, char *short_name, char *long_name, int rank, int *dims, char *units, float *nodata)
{
  int var_id, stat;
  float scale=1.0, offset=0.0;  
  stat = nc_def_var(ncid, short_name, NC_FLOAT, rank, dims, &var_id);  check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, var_id, "long_name", strlen(long_name), long_name); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_float(ncid, var_id, "scale_factor", NC_FLOAT, 1, &scale); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_float(ncid, var_id, "offset", NC_FLOAT, 1, &offset); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_text(ncid, var_id, "units", strlen(units), units); check_err1(stat,short_name,__LINE__,__FILE__);
  stat = nc_put_att_float(ncid, var_id, "_NoData", NC_FLOAT, 1, nodata); check_err1(stat,short_name,__LINE__,__FILE__);
  /*FillValue = -999.0;
    stat = nc_put_att_float(ncid, var_id, "_FillValue", NC_FLOAT, 1, &FillValue); check_err(stat,__LINE__,__FILE__);*/
  return(var_id);      
}


/* note: going in/out of define mode for each definition is not efficient
   but it simplifies code interface */

int write_gatt_int_(int *ncid, char *name, int *val)
{
  int stat, stat1;
  /* re-enter define mode */
  stat = nc_redef(*ncid);  check_err(stat,__LINE__,__FILE__);
  /* write global attribute as int */
  /* printf("put global attribute %d %d >%s< %d\n",*ncid,strlen(text),name,*int); */
  stat1 = nc_put_att_int(*ncid, NC_GLOBAL, name, NC_INT, 1, val); check_err(stat1,__LINE__,__FILE__);  
  /* leave define mode so we can begin variable writing */
  stat = nc_enddef(*ncid);  check_err(stat,__LINE__,__FILE__);
  return(stat1);  
}

int write_gatt_float_(int *ncid, char *name, float *val)
{
  int stat, stat1;
  /* re-enter define mode */
  stat = nc_redef(*ncid);  check_err(stat,__LINE__,__FILE__);
  /* write global attribute as int */
  stat1 = nc_put_att_float(*ncid, NC_GLOBAL, name, NC_FLOAT, 1, val); check_err(stat1,__LINE__,__FILE__);  
  /* leave define mode so we can begin variable writing */
  stat = nc_enddef(*ncid);  check_err(stat,__LINE__,__FILE__);
  return(stat1);  
}

int write_gatt_double_(int *ncid, char *name, double *val)
{
  int stat, stat1;
  /* re-enter define mode */
  stat = nc_redef(*ncid);  check_err(stat,__LINE__,__FILE__);
  /* write global attribute as int */
  stat1 = nc_put_att_double(*ncid, NC_GLOBAL, name, NC_DOUBLE, 1, val); check_err(stat1,__LINE__,__FILE__);  
  /* leave define mode so we can begin variable writing */
  stat = nc_enddef(*ncid);  check_err(stat,__LINE__,__FILE__);
  return(stat1);  
}


int write_gatt_text_(int *ncid, char *name, char *text)
{
  int stat, stat1;
  /* re-enter define mode */
  stat = nc_redef(*ncid);  check_err(stat,__LINE__,__FILE__);
  /* write global attribute as text */
   stat1 = nc_put_att_text(*ncid, NC_GLOBAL, name, strlen(text), text); check_err(stat1,__LINE__,__FILE__);  
  /* leave define mode so we can begin variable writing */
  stat = nc_enddef(*ncid);  check_err(stat,__LINE__,__FILE__);
  return(stat1);  
}


int read_gatt_text_(int *ncid, char *name, char *text)
{ /* CAUTION: output text array MUST be dimensioned large enough to recieve string! */
  int stat;
  stat = nc_get_att_text(*ncid, NC_GLOBAL, name, text); check_err(stat,__LINE__,__FILE__);  
  return(stat);  
}

int read_gatt_int_(int *ncid, char *name, int *val)
{
  int stat;
  stat = nc_get_att_int(*ncid, NC_GLOBAL, name, val); check_err(stat,__LINE__,__FILE__);  
  return(stat);  
}

int read_gatt_float_(int *ncid, char *name, float *val)
{
  int stat;
  stat = nc_get_att_float(*ncid, NC_GLOBAL, name, val); check_err(stat,__LINE__,__FILE__);  
  return(stat);  
}

int read_gatt_double_(int *ncid, char *name, double *val)
{
  int stat;
  stat = nc_get_att_double(*ncid, NC_GLOBAL, name, val); check_err(stat,__LINE__,__FILE__);  
  return(stat);  
}



/********************************************************************/
/* external routines */
/********************************************************************/

int nc_open_file_write_head(char *outfile, int *ncid_in, int nsx, int nsy, int iopt, 
			    float ascale, float bscale, float a0, float b0, float xdeg, float ydeg, 
			    int isday, int ieday, int ismin, int iemin, int iyear, int iregion, int ipol, 
			    int nsx2, int nsy2, int non_size_x, int non_size_y, float ascale2, float bscale2, 
			    float a02, float b02, float xdeg2, float ydeg2,
			    float a_init, int ibeam, int nits, int median_flag, int nout)
{
  int ncid, stat, nsx_dim, nsy_dim, nsx2_dim, nsy2_dim;
  
  /* create new netcdf file, clobbering existing file if necessary */
  stat = nc_create(outfile, NC_CLOBBER, &ncid);  check_err(stat,__LINE__,__FILE__);
  *ncid_in=ncid;  
  if (stat != 0) return(stat);

  /* define primary dimensions */
  //stat = nc_def_dim(ncid, "nsx", nsx, &nsx_dim);  check_err(stat,__LINE__,__FILE__);
  //stat = nc_def_dim(ncid, "nsy", nsy, &nsy_dim);  check_err(stat,__LINE__,__FILE__);
  //stat = nc_def_dim(ncid, "nsx2", nsx2, &nsx2_dim);  check_err(stat,__LINE__,__FILE__);
  //stat = nc_def_dim(ncid, "nsy2", nsy2, &nsy2_dim);  check_err(stat,__LINE__,__FILE__);

  /* leave define mode to enable use for write_gatt routines */
  stat = nc_enddef(ncid);  check_err(stat,__LINE__,__FILE__);

  /* write variables as global attributes */
  stat = write_gatt_int_(&ncid, "nsx", &nsx); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "nsy", &nsy); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "iopt", &iopt); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "ascale", &ascale); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "bscale", &bscale); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "a0", &a0); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "b0", &b0); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "xdeg", &xdeg); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "ydeg", &ydeg); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "isday", &isday); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "ieday", &ieday); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "ismin", &ismin); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "iemin", &iemin); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "iyear", &iyear); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "iregion", &iregion); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "ipol", &ipol); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "nsx2", &nsx2); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "nsy2", &nsy2); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "non_size_x", &non_size_x); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "non_size_y", &non_size_y); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "a02", &a02); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "ascale2", &ascale2); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "bscale2", &bscale2); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "b02", &b02); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "xdeg2", &xdeg2); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "ydeg2", &ydeg2); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_float_(&ncid, "a_init", &a_init); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "ibeam", &ibeam); check_err(stat,__LINE__,__FILE__);  
  stat = write_gatt_int_(&ncid, "nits", &nits); check_err(stat,__LINE__,__FILE__);  
  stat = write_gatt_int_(&ncid, "median_flag", &median_flag); check_err(stat,__LINE__,__FILE__);
  stat = write_gatt_int_(&ncid, "Nout", &nout); check_err(stat,__LINE__,__FILE__);

  return(stat);  
}

int add_string_nc(int ncid, char *name, char *str, int maxc)
{
  int stat, i;
  char text[1024];

  /* to be save, copy string to zero-filled buffer */
  for (i=0;i<1024;) 
    text[i++]='\0';
  strncpy(text,str,maxc);  

  /* write string as a global attribute */
  stat = write_gatt_text_(&ncid, name, text); check_werr(stat,"write string",__LINE__,__FILE__);
  return(stat);
}

int add_float_array_nc(int ncid, char *name, float *val, int nsx, int nsy, float anodata)
{
  int stat, var_id, dims[3], nsx_dim, nsy_dim;
  static size_t wstart[3], wcount[3];
  char str[250];  

  /* re-enter define mode */
  stat = nc_redef(ncid); check_err(stat,__LINE__,__FILE__);

  /* define custom dimension for this array */
  sprintf(str,"%s_nsx",name);  
  stat = nc_def_dim(ncid, str, nsx, &nsx_dim); check_err(stat,__LINE__,__FILE__);

  sprintf(str,"%s_nsy",name);  
  stat = nc_def_dim(ncid, str, nsy, &nsy_dim); check_err(stat,__LINE__,__FILE__);

  dims[0] =nsx_dim;
  dims[1] =nsy_dim;
  wstart[0]=0;
  wstart[1]=0;
  wcount[0]=nsx;
  wcount[1]=nsy;

  /* define variable */
  var_id=define_var_float(ncid, name, name, RANK_2, dims, "N/A", &anodata);
  /* leave define mode so we can begin variable writing */
  stat = nc_enddef(ncid);  check_err(stat,__LINE__,__FILE__);
  /* write variable */
  stat = nc_put_vara_float(ncid, var_id, wstart, wcount, val); 

  return(stat);

}

int nc_close_file(int ncid)
{
  int stat;
  stat = nc_close(ncid); check_werr(stat,"close file",__LINE__,__FILE__);
  return(stat);  
}



int nc_open_file_read_head(char *filename, int *ncid_in, int *nsx, int *nsy, int *iopt, 
		 float *ascale, float *bscale, float* a0, float *b0, float *xdeg, float *ydeg, 
		 int *isday, int *ieday, int *ismin, int *iemin, int *iyear, int *iregion, int *ipol, 
		 int *nsx2, int *nsy2, int *non_size_x, int *non_size_y, float *ascale2, float *bscale2, 
		 float *a02, float *b02, float *xdeg2, float *ydeg2,
			   float *a_init, int *ibeam, int *nits, int *median_flag, int *nout)
{
  int ncid, stat;
  int ndims_in, nvars_in, ngatts_in, unlimdimid_in;
  char n1[25], n2[25];  
  size_t d1, d2;
  
  /* open existing netcdf file, failing if file does not exist */
  stat = nc_open(filename, NC_NOWRITE, &ncid); check_err(stat,__LINE__,__FILE__);
  *ncid_in=ncid;  
  if (stat != 0) return(stat);

  /* determine number of parameters in netcdf file */
  stat = nc_inq(ncid, &ndims_in, &nvars_in, &ngatts_in, &unlimdimid_in); check_err(stat,__LINE__,__FILE__);
  
  printf("NetCDF file %s\n has %d dimensions   %d variables  %d global attributes  %d unlimited\n",filename, ndims_in, nvars_in, ngatts_in, unlimdimid_in);  

  /* define primary dimensions, which will be in order they were written */
  //stat = nc_inq_dim(ncid, 0, n1, &d1 ); check_err(stat,__LINE__,__FILE__); /* nsx */
  //stat = nc_inq_dim(ncid, 1, n2, &d2 ); check_err(stat,__LINE__,__FILE__); /* nsy */
  //nsx=d1;
  //nsy=d2;  
  //stat = nc_inq_dim(ncid, 2, n1, &d1 ); check_err(stat,__LINE__,__FILE__); /* nsx */
  //stat = nc_inq_dim(ncid, 3, n2, &d2 ); check_err(stat,__LINE__,__FILE__); /* nsy */
  //nsx2=d1;
  //nsy2=d2;  

  /* read variables from global attributes */
  stat = read_gatt_int_(&ncid, "nsx", nsx); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "nsy", nsy); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "iopt", iopt); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "ascale", ascale); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "bscale", bscale); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "a0", a0); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "b0", b0); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "xdeg", xdeg); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "ydeg", ydeg); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "isday", isday); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "ieday", ieday); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "ismin", ismin); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "iemin", iemin); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "iyear", iyear); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "iregion", iregion); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "ipol", ipol); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "nsx2", nsx2); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "nsy2", nsy2); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "non_size_x", non_size_x); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "non_size_y", non_size_y); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "a02", a02); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "ascale2", ascale2); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "bscale2", bscale2); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "b02", b02); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "xdeg2", xdeg2); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "ydeg2", ydeg2); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_float_(&ncid, "a_init", a_init); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "ibeam", ibeam); check_err(stat,__LINE__,__FILE__);  
  stat = read_gatt_int_(&ncid, "nits", nits); check_err(stat,__LINE__,__FILE__);  
  stat = read_gatt_int_(&ncid, "median_flag", median_flag); check_err(stat,__LINE__,__FILE__);
  stat = read_gatt_int_(&ncid, "Nout", nout); check_err(stat,__LINE__,__FILE__);

  return(stat);
}

int get_string_nc(int ncid, char *name, char *str, int maxc)
{
  int stat,i;
  char text[1024];
  
  /* since read routnine does not zero terminate, first fill read buffer with zeros */
  for (i=0; i<1024;)
    text[i++]='\0';  
  stat = read_gatt_text_(&ncid, name, text); check_err(stat,__LINE__,__FILE__);
  strncpy(str,text,maxc);

  return(stat);  
}


int get_float_array_nc(int ncid, char *name, float *val, int *nsx, int *nsy, float *anodata)
{
  /* caution: array val must be large enough to receive all the data in the array */

  int stat, var_id, dims[3], nsx_id, nsy_id;
  static size_t wstart[3], wcount[3], nsx_dim, nsy_dim;
  char str[250];  

  /* get variable dimensions */
  sprintf(str,"%s_nsx",name);
  stat = nc_inq_dimid(ncid, str, &nsx_id); check_rerr(stat,name,__LINE__,__FILE__);
  stat = nc_inq_dimlen(ncid, nsx_id, &nsx_dim); check_rerr(stat,name,__LINE__,__FILE__);
  *nsx=nsx_dim;  
  sprintf(str,"%s_nsy",name);
  stat = nc_inq_dimid(ncid, str, &nsy_id); check_rerr(stat,name,__LINE__,__FILE__);
  stat = nc_inq_dimlen(ncid, nsy_id, &nsy_dim); check_rerr(stat,name,__LINE__,__FILE__);
  *nsy=nsy_dim;  

  /* read the full array*/
  wstart[0] = 0;
  wstart[1] = 0;
  wcount[0] = *nsx;
  wcount[1] = *nsy;
 
  /* get variable info and array values */
  stat = nc_inq_varid(ncid, name, &var_id); check_rerr(stat,name,__LINE__,__FILE__);
  stat = nc_get_att_float(ncid, var_id, "_NoData", anodata); check_rerr(stat,name,__LINE__,__FILE__);
  stat = nc_get_vara_float(ncid, var_id, wstart, wcount, val); check_rerr(stat,name,__LINE__,__FILE__);

  return(stat);
}
