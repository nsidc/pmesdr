/*
 * gsx- Utilities for Importing GSX files
 *
 *  gsx == eXtended Generic Swath
 *   all input swath files from each different sensor in the PMESDR project
 *   will be translated into gsx format before being read into the meas_meta system from BYU
 *
 * 03-Aug-2015 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "cetb.h"
#include "gsx.h"

/*
 * Private Functions
 */
static char *get_att_text( int fileid, int varid, const char* varname );
static gsx_class *get_gsx_file( char *filename );
static int get_gsx_dims( gsx_class *this );
static int get_gsx_global_attributes( gsx_class *this );
static int get_gsx_global_variables( gsx_class *this );
static int get_gsx_variable_attributes( gsx_class *this );
static int get_gsx_positions( gsx_class *this );
static int get_gsx_temperature( gsx_class *this, int varid, int count, int scans, int measurements );
static int init_gsx_pointers( gsx_class *this );

/*
 * this function takes a gsx file name and opens it as a netCDF4
 * file and returns a pointer to a structure that is populated with
 * the information in the input gsx file

 *
 *  Input:
 *    GSX file name
 *
 *  Return:
 *    pointer to gsx_struct
 *    NULL on failure
 */
gsx_class *gsx_init ( char *filename ) {

  gsx_class *this=NULL;
  int status;
  int counter;

  /* first check to make sure you have a netcdf file and get gsx version*/
  this = get_gsx_file( filename );
  if ( NULL == this ) { perror( __FUNCTION__ ); return NULL; }

  /* initialize all variable pointers in gsx_struct if you have a gsx file*/
  if ( NULL == this->gsx_version ) { perror( __FUNCTION__ ); return NULL; }
  status = init_gsx_pointers( this );
  if ( 0 != status ) { perror( __FUNCTION__ ); return NULL; }
    
  /* Now call get_gsx_dims to get more variables */
  status = get_gsx_dims( this );
  if ( 0 != status ) {
    fprintf( stderr, "%s: bad return from get_gsx_dims \n", __FUNCTION__ );
    return this;
  }
  
  /* Now get the global attributes */
  status = get_gsx_global_attributes( this );
  if ( 0 != status ) {
    fprintf( stderr, "%s: bad return from get_gsx_global_attributes \n", __FUNCTION__ );
    return this;
  }
  status = get_gsx_global_variables( this );
  if ( 0 != status ) {
    fprintf( stderr, "%s: bad return from get_gsx_global_variables \n", __FUNCTION__ );
    return this;
  }
  status = get_gsx_variable_attributes( this );
  if ( 0 != status ) {
    fprintf( stderr, "%s: bad return from get_gsx_variable_attributes \n", __FUNCTION__ );
    return this;
  }
  return this;

}

/*
 * this function takes a gsx structure and frees the memory
 * associated with any pointers and then then frees the entire struct
 *
 *  Input:
 *    pointer to gsx_struct
 *
 *  Return:
 *    void function
 */
void gsx_close ( gsx_class *this ) {
  int status;
  int counter;
  
  if ( NULL == this ) return;
  if ( status = nc_close( this->fileid ) ) {
    fprintf( stderr, "%s: nc_close error=%s \n",
  	     __FUNCTION__, nc_strerror(status) );
  }
  /* free the malloc'd arrays before you free the gsx_struct */
  if ( NULL != this->gsx_version ) free( this->gsx_version );
  if ( NULL != this->source_file ) free( this->source_file );
  if ( NULL != this->short_sensor ) free( this->short_sensor );
  if ( NULL != this->short_platform ) free( this->short_platform );
  if ( NULL != this->input_provider ) free( this->input_provider );
  if ( NULL != this->latitude_loc1 ) free( this->latitude_loc1 );
  if ( NULL != this->latitude_loc2 ) free( this->latitude_loc2 );
  if ( NULL != this->latitude_loc3 ) free( this->latitude_loc3 );
  if ( NULL != this->longitude_loc1 ) free( this->longitude_loc1 );
  if ( NULL != this->longitude_loc2 ) free( this->longitude_loc2 );
  if ( NULL != this->longitude_loc3 ) free( this->longitude_loc3 );
  if ( NULL != this->sc_latitude_loc1 ) free( this->sc_latitude_loc1 );
  if ( NULL != this->sc_latitude_loc2 ) free( this->sc_latitude_loc2 );
  if ( NULL != this->sc_latitude_loc3 ) free( this->sc_latitude_loc3 );
  if ( NULL != this->sc_longitude_loc1 ) free( this->sc_longitude_loc1 );
  if ( NULL != this->sc_longitude_loc2 ) free( this->sc_longitude_loc2 );
  if ( NULL != this->sc_longitude_loc3 ) free( this->sc_longitude_loc3 );
  if ( NULL != this->scantime_loc1 ) free( this->scantime_loc1 );
  if ( NULL != this->scantime_loc2 ) free( this->scantime_loc2 );
  if ( NULL != this->scantime_loc3 ) free( this->scantime_loc3 );
  if ( NULL != this->eia_loc1 ) free( this->eia_loc1 );
  if ( NULL != this->eia_loc2 ) free( this->eia_loc2 );
  if ( NULL != this->eia_loc3 ) free( this->eia_loc3 );
  if ( NULL != this->eaz_loc1 ) free( this->eaz_loc1 );
  if ( NULL != this->eaz_loc2 ) free( this->eaz_loc2 );
  if ( NULL != this->eaz_loc3 ) free( this->eaz_loc3 );
  for ( counter=0; counter<this->channel_number; counter++ ) {
    if ( NULL != this->channel_names[counter] ) free( this->channel_names[counter] );
    if ( NULL != this->efov[counter] ) free( this->efov[counter] );
    if ( NULL != this->brightness_temps[counter] ) free( this->brightness_temps[counter] );
  }
  free( this );
  return;
}

/*
 * this function takes a gsx_class structure pointer and populates
 * some of the dimension variables from the netcdf file
 *
 * Note - this function is only looking for the dimension vars that give the
 * number of scan lines and the number of measurements per scan line
 *
 * All other dimensions are ignored
 *
 *  Input:
 *    gsx_class struct
 *
 *  Return:
 *    status variable returned 0 on success
 *    
 */
int get_gsx_dims( gsx_class *this ) {
  int status;
  int i;
  size_t dim_length;
  char *dim_name;
  int nc_dims;
  int nc_vars;
  int nc_atts;
  int nc_unlimdims;

  if ( NULL == this ) return -1;

  if ( status = nc_inq( this->fileid, &nc_dims, &nc_vars, &nc_atts, &nc_unlimdims ) ) {
    fprintf( stderr, "%s: nc_inq error=%s: fileid=%d \n",
	     __FUNCTION__, nc_strerror(status), this->fileid );
    free( this );
    return -1;
  }

  this->dims = nc_dims;
  this->vars = nc_vars;
  this->atts = nc_atts;
  this->unlimdims = nc_unlimdims;

  dim_name = malloc( sizeof( char )*(NC_MAX_NAME+1) );
  if ( NULL == dim_name ) {
    fprintf( stderr, "%s: unable to allocate memory for dimension names\n", __FUNCTION__ );
    return -1;
  }
  
  for ( i = 0; i < this->dims; i++ ) {
    if ( status = nc_inq_dimname( this->fileid, i, dim_name ) ) {
      fprintf ( stderr, "%s: couldn't get dim name info error %s\n", __FUNCTION__, nc_strerror( status ) );
      return -1;
    }
    if ( status = nc_inq_dimlen( this->fileid, i, &dim_length ) ) {
      fprintf ( stderr, "%s: couldn't get dim length, error: %s\n", __FUNCTION__, nc_strerror( status ) );
      return -1;
    }
    if ( strncmp( dim_name, "scans_loc1", strlen( dim_name ) ) == 0 ) this->scans_loc1 = dim_length;
    if ( strncmp( dim_name, "scans_loc2", strlen( dim_name ) ) == 0 ) this->scans_loc2 = dim_length;
    if ( strncmp( dim_name, "scans_loc3", strlen( dim_name ) ) == 0 ) this->scans_loc3 = dim_length;
    if ( strncmp( dim_name, "measurements_loc1", strlen( dim_name ) ) == 0 ) this->measurements_loc1 = dim_length;
    if ( strncmp( dim_name, "measurements_loc2", strlen( dim_name ) ) == 0 ) this->measurements_loc2 = dim_length;
    if ( strncmp( dim_name, "measurements_loc3", strlen( dim_name ) ) == 0 ) this->measurements_loc3 = dim_length;
  }
  
  free( dim_name );

  return 0;
}

/*
 * this function takes a gsx_class struct and populates the global attributes
 *
 *  Input:
 *    gsx_class structure
 *
 *  Return:
 *    status variable returns 0 for success and non-zero on failure
 *    
 */
int get_gsx_global_attributes( gsx_class *this ) {
  
  int status;
  int att_len;

  if ( NULL == this ) {
    return -1;
  }

  this->source_file = get_att_text( this->fileid, NC_GLOBAL, "gsx_source" );
  if ( NULL == this->source_file ) {
    fprintf( stderr, "%s: no gsx_source\n", __FUNCTION__ );
    return -1;
  }

  this->short_platform = get_att_text( this->fileid, NC_GLOBAL, "short_platform" );
  if ( NULL == this->source_file ) {
    fprintf( stderr, "%s: no gsx_source\n", __FUNCTION__ );
    return -1;
  }

  this->short_sensor = get_att_text( this->fileid, NC_GLOBAL, "short_sensor" );
  if ( NULL == this->short_sensor ) {
    fprintf( stderr, "%s: no short_sensor\n", __FUNCTION__ );
    return -1;
  }

  this->input_provider = get_att_text( this->fileid, NC_GLOBAL, "input_provider" );
  if ( NULL == this->input_provider ) {
    fprintf( stderr, "%s: no input_provider\n", __FUNCTION__ );
    return -1;
  }

  return 0;
}
      
/*
 * this function takes a gsx_class struct and gets the names of the channels
 * in the file and populates the structure with the list
 * Also counts the number of channels in the file
 *
 *  Input:
 *    gsx_class structure
 *
 *  Return:
 *    status variable 0 on success and !=0 on failure
 *    
 */
int get_gsx_global_variables( gsx_class *this ) {
  int status;
  int att_len;
  int channel_number;
  char *channel_list;
  char *delim=",";
  int count;
  char *token;
  char *channel_ptr;
  char *space=" ";

  if ( NULL == this ) {
    return -1;
  }

  channel_list = get_att_text( this->fileid, NC_GLOBAL, "gsx_variables" );
  channel_ptr = channel_list;

  count = 0;
  while ( NULL != channel_ptr ) {
    token = strsep( &channel_ptr, delim );
    if ( 0 == strncmp( space, token, 1 ) ) token++;
    att_len = strlen( token );
    this->channel_names[count] = (char*)malloc( (size_t) att_len+1 );
    strcpy( this->channel_names[count], token );
    count++;
  }
  *(this->channel_names[count-1]+att_len) = '\0';
  
  this->channel_number = count;
  free( channel_list );

  return 0;
}

/*
 * this function takes a gsx_class structure and fills it with data for
 * each channel in the file
 *
 *  Input:
 *    gsx_class struct
 *
 *  Return:
 *    status is returned 0 on success and !=0 on failure
 *    
 */
int get_gsx_variable_attributes( gsx_class *this ) {
  int status;
  int att_len;
  int varid;
  char *delim=" ";
  int count;
  char *token;
  char *dim_ptr;
  float fillvalue;
  nc_type var_type;
  int ndims;
  int dimid[2];
  int natts;
  char *dimname;

  if ( NULL == this ) {
    return -1;
  }

  for ( count=0; count<this->channel_number; count++ ) {

    if ( status = nc_inq_varid( this->fileid, this->channel_names[count], &varid ) ) {
      fprintf( stderr, "%s: file id %d %d count '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, count, this->channel_names[count], nc_strerror( status ) );
      return -1;
    }

    if ( status = nc_inq_vardimid( this->fileid, varid, dimid ) ) {
      fprintf( stderr, "%s: couldn't get %s dimension ids\n", __FUNCTION__, this->channel_names[count] );
      return -1;
    }

    dimname = (char *)malloc(NC_MAX_NAME+1);
    if ( status = nc_inq_dimname( this->fileid, dimid[0], dimname ) ) {
      fprintf( stderr, "%s: couldn't get %s dimension name from id %d\n", \
	       __FUNCTION__, this->channel_names[count], dimid[0] );
      return -1;
    }

    dim_ptr = strstr( dimname, "_loc1");
    if ( NULL != dim_ptr ) { //scans_loc1 and measurements_loc1 for this variable
      status = get_gsx_temperature( this, varid, count, this->scans_loc1, this->measurements_loc1 );
    }

    dim_ptr = strstr( dimname, "_loc2");
    if ( NULL != dim_ptr ) { //scans_loc2 and measurements_loc2 for this variable
      status = get_gsx_temperature( this, varid, count, this->scans_loc2, this->measurements_loc2 );
    }
    
    dim_ptr = strstr( dimname, "_loc3");
    if ( NULL != dim_ptr ) { //scans_loc3 and measurements_loc3 for this variable
      status = get_gsx_temperature( this, varid, count, this->scans_loc3, this->measurements_loc3 );
    }

    if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &fillvalue) ) {
      fprintf( stderr, "%s: no fill value attribute for %s variable\n", \
	       __FUNCTION__, this->channel_names[count] );
      return -1;
    }
    this->fillvalue = fillvalue;
  }

  status = get_gsx_positions( this );

  return 0;
}

/*
 * this function takes a gsx_class struct and gets the position variables
 * file and returns a pointer to a structure that is populated with the
 * lat and lons as well as the spacecraft position per scan line and the angles
 * along the scan lines
 *
 *  Input:
 *    gsx_class structure
 *
 *  Return:
 *    returns 0 on success and !=0 on failures
 *    
 */
int get_gsx_positions( gsx_class *this ) {
  int status;

  status = 0;

  return status;

}

int get_gsx_temperature( gsx_class *this, int varid, int count, int scans, int measurements ) {
  int status=0;

  this->brightness_temps[count] = (float *)malloc( sizeof(float)*scans*measurements );
  if ( NULL != this->brightness_temps[count] ) {
    if ( status = nc_get_var_float( this->fileid, varid, this->brightness_temps[count] ) ) {
      fprintf( stderr, "%s: tb from %s\n", __FUNCTION__, this->channel_names[count] );
      status = -1;
    }
  } else {
    status = -1;
  }

  return status;
}
  
/*
 * this function takes a gsx_class structure and sets all pointers
 * EXCEPT for gsx_version to NULL
 * 
 * Note that the gsx_version is pulled from the file when it is first opened
 * as a way to check that this is a valid gsx file so this pointer should NOT
 * be nulled in this function

 *
 *  Input:
 *    pointer to gsx_struct
 *
 *  Return:
 *    status = 0 on success !=0 on failure
 *    
 */
int init_gsx_pointers( gsx_class *this ) {
  int status=0;
  int counter;

  this->source_file = NULL;
  this->short_sensor = NULL;
  this->short_platform = NULL;
  this->input_provider = NULL;
  this->latitude_loc1 = NULL;
  this->latitude_loc2 = NULL;
  this->latitude_loc3 = NULL;
  this->longitude_loc1 = NULL;
  this->longitude_loc2 = NULL;
  this->longitude_loc3 = NULL;
  this->sc_latitude_loc1 = NULL;
  this->sc_latitude_loc2 = NULL;
  this->sc_latitude_loc3 = NULL;
  this->sc_longitude_loc1 = NULL;
  this->sc_longitude_loc2 = NULL;
  this->sc_longitude_loc3 = NULL;
  this->scantime_loc1 = NULL;
  this->scantime_loc2 = NULL;
  this->scantime_loc3 = NULL;
  this->eia_loc1 = NULL;
  this->eia_loc2 = NULL;
  this->eia_loc3 = NULL;
  this->eaz_loc1 = NULL;
  this->eaz_loc2 = NULL;
  this->eaz_loc3 = NULL;
  for ( counter=0; counter < GSX_MAX_CHANNELS; counter++ ) {
    this->channel_names[counter] = NULL;
    this->efov[counter] = NULL;
    this->brightness_temps[counter] = NULL;
  }

  return status;
}

/*
 * function get_gsx_file
 *
 *  Input:
 *    filename to be opened and checked
 *
 *  Return:
 *    pointer to gsx_class struct upon 
 *    NULL if not a gsx file or file not found
 *
 */
gsx_class *get_gsx_file( char *filename ){
  int status;
  int nc_fileid;
  gsx_class *this;
  int att_len;
  
  if ( status = nc_open( filename, NC_NOWRITE, &nc_fileid ) ) {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    return NULL;
  }

  this = (gsx_class *)calloc(1, sizeof(gsx_class));
  if ( NULL == this ) { perror( __FUNCTION__ ); return NULL; }
  this->fileid = nc_fileid;

  /* now check for existence of gsx_version string indicating this is a gsx file */

  if ( status = nc_inq_attlen( this->fileid, NC_GLOBAL, "gsx_version", (size_t*)&att_len ) ) {
    fprintf( stderr, "%s: no gsx_version, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    free ( this );
    return NULL;
  }

  this->gsx_version = (char *)malloc( (size_t)(att_len+1) );
  if ( status = nc_get_att_text( this->fileid, NC_GLOBAL, "gsx_version", this->gsx_version ) ) { 
    fprintf( stderr, "%s: couldn't get gsx version, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    free( this );
    return NULL;
  }

  return this;
}

/*
 * get_att_text
 *
 *  Input:
 *    fileid - netcdf file id
 *    varid - the variable id of the attribute to be retrieved (cound be NC_GLOBAL)
 *    varname - the attribute name
 *
 *  Result:
 *    returns a pointer to the string returned from the file
 *    NULL is returned on failure
 *
 */
char *get_att_text( int fileid, int varid, const char* varname ) {
  int status=0;
  int att_len;
  char *att_text;

  if ( status = nc_inq_attlen( fileid, varid, varname, (size_t*)&att_len ) ) {
    fprintf( stderr, "%s: no attribute %s, error : %s\n", __FUNCTION__, varname, nc_strerror( status ) );
    return NULL;
  }
  
  att_text = (char *)malloc( (size_t)(att_len+1) );
  if ( NULL != att_text ) {
    if ( status = nc_get_att_text( fileid, varid, varname, att_text ) ) { 
      fprintf( stderr, "%s: couldn't get attribute %s string, error : %s\n", \
	     __FUNCTION__, varname, nc_strerror( status ) );
      return NULL;
    }
    *(att_text+att_len) = '\0';
    fprintf( stderr, "%s: attribute text for %s is %s\n", __FUNCTION__, varname, att_text );
  }
  
  return att_text;
}


  