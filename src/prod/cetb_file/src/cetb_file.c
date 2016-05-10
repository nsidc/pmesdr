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
#include "utils.h"

/*********************************************************************
 * Internal data and function prototypes
 *********************************************************************/

#define STATUS_OK 0
#define STATUS_FAILURE 1
#define MAX_STR_LENGTH 256
#define DEFLATE_LEVEL 9

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
					const char *units,
					double *days_since_epoch );

/*********************************************************************
 * Public functions
 *********************************************************************/

/*
 * Temporaray function, to parse the metafile info_file string
 * to figure out what the pass direction is
 */
cetb_direction_id cetb_get_direction_id_from_info_name( const char *info_name ) {

  int pos = 3; /* This is very fragile! */
  cetb_direction_id id = CETB_NO_DIRECTION;
  
  if ( 'm' == info_name[ pos ] ) {
    id = CETB_MORNING_PASSES;
  } else if ( 'e' == info_name[ pos ] ) {
    id = CETB_EVENING_PASSES;
  } else if ( 'a' == info_name[ pos ] ) {
    id = CETB_ASC_PASSES;
  } else if ( 'd' == info_name[ pos ] ) {
    id = CETB_DES_PASSES;
  } else {
    fprintf( stderr, "%s: Error parsing info_name=%s for pass direction\n",
	     __FUNCTION__, info_name );
  }

  return id;
  
}

/*
 * Temporaray function, to parse the output path string
 * to figure out what the swath producer is
 * Assumes the path contains something like "sirCSU"
 */
cetb_swath_producer_id cetb_get_swath_producer_id_from_outpath( const char *outpath,
								const cetb_reconstruction_id reconstruction_id ) {

  cetb_swath_producer_id id = CETB_NO_PRODUCER;
  char *recon_str;
  char *ptr;
  int i;

  if ( STATUS_OK != valid_reconstruction_id( reconstruction_id ) ) {
    return id;
  }

  /* Convert the recon_str to lower case, so we find paths like "/.../bgiCSU" */
  recon_str = strdup( cetb_reconstruction_id_name[ reconstruction_id ] );
  for ( i=0; i < strlen( recon_str ); i++ ) {
    recon_str[i] = tolower( recon_str[i] );
  }
  
  ptr = strstr( outpath, recon_str );
  if ( !ptr ) {
    fprintf( stderr, "%s: Error parsing outpath=%s for reconstruction type=%s.\n",
	     __FUNCTION__, outpath, recon_str );
    free( recon_str );
    return id;
  }

  for ( i = 0; i < CETB_NUM_PRODUCERS; i++ ) {
    if ( 0 == strncasecmp( cetb_swath_producer_id_name[ i ],
			   ptr + strlen( recon_str ),
			   strlen( cetb_swath_producer_id_name[ i ] ) ) ) {
      id = (cetb_swath_producer_id) i;
      break;
    }
  }

  if ( CETB_NO_PRODUCER == id ) {
    fprintf( stderr, "%s: Error parsing outpath=%s for swath producer.\n",
	     __FUNCTION__, outpath );
  }

  free( recon_str );

  return id;

}

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
				 cetb_swath_producer_id producer_id ) {

  cetb_file_class *this=NULL;
  char *channel_str=NULL;
  cetb_region_id region_id;

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
       != utils_allocate_clean_aligned_memory( ( void * )&this, sizeof( cetb_file_class ) ) ) {
    return NULL;
  }
  
  this->fid = 0;
  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&(this->filename), FILENAME_MAX + 1 ) ) {
    return NULL;
  }
  
  this->year = year;
  this->doy = doy;
  if ( STATUS_OK != set_epoch_string( this ) ) {
    fprintf( stderr, "%s: Error setting %s.\n", __FUNCTION__, "epoch_string" );
    return NULL;
  }
  this->platform_id = platform_id;
  this->region_id = region_id;
  this->factor = factor;
  this->sensor_id = sensor_id;
  this->reconstruction_id = reconstruction_id;
  this->cols_dim_id = INT_MIN;
  this->rows_dim_id = INT_MIN;
  this->time_dim_id = INT_MIN;

  snprintf( this->filename, FILENAME_MAX,
  	    "%s/%s%s.%s_%s.%4.4d%3.3d.%s.%s.%s.%s.%s.nc",
  	    dirname,
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
  	    CETB_FILE_FORMAT_VERSION );

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
  unsigned char num_samples_max = CETB_FILE_TB_NUM_SAMPLES_MAX;
  char *flag_meanings = "num_samples GE 255";
  
  packing_convention = strdup( CETB_FILE_PACKING_CONVENTION );
  packing_convention_description = strdup( CETB_FILE_PACKING_CONVENTION_DESC );
  grid_mapping = strdup( CETB_FILE_GRID_MAPPING );
  coverage_content_type = strdup( CETB_FILE_COVERAGE_CONTENT_TYPE );
  
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
       * Using nc_put_vara_xtype was too slow, especially for non-square (i.e. T) grids
       * Changed this to copy the data across in the correct order and use nc_put_var_xtype
       * to write all the data at once
       */
      
      for ( row=0; row<rows; row++ ) {
	for ( i=0;i <cols; i++ ) {
	  *( ushort_data + ((row*cols)+i) ) = CETB_FILE_PACK_DATA( scale_factor, add_offset,
								   *( (float *)data + (((rows-row-1)*cols)+i) ) );
	}
      }

      if ( ( status = nc_put_var_ushort( this->fid, var_id, ushort_data ) ) ) {
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
	  *( short_data + ((row*cols)+i) ) = CETB_FILE_PACK_DATA( scale_factor, add_offset,
								  *( (float *)data + (((rows-row-1)*cols)+i) ) );
	}
      }

      if ( ( status = nc_put_var_short( this->fid, var_id, short_data ) ) ) {
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
	
      if ( ( status = nc_put_var( this->fid, var_id, ( void * )float_data ) ) ) {
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
	
      if ( ( status = nc_put_var( this->fid, var_id, ( void * )uchar_data ) ) ) {
	fprintf( stderr, "%s: Error putting uchar variable: %s.\n",
		 __FUNCTION__, nc_strerror( status ) );
	return 1;
      }
      if ( ( status = nc_put_att( this->fid, var_id, "flag_values", NC_UBYTE,
				  (size_t)1, &(num_samples_max) ) ) ) {
	fprintf( stderr, "%s: Error setting %s %s %d: %s.\n",
		 __FUNCTION__, var_name, "flag_values", CETB_FILE_TB_NUM_SAMPLES_MAX, nc_strerror( status ) );
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
    fprintf( stderr, "%s: Cannot set GRD parameters on non-SIR file.\n", __FUNCTION__ );
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

/*********************************************************************
 * Internal function definitions
 *********************************************************************/

/*
 * channel_name - Determine the frequency and polarization string from
 *               the input sensor and beam_id
 *               Beam_id values must correspond to the values
 *               set in the meas_meta_make processing.
 *               THIS SHOULD CHANGE WHEN WE START USING GSX INPUT
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
  } else {
    fprintf( stderr, "%s: Invalid sensor_id=%d\n", __FUNCTION__, sensor_id );
    fprintf( stderr, "%s: This implementation should be removed when we start using gsx\n",
	     __FUNCTION__ );
  }

  return channel_str;

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

  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "sensor",
				   strlen( cetb_gcmd_sensor_keyword[ this->sensor_id ] ),
				   cetb_gcmd_sensor_keyword[ this->sensor_id ] ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "sensor", nc_strerror( status ) );
    return 1;
  }

  time_stamp = current_time_stamp();
  if ( ( status = nc_put_att_text( this->fid, NC_GLOBAL, "date_created", 
				   strlen( time_stamp ), 
				   time_stamp ) ) ) {
    fprintf( stderr, "%s: Error setting %s: %s.\n",
  	     __FUNCTION__, "date_created", nc_strerror( status ) );
    return 1;
  }
  free( time_stamp );

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
 *           in variable crs
 *           will be populated in the output file
 *
 */
int fetch_crs( cetb_file_class *this, int template_fid ) {

  int status;
  int crs_id;
  char att_name[ MAX_STR_LENGTH ] = "";
  char crs_name[ MAX_STR_LENGTH ] = "crs_";
  char long_name[ MAX_STR_LENGTH ] = "";
  
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
   * scale_factor_at_projection_origin = actual value (function of projection
   *                                     and resolution/scale)
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
  
  strcpy( att_name, "scale_factor_at_projection_origin" );
  if ( ( status = nc_put_att_double( this->fid, crs_id, att_name, 
				     NC_DOUBLE, 1,
				     &cetb_exact_scale_m[ this->region_id ][ this->factor ] ) ) ) {
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
 * set_all_dimensions - Sets dimension variables (time, rows, cols) in the output file
 *                      to the expected size for the grid that will be stored here
 *                      grid is determined by region_id and factor
 *
 *  input : 
 *    this : pointer to initialized/opened cetb_file_class object
 *
 *  output : n/a
 *
 *  result : STATUS_OK on success, otherwise STATUS_FAILURE and
 *           reason writtent to stderr
 *           Upon successful completion, the required dimension variables
 *           (time, rows, cols )
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
   * Allocate and populate the array of y-dimension values This
   * is the coordinate in meters of the center of each cell
   * decreasing from a maximum at the top row to the bottom row.
   */
  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&vals, rows * sizeof( double ) ) ) {
    return STATUS_FAILURE;
  }

  for ( i = 0; i < rows; i++ ) {
    vals[ rows - i - 1 ]
      = ( (double) i - ( (double) rows / 2.0 ) )
      * cetb_exact_scale_m[ this->region_id ][ this->factor ] + half_pixel_m;
  }
  
  valid_range[ 0 ] = vals[ rows - 1 ] - half_pixel_m;
  valid_range[ 1 ] = vals[ 0 ] + half_pixel_m;
  status = set_dimension( this, "rows", rows, vals,
			  "projection_y_coordinate", NULL,
			  "meters",
			  NULL,
			  "Y",
			  valid_range,
			  &( this->rows_dim_id ) );
  if ( 0 != status ) {
    fprintf( stderr, "%s: Error setting %s.\n", __FUNCTION__, "rows" );
    return STATUS_FAILURE;
  }
  free( vals );
  this->rows = (long int) rows;
  
  /*
   * Allocate and populate the array of x-dimension values. This
   * is the coordinate in meters of the center of each cell
   * increasing from the minimum at the left to the maximum at
   * the right
   */
  if ( STATUS_OK
       != utils_allocate_clean_aligned_memory( ( void * )&vals, cols * sizeof( double ) ) ) {
    return STATUS_FAILURE;
  }

  for ( i = 0; i < cols; i++ ) {
    vals[ i ] = ( (double) i - ( (double) cols / 2.0 ) )
      * cetb_exact_scale_m[ this->region_id ][ this->factor ] + half_pixel_m;
  }
  
  valid_range[ 0 ] = vals[ 0 ] - half_pixel_m;
  valid_range[ 1 ] = vals[ cols - 1 ] + half_pixel_m;
  status = set_dimension( this, "cols", cols, vals,
			  "projection_x_coordinate", NULL,
			  "meters",
			  NULL,
			  "X",
			  valid_range,
			  &( this->cols_dim_id ) );
  if ( 0 != status ) {
    fprintf( stderr, "%s: Error setting %s.\n", __FUNCTION__, "cols" );
    return STATUS_FAILURE;
  }
  free( vals );
  this->cols = (long int) cols;

  /*
   * Work on the time dimension:
   * convert the date to "days since 1972" and save that in the time dimension
   * and save the formatted date string in its own variable
   */
  if ( STATUS_OK !=
       ( status = yyyydoy_to_days_since_epoch( this->year, this->doy,
  					       units,
  					       &days_since_epoch ) ) ) {
    fprintf( stderr, "%s: Error converting date to epoch..\n", __FUNCTION__ );
    return STATUS_FAILURE;
  }

  valid_range[ 0 ] = 0.0;
  valid_range[ 1 ] = DBL_MAX;
  status = set_dimension( this, "time", 1, &days_since_epoch,
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

  return STATUS_OK;
  
}

/*
 * set_dimension - Sets a single dimension variable, time, rows, or cols in the output file
 *
 *  input : 
 *    this : pointer to initialized/opened cetb_file_class object
 *    name : name of new dimension variable
 *    size : size of dimension variable
 *    vals : pointer to values for this variable
 *    standard_name : CF standard name
 *    long_name : only needed for time dimension
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
  if ( ( status = nc_put_var_double( this->fid, var_id, vals ) ) ) {
    fprintf( stderr, "%s: Error setting %s values: %s.\n",
	     __FUNCTION__, name, nc_strerror( status ) );
    return 1;
  }
  if ( ( status = nc_put_att_text( this->fid, var_id, "standard_name",
				   strlen(standard_name), standard_name ) ) ) {
    fprintf( stderr, "%s: Error setting %s %s: %s.\n",
  	     __FUNCTION__, name, standard_name, nc_strerror( status ) );
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
       || CETB_RSS == producer_id ) {
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
				 const char *units,
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
