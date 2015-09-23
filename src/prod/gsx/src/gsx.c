/*
 * gsx- Utilities for Importing GSX files
 *
 * 03-Aug-2015 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "gsx.h"

int gsx_version ( void ) {
  fprintf( stderr, "%s: gsx version = %s \n",
	     __FUNCTION__, GSX_VERSION );
  fprintf( stderr, "%s: netcdf version = %s \n",
	   __FUNCTION__, nc_inq_libvers() );

  return 1;
}

gsx_class *gsx_init ( char *filename ) {

  gsx_class *this=NULL;
  int status;
  int nc_fileid;
  int nc_dims;
  int nc_vars;
  int nc_atts;
  int nc_unlimdims;
  int counter;

  this = (gsx_class *)calloc(1, sizeof(gsx_class));
  if ( NULL == this ) { perror( __FUNCTION__ ); return NULL; }
  
  //fprintf( stderr, "\n%s: gsx file name = %s \n", __FUNCTION__, filename );
  if ( status = nc_open( filename, NC_NOWRITE, &nc_fileid ) ) {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    free( this );
    return NULL;
  }

  this->fileid = nc_fileid;
  /* initialize all variable pointers in gsx_struct */
  status = gsx_init_pointers( this );

  if ( status = nc_inq( this->fileid, &nc_dims, &nc_vars, &nc_atts, &nc_unlimdims ) ) {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    free( this );
    return NULL;
  }

  this->dims = nc_dims;
  this->vars = nc_vars;
  this->atts = nc_atts;
  this->unlimdims = nc_unlimdims;

  /* Now call gsx_inq_dims to get more variables */
  /* only make the next calls if you have a valid gsx file */
  if ( 3 >= this->dims ) return this;
  status = gsx_inq_dims( this );

  if ( 0 != status ) {
    fprintf( stderr, "%s: bad return from gsx_inq_dims \n", __FUNCTION__ );
    return this;
  }
  /* Now get the global attributes */
  status = gsx_inq_global_attributes( this );
  if ( 0 != status ) {
    fprintf( stderr, "%s: bad return from gsx_inq_global_attributes \n", __FUNCTION__ );
    return this;
  }
  status = gsx_inq_global_variables( this );
  if ( 0 != status ) {
    fprintf( stderr, "%s: bad return from gsx_inq_global_variables \n", __FUNCTION__ );
    return this;
  }
  status = gsx_inq_variable_attributes( this );
  if ( 0 != status ) {
    fprintf( stderr, "%s: bad return from gsx_inq_variable_attributes \n", __FUNCTION__ );
    return this;
  }
  return this;

}

void gsx_close ( gsx_class *this ) {
  int status;
  int counter;
  
  if ( NULL == this ) return;
  if ( status = nc_close( this->fileid ) ) {
    fprintf( stderr, "%s: nc_close error=%s \n",
  	     __FUNCTION__, nc_strerror(status) );
  }
  /* free the malloc'd arrays before you free the gsx_struct */
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

int gsx_inq_dims( gsx_class *this ) {
  int status;
  int i;
  size_t dim_length;
  char *dim_name;

  if ( NULL == this ) return -1;

  for ( i = 0; i < this->dims; i++ ) {
    dim_name = malloc( sizeof( char )*(NC_MAX_NAME+1) );
    if ( NULL == dim_name ) {
      fprintf( stderr, "%s: unable to allocate memory for dimension names\n", __FUNCTION__ );
      return -1;
    } 
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

int gsx_inq_global_attributes( gsx_class *this ) {
  
  int status;
  char *source_file;
  char *platform;
  char *sensor;
  char *input_provider;
  int att_len;

  if ( NULL == this ) {
    return -1;
  }

  if ( status = nc_inq_attlen( this->fileid, NC_GLOBAL, "gsx_source", (size_t *)&att_len ) ) {
    fprintf( stderr, "%s: no gsx source file length, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  } else {
    source_file = (char *)malloc( ( size_t )att_len+1);
  }
  if ( status = nc_get_att_text( this->fileid, NC_GLOBAL, "gsx_source", source_file ) ) {
    fprintf( stderr, "%s: no gsx source file, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  this->source_file = source_file;
  *(this->source_file+att_len) = '\0';

  if ( status = nc_inq_attlen( this->fileid, NC_GLOBAL, "short_platform", (size_t*)&att_len ) ) {
    fprintf( stderr, "%s: no gsx short platform, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }

  platform = ( char * )malloc( ( size_t )att_len+1 );
  if ( status = nc_get_att_text( this->fileid, NC_GLOBAL, "short_platform", platform ) ) {
    fprintf( stderr, "%s: no gsx short platform, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  this->short_platform = platform;
  *(this->short_platform+att_len) = '\0';
  
  if ( status = nc_inq_attlen( this->fileid, NC_GLOBAL, "short_sensor", (size_t*)&att_len ) ) {
    fprintf( stderr, "%s: no gsx short sensor, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  sensor = ( char * )malloc( (size_t) att_len+1 );
  if ( status = nc_get_att_text( this->fileid, NC_GLOBAL, "short_sensor", sensor ) ) {
    fprintf( stderr, "%s: no gsx short_sensor, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  this->short_sensor = sensor;
  *(this->short_sensor+att_len) = '\0';

  if ( status = nc_inq_attlen( this->fileid, NC_GLOBAL, "input_provider", (size_t*)&att_len ) ) {
    fprintf( stderr, "%s: no gsx input provider, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  input_provider = ( char * )malloc( (size_t) att_len+1 );
  if ( status = nc_get_att_text( this->fileid, NC_GLOBAL, "input_provider", input_provider ) ) {
    fprintf( stderr, "%s: no gsx input provider, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  this->input_provider = input_provider;
  *(this->input_provider+att_len) = '\0';
  
  return 0;
}
      
int gsx_inq_global_variables( gsx_class *this ) {
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

  if ( status = nc_inq_attlen( this->fileid, NC_GLOBAL, "gsx_variables", (size_t*)&att_len ) ) {
    fprintf( stderr, "%s: no gsx_variables, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  //  fprintf( stderr, "%s: %d is length from nc_inq_attlen\n", __FUNCTION__, att_len );

  channel_list = (char*)malloc( (size_t) (att_len+1) );
  if ( status = nc_get_att_text( this->fileid, NC_GLOBAL, "gsx_variables", channel_list ) ) {
    fprintf( stderr, "%s: couldn't retrieve gsx_variables, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  *(channel_list+att_len) = '\0';
  
  channel_ptr = channel_list;

  //fprintf( stderr, "%s: length of channel_list %d \n\t channel_list '%s' and \n\t channel_ptr '%s'\n", \
  //	   __FUNCTION__, (int)strlen( channel_list ), channel_list, channel_ptr );

  count = 0;
  while ( NULL != channel_ptr ) {
    token = strsep( &channel_ptr, delim );
    if ( 0 == strncmp( space, token, 1 ) ) token++;
    att_len = strlen( token );
    this->channel_names[count] = (char*)malloc( (size_t) att_len+1 );
    strcpy( this->channel_names[count], token );
    //fprintf( stderr, "%s: channel name is '%s' and token '%s'\n", __FUNCTION__, this->channel_names[count], token ); 
    count++;
  }
  *(this->channel_names[count-1]+att_len) = '\0';
  
  this->channel_number = count;
  free( channel_list );

  return 0;
}

int gsx_inq_variable_attributes( gsx_class *this ) {
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
      fprintf( stderr, "%s: comparison is %d\n", __FUNCTION__, strncmp( this->channel_names[count], delim, 1 ) );
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
      status = gsx_get_temperature( this, varid, count, this->scans_loc1, this->measurements_loc1 );
    }

    dim_ptr = strstr( dimname, "_loc2");
    if ( NULL != dim_ptr ) { //scans_loc2 and measurements_loc2 for this variable
      status = gsx_get_temperature( this, varid, count, this->scans_loc2, this->measurements_loc2 );
    }
    
    if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &fillvalue) ) {
      fprintf( stderr, "%s: no fill value attribute for %s variable\n", \
	       __FUNCTION__, this->channel_names[count] );
      return -1;
    }
    this->fillvalue = fillvalue;
  }

  status = gsx_get_positions( this );

  return 0;
}

int gsx_get_positions( gsx_class *this ) {
  int status;

  status = 0;

  return status;

}

int gsx_get_temperature( gsx_class *this, int varid, int count, int scans, int measurements ) {
  int status;
  float *tb;

  status = 0;
  tb = (float *)malloc( sizeof(float)*scans*measurements );
  if ( status = nc_get_var_float( this->fileid, varid, tb ) ) {
    fprintf( stderr, "%s: tb from %s\n", __FUNCTION__, this->channel_names[count] );
    return -1;
  }

  this->brightness_temps[count] = tb;
  return status;
}
  
int gsx_init_pointers( gsx_class *this ) {
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
  
  

  


  
