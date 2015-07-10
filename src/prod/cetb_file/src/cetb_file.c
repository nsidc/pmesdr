/*
 * cetb_files- Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "cetb_file.h"

/*********************************************************************
 * Internal data and function prototypes
 *********************************************************************/

/*
 * Maximum lengths of channel strings, including null char
 * THIS SHOULD BE REMOVED WHEN WE START USING GSX
 */
#define channel_str_length 4

/*
 * SSM/I channel IDs
 * THIS SHOULD BE REMOVED WHEN WE START USING GSX
 */
typedef enum {
  SSMI_19H=1,
  SSMI_19V,
  SSMI_22V,
  SSMI_37H,
  SSMI_37V,
  SSMI_85H,
  SSMI_85V
} ssmi_channel_id;
  
static const char *ssmi_channel_name[] = {
  "XXX",
  "19H",
  "19V",
  "22V",
  "37H",
  "37V",
  "85H",
  "85V"
};

/*
 * AMSR-E channel IDs
 * THIS SHOULD BE REMOVED WHEN WE START USING GSX
 */
typedef enum {
  AMSRE_06H=1,
  AMSRE_06V,
  AMSRE_10H,
  AMSRE_10V,
  AMSRE_18H,
  AMSRE_18V,
  AMSRE_23H,
  AMSRE_23V,
  AMSRE_36H,
  AMSRE_36V,
  AMSRE_89H,
  AMSRE_89V
} amsre_channel_id;
  
/*
 * AMSR-E channel ID names
 * THIS SHOULD BE REMOVED WHEN WE START USING GSX
 */
static const char *amsre_channel_name[] = {
  "XXX",
  "06H",
  "06V",
  "10H",
  "10V",
  "18H",
  "18V",
  "23H",
  "23V",
  "36H",
  "36V",
  "89H",
  "89V"
};

static int channel_name( char *channel_str, cetb_sensor_id sensor_id, int beam_id );
static int valid_date( int year, int doy );
static int valid_pass_direction( cetb_region_id region_id, cetb_direction_id direction_id );
static int valid_platform_id( cetb_platform_id platform_id );
static int valid_reconstruction_id( cetb_reconstruction_id reconstruction_id );
static int valid_region_id( cetb_region_id region_id );
static int valid_resolution_factor( int factor );
static int valid_sensor_id( cetb_sensor_id sensor_id );
static int valid_swath_producer_id( cetb_swath_producer_id producer_id );

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
  int max_len = 10;
  char swath_producer_str[ max_len ];
  char recon_str[ max_len ];
  char *ptr;
  int i;
  
  if ( !valid_reconstruction_id( reconstruction_id ) ) {
    return id;
  }

  /* Convert the recon_str to lower case, so we find paths like "/.../bgiCSU" */
  strncpy( recon_str, cetb_reconstruction_id_name[ reconstruction_id ], max_len - 1 );
  for ( i=0; i < strlen( recon_str ); i++ ) {
    recon_str[i] = tolower( recon_str[i] );
  }
  
  ptr = strstr( outpath, recon_str );
  if ( !ptr ) {
    fprintf( stderr, "%s: Error parsing outpath=%s for reconstruction type=%s.\n",
	     __FUNCTION__, outpath, recon_str );
    return id;
  }
  strncpy( swath_producer_str, ptr + strlen( recon_str ), 3 );

  for ( i = 0; i < CETB_NUM_PRODUCERS; i++ ) {
    if ( 0 == strcasecmp( cetb_swath_producer_id_name[ i ], swath_producer_str ) ) {
      id = (cetb_swath_producer_id) i;
      break;
    }
  }

  if ( CETB_NO_PRODUCER == id ) {
    fprintf( stderr, "%s: Error parsing outpath=%s for swath producer.\n",
	     __FUNCTION__, outpath );
  }

  return id;

}

/*
 * cetb_filename - returns a pointer to an output filename with
 *                 the requested characteristics; Only limited
 *                 checking will be done for consistencies like
 *                 dates and sensors, for example doy is checked
 *                 for range in 1-365 or 366 for leap years, and
 *                 years prior to 1978 are not allowed, but
 *                 mostly depends on the caller to input a
 *                 combination that makes sense
 *
 *  input :
 *
 *  output :
 *    filename : pointer to CETB standard filename specified by input arguments
 *               filename convention is described in CETB ATBD
 *
 *  result : 0 on error, 1 otherwise
 *
o */
int cetb_filename( char *filename, size_t max_length, char *dirname,
		   cetb_region_id region_id,
		   int factor,
		   cetb_platform_id platform_id,
		   cetb_sensor_id sensor_id,
		   int year,
		   int doy,
		   int beam_id,
		   cetb_direction_id direction_id,
		   cetb_reconstruction_id reconstruction_id,
		   cetb_swath_producer_id producer_id ) {

  char channel_str[channel_str_length] = "";
  
  if ( !valid_region_id( region_id ) ) return 0;
  if ( !valid_resolution_factor( factor ) ) return 0;
  if ( !valid_platform_id( platform_id ) ) return 0;
  if ( !valid_sensor_id( sensor_id ) ) return 0;
  if ( !valid_date( year, doy ) ) return 0;
  if ( !channel_name( channel_str, sensor_id, beam_id ) ) return 0;
  if ( !valid_pass_direction( region_id, direction_id ) ) return 0;
  if ( !valid_reconstruction_id( reconstruction_id ) ) return 0;
  if ( !valid_swath_producer_id( producer_id ) ) return 0;

  /*
   * Needs check for sum of parts not exceeding max_length
   */
  snprintf( filename, max_length, 
	    "%s/%s%s.%s_%s.%4.4d%3.3d.%3s.%s.%s.%s.%s.nc",
	    dirname,
	    cetb_region_id_name[ region_id - CETB_EASE2_N ],
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
  
  return 1;
  
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
 *  output :
 *    channel_str : channel string, 2-digit frequency, 1-letter polarization,  e.g. "19H"
 *
 *  result : 1 for success, 0 otherwise
 *
 */
static int channel_name( char *channel_str, cetb_sensor_id sensor_id, int beam_id ) {

  if ( CETB_SSMI == sensor_id ) {
    if ( 0 < beam_id && beam_id <= SSMI_85V ) {
      strncpy( channel_str, ssmi_channel_name[ beam_id ], channel_str_length );
    } else {
      fprintf( stderr, "%s: Invalid sensor_id=%d/beam_id=%d\n", __FUNCTION__, sensor_id, beam_id );
      return 0;
    }
  } else if ( CETB_AMSRE == sensor_id ) {
    if ( 0 < beam_id && beam_id <= AMSRE_89V ) {
      strncpy( channel_str, amsre_channel_name[ beam_id ], channel_str_length );
    } else {
      fprintf( stderr, "%s: Invalid sensor_id=%d/beam_id=%d\n", __FUNCTION__, sensor_id, beam_id );
      return 0;
    }
  } else {
    fprintf( stderr, "%s: Invalid sensor_id=%d\n", __FUNCTION__, sensor_id );
    fprintf( stderr, "%s: This implementation should be removed when we start using gsx\n",
	     __FUNCTION__ );
    return 0;
  }

  return 1;

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
 *  result : 1 is valid, 0 otherwise
 *
 */
static int valid_date( int year, int doy ) {

  int doy_min = 1;
  int doy_max = 365;
  
  if ( CETB_YEAR_START > year ) {
    fprintf( stderr, "%s: year=%d is out of range\n", __FUNCTION__, year );
    return 0;
  }

  if ( 0 == (year % 4) ) {
    doy_max = 366;
  }

  if ( doy_min <= doy && doy <= doy_max ) {
    return 1;
  } else {
    fprintf( stderr, "%s: doy=%d is out of range for year=%d\n", __FUNCTION__, doy, year );
    return 0;
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
 *  result : 1 is valid, 0 otherwise
 *
 */
static int valid_pass_direction( cetb_region_id region_id, cetb_direction_id direction_id ) {

  if ( CETB_EASE2_N == region_id || CETB_EASE2_S == region_id ) {
    if ( CETB_ALL_PASSES == direction_id
	 || CETB_MORNING_PASSES == direction_id
	 || CETB_EVENING_PASSES == direction_id ) {
      return 1;
    } else {
      fprintf( stderr, "%s: region=%s not valid with pass direction=%d\n", __FUNCTION__,
	       cetb_region_id_name[ region_id - CETB_EASE2_N ],
	       direction_id );
      return 0;
    }
  } else if ( CETB_EASE2_T == region_id ) {
    if ( CETB_ALL_PASSES == direction_id
	 || CETB_ASC_PASSES == direction_id
	 || CETB_DES_PASSES == direction_id ) {
      return 1;
    } else {
      fprintf( stderr, "%s: region=%s not valid with pass direction=%d\n", __FUNCTION__,
	       cetb_region_id_name[ region_id - CETB_EASE2_N ],
	       direction_id );
      return 0;
    }
  } else {
      fprintf( stderr, "%s: Invalid region=%s\n", __FUNCTION__,
	       cetb_region_id_name[ region_id - CETB_EASE2_N ] );
      return 0;
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
 *  result : 1 is valid, 0 otherwise
 *
 */
static int valid_platform_id( cetb_platform_id platform_id ) {

  if ( 0 <= platform_id && platform_id < CETB_NUM_PLATFORMS ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid platform_id=%d\n", __FUNCTION__, platform_id );
    return 0;
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
 *  result : 1 is valid, 0 otherwise
 *
 */
static int valid_reconstruction_id( cetb_reconstruction_id reconstruction_id ) {

  if ( CETB_SIR == reconstruction_id
       || CETB_BGI == reconstruction_id ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid reconstruction_id=%d\n", __FUNCTION__, reconstruction_id );
    return 0;
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
 *  result : 1 is valid, 0 otherwise
 *
 */
static int valid_region_id( cetb_region_id region_id ) {

  if ( CETB_EASE2_N == region_id
       || CETB_EASE2_S == region_id
       || CETB_EASE2_T == region_id ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid region_id=%d\n", __FUNCTION__, region_id );
    return 0;
  }    
  
}

/*
 * valid_resolution_factor - checks for valid grid resolution factor
 *
 *  input :
 *    factor : integer resolution factor
 *
 *  output : n/a
 *
 *  result : 1 is valid, 0 otherwise
 *
 */
static int valid_resolution_factor( int factor ) {

  if ( CETB_MIN_RESOLUTION_FACTOR <= factor && factor <= CETB_MAX_RESOLUTION_FACTOR ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid factor=%d\n", __FUNCTION__, factor );
    return 0;
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
 *  result : 1 is valid, 0 otherwise
 *
 */
static int valid_sensor_id( cetb_sensor_id sensor_id ) {

  if ( 0 <= sensor_id && sensor_id < CETB_NUM_SENSORS ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid sensor_id=%d\n", __FUNCTION__, sensor_id );
    return 0;
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
 *  result : 1 is valid, 0 otherwise
 *
 */
static int valid_swath_producer_id( cetb_swath_producer_id producer_id ) {

  if ( CETB_CSU == producer_id
       || CETB_RSS == producer_id ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid producer_id=%d\n", __FUNCTION__, producer_id );
    return 0;
  }    
  
}

