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

  return 1;
}

gsx_class *gsx_init ( char *filename ) {
  gsx_class *this=NULL;
  int status;
  int *nc_fileid;
  
  this = (gsx_class *)calloc(1, sizeof(gsx_class));
  if ( NULL == this ) { perror( __FUNCTION__ ); return NULL; }
  
  fprintf( stderr, "%s: gsx file name = %s. \n",
	     __FUNCTION__, filename );
  status = nc_open( filename, NC_SHARE, nc_fileid );
  fprintf( stderr, "%s: nc_open return=%s: filename=%s \n",
	   __FUNCTION__, nc_strerror(status), filename );

  if ( NC_NOERR == status ) {
    this->fileid = nc_fileid;
    return this;
  } else {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s \n",
	     __FUNCTION__, nc_strerror(status), filename );
    free( this );
    return NULL;
  }
}

//void gsx_close ( gsx_class *this ) {
//int status;
  //  if ( NULL == this ) return;
  //status = nc_close( this->fileid );
  //if ( NC_NOERR != status ) {
  //  fprintf( stderr, "%s: nc_close error=%s \n",
  //	     __FUNCTION__, nc_strerror(status) );
  //}
  //free( this );
  //return;
//}
  
  


  
