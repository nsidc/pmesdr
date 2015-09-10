#include <string.h>
#include "unity.h"
#include "cetb_file.h"

/*
 * global variables used in multiple tests
  */
cetb_file_class *cetb;
int status;
char filename[ FILENAME_MAX ];
char dirname[ FILENAME_MAX ];
cetb_region_id region_id;
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
  strcpy( filename, "" );
  strcpy( dirname, "./test" );
  region_id = CETB_EASE2_N;
  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 1;
  beam_id = 1;
  direction_id = CETB_MORNING_PASSES;
  reconstruction_id = CETB_SIR;
  producer_id = CETB_CSU;

  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_TRUE( cetb );
  TEST_ASSERT_EQUAL_STRING( "./test/EASE2_N25km.F13_SSMI.1991001.19H.M.SIR.CSU.v0.1.nc",
			    cetb->filename );
  
}

void tearDown( void ) {

  cetb_file_close( cetb );
}

void test_cetb_open_with_bad_filename( void ) {

  cetb->filename = NULL;
  status = cetb_file_open( cetb );
  TEST_ASSERT_FALSE( status );
  
}

void test_cetb_open( void ) {

  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE( status );
  
}

