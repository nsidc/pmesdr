/*
 * cetb_files- Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <udunits2.h>

#include "calcalcs.h"
#include "cetb.h"
#include "cetb_file.h"
#include "cetb_ncatts.h"
#include "utils.h"

/*********************************************************************
 * Internal data and function prototypes
 *********************************************************************/

#define STATUS_OK 0
#define STATUS_FAILURE 1
#define MAX_STR_LENGTH 256
#define DEFLATE_LEVEL 9
#define HOURS_PER_DAY 24

static int cetb_file_set_time_coverage( cetb_file_class *this, float *tb_data, int xdim, int ydim ); 
static char *channel_name( cetb_sensor_id sensor_id, int beam_id );
static char *current_time_stamp( void );
static int fetch_crs( cetb_file_class *this, int template_fid );
static int fetch_global_atts( cetb_file_class *this, int template_fid );
static char *pmesdr_release_version( void );
static char *pmesdr_top_dir( void );
static int set_all_dimensions( cetb_file_class *this );
static int set_dimension( cetb_file_class *this, const char *name, size_t size, double *vals,
			  const char *standard_name,
			  const char *long_name,
			  const char *units,
			  const char *calendar,
			  const char *axis,
			  double *valid_range,
			  int *dim_id );
static int set_epoch_string( cetb_file_class *this );
static int valid_date( int year, int doy );
static int valid_pass_direction( cetb_region_id region_id, cetb_direction_id direction_id );
static int valid_platform_id( cetb_platform_id platform_id );
static int valid_reconstruction_id( cetb_reconstruction_id reconstruction_id );
static cetb_region_id valid_region_id( int region_number );
static int valid_resolution_factor( int factor );
static int valid_sensor_id( cetb_sensor_id sensor_id );
static int valid_swath_producer_id( cetb_swath_producer_id producer_id );
static int yyyydoy_to_days_since_epoch( int year, int doy,
					double *days_since_epoch );
static int yyyydoy_to_yyyymmdd( int year, int doy, int *month, int *day );
static char *iso_date_string( int year, int doy, float tb_minutes );
static char *duration_time_string( float tb_time_min, float tb_time_max );
static char *set_source_value( cetb_file_class *this );

/*********************************************************************
 * Public functions
 *********************************************************************/

/*
 * cetb_file_init - returns a pointer to an initialized cetb_file object
 *
 *  input :
 *    dirname : directory location for cetb file
 *    region_number : meas_meta region number (308, 309, 310)
 *    factor : grid resolution factor
 *    platform_id : satellite platform id
 *    sensor_id : passive microwave sensor id
 *    year : 4-digit year
 *    doy : 3-digit day of year
 *    beam_id : beam (channel) id
 *    direction_id : direction id for temporal subsetting
 *    reconstruction_id : image reconstruction method id
 *    producer_id : swath data producer id
 *    progname : program name of caller, will be saved in cetb file history
 *
 *  output : n/a
 *
 *  result : pointer to initialized cetb_file_class object or NULL if an error occurred
 *
 */
cetb_file_class *cetb_file_init( char *dirname,
				 int region_number,
				 int factor,
				 cetb_platform_id platform_id,
				 cetb_sensor_id sensor_id,
				 int year,
				 int doy,
				 int beam_id,
				 cetb_direction_id direction_id,
				 cetb_reconstruction_id reconstruction_id,
				 cetb_swath_producer_id producer_id,
				 char *progname ) {

  cetb_file_class *this=NULL;
  char *channel_str=NULL;
  cetb_region_id region_id;
  char *ptr_path, template_filename[FILENAME_MAX];
  int template_fid;
  int status;
  char *file_version=NULL;
  size_t len_version;

  if ( CETB_NO_REGION == ( region_id = valid_region_id( region_number ) ) ) return NULL;
  if ( STATUS_OK != valid_resolution_factor( factor ) ) return NULL;
  if ( STATUS_OK != valid_platform_id( platform_id ) ) return NULL;
  if ( STATUS_OK != valid_sensor_id( sensor_id ) ) return NULL;
  if ( STATUS_OK != valid_date( year, doy ) ) return NULL;
  if ( !( channel_str = channel_name( sensor_id, beam_id ) ) ) return NULL;
  if ( STATUS_OK != valid_pass_direction( region_id, direction_id ) ) return NULL;
  if ( STATUS_OK != valid_reconstruction_id( reconstruction_id ) ) return NULL;
  if ( STATUS_OK != valid_swath_producer_id( producer_id ) ) return NULL;

  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&this,
					       sizeof( cetb_file_class ) ) ) {
    return NULL;
  }
  
  this->fid = 0;
  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&(this->filename),
					       FILENAME_MAX + 1 ) ) {
    return NULL;
  }
  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&(this->progname),
					       MAX_STR_LENGTH + 1 ) ) {
    return NULL;
  }
  
  this->year = year;
  this->doy = doy;
  if ( STATUS_OK != set_epoch_string( this ) ) {
    fprintf( stderr, "%s: Error setting %s.\n", __FUNCTION__, "epoch_string" );
    return NULL;
  }
  this->producer_id = producer_id;
  this->platform_id = platform_id;
  this->region_id = region_id;
  this->direction_id = direction_id;
  this->factor = factor;
  this->sensor_id = sensor_id;
  this->beam_id = beam_id;
  this->reconstruction_id = reconstruction_id;
  this->cols_dim_id = INT_MIN;
  this->rows_dim_id = INT_MIN;
  this->time_dim_id = INT_MIN;

  /*
   * Find and open the CETB template file with the global attribute data
   * - all we need here is the file format version
   */
  if ( !( ptr_path = pmesdr_top_dir( ) ) ) return 0;
  strncpy( template_filename, ptr_path, FILENAME_MAX );
  strcat( template_filename,
  	  "/src/prod/cetb_file/templates/cetb_global_template.nc" );
  if ( ( status = nc_open( template_filename, NC_NOWRITE, &template_fid ) ) ) {
    fprintf( stderr, "%s: Error opening template_filename=%s: %s.\n",
  	     __FUNCTION__, template_filename, nc_strerror( status ) );
    return 0;
  }

  if ( ( status = nc_inq_attlen( template_fid, NC_GLOBAL, "product_version", &len_version ) ) ) {
    fprintf( stderr, "%s: Error getting file version length=%s\n",
	     __FUNCTION__, nc_strerror( status ) );
  }

  status = utils_allocate_clean_aligned_memory( ( void * )&file_version,
						( sizeof( char ) * len_version ) + 1 );
  
  if ( ( status = nc_get_att_text( template_fid, NC_GLOBAL, "product_version",
				   file_version ) ) ) {
    fprintf( stderr, "%s: Error retrieving file version string=%s\n",
	     __FUNCTION__, nc_strerror( status ) );
  }
  file_version[ len_version ] = '\0';
  
  if ( ( status = nc_close( template_fid ) ) ) {
    fprintf( stderr, "%s: Error closing template_filename=%s: %s.\n",
	     __FUNCTION__, template_filename, nc_strerror( status ) );
  }

  snprintf( this->filename, FILENAME_MAX,
  	    "%s/%s-%s%s-%s_%s-%4.4d%3.3d-%s-%s-%s-%s-%s.nc",
  	    dirname,
	    cetb_NSIDC_dataset_id[ sensor_id ],
  	    cetb_region_id_name[ region_id ],
  	    cetb_resolution_name[ factor ],
  	    cetb_platform_id_name[ platform_id ],
  	    cetb_sensor_id_name[ sensor_id ],
  	    year,
  	    doy,
  	    channel_str,
  	    cetb_direction_id_name[ direction_id ],
  	    cetb_reconstruction_id_name[ reconstruction_id ],
  	    cetb_swath_producer_id_name[ producer_id ],
  	    file_version );

  snprintf( this->progname, MAX_STR_LENGTH, "%s", progname );

  free( file_version );
  free( channel_str );
  return this;
  
}

/*
 * cetb_file_open - Open the file associated with this object and
 *                  populate its global file attributes,
 *                  projection/grid metadata and
 *                  dimension variables
 *
 * input :
 *    this : pointer to initialized cetb_file_class object
 *
 * output : n/a
 *
 *  result : 0 on success
 *           1 if an error occurs; error message will be written to stderr
 *           The CETB file is created and populated with required
 *           global attributes
  *
 */
int cetb_file_open( cetb_file_class *this ) {

  int status;
  char template_filename[ FILENAME_MAX ];
  char *ptr_path;
  int template_fid;

  if ( !this->filename ) {
    fprintf( stderr, "%s: Cannot open cetb file with empty filename.\n",
  	     __FUNCTION__ );
    return 1;
  }
  
  /* Create a new cetb file */
  if ( ( status = nc_create( this->filename, NC_NETCDF4, &(this->fid) ) ) ) {
    fprintf( stderr, "%s: Error creating cetb_filename=%s: %s.\n",
  	     __FUNCTION__, this->filename, nc_strerror( status ) );
    return 1;
  }
  
  if ( STATUS_OK != set_all_dimensions( this ) ) {
    fprintf( stderr, "%s: Error setting dimensions on cetb_filename=%s.\n",
  	     __FUNCTION__, this->filename );
    return 1;
  } 

  /*
   * Find and open the CETB template file with the global attribute data
   */
  if ( !( ptr_path = pmesdr_top_dir( ) ) ) return 0;
  strncpy( template_filename, ptr_path, FILENAME_MAX );
  strcat( template_filename,
  	  "/src/prod/cetb_file/templates/cetb_global_template.nc" );
  if ( ( status = nc_open( template_filename, NC_NOWRITE, &template_fid ) ) ) {
    fprintf( stderr, "%s: Error opening template_filename=%s: %s.\n",
  	     __FUNCTION__, template_filename, nc_strerror( status ) );
    return 0;
  } 

  status = fetch_global_atts( this, template_fid );
  if ( 0 != status ) {
    fprintf( stderr, "%s: Error fetching global attributes from template.\n",
	     __FUNCTION__ ); 
    return 1;
  }
  
  status = fetch_crs( this, template_fid );
  if ( 0 != status ) {
    fprintf( stderr, "%s: Error fetching crs from template.\n", __FUNCTION__ ); 
    return 1;
  }
  
  if ( ( status = nc_close( template_fid ) ) ) {
    fprintf( stderr, "%s: Error closing template_filename=%s: %s.\n",
	     __FUNCTION__, template_filename, nc_strerror( status ) );
  }

  return 0;
  
}

/*
 * cetb_file_add_filenames - add additional information to the global attributes for this file
 *                         specifically add in the number of input data swath files and
 *                         the list of the names of those files and which version of GSX
 *                         was used to generate them
 *
 * input :
 *    this : pointer to the opened cetb_file_class object
 *    input_file_number : the number of input swath files used
 *    list_of_file_names : a list of all of the input swath file nams and GSX versions
 *
 * output :
 *    0 on success
 *    1 on failure
 *
 */
int cetb_file_add_filenames( cetb_file_class *this, int input_file_number,
			     char **list_of_file_names ) {

  int status=0;
  int count;
  char input_file[MAX_STR_LENGTH];

  if ( ( status = nc_put_att_int( this->fid, NC_GLOBAL, "number_of_input_files",
				  NC_INT, 1, &input_file_number ) ) ) {
    fprintf( stderr, "%s: Error setting %s %d: %s.\n",
	     __FUNCTION__, "number of input files", input_file_number,
	     nc_strerror( status ) );
    return 1;
  }

  for ( count = 0; count < input_file_number; count++ ) {
    sprintf( input_file, "input_file%d", count+1 );
    if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, input_file,
				     strlen( *(list_of_file_names+count) ),
				     *(list_of_file_names+count) ) ) ) {
      fprintf( stderr, "%s: Error writing out file %d, named %s\n",
	       __FUNCTION__, count, *(list_of_file_names+count) );
      return 1;
    }

  }

  return status;
  
}
/*
 * cetb_file_add_var - Add a new variable to the output file
 *                     Float data will be packed as ushorts,
 *                     integer data will not be packed
 *                     CETB standard variable attributes will be included.
 *                     meas_meta_ processing convention puts beginning of
 *                     data in lower-left corner.  So input data will
 *                     flipped top-to-bottom as it is stored
 *
 * input :
 *    this : pointer to initialized/opened cetb_file_class object
 *    var_name : string variable name to create
 *               For CETB data:
 *               "TB"
 *               "TB_std_dev"
 *               (other names from SIR/BGI processing)
 *    data : pointer to beginning of float data array
 *    standard_name : CF standard_names
 *                    http://cfconventions.org/standard-names.html
 *                    or NULL if a standard_name does not exist.
 *                    For CETB data, only "brightness_temperature"
 *                    variables should have a standard_name
 *    long_name : free-text field to describe the variable, may
 *                be used by application programs for plot/image titles
 *                e.g. "SIR TB" or "SIR TB Std Dev"
 *    units : CF unit name
 *    cols, rows : number of columns/rows (must match expected dimensions
 *                 in the cetb_file)
 *    fill_value : data fill_value
 *    missing_value : data missing_value
 *    min_value, max_value : data valid_range
 *
 * output :
 *
 *  result : 0 on success
 *           1 if an error occurs; error message will be written to stderr
 *           The data array is packed and added to the CETB file
 *
 * Refs:
 * Special thanks to the authors of:
 * http://www.nodc.noaa.gov/data/formats/netcdf/v1.1/
 *
 */
int cetb_file_add_var( cetb_file_class *this,
		       char *var_name,
		       nc_type xtype,
		       void *data,
		       long int cols,
		       long int rows,
		       char *standard_name,
		       char *long_name,
		       char *units,
		       void *fill_value_p,
		       void *missing_value_p,
		       void *valid_range_p,
		       int do_pack,
		       float scale_factor,
		       float add_offset,
		       char *calendar ) {

  int status;
  int dim_ids[ ] = { this->time_dim_id, this->rows_dim_id, this->cols_dim_id };
  size_t count[3];
  size_t start[ ] = { 0, 0, 0 };
  long int row;
  float *float_data;
  unsigned char *uchar_data;
  int var_id;
  int i;
  short *short_data;
  unsigned short *ushort_data;
  char *packing_convention;
  char *packing_convention_description;
  char *grid_mapping;
  char *coverage_content_type;
  unsigned char num_samples_max = CETB_NCATTS_TB_NUM_SAMPLES_MAX;
  char *flag_meanings = "num_samples GE 255";

  count[0] = 1;
  count[1] = rows;
  count[2] = cols;
  packing_convention = strdup( CETB_FILE_PACKING_CONVENTION );
  packing_convention_description = strdup( CETB_FILE_PACKING_CONVENTION_DESC );
  grid_mapping = strdup( CETB_FILE_GRID_MAPPING );

  /* set the coverage content type depending on the variable - test against var name */
  if ( !strcmp( "TB", var_name ) ) {
    coverage_content_type = strdup( CETB_FILE_COVERAGE_CONTENT_TYPE_IMAGE );
  } else {
    coverage_content_type = strdup( CETB_FILE_COVERAGE_CONTENT_TYPE_AUX );
  }
  
  /* Check that dimensions match what's expected for this file */
  if ( this->cols != cols || this->rows != rows ) {
    fprintf( stderr,
	     "%s: dimensions mismatch, expected (%ld,%ld) but got (%ld,%ld)\n",
	     __FUNCTION__, this->cols, this->rows, cols, rows ); 
    return 1;
  }

  /*
   * Define a new variable in the cetb file This requires the
   * dimensions ids already defined.  The order of dim_ids
   * follows DIWG convention, with "most rapidly-changing
   * dimension last in C arrays"
   */
  if ( ( status = nc_def_var( this->fid, var_name, xtype, 3, dim_ids, &var_id ) ) ) {
    fprintf( stderr, "%s: Error defining %s variable : %s.\n",
  	     __FUNCTION__, var_name, nc_strerror( status ) );
    return 1;
  }

  /* Check to see if you're setting the TB_time variable and then set the coverage */
  if ( 0 == strcmp( "TB_time", var_name ) ) {
    fprintf( stderr, "%s: setting time limits for %s variable\n",
	     __FUNCTION__, var_name );
    if ( 0 != cetb_file_set_time_coverage( this, (float*)data, cols, rows ) ) {
      fprintf( stderr, "%s: couldn't set time coverage\n", __FUNCTION__ );
      return 1;
    }
  }
  /*
   * Set compression level for this variable
   * We may need to make this controllable at the caller's level
   * See this good article by Russ Rew for ideas and how to test:
   * http://www.unidata.ucar.edu/blogs/developer/en/entry/netcdf_compression
   */
  nc_def_var_deflate( this->fid, var_id, 1, 1, DEFLATE_LEVEL );

  if ( NULL != standard_name ) {
    if ( ( status = nc_put_att_text( this->fid, var_id, "standard_name",
				     strlen(standard_name), standard_name ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s %s: %s.\n",
	       __FUNCTION__, var_name, "standard_name", standard_name, nc_strerror( status ) );
      return 1;
    }
  }
  
  if ( ( status = nc_put_att_text( this->fid, var_id, "long_name",
				   strlen(long_name), long_name ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s %s: %s.\n",
	     __FUNCTION__, var_name, "long_name", long_name, nc_strerror( status ) );
    return 1;
  }
  
  if ( ( status = nc_put_att_text( this->fid, var_id, "units",
				   strlen(units), units ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s %s: %s.\n",
	     __FUNCTION__, var_name, "units", units, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_def_var_fill( this->fid, var_id, 0, fill_value_p ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s: %s.\n",
	     __FUNCTION__, var_name, "fill_value", nc_strerror( status ) );
    return 1;
  }

  if ( NULL != missing_value_p ) {
    if ( ( status = nc_put_att( this->fid, var_id, "missing_value",
				xtype, 1, missing_value_p ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s: %s.\n",
	       __FUNCTION__, var_name, "missing_value", nc_strerror( status ) );
      return 1;
    }
  }
  if ( ( status = nc_put_att( this->fid, var_id, "valid_range",
			      xtype, 2, valid_range_p ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s: %s.\n",
  	     __FUNCTION__, var_name, "valid_range", nc_strerror( status ) );
    return 1;
  }

  if ( CETB_PACK == do_pack ) {
    if ( ( status = nc_put_att_text( this->fid, var_id, "packing_convention",
				     strlen(packing_convention), packing_convention ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s %s: %s.\n",
	       __FUNCTION__, var_name, "packing_convention", packing_convention,
	       nc_strerror( status ) );
      return 1;
    }
  
    if ( ( status = nc_put_att_text( this->fid, var_id, "packing_convention_description",
				   strlen(packing_convention_description),
				     packing_convention_description ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s %s: %s.\n",
	       __FUNCTION__, var_name, "packing_convention_description",
	       packing_convention_description, nc_strerror( status ) );
      return 1;
    }
  
    if ( ( status = nc_put_att_float( this->fid, var_id, "scale_factor",
				      NC_FLOAT, 1, &scale_factor ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s %f: %s.\n",
	       __FUNCTION__, var_name, "scale_factor", scale_factor, nc_strerror( status ) );
      return 1;
    }

    if ( ( status = nc_put_att_float( this->fid, var_id, "add_offset",
				      NC_FLOAT, 1, &add_offset ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s %f: %s.\n",
	       __FUNCTION__, var_name, "add_offset", add_offset, nc_strerror( status ) );
      return 1;
    }

    /*
     * Now pack the data.
     * Assumes input data is float *
     * Assumes variable dimensions of 1 time by rows by cols. If
     * this assumption changes, will need to inquire for the
     * size of each dimension
     */
    if ( NC_USHORT == xtype ) {

      status = utils_allocate_clean_aligned_memory( ( void * )&ushort_data,
    					      sizeof( unsigned short ) * 1 * rows * cols );
      if ( STATUS_OK != status ) {
    	fprintf( stderr, "%s: Error allocating space for packed data: %s.\n",
    		 __FUNCTION__, nc_strerror( status ) );
    	return 1;
      }

      /*
       * Meas_meta_ processing stores gridded array data from bottom to top.
       * NetCDF conventions expect it to be stored from top to bottom
       * Using nc_put_vara_xtype was too slow to write the data out in chunks,
       * especially for non-square (i.e. T) grids
       * Changed this to copy the data across in the correct order but still use
       * nc_put_vara_xtype, because it allows you to write the first of the unlimited time dimensions
       * and the data are still written in one chunk
       */

      for ( row=0; row<rows; row++ ) {
	for ( i=0;i <cols; i++ ) {
	  *( ushort_data + ((row*cols)+i) ) = (unsigned short)CETB_FILE_PACK_DATA( scale_factor, add_offset,
							( *( (float *)data + (((rows-row-1)*cols)+i) ) ) );
	}
      }

      if ( ( status = nc_put_vara_ushort( this->fid, var_id, start, count, ushort_data ) ) ) {
	  fprintf( stderr, "%s: Error putting short variable %s.\n", __FUNCTION__, nc_strerror( status ) );
	  return 1;
	}
	
      free( ushort_data );

    } else if ( NC_SHORT == xtype ) {

      status = utils_allocate_clean_aligned_memory( ( void * )&short_data,
    					      sizeof( short ) * 1 * rows * cols );
      if ( STATUS_OK != status ) {
    	fprintf( stderr, "%s: Error allocating space for packed data: %s.\n",
    		 __FUNCTION__, nc_strerror( status ) );
    	return 1;
      }

      /*
       * Meas_meta_ processing stores gridded array data from bottom to top.
       * NetCDF conventions expect it to be stored from top to bottom
       * Using nc_put_vara_xtype was too slow, especially for non-square (i.e. T) grids
       * Changed this to copy the data across in the correct order and use nc_put_var_xtype
       * to write all the data at once
       */
      
      for ( row=0; row<rows; row++ ) {
	for ( i=0; i<cols; i++ ) {
	  *( short_data + ((row*cols)+i) ) = (short)CETB_FILE_PACK_DATA( scale_factor, add_offset,
						    (*( (float *)data + (((rows-row-1)*cols)+i) ) ) );
	}
      }

      if ( ( status = nc_put_vara_short( this->fid, var_id, start, count, short_data ) ) ) {
	fprintf( stderr, "%s: Error putting short variable %s.\n", __FUNCTION__, nc_strerror( status ) );
	return 1;
      }

      free( short_data );

    } else {
      
      fprintf ( stderr, "%s: No implementation for packing to netcdf type=%d\n",
    		__FUNCTION__, xtype );
      return 1;
      
    }

  } else {

    /*
     * Otherwise, just write the (flipped) data without packing
     * This requires local copies so that pointer arithmetic works
     * (it's a runtime error to try to deference a (void *))
     */
    if ( NC_FLOAT == xtype ) {

      status = utils_allocate_clean_aligned_memory( ( void * )&float_data, sizeof( float ) * 1 * cols * rows );
      if ( STATUS_OK != status ) {
    	fprintf( stderr, "%s: Error allocating space for flipped float_data.\n", __FUNCTION__ );
    	return 1;
      }
      for ( row=0; row<rows; row++ ) {
	memcpy( ( void * )( float_data + ( ( rows - row - 1 ) * cols ) ),
		( void * )( (float *)data + ( row * cols ) ),
		sizeof( float ) * cols );
      }
	
      if ( ( status = nc_put_vara( this->fid, var_id, start, count, ( void * )float_data ) ) ) {
	fprintf( stderr, "%s: Error putting float variable: %s.\n",
		 __FUNCTION__, nc_strerror( status ) );
	return 1;
      }

      free( float_data );
      
    } else if ( NC_UBYTE == xtype ) {

      status = utils_allocate_clean_aligned_memory( ( void * )&uchar_data,
					      sizeof( unsigned char ) * 1 * cols * rows );
      if ( STATUS_OK != status ) {
    	fprintf( stderr, "%s: Error allocating space for flipped uchar_data.\n", __FUNCTION__ );
    	return 1;
      }
      for ( row=0; row<rows; row++ ) {
	memcpy( ( void * )( uchar_data + ( ( rows - row - 1 ) * cols ) ),
		( void * )( (unsigned char *)data + ( row * cols ) ),
		sizeof( unsigned char ) * cols );
      }
	
      if ( ( status = nc_put_vara( this->fid, var_id, start, count, ( void * )uchar_data ) ) ) {
	fprintf( stderr, "%s: Error putting uchar variable: %s.\n",
		 __FUNCTION__, nc_strerror( status ) );
	return 1;
      }
      if ( ( status = nc_put_att( this->fid, var_id, "flag_values", NC_UBYTE,
				  (size_t)1, &(num_samples_max) ) ) ) {
	fprintf( stderr, "%s: Error setting %s %s %d: %s.\n",
		 __FUNCTION__, var_name, "flag_values", CETB_NCATTS_TB_NUM_SAMPLES_MAX, nc_strerror( status ) );
	return 1;
      }
  
      if ( ( status = nc_put_att_text( this->fid, var_id, "flag_meanings", 
				       strlen(flag_meanings), flag_meanings ) ) ) {
	fprintf( stderr, "%s: Error setting %s %s %s: %s.\n",
		 __FUNCTION__, var_name, "flag_meanings", flag_meanings, nc_strerror( status ) );
	return 1;
      }
  
      free( uchar_data );
      
    } else {
      
      fprintf( stderr, "%s: Unrecognized xtype=%d.\n", __FUNCTION__, xtype );
      return 1;
      
    }
    
  }

  if ( ( status = nc_put_att_text( this->fid, var_id, "grid_mapping",
				   strlen(grid_mapping),
				   grid_mapping ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s %s: %s.\n",
  	     __FUNCTION__, var_name, "grid_mapping", grid_mapping, nc_strerror( status ) );
    return 1;
  }
  
  if ( ( status = nc_put_att_text( this->fid, var_id, "coverage_content_type",
				   strlen(coverage_content_type),
				   coverage_content_type ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s %s: %s.\n",
  	     __FUNCTION__, var_name, "coverage_content_type",
	     coverage_content_type, nc_strerror( status ) );
    return 1;
  }
  
  if ( NULL != calendar ) {
    if ( ( status = nc_put_att_text( this->fid, var_id, "calendar",
				     strlen( calendar ), calendar ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s: %s.\n",
	       __FUNCTION__, var_name, "calendar", nc_strerror( status ) );
      return 1;
    }
  }

  free( packing_convention );
  free( packing_convention_description );
  free( grid_mapping );
  free( coverage_content_type );
  return 0;

}

/*
 * cetb_file_add_bgi_parameters - Add BGI-specific TB variable attributes
 *
 * input :
 *    this : pointer to initialized cetb_file_class object
 *    gamma : double, BGI "noise-tuning" parameter gamma
 *            gamma rangins from 0 to pi/2
 *    dimensional_tuning_parameter : float, BGI dimensional tuning parameter value
 *            ATBD says dimensional-tuning parameter should be 0.001
 *    noise_variance : float, BGI noise variance parameter value, in K^2
 *    db_threshold : float, BGI db_threshold (determines size of neighborhood for
 *                   measurements to be used in BGI matrix)
 *    diff_threshold : float, BGI diff_threshold in Kelvins
 *                     BGI values further than this threshold from the AVE value are
 *                     reset to AVE value
 *    median_filter : integer median_filtering flag: 0=off, 1=on
 *
 * output : n/a
 *
 * result : 0 on success
 *          1 if an error occurs; error message will be written to stderr
 *          The CETB file is populated with BGI-specific TB variaable attributes
 *
 * Reference : See definitions for tuning parameters at
 *
 * Brodzik, M. J. and D. G. Long.  2015. Calibrated Passive
 * Microwave Daily EASE-Grid 2.0 Brightness Temperature ESDR
 * (CETB) Algorithm Theoretical Basis Document. MEaSUREs Project
 * White Paper.  NSIDC.  Boulder, CO.
 * http://nsidc.org/pmesdr/files/2015/09/MEaSUREs_CETB_ATBD_v0.10.pdf
 *
 */
int cetb_file_add_bgi_parameters( cetb_file_class *this,
				  double gamma,
				  float dimensional_tuning_parameter,
				  float noise_variance,
				  float db_threshold,
				  float diff_threshold,
				  int median_filter ) {

  int status;
  int var_id;
  
  if ( !this ) {
    fprintf( stderr, "%s: Invalid cetb_file pointer.\n", __FUNCTION__ );
    return 1;
  }

  if ( CETB_BGI != this->reconstruction_id ) {
    fprintf( stderr, "%s: Cannot set BGI parameters on non-BGI file.\n", __FUNCTION__ );
    return 1;
  }
  
  if ( ( status = nc_inq_varid( this->fid, "TB", &var_id ) ) ) {
    fprintf( stderr, "%s: No 'TB' variable to attach BGI attributes: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }
    
  if ( ( status = nc_put_att_double( this->fid, var_id, "bgi_gamma",
				     NC_DOUBLE, 1, &gamma ) ) ) {
    fprintf( stderr, "%s: Error setting bgi_gamma: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_float( this->fid, var_id, "bgi_dimensional_tuning_parameter",
				    NC_FLOAT, 1, &dimensional_tuning_parameter ) ) ) {
    fprintf( stderr, "%s: Error setting bgi_dimensional_tuning_parameter: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_float( this->fid, var_id, "bgi_noise_variance",
				    NC_FLOAT, 1, &noise_variance ) ) ) {
    fprintf( stderr, "%s: Error setting bgi_noise_variance: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_float( this->fid, var_id, "bgi_db_threshold",
				    NC_FLOAT, 1, &db_threshold ) ) ) {
    fprintf( stderr, "%s: Error setting bgi_db_threshold: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_float( this->fid, var_id, "bgi_diff_threshold",
				    NC_FLOAT, 1, &diff_threshold ) ) ) {
    fprintf( stderr, "%s: Error setting bgi_diff_threshold: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_int( this->fid, var_id, "median_filter",
				  NC_INT, 1, &median_filter ) ) ) {
    fprintf( stderr, "%s: Error setting median_filter: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  return 0;
  
}

/*
 * cetb_file_add_sir_parameters - Add SIR-specific global file attributes
 *
 * input :
 *    this : pointer to initialized cetb_file_class object
 *    number_of_iterations : integer SIR nits
 *    median_filter : integer median_filtering flag: 0=off, 1=on
 *
 * output : n/a
 *
 * result : 0 on success
 *          1 if an error occurs; error message will be written to stderr
 *          The CETB file is populated with SIR-specific TB variable attributes
 *
 */
int cetb_file_add_sir_parameters( cetb_file_class *this,
				  int number_of_iterations,
				  int median_filter ) {

  int status;
  int var_id;
  
  if ( !this ) {
    fprintf( stderr, "%s: Invalid cetb_file pointer.\n", __FUNCTION__ );
    return 1;
  }

  if ( CETB_SIR != this->reconstruction_id ) {
    fprintf( stderr, "%s: Cannot set SIR parameters on non-SIR file.\n", __FUNCTION__ );
    return 1;
  }

  if ( ( status = nc_inq_varid( this->fid, "TB", &var_id ) ) ) {
    fprintf( stderr, "%s: No 'TB' variable to attach SIR attributes: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }
    
  if ( ( status = nc_put_att_int( this->fid, var_id, "sir_number_of_iterations",
				  NC_INT, 1, &number_of_iterations ) ) ) {
    fprintf( stderr, "%s: Error setting sir_number_of_iterations: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_int( this->fid, var_id, "median_filter",
				  NC_INT, 1, &median_filter ) ) ) {
    fprintf( stderr, "%s: Error setting sir_median_filter: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  return 0;
  
}

/*
 * cetb_file_add_grd_parameters - Add GRD-specific global file attributes
 *
 * input :
 *    this : pointer to initialized cetb_file_class object
 *    median_filter : integer median_filtering flag: 0=off, 1=on
 *
 * output : n/a
 *
 * result : 0 on success
 *          1 if an error occurs; error message will be written to stderr
 *          The CETB file is populated with SIR-specific TB variable attributes
 *
 */
int cetb_file_add_grd_parameters( cetb_file_class *this,
				  int median_filter ) {

  int status;
  int var_id;
  
  if ( !this ) {
    fprintf( stderr, "%s: Invalid cetb_file pointer.\n", __FUNCTION__ );
    return 1;
  }

  if ( CETB_GRD != this->reconstruction_id ) {
    fprintf( stderr, "%s: Cannot set GRD parameters on non-GRD file.\n", __FUNCTION__ );
    return 1;
  }
  
  if ( ( status = nc_inq_varid( this->fid, "TB", &var_id ) ) ) {
    fprintf( stderr, "%s: No 'TB' variable to attach GRD attributes: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }
    
  if ( ( status = nc_put_att_int( this->fid, var_id, "median_filter",
				  NC_INT, 1, &median_filter ) ) ) {
    fprintf( stderr, "%s: Error setting median_filter: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  return 0;
  
}

/*
 * cetb_file_add_TB_parameters - Add remaining TB variable attributes
 *                             - these attributes are independent of the processing method
 *                             - that is they apply equally to bgi, sir or grd files
 *
 * input :
 *    this : pointer to initialized cetb_file_class object
 *    rthreshold : response threshold in dB
 *    box_size_km : size of box in km in which to search for measurements that
 *                  meet the response threshold
 *
 * operation : the function writes out the input parameters as attributes
 *             of the TB variable and then uses the information in the cetb_file_class
 *             pointer to retrieve the appropriate frequency and polarization as well as
 *             LTOD information from cetb.h and writes all of this out as TB attributes
 *
 * output : n/a
 *
 * result : 0 on success
 *          1 if an error occurs; error message will be written to stderr
 *          The CETB file is populated with TB variable attributes
 *
 */
int cetb_file_add_TB_parameters( cetb_file_class *this,
				 float rthreshold,
				 float box_size_km,
				 float ltod_morning,
				 float ltod_evening ) {

  int status;
  int var_id;
  float ltod_tmp_morning, ltod_tmp_evening;
  char *channel_str;
  
  if ( !this ) {
    fprintf( stderr, "%s: Invalid cetb_file pointer.\n", __FUNCTION__ );
    return 1;
  }

  if ( ( status = nc_inq_varid( this->fid, "TB", &var_id ) ) ) {
    fprintf( stderr, "%s: No 'TB' variable to attach attributes: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }
    
  if ( ( status = nc_put_att_float( this->fid, var_id, "measurement_response_threshold_dB",
				    NC_FLOAT, 1, &rthreshold ) ) ) {
    fprintf( stderr, "%s: Error setting sir_response_threshold: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_float( this->fid, var_id, "measurement_search_bounding_box_km",
				    NC_FLOAT, 1, &box_size_km ) ) ) {
    fprintf( stderr, "%s: Error setting measurement_bounding_box: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  /* Now write out time division information based on the projection,
     pass direction and the times in cetb.h */
  if ( ( status = nc_put_att_text( this->fid, var_id, "temporal_division",
				   strlen( cetb_direction_id_name_full[ this->direction_id ] )+1,
				   cetb_direction_id_name_full[ this->direction_id ] ) ) ) {
    fprintf( stderr, "%s: Error setting satellite pass direction: %s\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }
  
  /*
   * Only set the next 2 attributes for N and S projections.
   * The 24-hour addition to the evening pass end time is done
   * to agree with the same offset actually used in the
   * setup processing when it does the temporal filtering.
   */
  if ( CETB_EASE2_T != this->region_id ) { 
    if ( CETB_EVENING_PASSES == this->direction_id ) {
      ltod_tmp_morning = ltod_evening;
      ltod_tmp_evening = ltod_morning + HOURS_PER_DAY;
    } else {
      ltod_tmp_morning = ltod_morning;
      ltod_tmp_evening = ltod_evening;
    }
    fprintf( stderr, "%s: morning time %f and evening time %f\n", __FUNCTION__,
	     ltod_tmp_morning, ltod_tmp_evening );
    if ( ( status = nc_put_att_float( this->fid, var_id, "temporal_division_local_start_time",
				      NC_FLOAT, 1, &ltod_tmp_morning ) ) ) {
      fprintf( stderr, "%s: Error setting start local time of day: %s\n",
	       __FUNCTION__, nc_strerror( status ) );
      return 1;
    }
    if ( ( status = nc_put_att_float( this->fid, var_id, "temporal_division_local_end_time",
				      NC_FLOAT, 1, &ltod_tmp_evening ) ) ) {
      fprintf( stderr, "%s: Error setting end local time of day: %s\n",
	       __FUNCTION__, nc_strerror( status ) );
      return 1;
    }
  }

  /* Finally write out the channel for these TB values */
  channel_str = channel_name( this->sensor_id, this->beam_id );
  if ( ( status = nc_put_att_text( this->fid, var_id, "frequency_and_polarization",
				   strlen( channel_str )+1, channel_str ) ) ) {
    fprintf( stderr, "%s: Error setting channel: %s\n", __FUNCTION__, nc_strerror( status ) );
    return 1;
  }
  
  return 0;
  
}

/*
 * cetb_file_close - close the CETB file and free all memory
 *                   associated with this object
 *
 *  input :
 *    this : pointer to cetb_file_class object
 *
 *  output : n/a
 *
 *  result : File is closed, and memory is freed
 *
 */
void cetb_file_close( cetb_file_class *this ) {

  int status;
  
  if ( !this ) return;
  if ( this->filename ) {
    if ( ( status = nc_close( this->fid ) ) ) {
      fprintf( stderr, "%s: Error closing file=%s: %s.\n",
	       __FUNCTION__, this->filename, nc_strerror( status ) );
    }
    fprintf( stderr, "> %s: Wrote cetb file=%s\n", __FUNCTION__, this->filename );
    free( this->filename );
  }
  
  free( this );
  
  return;

}

/*
 * cetb_file_check_consistency - check to make sure the variables are within
 *                               valid range 
 *
 *  input: NETCDF file name
 *
 *  output: status variable
 *
 *  result: OOR values are set to missing 
 *
 *  this function retrieves the TB values from the file and
 *  checks them all to make sure they are within the required
 *  range.  Any values outside the required range, but != the
 *  fill value should be set to missing.  IFF any TB values are
 *  set to missing, then the corresponding TB_std_dev value
 *  should be set to missing.
 *
 */
int cetb_file_check_consistency( char *file_name ) {
  int status=0;
  int nc_fileid;
  int tb_varid, tb_std_dev_varid, y_varid, x_varid;
  unsigned short *tb_ushort_data, *tb_std_dev_ushort_data;
  size_t rows, cols;
  int missing_flag=0;
  unsigned int index;
  
  if ( ( status = nc_open( file_name, NC_WRITE, &nc_fileid ) ) ) {
    fprintf( stderr, "%s: nc_open error=%s: filename=%s\n", __FUNCTION__, nc_strerror(status), file_name );
    return -1;
  }

  if ( ( status = nc_inq_varid( nc_fileid, "TB", &tb_varid ) ) ) {
    fprintf( stderr, "%s: nc_inq_varid error=%s: TB\n", __FUNCTION__, nc_strerror(status) );
    return -1;
  } 

  if ( ( status = nc_inq_varid( nc_fileid, "TB_std_dev", &tb_std_dev_varid ) ) ) {
    fprintf( stderr, "%s: nc_inq_varid error=%s: TB_std_dev\n", __FUNCTION__, nc_strerror(status) );
    return -1;
  } 

  if ( ( status = nc_inq_dimid( nc_fileid, "y", &y_varid ) ) ) {
    fprintf( stderr, "%s: nc_inq_dimid error=%s: y\n", __FUNCTION__, nc_strerror(status) );
    return -1;
  }

  if ( ( status = nc_inq_dimlen( nc_fileid, y_varid, &rows ) ) ) {
    fprintf( stderr, "%s: nc_inq_dimlen error=%s: y length\n", __FUNCTION__, nc_strerror(status) );
    return -1;
  }

  if ( ( status = nc_inq_dimid( nc_fileid, "x", &x_varid ) ) ) {
    fprintf( stderr, "%s: nc_inq_dimid error=%s: x\n", __FUNCTION__, nc_strerror(status) );
    return -1;
  }

  if ( ( status = nc_inq_dimlen( nc_fileid, x_varid, &cols ) ) ) {
    fprintf( stderr, "%s: nc_inq_dimid error=%s: x length\n", __FUNCTION__, nc_strerror(status) );
    return -1;
  }

  status = utils_allocate_clean_aligned_memory( ( void * )&tb_ushort_data,
						sizeof( *tb_ushort_data ) * 1 * cols * rows );
  if ( status != 0 ) {
    fprintf( stderr, "%s: couldn't allocate memory for TB array\n", __FUNCTION__ );
    return -1;
  }

  if ( ( status = nc_get_var_ushort( nc_fileid, tb_varid, tb_ushort_data ) ) ) {
    fprintf( stderr, "%s: couldn't retrieve temperature data, error=%s\n", __FUNCTION__, nc_strerror(status) );
    return -1;
  }

  status = utils_allocate_clean_aligned_memory( ( void * )&tb_std_dev_ushort_data,
						sizeof( *tb_std_dev_ushort_data ) * 1 * cols * rows );
  if ( status != 0 ) {
    fprintf( stderr, "%s: couldn't allocate memory for TB std dev array\n", __FUNCTION__ );
    return -1;
  }

  if ( ( status = nc_get_var_ushort( nc_fileid, tb_std_dev_varid, tb_std_dev_ushort_data ) ) ) {
    fprintf( stderr, "%s: couldn't retrieve temp std dev data, error=%s\n", __FUNCTION__, nc_strerror(status) );
    return -1;
  }

  for ( index=0; index<(int)rows*cols; index++ ) {
    if ( CETB_NCATTS_TB_FILL_VALUE != *(tb_ushort_data+index) ) {
      if ( ( CETB_NCATTS_TB_MIN > *(tb_ushort_data+index) ) || ( CETB_NCATTS_TB_MAX < *(tb_ushort_data+index) ) ) {
	*(tb_ushort_data+index) = CETB_NCATTS_TB_MISSING_VALUE;
	*(tb_std_dev_ushort_data+index) = CETB_NCATTS_TB_STDDEV_MISSING_VALUE;
	missing_flag = 1;
      }
    }
  }

  if ( 1 == missing_flag ) { // need to write out the modified data
    if ( ( status = nc_put_var_ushort( nc_fileid, tb_varid, tb_ushort_data ) ) ) {
      fprintf( stderr, "%s: error=%s re-writing TB data to file=%s\n", __FUNCTION__,
	       nc_strerror(status), file_name );
      return -1;
    }
    if ( ( status = nc_put_var_ushort( nc_fileid, tb_std_dev_varid, tb_std_dev_ushort_data ) ) ) {
      fprintf( stderr, "%s: error=%s re-writing TB std dev data to file=%s\n", __FUNCTION__,
	       nc_strerror(status), file_name );
      return -1;
    }
  }

  free( tb_ushort_data );
  free( tb_std_dev_ushort_data );
  
  if ( ( status = nc_close( nc_fileid ) ) ) {
    fprintf( stderr, "%s: nc_close error=%s: filename=%s\n", __FUNCTION__, nc_strerror(status), file_name );
    return -1;
  }

  return status;

}

/*********************************************************************
 * Internal function definitions
 *********************************************************************/

/*
 * channel_name - Determine the frequency and polarization string from
 *               the input sensor and beam_id
 *               Beam_id values must correspond to the values
 *               set in the meas_meta_make processing.
 *
 *  input :
 *    sensor_id : sensor_id (determines which list of beam_ids to index)
 *    beam_id : beam_id - integer id for channels, e.g. for SSM/I,
 *              beam_id = 1 (19H), 2 (19V), etc.
 *
 *  result : (newly-allocated) channel string, with frequency and polarization,  e.g. "19H", or
 *           NULL on error
 */
char *channel_name( cetb_sensor_id sensor_id, int beam_id ) {

  char *channel_str = NULL;
  
  if ( CETB_SSMI == sensor_id ) {
    if ( 0 < beam_id && beam_id <= SSMI_NUM_CHANNELS ) {
      channel_str = strdup( cetb_ssmi_channel_name[ cetb_ibeam_to_cetb_ssmi_channel[ beam_id ] ] );
    } else {
      fprintf( stderr, "%s: Invalid sensor_id=%d/beam_id=%d\n", __FUNCTION__, sensor_id, beam_id );
    }
  } else if ( CETB_AMSRE == sensor_id ) {
    if ( 0 < beam_id && beam_id <= AMSRE_NUM_CHANNELS ) {
      channel_str = strdup ( cetb_amsre_channel_name[ cetb_ibeam_to_cetb_amsre_channel[ beam_id ] ] );
    } else {
      fprintf( stderr, "%s: Invalid sensor_id=%d/beam_id=%d\n", __FUNCTION__, sensor_id, beam_id );
    }
  } else if ( CETB_SSMIS == sensor_id ) {
    if ( 0 < beam_id && beam_id <= SSMIS_NUM_CHANNELS ) {
      channel_str = strdup ( cetb_ssmis_channel_name[ cetb_ibeam_to_cetb_ssmis_channel[ beam_id ] ] );
    } else {
      fprintf( stderr, "%s: Invalid sensor_id=%d/beam_id=%d\n", __FUNCTION__, sensor_id, beam_id );
    }
  } else if ( CETB_SMMR == sensor_id ) {
    if ( 0 < beam_id && beam_id <= SMMR_NUM_CHANNELS ) {
      channel_str = strdup ( cetb_smmr_channel_name[ cetb_ibeam_to_cetb_smmr_channel[ beam_id ] ] );
    } else {
      fprintf( stderr, "%s: Invalid sensor_id=%d/beam_id=%d\n", __FUNCTION__, sensor_id, beam_id );
    }
  } else if ( CETB_SMAP_RADIOMETER == sensor_id ) {
    if ( 0 < beam_id && beam_id <= SMAP_NUM_CHANNELS ) {
      channel_str = strdup ( cetb_smap_channel_name[ cetb_ibeam_to_cetb_smap_channel[ beam_id ] ] );
    } else {
      fprintf( stderr, "%s: Invalid sensor_id=%d/beam_id=%d\n", __FUNCTION__, sensor_id, beam_id );
    }
  } else {
    fprintf( stderr, "%s: Invalid sensor_id=%d\n", __FUNCTION__, sensor_id );
    fprintf( stderr, "%s: This implementation should be removed when we start using gsx\n",
	     __FUNCTION__ );
  }

  return channel_str;

}
/*
 * cetb_file_set_time_coverage - find the min and max minute values stored in the
 *                               tb_time variable and save them to the netCDF
 *                               file attributes for ACDD compliance
 *
 *  input :
 *    cetb_file_pointer : pointer to the cetb file object
 *    tb_time_data      : pointer to the recently calculated tb_time_data
 *    xdim              : x dimension of the data
 *    ydim              : y dimension
 *
 *  output :
 *    status variable
 *
 *  result :
 *    values are calculated for 3 ACDD required attributes, viz.
 *                     time_coverage_start
 *                     time_coverage_end
 *                     time_coverage_duration
 *
 */
int cetb_file_set_time_coverage( cetb_file_class *this, float *tb_time_data,
				 int xdim, int ydim ) {

  float tb_time_min=(float)CETB_NCATTS_TB_TIME_MAX;
  float tb_time_max=(float)CETB_NCATTS_TB_TIME_MIN;
  int index, status;
  char *time_string;
  int flag=0;

  for ( index = 0; index < (xdim*ydim); index++ ) {
    if ( CETB_NCATTS_TB_TIME_FILL_VALUE < *(tb_time_data+index) ) {
      if ( *(tb_time_data+index) > tb_time_max ) {
	flag = 1;
	tb_time_max = *(tb_time_data+index);
      }
      if ( *(tb_time_data+index) < tb_time_min ) {
	flag = 1;
	tb_time_min = *(tb_time_data+index);
      }
    }
  }

  if ( flag == 0 ) {  // Time is set to fill value so set coverage to midnight
    tb_time_min = 0;
    tb_time_max = 0;
  }
  time_string = iso_date_string( this->year, this->doy, tb_time_min );
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "time_coverage_start",
				   strlen( time_string ),
				   time_string ) ) ) {
    fprintf( stderr, "%s: Error setting %s to %s: %s.\n",
  	     __FUNCTION__, "time_coverage_start", time_string, nc_strerror( status ) );
    return 1;
  }
  free( time_string );

  time_string = iso_date_string( this->year, this->doy, tb_time_max );
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "time_coverage_end",
				   strlen( time_string ),
				   time_string ) ) ) {
    fprintf( stderr, "%s: Error setting %s to %s: %s.\n",
  	     __FUNCTION__, "time_coverage_end", time_string, nc_strerror( status ) );
    return 1;
  }
  free( time_string );

  time_string = duration_time_string( tb_time_min, tb_time_max );
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "time_coverage_duration",
				   strlen( time_string ),
				   time_string ) ) ) {
    fprintf( stderr, "%s: Error setting %s to %s: %s.\n",
  	     __FUNCTION__, "time_coverage_duration", time_string, nc_strerror( status ) );
    return 1;
  }
  free( time_string );

  return STATUS_OK;
  
}

/*
 * current_time_stamp - Formats a string with the current date and time.
 *
 *  input : n/a
 *
 *  output : n/a
 *
 *  result : ptr to newly-allocated string with current date/time
 *           or NULL on error
 *           Any errors that occur will be written to stderr
 *
 */
char *current_time_stamp( void ) {

  char *p;
  time_t curtime;
  struct tm *loctime;

  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&p, MAX_STR_LENGTH + 1 ) ) {
    return NULL;
  }

  /*
   * Get the current time
   * and convert it to local time representation
   */
  curtime = time( NULL );
  loctime = localtime( &curtime );

  /* Now format it the way I want it. */
  strftime( p, MAX_STR_LENGTH, "%Y-%m-%dT%H:%M:%S%Z", loctime );

  return p;

}
  
/*
 * fetch_global_atts - Fetches the global file attributes
 *                     from the template file to the output file.
 *
 *  input : 
 *    this : pointer to initialized cetb_file_class object
 *    template_fid : fileID for the template file
 *
 *  output : n/a
 *
 *  result : 0 success, otherwise error
 *           Upon successful completion, the global file attributes
 *           will be populated in the output file
 *
 */
int fetch_global_atts( cetb_file_class *this, int template_fid ) {

  int i;
  int status;
  int num_attributes;
  char attribute_name[ MAX_STR_LENGTH ];
  char *time_stamp;
  char *software_version;
  char *source_value;
  char epoch_date_str[ MAX_STR_LENGTH ];
  int month;
  int day;

  if ( ( status = nc_inq_natts( template_fid, &num_attributes ) ) ) {
    fprintf( stderr, "%s: "
  	     "Error getting num attributes from cetb template file: %s.\n",
  	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  } 

  /*
   * Copy all global attributes from template file to CETB file
   */
  for ( i = 0; i < num_attributes; i++ ) { 
    if ( ( status = nc_inq_attname( template_fid, NC_GLOBAL,
				    i, attribute_name ) ) ) {
      fprintf( stderr, "%s: Error getting attribute index %d: %s.\n",
  	       __FUNCTION__, i, nc_strerror( status ) );
      return 1;
    }
    if ( ( status = nc_copy_att( template_fid, NC_GLOBAL, attribute_name,
				 this->fid, NC_GLOBAL ) ) ) {
      fprintf( stderr, "%s: Error copying %s: %s.\n",
  	       __FUNCTION__, attribute_name, nc_strerror( status ) );
      return 1;
    }
  }

  /*
   * Set the global attributes that need to be specific for this file:
   */
  software_version = pmesdr_release_version();
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "software_version_id",
				   strlen( software_version ),
				   software_version ) ) ) {
    fprintf( stderr, "%s: Error setting %s to %s: %s.\n",
  	     __FUNCTION__, "software_version_id", software_version, nc_strerror( status ) );
    return 1;
  }
  free( software_version );
  
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "platform",
				   strlen( cetb_gcmd_platform_keyword[ this->platform_id ] ),
				   cetb_gcmd_platform_keyword[ this->platform_id ] ) ) ) {
     fprintf( stderr, "%s: Error setting %s: %s.\n",
   	     __FUNCTION__, "platform", nc_strerror( status ) );
     return 1;
  } 

  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "instrument",
				   strlen( cetb_gcmd_sensor_keyword[ this->sensor_id ] ),
				   cetb_gcmd_sensor_keyword[ this->sensor_id ] ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "instrument", nc_strerror( status ) );
    return 1;
  } 

  source_value = set_source_value( this );
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "source",
				   strlen( source_value ), source_value ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "source", nc_strerror( status ) );
    return 1;
  } 
  free( source_value );

  time_stamp = current_time_stamp();
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "date_created", 
				   strlen( time_stamp ), 
				   time_stamp ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "date_created", nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "date_modified", 
				   strlen( time_stamp ), 
				   time_stamp ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "date_modified", nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "date_issued",
  				   strlen( time_stamp ),
  				   time_stamp ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "date_issued", nc_strerror( status ) );
    return 1;
  }
 if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "date_metadata_modified", 
				   strlen( time_stamp ), 
				   time_stamp ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "date_metadata_modified", nc_strerror( status ) );
    return 1;
  }
  free( time_stamp );

  if ( STATUS_OK !=
       ( status = yyyydoy_to_yyyymmdd( this->year, this->doy, &month, &day ) ) ) {
    fprintf( stderr, "%s: Error converting date to yyyymmdd.\n", __FUNCTION__ );
    return STATUS_FAILURE;
  }
  sprintf( epoch_date_str, "Epoch date for data in this file: %04d-%02d-%02d 00:00:00Z",
	   this->year, month, day);
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "comment", 
				   strlen( epoch_date_str ), 
				   epoch_date_str ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "comment", nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "history", 
				   strlen( this->progname ), 
				   this->progname ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "history", nc_strerror( status ) );
    return 1;
  }

  return 0;
  
}

/*
 * fetch_crs - Fetches the coordinate reference system (crs)
 *             projection information from the template file to the output file.
 *
 *  input : 
 *    this : pointer to initialized cetb_file_class object
 *    template_fid : fileID for to the template file
 *
 *  output : n/a
 *
 *  result : 0 success, otherwise error
 *           Upon successful completion, the projection metadata
 *           in variable crs will be populated in the output file
 *           Additionally the GLOBAL attributes related to the projection
 *           are set in the function rather than in fetch_global_attributes
 *
 */
int fetch_crs( cetb_file_class *this, int template_fid ) {

  int status;
  int crs_id;
  char att_name[ MAX_STR_LENGTH ] = "";
  char crs_name[ MAX_STR_LENGTH ] = "crs_";
  char long_name[ MAX_STR_LENGTH ] = "";
  char geospatial_resolution[ MAX_STR_LENGTH ] = "";
  
  /* Copy/set the coordinate reference system (crs) metadata */
  strcat( crs_name, cetb_region_id_name[ this->region_id ] );
  if ( ( status = nc_inq_varid( template_fid, crs_name, &crs_id ) ) ) {
    fprintf( stderr, "%s: Error getting template file crs variable_id: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }
  
  if ( ( status = nc_copy_var( template_fid, crs_id, this->fid ) ) ) {
      fprintf( stderr, "%s: Error copying crs: %s.\n",
  	       __FUNCTION__, nc_strerror( status ) );
      return 1;
    }

  if ( ( status = nc_inq_varid( this->fid, crs_name, &crs_id ) ) ) {
    fprintf( stderr, "%s: Error getting output file crs variable_id: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_rename_var( this->fid, crs_id, "crs" ) ) ) {
    fprintf( stderr, "%s: Error renaming crs variable: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }

  /*
   * Put the dataset into define mode so that "dimensions,
   * variables and attributes can be added/renamed and attributes
   * can be deleted"
   */
  if ( ( status = nc_redef( this->fid ) ) ) {
    fprintf( stderr, "%s: Error changing to netcdf define mode: %s.\n",
	     __FUNCTION__, nc_strerror( status ) );
    return 1;
  }
    
  /*
   * Set the TBD placeholder items to values for this projection/resolution:
   * long_name = <region_id_name><resolution(km)> (basically the .gpd name)
   * geospatial_resolution = actual value (function of projection and resolution/scale)
   */
  strcat( long_name, cetb_region_id_name[ this->region_id ] );
  strcat( long_name, cetb_resolution_name[ this->factor ] );
  if ( ( status = nc_put_att_text( this->fid, crs_id, "long_name",
				   strlen( long_name ),
				   long_name ) ) ) {
    fprintf( stderr, "%s: Error setting %s to %s: %s.\n",
  	     __FUNCTION__, "long_name", long_name, nc_strerror( status ) );
    return 1;
  }
  
  sprintf( geospatial_resolution, "%.2f meters",
	   cetb_exact_scale_m[ this->region_id ][ this->factor ] );
  strcpy( att_name, "geospatial_x_resolution" );
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, att_name, 
				   strlen( geospatial_resolution ),
				   geospatial_resolution ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, att_name, nc_strerror( status ) );
    return 1;
  }
  strcpy( att_name, "geospatial_y_resolution" );
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, att_name, 
				   strlen( geospatial_resolution ),
				   geospatial_resolution ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, att_name, nc_strerror( status ) );
    return 1;
  }

  if ( ( status = nc_put_att_double( this->fid, NC_GLOBAL, "geospatial_lat_min", 
				     NC_DOUBLE, 1,
				     &( cetb_latitude_extent[ this->region_id ][0] ) ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "geospatial_lat_min", nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_double( this->fid, NC_GLOBAL, "geospatial_lat_max", 
				     NC_DOUBLE, 1,
				     &( cetb_latitude_extent[ this->region_id ][1] ) ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "geospatial_lat_max", nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_double( this->fid, NC_GLOBAL, "geospatial_lon_min", 
				     NC_DOUBLE, 1,
				     &( cetb_longitude_extent[ this->region_id ][0] ) ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "geospatial_lon_min", nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_double( this->fid, NC_GLOBAL, "geospatial_lon_max", 
				     NC_DOUBLE, 1,
				     &( cetb_longitude_extent[ this->region_id ][1] ) ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "geospatial_lon_max", nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "geospatial_bounds_crs", 
				   strlen( cetb_geospatial_bounds_crs[ this->region_id ] ),
				   cetb_geospatial_bounds_crs[ this->region_id ] ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, att_name, nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "geospatial_bounds", 
				   strlen( cetb_geospatial_bounds[ this->region_id ] ),
				   cetb_geospatial_bounds[ this->region_id ] ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, att_name, nc_strerror( status ) );
    return 1;
  }

  return 0;

}

/*
 * pmesdr_release_version - Fetches the PMESDR software project release version id
 #                          from the project VERSION file.
 *
 *  input : n/a
 *
 *  output : n/a
 *
 *  result : ptr to newly-allocated string with system software version,
 *           like "v0.0.1" or NULL on error
 *           Any errors that occur will be written to stderr
 *
 */
char *pmesdr_release_version( void ) {

  char filename[ FILENAME_MAX ];
  char *ptr_path;
  FILE *filep;
  char *cp;
  struct stat fileinfo;
  char *version_str;

  /*
   * Use the environment to build the complete VERSION path
   */
  if ( !( ptr_path = pmesdr_top_dir( ) ) ) return NULL;
  strncpy( filename, ptr_path, FILENAME_MAX );
  strcat( filename, "/VERSION" );

  if ( stat( filename, &fileinfo ) ) {
    fprintf( stderr, "%s: Error fetching status for %s: %s.\n",
	     __FUNCTION__, filename, strerror( errno ) );
    return NULL;
  }
  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&version_str, fileinfo.st_size + 1 ) ) {
    return NULL;
  }
  

  if ( !( filep = fopen( filename, "rt" ) ) ) { 
    fprintf( stderr, "%s: Error opening software version file %s: %s.\n",
	     __FUNCTION__, filename, strerror( errno ) );
    return NULL;
  }
  fread( version_str, sizeof( char ), fileinfo.st_size, filep );
  fclose( filep );

  /* Replace any newline with null char */
  if ( ( cp = strchr( version_str, '\n' ) ) ) *cp = '\0';

  return version_str;

}
  
/*
 * pmesdr_top_dir - Retrieves the value of environment variable PMESDR_TOP_DIR
 *
 *  input : n/a
 *
 *  output : n/a
 *
 *  result : ptr to string with value of PMESDR_TOP_DIR
 *           if not set or set to "", returns NULL
 *           Any errors that occur will be written to stderr
 *
 */
char *pmesdr_top_dir( void ) {

  char *ptr_path;

  ptr_path = getenv( "PMESDR_TOP_DIR" );
  if ( !ptr_path || 0 == strcmp( "", ptr_path ) ) {
      fprintf( stderr, "%s: Warning: No environment value found for PMESDR_TOP_DIR.\n",
	       __FUNCTION__ );
      return NULL;
  }
  
  return ptr_path;

}

/*
 * set_all_dimensions - Sets dimension variables (time, y, x) in the output file
 *                      to the expected size for the grid that will be stored here
 *                      grid is determined by region_id and factor
 *
 *  input : 
 *    this : pointer to initialized/opened cetb_file_class object
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, otherwise STATUS_FAILURE and
 *           reason written to stderr
 *           Upon successful completion, the required dimension variables
 *           (time, y, x )
 *           will be populated in the output file.
 */
int set_all_dimensions( cetb_file_class *this ) {

  long int i;
  int status;
  double *vals;
  double half_pixel_m;
  double valid_range[ 2 ];
  char *units = "days since 1972-01-01 00:00:00";
  double days_since_epoch;
  size_t rows;
  size_t cols;

  half_pixel_m = cetb_exact_scale_m[ this->region_id ][ this->factor ] / 2.0;
  rows = cetb_grid_rows[ this->region_id ][ this->factor ];
  cols = cetb_grid_cols[ this->region_id ][ this->factor ];

  /*
   * Work on the time dimension:
   * convert the date to "days since 1972" and save that in the time dimension
   * and save the formatted date string in its own variable
   */
  if ( STATUS_OK !=
       ( status = yyyydoy_to_days_since_epoch( this->year, this->doy,
  					       &days_since_epoch ) ) ) {
    fprintf( stderr, "%s: Error converting date to epoch..\n", __FUNCTION__ );
    return STATUS_FAILURE;
  } 

  valid_range[ 0 ] = 0.0;
  valid_range[ 1 ] = DBL_MAX;
  status = set_dimension( this, "time", NC_UNLIMITED, &days_since_epoch,
			  "time", "ANSI date",
			  units,
			  "gregorian",
			  "T",
			  valid_range,
			  &( this->time_dim_id ) );
  if ( 0 != status ) {
    fprintf( stderr, "%s: Error setting %s.\n", __FUNCTION__, "time" );
    return STATUS_FAILURE;
  }

  /*
   * Allocate and populate the array of y-dimension values.
   * This is the coordinate in meters of the center of each cell
   * decreasing from a maximum at the top row to the bottom row.
   */
  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&vals, rows * sizeof( double ) ) ) {
    return STATUS_FAILURE;
  }

  for ( i = 0; i < (int)rows; i++ ) {
    vals[ rows - i - 1 ]
      = ( (double) i - ( (double) rows / 2.0 ) )
      * cetb_exact_scale_m[ this->region_id ][ this->factor ] + half_pixel_m;
  }
  
  valid_range[ 0 ] = vals[ rows - 1 ] - half_pixel_m;
  valid_range[ 1 ] = vals[ 0 ] + half_pixel_m;
  status = set_dimension( this, "y", rows, vals,
			  "projection_y_coordinate", "y",
			  "meters",
			  NULL,
			  "Y",
			  valid_range,
			  &( this->rows_dim_id ) );
  if ( 0 != status ) {
    fprintf( stderr, "%s: Error setting %s.\n", __FUNCTION__, "y" );
    return STATUS_FAILURE;
  }
  free( vals );
  this->rows = (long int) rows;
  
  /*
   * Allocate and populate the array of x-dimension values.
   * This is the coordinate in meters of the center of each cell
   * increasing from the minimum at the left to the maximum at
   * the right
   */
  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&vals, cols * sizeof( double ) ) ) {
    return STATUS_FAILURE;
  }

  for ( i = 0; i < (int)cols; i++ ) {
    vals[ i ] = ( (double) i - ( (double) cols / 2.0 ) )
      * cetb_exact_scale_m[ this->region_id ][ this->factor ] + half_pixel_m;
  }
  
  valid_range[ 0 ] = vals[ 0 ] - half_pixel_m;
  valid_range[ 1 ] = vals[ cols - 1 ] + half_pixel_m;
  status = set_dimension( this, "x", cols, vals,
			  "projection_x_coordinate", "x",
			  "meters",
			  NULL,
			  "X",
			  valid_range,
			  &( this->cols_dim_id ) );
  if ( 0 != status ) {
    fprintf( stderr, "%s: Error setting %s.\n", __FUNCTION__, "x" );
    return STATUS_FAILURE;
  }
  free( vals );
  this->cols = (long int) cols;

  return STATUS_OK;
  
}

/*
 * set_dimension - Sets a single dimension variable, time, y, or x in the output file
 *
 *  input : 
 *    this : pointer to initialized/opened cetb_file_class object
 *    name : name of new dimension variable
 *    size : size of dimension variable
 *    vals : pointer to values for this variable
 *    standard_name : CF standard name
 *    long_name : needed for all dimensions per ACDD-1.3
 *    units : dimension CF units
 *    calendar : only needed for time dimension
 *    axis : dimension axis
 *    valid_range : 2-element array with dimension valid_range
 *
 *  output :
 *    dim_id : the NC dimension id for the newly created variable
 *
 *  result : STATUS_OK on success, otherwise STATUS_FAILURE and
 *           reason written to stderr
 *           Upon successful completion, the dimension variable
 *           will be populated in the output file.
 */
int set_dimension( cetb_file_class *this,
		   const char *name,
		   size_t size,
		   double *vals,
		   const char *standard_name,
		   const char *long_name,
		   const char *units,
		   const char *calendar,
		   const char *axis,
		   double *valid_range,
		   int *dim_id ) {

  int status;
  int var_id;
  int dim_ids[ 1 ];
  char *coverage_content_type;
  size_t index[ ] = { 0 };

  coverage_content_type = strdup( CETB_FILE_COVERAGE_CONTENT_TYPE_COORD );
  if ( ( status = nc_def_dim( this->fid, name, size, dim_id ) ) ) {
    fprintf( stderr, "%s: Error setting %s dim: %s.\n",
  	     __FUNCTION__, name, nc_strerror( status ) );
    return 1;
  }
  dim_ids[ 0 ] = *dim_id;
  if ( ( status = nc_def_var( this->fid, name, NC_DOUBLE, 1, dim_ids, &var_id ) ) ) {
    fprintf( stderr, "%s: Error defining %s variable : %s.\n",
  	     __FUNCTION__, name, nc_strerror( status ) );
    return 1;
  }
  if ( size == NC_UNLIMITED ) { // for unlimited dimension use put var1
    if ( ( status = nc_put_var1_double( this->fid, var_id, index, vals ) ) ) {
      fprintf( stderr, "%s: Error setting %s values: %s.\n",
	       __FUNCTION__, name, nc_strerror( status ) );
      return 1;
    }
  } else {
    
    if ( ( status = nc_put_var_double( this->fid, var_id, vals ) ) ) {
      fprintf( stderr, "%s: Error setting %s values: %s.\n",
	       __FUNCTION__, name, nc_strerror( status ) );
      return 1;
    }
  }
  if ( ( status = nc_put_att_text( this->fid, var_id, "standard_name",
				   strlen(standard_name), standard_name ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s: %s.\n",
  	     __FUNCTION__, name, standard_name, nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_text( this->fid, var_id, "coverage_content_type",
				   strlen(coverage_content_type), coverage_content_type ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s: %s.\n",
  	     __FUNCTION__, name, coverage_content_type, nc_strerror( status ) );
    return 1;
  }
  
  /* long_name attribute isn't required for all variables, only for time */
  if ( NULL != long_name ) {
    if ( ( status = nc_put_att_text( this->fid, var_id, "long_name",
				     strlen(long_name), long_name ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s: %s.\n",
	       __FUNCTION__, name, long_name, nc_strerror( status ) );
      return 1;
    }
  }
  
  if ( ( status = nc_put_att_text( this->fid, var_id, "units",
				   strlen(units), units ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s: %s.\n",
  	     __FUNCTION__, name, units, nc_strerror( status ) );
    return 1;
  }

  /* calendar attribute isn't required for all variables, only for time */
  if ( NULL != calendar ) {
    if ( ( status = nc_put_att_text( this->fid, var_id, "calendar",
				     strlen(calendar), calendar ) ) ) {
      fprintf( stderr, "%s: Error setting %s %s: %s.\n",
	       __FUNCTION__, name, calendar, nc_strerror( status ) );
      return 1;
    }
  }
  
  if ( ( status = nc_put_att_text( this->fid, var_id, "axis",
				   strlen(axis), axis ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s: %s.\n",
  	     __FUNCTION__, name, axis, nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_double( this->fid, var_id, "valid_range",
				     NC_DOUBLE, 2, valid_range ) ) ) {
    fprintf( stderr, "%s: Error setting %s valid_range: %s.\n",
  	     __FUNCTION__, name, nc_strerror( status ) );
    return 1;
  }

  free( coverage_content_type );

  return 0;
  
}

/*
 * set_epoch_string - create the string version of the input date as an epoch string
 *
 *  input :
 *    this : cetb_file object, with year and doy already set
 *
 *  output : n/a
 *
 *  result :
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *           the epoch string in the object is set to match year, doy
 *
 */
int set_epoch_string( cetb_file_class *this ) {

  int status;
  char *calendar = "Standard";
  calcalcs_cal *cal = NULL;
  int month;
  int day;

  if ( NULL == ( cal = ccs_init_calendar( calendar ) ) ) {
    fprintf( stderr, "%s: Error initializing calendar.\n", __FUNCTION__ );
    return STATUS_FAILURE;
  }

  if ( 0 != ( status = ccs_doy2date( cal, this->year, this->doy, &month, &day ) ) ) {
    fprintf( stderr, "%s: Error in ccs_doy2date for year=%d, doy=%d: %d\n",
  	     __FUNCTION__, this->year, this->doy, status );
    return STATUS_FAILURE;
  }

  sprintf( this->epoch_string, "minutes since %4.4d-%2.2d-%2.2d 00:00:00",
	   this->year, month, day );

  ccs_free_calendar( cal );

  return STATUS_OK;
  
}

/*
 * valid_date - checks for valid years (1978 or later) and
 *                   valid doy range (1-365 or 366 for leap years)
 *
 *  input :
 *    year : integer year
 *    doy : integer day of year
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int valid_date( int year, int doy ) {

  int doy_min = 1;
  int doy_max = 365;
  
  if ( CETB_YEAR_START > year ) {
    fprintf( stderr, "%s: year=%d is out of range\n", __FUNCTION__, year );
    return STATUS_FAILURE;
  }

  if ( 0 == (year % 4) ) {
    doy_max = 366;
  }

  if ( doy_min <= doy && doy <= doy_max ) {
    return STATUS_OK;
  } else {
    fprintf( stderr, "%s: doy=%d is out of range for year=%d\n", __FUNCTION__, doy, year );
    return STATUS_FAILURE;
  }    
  
}

/*
 * valid_pass_direction - checks for NS regions with both or morning/evening
 *                             and T region with both or asc/des
 *
 *  input :
 *    region_id : region id
 *    direction_id : pass direction id
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int valid_pass_direction( cetb_region_id region_id, cetb_direction_id direction_id ) {

  if ( CETB_EASE2_N == region_id || CETB_EASE2_S == region_id ) {
    if ( CETB_ALL_PASSES == direction_id
	 || CETB_MORNING_PASSES == direction_id
	 || CETB_EVENING_PASSES == direction_id ) {
      return STATUS_OK;
    } else {
      fprintf( stderr, "%s: region=%s not valid with pass direction=%d\n", __FUNCTION__,
	       cetb_region_id_name[ region_id ],
	       direction_id );
      return STATUS_FAILURE;
    }
  } else if ( CETB_EASE2_T == region_id ) {
    if ( CETB_ALL_PASSES == direction_id
	 || CETB_ASC_PASSES == direction_id
	 || CETB_DES_PASSES == direction_id ) {
      return STATUS_OK;
    } else {
      fprintf( stderr, "%s: region=%s not valid with pass direction=%d\n", __FUNCTION__,
	       cetb_region_id_name[ region_id ],
	       direction_id );
      return STATUS_FAILURE;
    }
  } else {
      fprintf( stderr, "%s: Invalid region=%s\n", __FUNCTION__,
	       cetb_region_id_name[ region_id ] );
      return STATUS_FAILURE;
  }

}

/*
 * valid_platform - checks for valid platform id
 *
 *  input :
 *    platform_id : integer platform id
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int valid_platform_id( cetb_platform_id platform_id ) {

  if ( 0 <= platform_id && platform_id < CETB_NUM_PLATFORMS ) {
    return STATUS_OK;
  } else {
    fprintf( stderr, "%s: Invalid platform_id=%d\n", __FUNCTION__, platform_id );
    return STATUS_FAILURE;
  }    
  
}

/*
 * valid_reconstruction_id - checks for valid reconstruction id
 *
 *  input :
 *    reconstruction_id : reconstruction id
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int valid_reconstruction_id( cetb_reconstruction_id reconstruction_id ) {

  if ( 0 <= reconstruction_id && reconstruction_id < CETB_NUM_RECONSTRUCTIONS ) {
    return STATUS_OK;
  } else {
    fprintf( stderr, "%s: Invalid reconstruction_id=%d\n", __FUNCTION__, reconstruction_id );
    return STATUS_FAILURE;
  }    
  
}

/*
 * valid_region_id - checks for valid region_id number
 *
 *  input :
 *    region_id : integer region_id number
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
cetb_region_id valid_region_id( int region_number ) {

  int i;
  cetb_region_id region_id=CETB_NO_REGION;

  for ( i = 0; i < CETB_NUM_REGIONS; i++ ) {
    if ( region_number == cetb_region_number[ i ] ) {
      region_id = ( cetb_region_id ) i;
      break;
    }
  }

  return region_id;
  
}

/*
 * valid_resolution_factor - checks for valid grid resolution factor
 *
 *  input :
 *    factor : integer resolution factor
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int valid_resolution_factor( int factor ) {

  if ( CETB_MIN_RESOLUTION_FACTOR <= factor
       && factor <= CETB_MAX_RESOLUTION_FACTOR ) {
    return STATUS_OK;
  } else {
    fprintf( stderr, "%s: Invalid factor=%d\n", __FUNCTION__, factor );
    return STATUS_FAILURE;
  }    
  
}

/*
 * valid_sensor - checks for valid sensor id
 *
 *  input :
 *    sensor_id : integer sensor id
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int valid_sensor_id( cetb_sensor_id sensor_id ) {

  if ( 0 <= sensor_id && sensor_id < CETB_NUM_SENSORS ) {
    return STATUS_OK;
  } else {
    fprintf( stderr, "%s: Invalid sensor_id=%d\n", __FUNCTION__, sensor_id );
    return STATUS_FAILURE;
  }    
  
}

/*
 * valid_swath_producer_id - checks for valid swath producer id
 *
 *  input :
 *    producer_id : producer id
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int valid_swath_producer_id( cetb_swath_producer_id producer_id ) {

  if ( CETB_CSU == producer_id
       || CETB_RSS == producer_id
       || CETB_JPL == producer_id ) {
    return STATUS_OK;
  } else {
    fprintf( stderr, "%s: Invalid producer_id=%d\n", __FUNCTION__, producer_id );
    return STATUS_FAILURE;
  }    
  
}

/*
 * yyyydoy_to_days_since_epoch - convert data date to epoch-specific date
 *
 *  input :
 *    year : year
 *    doy  : day of year
 *
 *  output :
 *    days_since_epoch : days since 1972
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int yyyydoy_to_days_since_epoch( int year, int doy,
				 double *days_since_epoch ) {

  int status;
  char *calendar = "Standard";
  calcalcs_cal *cal = NULL;
  int month;
  int day;
  int date_jday;
  int epoch_jday;

  if ( NULL == ( cal = ccs_init_calendar( calendar ) ) ) {
    fprintf( stderr, "%s: Error initializing calendar.\n", __FUNCTION__ );
    return STATUS_FAILURE;
  }

  if ( 0 != ( status = ccs_doy2date( cal, year, doy, &month, &day ) ) ) {
    fprintf( stderr, "%s: Error in ccs_doy2date for year=%d, doy=%d: %d\n",
  	     __FUNCTION__, year, doy, status );
    return STATUS_FAILURE;
  }

  if ( 0 != ( status = ccs_date2jday( cal, year, month, day, &date_jday ) ) ) {
    fprintf( stderr, "%s: Error in ccs_date2jday for date: %d\n",
  	     __FUNCTION__, status );
    return STATUS_FAILURE;
  }
      
  if ( 0 != ( status = ccs_date2jday( cal, 1972, 1, 1, &epoch_jday ) ) ) {
    fprintf( stderr, "%s: Error in ccs_date2jday for epoch: %d\n",
  	     __FUNCTION__, status );
    return STATUS_FAILURE;
  }

  *days_since_epoch = (double) date_jday - (double) epoch_jday;

  ccs_free_calendar( cal );

  return STATUS_OK;
  
}

/*
 * yyyydoy_to_yyyymmdd - convert day-of-year to gregorian date
 *
 *  input :
 *    year : year
 *    doy  : day of year
 *
 *  output :
 *    month : month (1-12)
 *    day   : day of month (1-31)
 *
 *  result : STATUS_OK on success, or STATUS_FAILURE with error message to stderr
 *
 */
int yyyydoy_to_yyyymmdd( int year, int doy, int *month, int *day ) {

  int status;
  char *calendar = "Standard";
  calcalcs_cal *cal = NULL;

  if ( NULL == ( cal = ccs_init_calendar( calendar ) ) ) {
    fprintf( stderr, "%s: Error initializing calendar.\n", __FUNCTION__ );
    return STATUS_FAILURE;
  }

  if ( 0 != ( status = ccs_doy2date( cal, year, doy, month, day ) ) ) {
    fprintf( stderr, "%s: Error in ccs_doy2date for year=%d, doy=%d: %d\n",
  	     __FUNCTION__, year, doy, status );
    return STATUS_FAILURE;
  }

  ccs_free_calendar( cal );

  return STATUS_OK;
  
}

/*
 * iso_date_string - convert doy + minutes offset to ISO date string
 *
 *  input :
 *    year : year
 *    doy  : day of year
 *    tb_minutes : offset in minutes from midnight on doy
 *
 *  result : character string encoded with the ISO date string
 *
 */
static char *iso_date_string( int year, int doy, float tb_minutes ) {

  char *iso_string;
  double my_time, second, resolution;
  int month, day, hour, minute;

  if ( STATUS_OK != yyyydoy_to_yyyymmdd( year, doy, &month, &day ) ) {
    return NULL;
  }

  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&iso_string, MAX_STR_LENGTH + 1 ) ) {
    return NULL;
  }

  second = tb_minutes - (int)tb_minutes;
  my_time = ut_encode_time( year, month, day, 0, (int)tb_minutes, second );
  ut_decode_time( my_time, &year, &month, &day, &hour, &minute, &second, &resolution );
  sprintf( iso_string, "%4d-%02d-%02dT%02d:%02d:%05.2lfZ",
	   year, month, day, hour, minute, second );
  return iso_string;
  
}

/*
 * duration_time_string - convert minutes offset to ISO duration string
 *
 *  input :
 *    tb_time_min : minimum minute value in array
 *    tb_time_max : maximum minute value in array
 *
 *  result : character string encoded with the ISO time duration string
 *
 */
static char *duration_time_string( float tb_time_min, float tb_time_max ) {

  char *iso_string;
  int hours, rhours, days;
  float tb_time_duration, minutes, seconds;

  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&iso_string, MAX_STR_LENGTH + 1 ) ) {
    return NULL;
  }

  tb_time_duration = tb_time_max - tb_time_min;

  hours = (int)( tb_time_duration/60 );
  if ( hours >= 24 ) {
    days = (int)( hours/24 );
    rhours = hours - 24;
  } else {
    rhours = hours;
    days = 0;
  }
  minutes = tb_time_duration - ( hours * 60.f );
  seconds = minutes - (int)minutes;
  
  sprintf( iso_string, "P%02dT%02d:%02d:%05.2f", days, rhours, (int)minutes, seconds );
  return iso_string;
  
}

/*
 * set_source_value - sets the value of the source file level metadata attribute
 *
 *                    for now the value of the source attribute is hard-coded
 *                    in this function because the necessary information is not
 *                    passed through into the GSX files
 *
 *   input:  pointer to cetb_file structure
 *
 *   return:  pointer to string that will be written into the source variable
 *
 */
static char *set_source_value( cetb_file_class *this ) {

  char *source_value=NULL;
  
  if ( CETB_AMSRE == this->sensor_id ) {
    source_value = strdup( "10.5067/AMSR-E/AMSREL1A.003\n10.5067/AMSR-E/AE_L2A.003" );
  }

  if ( ( CETB_SSMI == this->sensor_id ) && ( CETB_CSU == this->producer_id ) ) {
    source_value = strdup( "CSU SSM/I FCDR V01" );
  }

  if ( ( CETB_SSMI == this->sensor_id ) && ( CETB_RSS == this->producer_id ) ) {
    source_value = strdup( "RSS SSM/I V7" );
  }

  if ( ( CETB_SSMIS == this->sensor_id ) && ( CETB_CSU == this->producer_id ) ) {
    source_value = strdup( "CSU SSMIS FCDR V01" );
  }

  if ( ( CETB_SMMR == this->sensor_id ) ) {
    source_value = strdup( "JPL SMMR" );
  }

  if ( ( CETB_SMAP_RADIOMETER == this->sensor_id ) ) {
    source_value = strdup( "JPL SMAP JPL CL#14-2285, JPL 400-1567" );
  }
  
  return source_value;
  
}
  
  
    
  
  
