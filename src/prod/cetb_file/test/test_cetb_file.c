/*
 * test_cetb_file - Unit tests for initializing cetb_file objects
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <string.h>
#include <udunits2.h>

#include "unity.h"
#include "calcalcs.h"
#include "cetb_file.h"

/*
 * global variables used in multiple tests
  */
cetb_file_class *cetb;
char filename[ FILENAME_MAX ];
char dirname[ FILENAME_MAX ];
int region_number;
cetb_resolution_id base_resolution;
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
  strcpy( dirname, "./test" );
  region_number = CETB_NORTH_PROJECTION; //cetb_region_number[ CETB_EASE2_N ];
  base_resolution = CETB_25KM;
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

  //  cetb_file_close( cetb );
  
}

void test_init_with_bogus_region_number( void ) {
  
  region_number = 311;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_direction_id( void ) {
  
  direction_id = CETB_NO_DIRECTION;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_resolution( void ) {

  base_resolution = CETB_NO_RESOLUTION;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_factor( void ) {

  factor = 5;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_platform( void ) {
  
  platform_id = CETB_NO_PLATFORM;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_sensor( void ) {

  sensor_id = CETB_NO_SENSOR;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_year( void ) {
  
  year = 1970;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_doy( void ) {
  
  doy = 366;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_north_pass_direction( void ) {

  region_number = CETB_NORTH_PROJECTION; //cetb_region_number[ CETB_EASE2_N ];
  direction_id = CETB_ASC_PASSES;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_temperate_pass_direction( void ) {

  region_number = CETB_CYLINDRICAL_PROJECTION; //cetb_region_number[ CETB_EASE2_T ];
  direction_id = CETB_EVENING_PASSES;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_beam_id( void ) {

  beam_id = 20;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_reconstruction_id( void ) {

  reconstruction_id = CETB_UNKNOWN_RECONSTRUCTION;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
}

void test_init_with_bogus_producer_id( void ) {

  producer_id = CETB_NO_PRODUCER;
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NULL( cetb );
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
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_N25km_F13_SSMI_M_19H_19910101_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}


void test_init_for_valid_cetb_south( void ) {

  region_number = CETB_SOUTH_PROJECTION; //cetb_region_number[ CETB_EASE2_S ];
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
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_BGI_EASE2_S12.5km_AQUA_AMSRE_E_06V_19910131_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}

void test_init_for_valid_cetb_temperate( void ) {

  region_number = CETB_CYLINDRICAL_PROJECTION; //cetb_region_number[ CETB_EASE2_T ];
  factor = 2;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 365;
  beam_id = 7;
  direction_id = CETB_ASC_PASSES;
  producer_id = CETB_CSU;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_T6.25km_F13_SSMI_A_85V_19911231_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}

void test_init_for_valid_cetb_36_north( void ) {

  region_number = CETB_NORTH_PROJECTION; //cetb_region_number[ CETB_EASE2_N36 ];
  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 1;
  base_resolution = CETB_36KM;
  direction_id = CETB_MORNING_PASSES;
  producer_id = CETB_CSU;
  
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_N36km_F13_SSMI_M_19H_19910101_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}

void test_init_for_valid_cetb_36_south( void ) {

  region_number = CETB_SOUTH_PROJECTION; //cetb_region_number[ CETB_EASE2_S36 ];
  factor = 1;
  base_resolution = CETB_36KM;
  platform_id = CETB_AQUA;
  sensor_id = CETB_AMSRE;
  year = 1991;
  doy = 31;
  beam_id = 2;
  direction_id = CETB_EVENING_PASSES;
  reconstruction_id = CETB_BGI;
  producer_id = CETB_RSS;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_BGI_EASE2_S18km_AQUA_AMSRE_E_06V_19910131_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}

void test_init_for_valid_cetb_36_temperate( void ) {

  region_number = CETB_CYLINDRICAL_PROJECTION; //cetb_region_number[ CETB_EASE2_M36 ];
  factor = 2;
  base_resolution = CETB_36KM;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 365;
  beam_id = 7;
  direction_id = CETB_ASC_PASSES;
  producer_id = CETB_CSU;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_M09km_F13_SSMI_A_85V_19911231_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}


void test_init_for_valid_cetb_24_north( void ) {

  region_number = CETB_NORTH_PROJECTION; //cetb_region_number[ CETB_EASE2_N24 ];
  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 1;
  base_resolution = CETB_24KM;
  direction_id = CETB_MORNING_PASSES;
  producer_id = CETB_CSU;
  
  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_N24km_F13_SSMI_M_19H_19910101_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}

void test_init_for_valid_cetb_24_south( void ) {

  region_number = CETB_SOUTH_PROJECTION; //cetb_region_number[ CETB_EASE2_S24 ];
  factor = 1;
  base_resolution = CETB_24KM;
  platform_id = CETB_AQUA;
  sensor_id = CETB_AMSRE;
  year = 1991;
  doy = 31;
  beam_id = 2;
  direction_id = CETB_EVENING_PASSES;
  reconstruction_id = CETB_BGI;
  producer_id = CETB_RSS;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_BGI_EASE2_S12km_AQUA_AMSRE_E_06V_19910131_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}

void test_init_for_valid_cetb_24_temperate( void ) {

  region_number = CETB_CYLINDRICAL_PROJECTION; //cetb_region_number[ CETB_EASE2_M24 ];
  factor = 2;
  base_resolution = CETB_24KM;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 365;
  beam_id = 7;
  direction_id = CETB_ASC_PASSES;
  producer_id = CETB_CSU;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_M06km_F13_SSMI_A_85V_19911231_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}

void test_init_for_valid_cetb_24_3_temperate( void ) {

  region_number = CETB_CYLINDRICAL_PROJECTION; //cetb_region_number[ CETB_EASE2_M24 ];
  factor = 3;
  base_resolution = CETB_24KM;
  platform_id = CETB_SMAP;
  sensor_id = CETB_SMAP_RADIOMETER;
  year = 1991;
  doy = 365;
  beam_id = 3;
  direction_id = CETB_ASC_PASSES;
  producer_id = CETB_JPL;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_INT( 0, cetb->fid );
  sprintf( filename, "%s/NSIDC0738_SIR_EASE2_M03km_SMAP_LRM_A_1.4F_19911231_v2.0.nc",
	   dirname );
  TEST_ASSERT_EQUAL_STRING( filename,
			    cetb->filename );
  
}
