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
static int get_gsx_temperatures( gsx_class *this, int varid, int count, int scans, int measurements );
static int init_gsx_pointers( gsx_class *this );
static int assign_channels( gsx_class *this, char *token );
static int get_gsx_dimensions( gsx_class *this, int varid, int *dim1, int *dim2 );
static int get_gsx_latitudes( gsx_class *this, int varid, int count, int scans, int measurements );
static int get_gsx_longitudes( gsx_class *this, int varid, int count, int scans, int measurements );
static int get_gsx_eias( gsx_class *this, int varid, int count, int scans, int measurements );
static int get_gsx_eazs( gsx_class *this, int varid, int count, int scans, int measurements );
static int get_gsx_byscan_variables( gsx_class *this, int count, int scans );

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
    free( this );
    this = NULL;
    return;
  }
  /* free the malloc'd arrays before you free the gsx_struct */
  free( this->gsx_version );
  free( this->source_file );
  for ( counter=0; counter<this->channel_number; counter++ ) {
    free ( this->channel_names[counter] );
    free( this->efov[counter] );
    free( this->brightness_temps[counter] );
  }
  for ( counter=0; counter<GSX_MAX_DIMS; counter++ ) {
    free( this->latitude[counter] );
    free( this->longitude[counter] );
    free( this->eia[counter] );
    free( this->eaz[counter] );
    free( this->sc_latitude[counter] );
    free( this->sc_longitude[counter] );
    free( this->scantime[counter] );
  }
  free( this );
  this = NULL;
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

  status = posix_memalign( (void**)&dim_name, CETB_MEM_ALIGNMENT, sizeof( char )*(NC_MAX_NAME+1) );
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to allocate memory for dimension names\n", __FUNCTION__ );
    return -1;
  }
  
  for ( i = 0; i < this->dims; i++ ) {
    memset( (void*)dim_name, 0, NC_MAX_NAME+1 );
    if ( status = nc_inq_dimname( this->fileid, i, dim_name ) ) {
      fprintf ( stderr, "%s: couldn't get dim name info error %s\n", __FUNCTION__, nc_strerror( status ) );
      return -1;
    }
    if ( status = nc_inq_dimlen( this->fileid, i, &dim_length ) ) {
      fprintf ( stderr, "%s: couldn't get dim length, error: %s\n", __FUNCTION__, nc_strerror( status ) );
      return -1;
    }
    if ( strncmp( dim_name, "scans_loc1", strlen( dim_name ) ) == 0 ) this->scans[0] = dim_length;
    if ( strncmp( dim_name, "scans_loc2", strlen( dim_name ) ) == 0 ) this->scans[1] = dim_length;
    if ( strncmp( dim_name, "scans_loc3", strlen( dim_name ) ) == 0 ) this->scans[2] = dim_length;
    if ( strncmp( dim_name, "measurements_loc1", strlen( dim_name ) ) == 0 ) this->measurements[0] = dim_length;
    if ( strncmp( dim_name, "measurements_loc2", strlen( dim_name ) ) == 0 ) this->measurements[1] = dim_length;
    if ( strncmp( dim_name, "measurements_loc3", strlen( dim_name ) ) == 0 ) this->measurements[2] = dim_length;
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
  char *temp;
  int i;

  if ( NULL == this ) {
    return -1;
  }

  this->source_file = get_att_text( this->fileid, NC_GLOBAL, "gsx_source" );
  if ( NULL == this->source_file ) {
    fprintf( stderr, "%s: no gsx_source\n", __FUNCTION__ );
    return -1;
  }

  temp = get_att_text( this->fileid, NC_GLOBAL, "short_platform" );
  if ( NULL == temp ) {
    fprintf( stderr, "%s: no gsx_source\n", __FUNCTION__ );
    return -1;
  }
  this->short_platform = CETB_NO_PLATFORM;
  for ( i=0; i< CETB_NUM_PLATFORMS; i++ ) {
    if ( 0 == strncmp( cetb_platform_id_name[i], temp, strlen(temp) ) ) {
      this->short_platform = (cetb_platform_id) i;
      break;
    }
  }
  free( temp );

  temp = get_att_text( this->fileid, NC_GLOBAL, "short_sensor" );
  if ( NULL == temp ) {
    fprintf( stderr, "%s: no short_sensor\n", __FUNCTION__ );
    return -1;
  }
  this->short_sensor = CETB_NO_SENSOR;
  for ( i=0; i< CETB_NUM_SENSORS; i++ ) {
    if ( 0 == strncmp( cetb_sensor_id_name[i], temp, strlen(temp) ) ) {
      this->short_sensor = (cetb_sensor_id) i;
      break;
    }
  }
  free( temp );
    
  temp = get_att_text( this->fileid, NC_GLOBAL, "input_provider" );
  if ( NULL == temp ) {
    fprintf( stderr, "%s: no input_provider\n", __FUNCTION__ );
    return -1;
  }
  this->input_provider = CETB_NO_PRODUCER;
  for ( i=0; i< CETB_NUM_PRODUCERS; i++ ) {
    if ( 0 == strncmp( cetb_swath_producer_id_name[i], temp, strlen(temp) ) ) {
      this->input_provider = (cetb_swath_producer_id) i;
      break;
    }
  }
  free( temp );

  if ( this->short_platform == CETB_NO_PLATFORM || \
       this->short_sensor == CETB_NO_SENSOR || \
       this->input_provider == CETB_NO_PRODUCER ) {
    if ( NULL != temp ) free( temp );
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
  char *channel_list;
  char *delim=",";
  char *token;
  char *channel_ptr;
  char *space=" ";
  char *temp;

  if ( NULL == this ) {
    return -1;
  }

  channel_list = get_att_text( this->fileid, NC_GLOBAL, "gsx_variables" );
  if ( NULL == channel_list ) {
    return -1;
  }
  channel_ptr = channel_list;

  while ( NULL != channel_ptr ) {
    token = strsep( &channel_ptr, delim );
    if ( 0 == strncmp( space, token, 1 ) ) token++;
    /* now match the channel name to the channel enum */
    status = assign_channels( this, token );
  }

  switch ( this->short_sensor ) {
  case CETB_SSMI:
    this->channel_number = SSMI_NUM_CHANNELS;
    break;
  case CETB_AMSRE:
    this->channel_number = AMSRE_NUM_CHANNELS;
    break;
  default:
    fprintf( stderr, "%s: sensor not implemented yet \n", __FUNCTION__ );
  }
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
  int varid;
  int count;
  float fillvalue;
  nc_type var_type;
  int ndims;
  int dimid[2];
  int dim1;
  int dim2;
  char *efov;

  if ( NULL == this ) {
    return -1;
  }

  if ( status = nc_inq_varid( this->fileid, "orbit", &varid ) ) {
    fprintf( stderr, "%s: file id %d variable 'orbit', error : %s\n", \
	     __FUNCTION__, this->fileid, nc_strerror( status ) );
    return -1;
  }
  if ( status = nc_get_var_int( this->fileid, varid, &(this->orbit) ) ) {
    fprintf( stderr, "%s: file id %d variable 'orbit', error : %s\n", \
	     __FUNCTION__, this->fileid, nc_strerror( status ) );
    return -1;
  }

  for ( count=0; count<this->channel_number; count++ ) {

    if ( status = nc_inq_varid( this->fileid, this->channel_names[count], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, this->channel_names[count], nc_strerror( status ) );
      return -1;
    }

    status = get_gsx_dimensions( this, varid, &dim1, &dim2 );
    if ( 0 != status ) {
      fprintf( stderr, "%s: couldn't get dimensions for %s\n", __FUNCTION__, this->channel_names[count] );
      return -1;
    }
    
    status = get_gsx_temperatures( this, varid, count, dim1, dim2 );

    if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &fillvalue ) ) {
      fprintf( stderr, "%s: couldn't get fillvalue from %s\n", __FUNCTION__, this->channel_names[count] );
      return -1;
    }
    this->fillvalue[count] = fillvalue;

    efov = get_att_text( this->fileid, varid, "gsx_field_of_view" );
    if ( status = nc_inq_varid( this->fileid, efov, &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, efov, nc_strerror( status ) );
      free( efov );
      return -1;
    }

    status = posix_memalign( (void**)&this->efov[count], CETB_MEM_ALIGNMENT, sizeof( float )*2 );
    if ( 0 != status ) {
      return -1;
    }
    if ( status = nc_get_var_float( this->fileid, varid, this->efov[count] ) ) {
      fprintf( stderr, "%s: couldn't get efov %s for error : %s\n",	\
	       __FUNCTION__, efov, nc_strerror( status ) );
      free( this->efov[count] );
      free( efov );
      return -1;
    }
    free( efov );
  }

  status = get_gsx_positions( this );

  return 0;
}

/*
 * this function takes a gsx_class struct and gets the position variables
 * file and returns a pointer to a structure that is populated with the
 * lat and lons as well and the angles along the scan lines
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
  int i;
  int varid;
  int scans;
  int measurements;

  status = 0;
  /* for each set of position (i.e. loc1, loc2, loc3 this routine retrieves the lat, lon, eia and eaz for each position */
  /*  Note that each GSX file can have 1, 2 or 3 sets of position parameters
   *  if there is only 1 set, i.e. scans_loc2 and scans_loc3 are == 0, then we are done here and the function returns success
   */

  for ( i=0; i<GSX_MAX_DIMS; i++) {
    if ( this->scans[i] != 0 ) {
      scans = this->scans[i];
      measurements = this->measurements[i];
    } else {
      return 0;
    }

    if ( status = nc_inq_varid( this->fileid, gsx_latitudes[i], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, gsx_latitudes[i], nc_strerror( status ) );
      return -1;
    }
    status = get_gsx_latitudes( this, varid, i, scans, measurements );
      
    if ( status = nc_inq_varid( this->fileid, gsx_longitudes[i], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, gsx_longitudes[i], nc_strerror( status ) );
      return -1;
    }
    status = get_gsx_longitudes( this, varid, i, scans, measurements );
      
    if ( status = nc_inq_varid( this->fileid, gsx_eias[i], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, gsx_eias[i], nc_strerror( status ) );
      return -1;
    }
    status = get_gsx_eias( this, varid, i, scans, measurements );

    if ( status = nc_inq_varid( this->fileid, gsx_eazs[i], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, gsx_eazs[i], nc_strerror( status ) );
      return -1;
    }
    status = get_gsx_eazs( this, varid, i, scans, measurements );

    if ( status = nc_inq_varid( this->fileid, gsx_latitudes[i], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, gsx_latitudes[i], nc_strerror( status ) );
      return -1;
    }
    status = get_gsx_byscan_variables( this, i, scans );
      
  }    

  return status;

}

/*
 * get_gsx_temperatures - retrieves a channel's worth of tb's from GSX file
 *
 *  Input:
 *    this - gsx_class pointer
 *    varid - the NCDF variable id for the channel
 *    count - the entry into the list of channels in the file
 *    scans - number of scans for this channel
 *    measurements - number of measurements across the scan line
 *
 *  Return:
 *    status variable 0 == success, !=0 failure
 *
 */
int get_gsx_temperatures( gsx_class *this, int varid, int count, int scans, int measurements ) {
  int status=0;

  status = posix_memalign( (void**)&this->brightness_temps[count], CETB_MEM_ALIGNMENT, sizeof(float)*scans*measurements );
  if ( 0 == status ) {
    if ( status = nc_get_var_float( this->fileid, varid, this->brightness_temps[count] ) ) {
      fprintf( stderr, "%s: tb from %s\n", __FUNCTION__, this->channel_names[count] );
      free( this->brightness_temps[count] );
      status = -1;
    }
  } else {
    status = -1;
  }

  return status;
}

/*
 * get_gsx_latitudes - returns latitude values for loc1, loc2 or loc3 depending on input
 *
 *  Input:
 *    this - pointer to gsx_class struct
 *    varid - netcdf variable id to be retrieved
 *    count - entry into the list of sets of positions in file
 *    scans - number of scans for this variable
 *    measurements - number of measurements across a scan line
 *
 *  Result:
 *    status == 0 on success, != 0 on failure
 *
 */
int get_gsx_latitudes( gsx_class *this, int varid, int count, int scans, int measurements ) {
  int status=0;

  status = posix_memalign( (void**)&this->latitude[count], CETB_MEM_ALIGNMENT, sizeof(float)*scans*measurements );
  if ( 0 == status ) {
    if ( status = nc_get_var_float( this->fileid, varid, this->latitude[count] ) ) {
      fprintf( stderr, "%s: error %s retrieving latitudes\n", __FUNCTION__, nc_strerror( status ) );
      free( this->latitude[count] );
      status = -1;
    } else {
      if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &(this->fill_latitude[count]) ) ) {
	fprintf( stderr, "%s: error %s retrieving latitude fill value\n", __FUNCTION__, nc_strerror( status ) );
	this->fill_latitude[count] = -500.0;
      }
    }
  } else {
    status = -1;
  }

  return status;
}

/*
 * get_gsx_longitudes - returns longitude values for loc1, loc2 or loc3 depending on input
 *
 *  Input:
 *    this - pointer to gsx_class struct
 *    varid - netcdf variable id to be retrieved
 *    count - entry into the list of sets of positions in file
 *    scans - number of scans for this variable
 *    measurements - number of measurements across a scan line
 *
 *  Result:
 *    status == 0 on success, != 0 on failure
 *
 */

int get_gsx_longitudes( gsx_class *this, int varid, int count, int scans, int measurements ) {
  int status=0;

  status = posix_memalign( (void**)&this->longitude[count], CETB_MEM_ALIGNMENT, sizeof(float)*scans*measurements );
  if ( 0 == status ) {
    if ( status = nc_get_var_float( this->fileid, varid, this->longitude[count] ) ) {
      fprintf( stderr, "%s: error %s retrieving longitudes\n", __FUNCTION__, nc_strerror( status ) );
      free( this->longitude[count] );
      status = -1;
    } else {
      if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &(this->fill_longitude[count]) ) ) {
	fprintf( stderr, "%s: error %s retrieving latitude fill value\n", __FUNCTION__, nc_strerror( status ) );
	this->fill_longitude[count] = -500.0;
      }
    }
  } else {
    status = -1;
  }

  return status;
}
/*
 * get_gsx_eias - returns eia values for loc1, loc2 or loc3 depending on input
 *
 *  Input:
 *    this - pointer to gsx_class struct
 *    varid - netcdf variable id to be retrieved
 *    count - entry into the list of sets of positions in file
 *    scans - number of scans for this variable
 *    measurements - number of measurements across a scan line
 *
 *  Result:
 *    status == 0 on success, != 0 on failure
 *
 */
int get_gsx_eias( gsx_class *this, int varid, int count, int scans, int measurements ) {
  int status=0;

  status = posix_memalign( (void**)&this->eia[count], CETB_MEM_ALIGNMENT, sizeof(float)*scans*measurements );
  if ( 0 == status ) {
    if ( status = nc_get_var_float( this->fileid, varid, this->eia[count] ) ) {
      fprintf( stderr, "%s: error %s retrieving ei angles\n", __FUNCTION__, nc_strerror( status ) );
      free( this->eia[count] );
      status = -1;
    } else {
      if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &(this->fill_eia[count]) ) ) {
	fprintf( stderr, "%s: error %s retrieving eia fill value\n", __FUNCTION__, nc_strerror( status ) );
	this->fill_latitude[count] = -500.0;
      }
    }
  } else {
    status = -1;
  }

  return status;
}

/*
 * get_gsx_eazs - returns eaz values for loc1, loc2 or loc3 depending on input
 *
 *  Input:
 *    this - pointer to gsx_class struct
 *    varid - netcdf variable id to be retrieved
 *    count - entry into the list of sets of positions in file
 *    scans - number of scans for this variable
 *    measurements - number of measurements across a scan line
 *
 *  Result:
 *    status == 0 on success, != 0 on failure
 *
 */
int get_gsx_eazs( gsx_class *this, int varid, int count, int scans, int measurements ) {
  int status=0;

  status = posix_memalign( (void**)&this->eaz[count], CETB_MEM_ALIGNMENT, sizeof(float)*scans*measurements );
  if ( 0 == status ) {
    if ( status = nc_get_var_float( this->fileid, varid, this->eaz[count] ) ) {
      fprintf( stderr, "%s: error %s retrieving eaz angle\n", __FUNCTION__, nc_strerror( status ) );
      free( this->eaz[count] );
      status = -1;
    } else {
      if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &(this->fill_eaz[count]) ) ) {
	fprintf( stderr, "%s: error %s retrieving eaz fill value\n", __FUNCTION__, nc_strerror( status ) );
	this->fill_latitude[count] = -500.0;
      }
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
  for ( counter=0; counter < GSX_MAX_CHANNELS; counter++ ) {
    this->channel_names[counter] = NULL;
    this->efov[counter] = NULL;
    this->brightness_temps[counter] = NULL;
  }
  for ( counter=0; counter<GSX_MAX_DIMS; counter++ ) {
    this->latitude[counter] = NULL;
    this->longitude[counter] = NULL;
    this->eia[counter] = NULL;
    this->eaz[counter] = NULL;
    this->sc_latitude[counter] = NULL;
    this->sc_longitude[counter] = NULL;
    this->scantime[counter] = NULL;
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

  status = posix_memalign( (void**)&this, CETB_MEM_ALIGNMENT, sizeof( gsx_class ) );
  if ( 0 != status ) { perror( __FUNCTION__ ); return NULL; }
  memset( this, 0, sizeof( gsx_class ) ); 
  this->fileid = nc_fileid;

  /* now check for existence of gsx_version string indicating this is a gsx file */

  if ( status = nc_inq_attlen( this->fileid, NC_GLOBAL, "gsx_version", (size_t*)&att_len ) ) {
    fprintf( stderr, "%s: no gsx_version, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    free ( this );
    return NULL;
  }

  status = posix_memalign( (void**)&this->gsx_version, CETB_MEM_ALIGNMENT, (att_len+1) );
  if ( 0 != status ) {
    fprintf( stderr, "%s: couldn't allocate gsx_version\n", __FUNCTION__ );
    free( this );
    return NULL;
  }
  if ( status = nc_get_att_text( this->fileid, NC_GLOBAL, "gsx_version", this->gsx_version ) ) { 
    fprintf( stderr, "%s: couldn't get gsx version, error : %s\n", __FUNCTION__, nc_strerror( status ) );
    gsx_close( this );
    return NULL;
  }

  return this;
}

/*
 * get_att_text - utility function to pull attribute text from a netcdf file
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
  
  status = posix_memalign( (void**)&att_text, CETB_MEM_ALIGNMENT, (att_len+1) );
  if ( 0 == status ) {
    if ( status = nc_get_att_text( fileid, varid, varname, att_text ) ) { 
      fprintf( stderr, "%s: couldn't get attribute %s string, error : %s\n", \
	     __FUNCTION__, varname, nc_strerror( status ) );
      free( att_text );
      return NULL;
    }
    *(att_text+att_len) = '\0';
  }
  
  return att_text;
}

/*
 * assign_channels
 *  - this function assigns the channels read in from the gsx file into the beam order
 *    that is expected by setup - this order is used across the system to correctly identify
 *    channels from each other
 *
 *  Input:
 *    gsx_class *this
 *    channel name read from the gsx_variables global attribute
 *
 *  Result:
 *    id number of the channel in cetb.h
 *
 */
int assign_channels( gsx_class *this, char *channel ) {
  int status=0;
  int count;

  if ( NULL == this ) {
    return -1;
  }
  
  switch ( this->short_sensor ) {
  case CETB_SSMI:
    count = 0;
    while ( ( 0 != strcmp( gsx_ssmi_channel_name[count], channel ) ) && count < (int) SSMI_NUM_CHANNELS ) count++;
    if ( SSMI_NUM_CHANNELS == count ) {
      status = -1;
    }
    break;
  case CETB_AMSRE:
    count = 0;
    while ( ( 0 != strcmp( gsx_amsre_channel_name[count], channel ) ) && count < (int) AMSRE_NUM_CHANNELS ) count++;
    if ( AMSRE_NUM_CHANNELS == count ) {
      status = -1;
    }
    break;
  default:
    status = -1;
  }
  if ( 0 == status ) {
    status = posix_memalign( (void**)&this->channel_names[count], CETB_MEM_ALIGNMENT, strlen(channel)+1 );
    if ( 0 != status ) {
      return -1;
    }
    strcpy( this->channel_names[count], channel );
    *(this->channel_names[count]+strlen(channel)) = '\0';
  }

  return status;
}

/*
 * function to get dimension values for a variable
 *
 *  Input:
 *    gsx_class *this - pointer to gsx structure
 *    int varid - the ncdf id of the variable whose dimensions are needed
 *
 *  Output:
 *    number of rows (scans) and columns (measurements)
 *
 *  Result:
 *    status is 0 upon success and != 0 upon failure
 *
 */
int get_gsx_dimensions( gsx_class *this, int varid, int *dim1, int *dim2 ) {
  int status;
  int dimid[2];
  char *dimname;
  char *dim_ptr;
  int i;
  
  if ( status = nc_inq_vardimid( this->fileid, varid, dimid ) ) {
    fprintf( stderr, "%s: couldn't get varid %d dimension ids\n", __FUNCTION__, varid );
    return -1;
  }

  status = posix_memalign( (void**)&dimname, CETB_MEM_ALIGNMENT, sizeof( char )*NC_MAX_NAME+1 );
  if ( 0 != status ) {
    return -1;
  }
  if ( status = nc_inq_dimname( this->fileid, dimid[0], dimname ) ) {
    fprintf( stderr, "%s: couldn't get %d dimension name from id %d\n", \
	     __FUNCTION__, varid, dimid[0] );
    free( dimname );
    return -1;
  }
  
  for (i=0; i<GSX_MAX_DIMS; i++ ) {
    dim_ptr = strstr( dimname, cetb_loc_id_name[i] );
    if ( NULL != dim_ptr ) {
      *dim1 = this->scans[i];
      *dim2 = this->measurements[i];
      status = 0;
      dim_ptr = NULL;
    }
  }
  return status;
}

/*
 * function to retrieve the variables that are one per scan line
 *
 *  Input:
 *    gsx_class *this - pointer to gsx structure
 *    int count - 0, 1, 2 corresponding to loc1, loc2 or loc3
 *    int scans - number of scan lines in the file
 *
 *  Result:
 *    status == 0 on success, != 0 on failure
 *
 */
int get_gsx_byscan_variables( gsx_class *this, int count, int scans ) {
  int status=0;
  int varid;

  if ( this->short_sensor != CETB_AMSRE ) { // because there is no sc lat and lon in AMSRE
    if ( status = nc_inq_varid( this->fileid, gsx_sc_latitudes[count], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, gsx_sc_latitudes[count], nc_strerror( status ) );
      return -1;
    }
    status = posix_memalign( (void**)&this->sc_latitude[count], CETB_MEM_ALIGNMENT, sizeof(float)*scans );
    if ( 0 == status ) {
      if ( status = nc_get_var_float( this->fileid, varid, this->sc_latitude[count] ) ) {
	fprintf( stderr, "%s: error %s retrieving sc_latitudes\n", __FUNCTION__, nc_strerror( status ) );
	free( this->sc_latitude[count] );
	status = -1;
      } else {
	if ( status = nc_get_att_float( this->fileid, varid, "_FillValue", &(this->fill_sc_latitude[count]) ) ) {
	  fprintf( stderr, "%s: error %s retrieving latitude fill value\n", __FUNCTION__, nc_strerror( status ) );
	  this->fill_latitude[count] = 0.0;
	}
      }
    } else {
      status = -1;
    }

    if ( status = nc_inq_varid( this->fileid, gsx_sc_longitudes[count], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, gsx_sc_longitudes[count], nc_strerror( status ) );
      return -1;
    }
    status = posix_memalign( (void**)&this->sc_longitude[count], CETB_MEM_ALIGNMENT, sizeof(float)*scans );
    if ( 0 == status ) {
      if ( status = nc_get_var_float( this->fileid, varid, this->sc_longitude[count] ) ) {
	fprintf( stderr, "%s: error %s retrieving sc_longitudes\n", __FUNCTION__, nc_strerror( status ) );
	free( this->sc_longitude[count] );
	status = -1;
      }
    } else {
      status = -1;
    }
  }
  
  if ( status = nc_inq_varid( this->fileid, gsx_scantime[count], &varid ) ) {
      fprintf( stderr, "%s: file id %d variable '%s', error : %s\n",	\
	       __FUNCTION__, this->fileid, gsx_scantime[count], nc_strerror( status ) );
      return -1;
  }
  status = posix_memalign( (void**)&this->scantime[count], CETB_MEM_ALIGNMENT, sizeof(double)*scans );
  if ( 0 == status ) {
    if ( status = nc_get_var_double( this->fileid, varid, this->scantime[count] ) ) {
      fprintf( stderr, "%s: error %s retrieving scantimes\n", __FUNCTION__, nc_strerror( status ) );
      free( this->scantime[count] );
      status = -1;
    } else {
      if ( status = nc_get_att_double( this->fileid, varid, "_FillValue", &(this->fill_scantime[count]) ) ) {
	fprintf( stderr, "%s: error %s retrieving scantime fill value\n", __FUNCTION__, nc_strerror( status ) );
	this->fill_scantime[count] = (double)-100.;
      }
    }
  } else {
    status = -1;
  }

  return status;
}




      

  
