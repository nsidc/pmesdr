#include <string.h>
#include "unity.h"
#include "cetb_file.h"

/*
 * global variables used in multiple tests
  */
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
  status = 0;
  strcpy( filename, "" );
  strcpy( dirname, "/path_to_file" );
  region_id = CETB_EASE2_N;
  factor = 1;
  platform_id = CETB_F08;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 1;
  beam_id = 1;
  direction_id = CETB_ALL_PASSES;
  reconstruction_id = CETB_SIR;
  producer_id = CETB_UNKNOWN_PRODUCER;
  
}

void tearDown( void ) {
}

void test_bogus_region_id( void ) {
  
  region_id = 311;
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_factor( void ) {

  factor = 5;
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_platform( void ) {
  
  platform_id = CETB_NO_PLATFORM;
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_sensor( void ) {

  sensor_id = CETB_NO_SENSOR;
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_year( void ) {
  
  year = 1970;
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_doy( void ) {
  
  doy = 366;
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_north_pass_direction( void ) {

  region_id = CETB_EASE2_N;
  direction_id = CETB_ASC_PASSES;
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_temperate_pass_direction( void ) {

  region_id = CETB_EASE2_T;
  direction_id = CETB_EVENING_PASSES;
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id, 
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( status );
}

void test_cetb_north_filename( void ) {

  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 1;
  direction_id = CETB_MORNING_PASSES;
  producer_id = CETB_CSU;
  
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_TRUE( status );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_N25km.F13_SSMI.1991001.19H.M.SIR.CSU.v0.1.nc", filename );
  
}

void test_cetb_south_filename( void ) {

  region_id = CETB_EASE2_S;
  factor = 1;
  platform_id = CETB_AQUA;
  sensor_id = CETB_AMSRE;
  year = 1991;
  doy = 31;
  beam_id = 2;
  direction_id = CETB_EVENING_PASSES;
  reconstruction_id = CETB_BGI;
  producer_id = CETB_RSS;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_TRUE( status );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_S12.5km.AQUA_AMSRE.1991031.06V.E.BGI.RSS.v0.1.nc", filename );
  
}

void test_cetb_temperate_filename( void ) {

  region_id = CETB_EASE2_T;
  factor = 2;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 365;
  beam_id = 7;
  direction_id = CETB_ASC_PASSES;
  producer_id = CETB_CSU;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname,
			  region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			  direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_TRUE( status );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_T6.25km.F13_SSMI.1991365.85V.A.SIR.CSU.v0.1.nc", filename );
  
}
