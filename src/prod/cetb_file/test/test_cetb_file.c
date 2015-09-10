/*
 * test_cetb_file - Unit tests for initializing cetb_file objects
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <string.h>
#include "unity.h"
#include "cetb_file.h"

/*
 * global variables used in multiple tests
  */
cetb_file_class *cetb;
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
  producer_id = CETB_NO_PRODUCER;
  
}

void tearDown( void ) {

  cetb_file_close( cetb );
  
}

void test_init_with_bogus_region_id( void ) {
  
  region_id = 311;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_factor( void ) {

  factor = 5;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_platform( void ) {
  
  platform_id = CETB_NO_PLATFORM;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_sensor( void ) {

  sensor_id = CETB_NO_SENSOR;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_year( void ) {
  
  year = 1970;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_doy( void ) {
  
  doy = 366;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_north_pass_direction( void ) {

  region_id = CETB_EASE2_N;
  direction_id = CETB_ASC_PASSES;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_temperate_pass_direction( void ) {

  region_id = CETB_EASE2_T;
  direction_id = CETB_EVENING_PASSES;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_beam_id( void ) {

  beam_id = 20;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_reconstruction_id( void ) {

  reconstruction_id = CETB_UNKNOWN_RECONSTRUCTION;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_with_bogus_producer_id( void ) {

  producer_id = CETB_NO_PRODUCER;
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_FALSE( cetb );
}

void test_init_for_valid_cetb_north( void ) {

  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 1;
  direction_id = CETB_MORNING_PASSES;
  producer_id = CETB_CSU;
  
  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_TRUE( cetb );
  TEST_ASSERT_FALSE( cetb->fid );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_N25km.F13_SSMI.1991001.19H.M.SIR.CSU.v0.1.nc",
			    cetb->filename );
  
}

void test_init_for_valid_cetb_south( void ) {

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

  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_TRUE( cetb );
  TEST_ASSERT_FALSE( cetb->fid );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_S12.5km.AQUA_AMSRE.1991031.06V.E.BGI.RSS.v0.1.nc",
			    cetb->filename );
  
}

void test_init_for_valid_cetb_temperate( void ) {

  region_id = CETB_EASE2_T;
  factor = 2;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 365;
  beam_id = 7;
  direction_id = CETB_ASC_PASSES;
  producer_id = CETB_CSU;

  cetb = cetb_file_init( dirname,
			 region_id, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_TRUE( cetb );
  TEST_ASSERT_FALSE( cetb->fid );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_T6.25km.F13_SSMI.1991365.85V.A.SIR.CSU.v0.1.nc",
			    cetb->filename );
  
}

void test_get_direction( void ) {

  char info_name[ FILENAME_MAX ] = "FD1m-stuff";
  TEST_ASSERT_TRUE( CETB_MORNING_PASSES == cetb_get_direction_id_from_info_name( info_name ) );
  
}

void test_get_bogus_direction( void ) {

  char info_name[ FILENAME_MAX ] = "FD1z-bogus";
  TEST_ASSERT_TRUE( CETB_NO_DIRECTION == cetb_get_direction_id_from_info_name( info_name ) );
  
}
    
void test_get_producer_sir( void ) {

  char outpath[ FILENAME_MAX ] = "/here/is/stuff/sirCSU/";
  TEST_ASSERT_TRUE( CETB_CSU == cetb_get_swath_producer_id_from_outpath( outpath, CETB_SIR ) );
  
}

void test_get_producer_bgi( void ) {

  char outpath[ FILENAME_MAX ] = "/home/vagrant/measures-byu/NSIDCtest/bgiCSU";
  TEST_ASSERT_TRUE( CETB_CSU == cetb_get_swath_producer_id_from_outpath( outpath, CETB_BGI ) );
  
}

void test_get_bogus_producer( void ) {

  char outpath[ FILENAME_MAX ] = "/here/is/stuff/bogus/";
  TEST_ASSERT_TRUE( CETB_NO_PRODUCER
		    == cetb_get_swath_producer_id_from_outpath( outpath, CETB_SIR ) );  
}

