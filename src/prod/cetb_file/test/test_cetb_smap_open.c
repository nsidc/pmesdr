/*
 * test_cetb_smap_open - Unit tests for making SMAP CETB files
 *
 * 05-Jul-2018 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2018 Regents of the University of Colorado and Brigham Young University
 */
#include <netcdf.h>
#include <stdlib.h>
#include <string.h>
#include <udunits2.h>

#include "unity.h"
#include "calcalcs.h"
#include "cetb_file.h"

/*
 * global variables used in multiple tests
  */
cetb_file_class *cetb;
int status;
char filename[ FILENAME_MAX ];
char dirname[ FILENAME_MAX ];
char auth_id[ 11 ];
int region_number;
int factor;
cetb_platform_id platform_id;
cetb_sensor_id sensor_id;
int year;
int doy;
int beam_id;
cetb_direction_id direction_id;
cetb_reconstruction_id reconstruction_id;
cetb_swath_producer_id producer_id;


void setUp( void ) {
  /*
   * Default values for globals
   * Tests only need to change the specific thing they're trying to find
   */
  cetb = NULL;
  status = 0;
  strcpy( dirname, getenv( "PMESDR_TOP_DIR" ) );
  strcat( dirname, "/src/prod/cetb_file/test" );
  strcpy( auth_id, "NSIDC-0738" );
  region_number = cetb_region_number[ CETB_EASE2_N ];
  factor = 1;
  platform_id = CETB_SMAP;
  sensor_id = CETB_SMAP_RADIOMETER;
  year = 2016;
  doy = 1;
  beam_id = 1;
  direction_id = CETB_MORNING_PASSES;
  reconstruction_id = CETB_SIR;
  producer_id = CETB_JPL;

  cetb = cetb_file_init( dirname,
			 region_number, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  sprintf( filename, "%s/%s-EASE2_N12.5km-SMAP_LRM-2016001-1.4H-M-SIR-JPL-v%.1f.nc",
	   dirname, auth_id, SMAP_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename, cetb->filename );
  TEST_ASSERT_EQUAL_INT( direction_id, cetb->direction_id );
  TEST_ASSERT_EQUAL_INT( sensor_id, cetb->sensor_id );
  
}

void tearDown( void ) {

  cetb_file_close( cetb );
}

void test_cetb_smap_open_with_bad_filename( void ) {

  cetb->filename = NULL;
  status = cetb_file_open( cetb );
  fprintf( stderr, "%s: status return with NULL filename %d\n", __FUNCTION__, status );
  TEST_ASSERT_TRUE( 0 != status  );
  
}

void test_cetb_smap_open( void ) {

  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE( 0 == status );
  
}
