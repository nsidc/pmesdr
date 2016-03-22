/*
 * test_cetb_populate - Unit tests for populating cetb_file objects
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
#include "utils.h"

/*
 * global variables used in multiple tests
  */
cetb_file_class *cetb;
int status;
char filename[ FILENAME_MAX ];
char dirname[ FILENAME_MAX ];
cetb_region_id region_id;
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
  strcpy( filename, "" );
  strcpy( dirname, "./test" );
  region_id = CETB_EASE2_N;
  region_number = cetb_region_number[ region_id ];
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
			 region_number, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_STRING( "./test/EASE2_N25km.F13_SSMI.1991001.19H.M.SIR.CSU.v0.1.nc",
			    cetb->filename );
  
}

void tearDown( void ) {

}

void test_cetb_populate_sir_parameters( void ) {

  int nc_fileid=0;
  int varid;
  int nits=20;
  int median_filter=1;
  int expected_nits=20;
  int expected_median_filter=1;
  float *float_data;
  size_t rows=cetb_grid_rows[ region_id ][ factor ];
  size_t cols=cetb_grid_cols[ region_id ][ factor ];
  unsigned short fill_value=CETB_TB_FILL_VALUE;
  unsigned short missing_value=CETB_TB_MISSING_VALUE;
  unsigned short valid_range[ 2 ] = {
    CETB_TB_MIN,
    CETB_TB_MAX
  };
  
  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_open" );

  status = utils_allocate_clean_aligned_memory( ( void * )&float_data,
					  sizeof( float ) * rows * cols );
  TEST_ASSERT_EQUAL_INT( 0, status );

  status = cetb_file_add_var( cetb, "TB",
			      NC_USHORT,
			      float_data,
			      cols, rows,
			      CETB_FILE_TB_STANDARD_NAME,
			      "SIR TB",
			      CETB_FILE_TB_UNIT,
			      &fill_value,
			      &missing_value,
			      &valid_range,
			      CETB_PACK,
			      (float) CETB_TB_SCALE_FACTOR,
			      (float) CETB_TB_ADD_OFFSET,
			      NULL );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "adding TB" );

  status = cetb_file_add_sir_parameters( cetb, nits, median_filter );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_add_sir_parameters" );
  cetb_file_close( cetb );

  /* Confirm the expected values are in the output file */
  status = nc_open( "./test/EASE2_N25km.F13_SSMI.1991001.19H.M.SIR.CSU.v0.1.nc",
		    NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_inq_varid( nc_fileid, "TB", &varid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_get_att_int( nc_fileid, varid, "sir_number_of_iterations", &nits );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status, "Error on nits attribute."  );
  TEST_ASSERT_EQUAL_INT_MESSAGE( expected_nits, nits, "Wrong value for nits attribute." );

  status = nc_get_att_int( nc_fileid, varid, "median_filter", &median_filter );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status,
			    "Error on median_filter attribute" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( expected_median_filter, median_filter,
			    "Wrong value for median_filter." );
  nc_close( nc_fileid );
  
}

void test_cetb_populate_bgi_parameters_on_sir_file( void ) {

  int nc_fileid=0;
  double gamma=0.0D;
  float dimensional_tuning_parameter=1.0;
  float noise_variance=1.5;
  float db_threshold=2.0;
  float diff_threshold=3.0;
  int median_filter=1;
  
  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_open" );
  status = cetb_file_add_bgi_parameters( cetb, gamma, dimensional_tuning_parameter,
					 noise_variance,
					 db_threshold, diff_threshold, median_filter );
  TEST_ASSERT_TRUE_MESSAGE( 0 != status, "cetb_file_add_bgi_parameters" );
  cetb_file_close( cetb );

}


    
