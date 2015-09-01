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
  status = nc_open( filename, NC_NOWRITE, &nc_fileid );
  fprintf( stderr, "%s: nc_open return=%s: filename=%s \n",
	   __FUNCTION__, nc_strerror(status), filename );
  
  if ( NC_NOERR == status ) {
    this->fileid = nc_fileid;
  } else {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    free( this );
    return NULL;
  }

  status = nc_inq( nc_fileid, &nc_dims, &nc_vars, &nc_atts, &nc_unlimdims );
  fprintf( stderr, "%s: nc_inq return=%s: ndims=%d, nvars=%d, natts=%d, nunlimdims=%d \n",
	   __FUNCTION__, nc_strerror(status), nc_dims, nc_vars, nc_atts, nc_unlimdims );
  
  if ( NC_NOERR == status ) {
    this->fileid = nc_fileid;
    this->dims = nc_dims;
    this->vars = nc_vars;
    this->atts = nc_atts;
    this->unlimdims = nc_unlimdims;
  } else {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    free( this );
    return NULL;
  }

  return this;

}

void gsx_close ( gsx_class *this ) {
  int status;
  
  if ( NULL == this ) return;
  status = nc_close( this->fileid );
  if ( NC_NOERR != status ) {
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
      fprintf( stderr, "%s: memallocated for %dth char pointer \n", __FUNCTION__, i );
    }
    fflush (stderr );
    status = nc_inq_dimname( this->fileid, i, dim_name[i] );
    if ( NC_NOERR == status ) {
      status = nc_inq_dimlen( this->fileid, i, &dim_length );
      if ( NC_NOERR == status ) {
	fprintf( stderr, "%s: %dth dim name=%s and length=%d\n", __FUNCTION__, i, dim_name[i], (int)dim_length );
	// assign them here
      } else {
	fprintf ( stderr, "%s: couldn't get dim length\n", __FUNCTION__ );
      }
    } else {
      fprintf ( stderr, "%s: couldn't get dim name info\n", __FUNCTION__ );
      return -1;
    }
  }
  return 0;
}

  
  
  


  
