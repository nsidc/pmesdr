/*
 * test_cetb_ltod - Unit tests for saved ltod metadata times
 *
 * 12-Jan-2018 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2018 Regents of the University of Colorado and Brigham Young University
 */
#include <netcdf.h>
#include <string.h>
#include <udunits2.h>

#include "unity.h"
#include "calcalcs.h"
#include "cetb_ncatts.h"
#include "cetb_file.h"
#include "utils.h"

/*
 * global variables used in multiple tests
  */
int nc_fileid;
int varid;
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
int nits;
int median_filter;
float *float_data;
float rthreshold;
float box_size_km;
size_t rows;
size_t cols;
unsigned short fill_value;
unsigned short missing_value;
unsigned short valid_range[ 2 ];
 
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
  reconstruction_id = CETB_SIR;
  producer_id = CETB_CSU;
  nits = 20;
  median_filter = 1;
  rthreshold = -10.0;
  box_size_km = 625.0;
  rows = cetb_grid_rows[ region_id ][ factor ];
  cols = cetb_grid_cols[ region_id ][ factor ];

}

void tearDown( void ) {

}

void test_cetb_morning_ltod( void ) {

  float ltod_start = 0.0;
  float ltod_end = 12.0;
  float expected_ltod_start = 0.0;
  float expected_ltod_end = 12.0;

  direction_id = CETB_MORNING_PASSES;
  
  cetb = cetb_file_init( dirname,
			 region_number, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_STRING( "./test/"
			    "NSIDC-0630-EASE2_N25km-F13_SSMI-1991001-19H-M-SIR-CSU-v1.3.nc",
			    cetb->filename );
  
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
			      (float) CETB_NCATTS_TB_SCALE_FACTOR,
			      (float) CETB_NCATTS_TB_ADD_OFFSET,
			      NULL );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "adding TB" );

  status = cetb_file_add_sir_parameters( cetb, nits, median_filter );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_add_sir_parameters" );

  status = cetb_file_add_TB_parameters( cetb, rthreshold, box_size_km, ltod_start, ltod_end );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_add_TB_parameters" );
  cetb_file_close( cetb );

  /* Confirm the expected values are in the output file */
  status = nc_open( "./test/"
		    "NSIDC-0630-EASE2_N25km-F13_SSMI-1991001-19H-M-SIR-CSU-v1.3.nc",
		    NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_inq_varid( nc_fileid, "TB", &varid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_get_att_float( nc_fileid, varid, "temporal_division_local_start_time", &ltod_start );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status, "Error on ltod_start attribute."  );
  TEST_ASSERT_EQUAL_INT_MESSAGE( expected_ltod_start, ltod_start, "Wrong value for ltod_start attribute." );

  status = nc_get_att_float( nc_fileid, varid, "temporal_division_local_end_time", &ltod_end );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status, "Error on ltod_end attribute."  );
  TEST_ASSERT_EQUAL_INT_MESSAGE( expected_ltod_end, ltod_end, "Wrong value for ltod_end attribute." );

  nc_close( nc_fileid );
  
}

void test_cetb_evening_ltod( void ) {

  float ltod_start = 0.0;
  float ltod_end = 12.0;
  float expected_ltod_start = 12.0;
  float expected_ltod_end = 24.0;

  direction_id = CETB_EVENING_PASSES;
  
  cetb = cetb_file_init( dirname,
			 region_number, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_STRING( "./test/"
			    "NSIDC-0630-EASE2_N25km-F13_SSMI-1991001-19H-E-SIR-CSU-v1.3.nc",
			    cetb->filename );
  
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
			      (float) CETB_NCATTS_TB_SCALE_FACTOR,
			      (float) CETB_NCATTS_TB_ADD_OFFSET,
			      NULL );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "adding TB" );

  status = cetb_file_add_sir_parameters( cetb, nits, median_filter );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_add_sir_parameters" );

  status = cetb_file_add_TB_parameters( cetb, rthreshold, box_size_km, ltod_start, ltod_end );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_add_TB_parameters" );
  cetb_file_close( cetb );

  /* Confirm the expected values are in the output file */
  status = nc_open( "./test/"
		    "NSIDC-0630-EASE2_N25km-F13_SSMI-1991001-19H-E-SIR-CSU-v1.3.nc",
		    NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_inq_varid( nc_fileid, "TB", &varid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_get_att_float( nc_fileid, varid, "temporal_division_local_start_time", &ltod_start );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status, "Error on ltod_start attribute."  );
  TEST_ASSERT_EQUAL_INT_MESSAGE( expected_ltod_start, ltod_start, "Wrong value for ltod_start attribute." );

  status = nc_get_att_float( nc_fileid, varid, "temporal_division_local_end_time", &ltod_end );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status, "Error on ltod_end attribute."  );
  TEST_ASSERT_EQUAL_INT_MESSAGE( expected_ltod_end, ltod_end, "Wrong value for ltod_end attribute." );

  nc_close( nc_fileid );
  
}
