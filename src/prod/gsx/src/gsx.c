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
  
  fprintf( stderr, "\n%s: gsx file name = %s \n", __FUNCTION__, filename );
  if ( status = nc_open( filename, NC_NOWRITE, &nc_fileid ) ) {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    free( this );
    return NULL;
  }

  this->fileid = nc_fileid;
  /* initialize all variable pointers in gsx_struct */
  this->source_file = NULL;
  this->short_sensor = NULL;
  this->short_platform = NULL;
  this->input_provider = NULL;
  for ( counter=0; counter < GSX_MAX_CHANNELS; counter++ ) {
    this->channel_names[counter] = NULL;
  }

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
  if ( this->dims == 0 ) return this;
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
  for ( counter=0; counter<this->channel_number; counter++ ) {
    if ( NULL != this->channel_names[counter] ) free( this->channel_names[counter] );
  }
  
  free( this );
  return;
}

int gsx_inq_dims( gsx_class *this ) {
  int status;
  int i;
  size_t dim_length;
  char *dim_name[GSX_MAX_DIMS];

  if ( NULL == this ) return -1;

  for ( i = 0; i < this->dims; i++ ) {
    dim_name[i] = malloc( sizeof( char )*(NC_MAX_NAME+1) );
    if ( NULL == dim_name[i] ) {
      fprintf( stderr, "%s: unable to allocate memory for dimension names\n", __FUNCTION__ );
      return -1;
    } 
    if ( status = nc_inq_dimname( this->fileid, i, dim_name[i] ) ) {
      fprintf ( stderr, "%s: couldn't get dim name info error %s\n", __FUNCTION__, nc_strerror( status ) );
      return -1;
    }
    if ( status = nc_inq_dimlen( this->fileid, i, &dim_length ) ) {
      fprintf ( stderr, "%s: couldn't get dim length, error: %s\n", __FUNCTION__, nc_strerror( status ) );
      return -1;
    }
    if ( strncmp( dim_name[i], "scans_loc1", strlen( dim_name[i] ) ) == 0 ) this->scans_loc1 = dim_length;
    if ( strncmp( dim_name[i], "scans_loc2", strlen( dim_name[i] ) ) == 0 ) this->scans_loc2 = dim_length;
    if ( strncmp( dim_name[i], "scans_loc3", strlen( dim_name[i] ) ) == 0 ) this->scans_loc3 = dim_length;
    if ( strncmp( dim_name[i], "measurements_loc1", strlen( dim_name[i] ) ) == 0 ) this->measurements_loc1 = dim_length;
    if ( strncmp( dim_name[i], "measurements_loc2", strlen( dim_name[i] ) ) == 0 ) this->measurements_loc2 = dim_length;
    if ( strncmp( dim_name[i], "measurements_loc3", strlen( dim_name[i] ) ) == 0 ) this->measurements_loc3 = dim_length;
  }
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

  //  fprintf( stderr, "%s: into function\n\n", __FUNCTION__ );
  if ( status = nc_inq_attlen( this->fileid, NC_GLOBAL, "gsx_source", (size_t *)&att_len ) ) {
    fprintf( stderr, "%s: no gsx source file length, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  } else {
    //fprintf( stderr, "%s: got length of gsx_source %d\n", __FUNCTION__, att_len );
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

  channel_list = (char*)malloc( (size_t) (att_len+1) );
  if ( status = nc_get_att_text( this->fileid, NC_GLOBAL, "gsx_variables", channel_list ) ) {
    fprintf( stderr, "%s: couldn't retrieve gsx_variables, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    return -1;
  }
  channel_ptr = channel_list;

  count = 0;
  while ( NULL != channel_list ) {
    token = strsep( &channel_list, delim );
    if ( 0 == strncmp( space, token, 1 ) ) token++;
    att_len = strlen( token );
    this->channel_names[count] = (char*)malloc( (size_t) att_len+1 );
    strcpy( this->channel_names[count], token );
    count++;
  }
  
  this->channel_number = count;

  return 0;
}

int gsx_inq_variable_attributes( gsx_class *this ) {
  int status;
  int att_len;
  int varid;
  char *channel_list;
  char *delim=" ";
  int count;
  char *token;
  char *channel_ptr;
  float fillvalue;
  nc_type var_type;
  int ndims;
  int dimid;
  int natts;

  if ( NULL == this ) {
    return -1;
  }

  for ( count=0; count<this->channel_number; count++ ) {

    if ( status = nc_inq_varid( this->fileid, this->channel_names[count], &varid ) ) {
      fprintf( stderr, "%s: no variable named '%s', error : %s\n",	\
	       __FUNCTION__, this->channel_names[0], nc_strerror( status ) );
      fprintf( stderr, "%s: comparison is %d\n", __FUNCTION__, strncmp( this->channel_names[count], delim, 1 ) );
      return -1;
    }

    //if ( 0 == strncmp( this->channel_names[0], delim, 1 ) ) fprintf( stderr, "%s: Hallelujah!\n", __FUNCTION__ );
    if ( status = nc_inq_varndims( this->fileid, varid, &ndims ) ) {
      fprintf( stderr, "%s: couldn't get %s variable attributes\n", __FUNCTION__, this->channel_names[count] );
      return -1;
    }

    fprintf( stderr, "%s: channel name is '%s' and variable ID is %d and ndims are %d\n", \
	     __FUNCTION__, this->channel_names[count], varid, ndims );

    if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &fillvalue) ) {
      fprintf( stderr, "%s: no fill value attribute for %s variable\n", \
	       __FUNCTION__, this->channel_names[count] );
      return -1;
    }

    this->fillvalue = fillvalue;
  }

  return 0;
}
  
  
  


  
