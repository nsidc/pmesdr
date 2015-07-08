/*
 * cetb_files- Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <stdio.h>
#include <string.h>

#include "cetb_file.h"

/*
 * cetb_valid_region_id - checks for valid region_id number
 *
 *  input :
 *    region_id : integer region_id number
 *
 *  output : n/a
 *
 *  result : 1 is valid, 0 otherwise
 *
 */
int cetb_valid_region_id( cetb_region_id region_id ) {

  if ( CETB_EASE2_N <= region_id && region_id < CETB_NUM_REGIONS ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid region_id=%d\n", __FUNCTION__, region_id );
    return 0;
  }    
  
}

/*
 * cetb_valid_resolution_factor - checks for valid grid resolution factor
 *
 *  input :
 *    factor : integer resolution factor
 *
 *  output : n/a
 *
 *  result : 1 is valid, 0 otherwise
 *
 */
int cetb_valid_resolution_factor( int factor ) {

  if ( CETB_MIN_RESOLUTION_FACTOR <= factor && factor <= CETB_MAX_RESOLUTION_FACTOR ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid factor=%d\n", __FUNCTION__, factor );
    return 0;
  }    
  
}

/*
 * cetb_valid_platform - checks for valid platform id
 *
 *  input :
 *    platform_id : integer platform id
 *
 *  output : n/a
 *
 *  result : 1 is valid, 0 otherwise
 *
 */
int cetb_valid_platform_id( cetb_platform_id platform_id ) {

  if ( 0 <= platform_id && platform_id < CETB_NUM_PLATFORMS ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid platform_id=%d\n", __FUNCTION__, platform_id );
    return 0;
  }    
  
}

/*
 * cetb_valid_sensor - checks for valid sensor id
 *
 *  input :
 *    sensor_id : integer sensor id
 *
 *  output : n/a
 *
 *  result : 1 is valid, 0 otherwise
 *
 */
int cetb_valid_sensor_id( cetb_sensor_id sensor_id ) {

  if ( 0 <= sensor_id && sensor_id < CETB_NUM_SENSORS ) {
    return 1;
  } else {
    fprintf( stderr, "%s: Invalid sensor_id=%d\n", __FUNCTION__, sensor_id );
    return 0;
  }    
  
}

/*
 * cetb_filename - returns a pointer to an output filename with the requested characteristics;
 *                 no checking will be done for consistencies like dates and sensors,
 #                 depends on the caller to input a combination that makes sense
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
int cetb_filename( char *filename, int max_length, char *dirname,
		   cetb_region_id region_id,
		   int factor,
		   cetb_platform_id platform_id,
		   cetb_sensor_id sensor_id,
		   int year,
		   int doy ) {

  float resolution;
  
  if ( !cetb_valid_region_id( region_id ) ) return 0;
  if ( !cetb_valid_resolution_factor( factor ) ) return 0;
  if ( !cetb_valid_platform_id( platform_id ) ) return 0;
  if ( !cetb_valid_sensor_id( sensor_id ) ) return 0;

  /*
   * Needs check for exceeding max length?
   */
  snprintf( filename, max_length, "%s/%s%s.%s_%s.%4.4d%3.3d.%s",
	    dirname,
	    cetb_region_id_name[ region_id - CETB_EASE2_N ],
	    cetb_resolution_name[ factor ],
	    cetb_platform_id_name[ platform_id ],
	    cetb_sensor_id_name[ sensor_id ],
	    year,
	    doy,
	    "TEST_FILE.nc" );
  
  return 1;
  
}
