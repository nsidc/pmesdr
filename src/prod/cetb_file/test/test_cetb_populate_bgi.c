/*
 * test_cetb_populate_bgi - Unit tests for populating cetb_file objects
 *
 * 01-Sep-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <netcdf.h>
#include <string.h>
#include <udunits2.h>

#include "unity.h"
#include "calcalcs.h"
#include "cetb_file.h"
#include "utCalendar2_cal.h"

/*
 * global variables used in multiple tests
  */
cetb_file_class *cetb;
int status;
char test_filename[ FILENAME_MAX ];
char dirname[ FILENAME_MAX ];
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
  strcpy( test_filename, "./test/EASE2_N25km.F13_SSMI.1991001.19H.M.BGI.CSU.v0.1.nc" );
  strcpy( dirname, "./test" );
  region_number = cetb_region_number[ CETB_EASE2_N ];
  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 1;
  beam_id = 1;
  direction_id = CETB_MORNING_PASSES;
  reconstruction_id = CETB_BGI;
  producer_id = CETB_CSU;

  cetb = cetb_file_init( dirname,
			 region_number, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_STRING( test_filename, cetb->filename );
  
}

void tearDown( void ) {

}

void test_cetb_populate_bgi_parameters( void ) {

  int nc_fileid=0;
  double gamma=0.0D;
  float dimensional_tuning_parameter=1.0;
  float noise_variance=1.5;
  float db_threshold=2.0;
  float diff_threshold=3.0;
  int median_filter=1;
  double expected_gamma=0.0D;
  float expected_dimensional_tuning_parameter=1.0;
  float expected_noise_variance=1.5;
  float expected_db_threshold=2.0;
  float expected_diff_threshold=3.0;
  int expected_median_filter=1;
  
  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_open" );
  status = cetb_file_add_bgi_parameters( cetb, gamma, dimensional_tuning_parameter,
					 noise_variance,
					 db_threshold, diff_threshold, median_filter );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_add_bgi_parameters" );
  cetb_file_close( cetb );

  /* Confirm the expected values are in the output file */
  status = nc_open( test_filename, NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_get_att_double( nc_fileid, NC_GLOBAL, "bgi_gamma", &gamma );
  TEST_ASSERT_TRUE( NC_NOERR == status );
  TEST_ASSERT_EQUAL_DOUBLE( expected_gamma, gamma );

  status = nc_get_att_float( nc_fileid, NC_GLOBAL, "bgi_dimensional_tuning_parameter",
			     &dimensional_tuning_parameter );
  TEST_ASSERT_TRUE( NC_NOERR == status );
  TEST_ASSERT_EQUAL_FLOAT( expected_dimensional_tuning_parameter, dimensional_tuning_parameter );

  status = nc_get_att_float( nc_fileid, NC_GLOBAL, "bgi_noise_variance",
			     &noise_variance );
  TEST_ASSERT_TRUE( NC_NOERR == status );
  TEST_ASSERT_EQUAL_FLOAT( expected_noise_variance, noise_variance );

  status = nc_get_att_float( nc_fileid, NC_GLOBAL, "bgi_db_threshold", &db_threshold );
  TEST_ASSERT_TRUE( NC_NOERR == status );
  TEST_ASSERT_EQUAL_FLOAT( expected_db_threshold, db_threshold );

  status = nc_get_att_float( nc_fileid, NC_GLOBAL, "bgi_diff_threshold", &diff_threshold );
  TEST_ASSERT_TRUE( NC_NOERR == status );
  TEST_ASSERT_EQUAL_FLOAT( expected_diff_threshold, diff_threshold );

  status = nc_get_att_int( nc_fileid, NC_GLOBAL, "bgi_median_filter", &median_filter );
  TEST_ASSERT_TRUE( NC_NOERR == status );
  TEST_ASSERT_EQUAL_INT( expected_median_filter, median_filter );

  nc_close( nc_fileid );

}

void test_cetb_populate_sir_parameters_on_bgi_file( void ) {

  int nc_fileid=0;
  int nits=20;
  int median_filter=1;
  
  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_open" );
  status = cetb_file_add_sir_parameters( cetb, nits, median_filter );
  TEST_ASSERT_TRUE_MESSAGE( 0 != status, "cetb_file_add_sir_parameters" );
  cetb_file_close( cetb );

}


    
