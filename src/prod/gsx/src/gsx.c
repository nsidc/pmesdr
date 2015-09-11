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

  this = (gsx_class *)calloc(1, sizeof(gsx_class));
  if ( NULL == this ) { perror( __FUNCTION__ ); return NULL; }
  
  fprintf( stderr, "%s: gsx file name = %s \n",
	     __FUNCTION__, filename );
  if ( status = nc_open( filename, NC_NOWRITE, &nc_fileid ) ) {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    free( this );
    return NULL;
  }

  this->fileid = nc_fileid;

  if ( status = nc_inq( this->fileid, &nc_dims, &nc_vars, &nc_atts, &nc_unlimdims ) ) {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    free( this );
    return NULL;
  }

  //fprintf( stderr, "%s: nc_inq return=%s: ndims=%d, nvars=%d, natts=%d, nunlimdims=%d \n",
  //	   __FUNCTION__, nc_strerror(status), nc_dims, nc_vars, nc_atts, nc_unlimdims );
  
  this->dims = nc_dims;
  this->vars = nc_vars;
  this->atts = nc_atts;
  this->unlimdims = nc_unlimdims;

  /* Now call gsx_inq_dims to get more variables */
  status = gsx_inq_dims( this );

  return this;

}

void gsx_close ( gsx_class *this ) {
  int status;
  
  if ( NULL == this ) return;
  if ( status = nc_close( this->fileid ) ) {
    fprintf( stderr, "%s: nc_close error=%s \n",
  	     __FUNCTION__, nc_strerror(status) );
  }
  free( this );
  return;
}

int gsx_inq_dims( gsx_class *this ) {
  int status;
  int i;
  size_t dim_length;
  char *dim_name[GSX_MAX_DIMS];

  if ( NULL == this ) return;

  for ( i = 0; i < this->dims; i++ ) {
    dim_name[i] = malloc( sizeof( char )*(NC_MAX_NAME+1) );
    if ( NULL == dim_name[i] ) {
      fprintf( stderr, "%s: unable to allocate memory for dimension names\n", __FUNCTION__ );
      return -1;
    } else {
      //fprintf( stderr, "%s: memallocated for %dth char pointer \n", __FUNCTION__, i );
    }
    if ( status = nc_inq_dimname( this->fileid, i, dim_name[i] ) ) {
      fprintf ( stderr, "%s: couldn't get dim name info error %s\n", __FUNCTION__, nc_strerror( status ) );
      return -1;
    }
    if ( status = nc_inq_dimlen( this->fileid, i, &dim_length ) ) {
      fprintf ( stderr, "%s: couldn't get dim length, error: %s\n", __FUNCTION__, nc_strerror( status ) );
    }
    //    fprintf( stderr, "%s: %dth dim name=%s and length=%d\n", __FUNCTION__, i, dim_name[i], (int)dim_length );
    if ( strncmp( dim_name[i], "scans_loc1", strlen( dim_name[i] ) ) == 0 ) this->scans_loc1 = dim_length;
    if ( strncmp( dim_name[i], "scans_loc2", strlen( dim_name[i] ) ) == 0 ) this->scans_loc2 = dim_length;
    if ( strncmp( dim_name[i], "measurements_loc1", strlen( dim_name[i] ) ) == 0 ) this->measurements_loc1 = dim_length;
    if ( strncmp( dim_name[i], "measurements_loc2", strlen( dim_name[i] ) ) == 0 ) this->measurements_loc2 = dim_length;
  }
  //  fprintf( stderr, "%s: %d loc1 %d=loc2 %d=meas1 %d=meas2\n",	\
  //	   __FUNCTION__, this->scans_loc1, this->scans_loc2, this->measurements_loc1, this->measurements_loc2 );
  return 0;
}

  
  
  


  
