/*
 * test_cetb_variables - Unit tests for populating cetb_file objects for variable data
 *
 * 01-Sep-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <netcdf.h>
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
  region_id = CETB_EASE2_T;
  region_number = cetb_region_number[ region_id ];
  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 1;
  beam_id = 1;
  direction_id = CETB_ASC_PASSES;
  reconstruction_id = CETB_SIR;
  producer_id = CETB_CSU;

  cetb = cetb_file_init( dirname,
			 region_number, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_STRING( "./test/EASE2_T25km.F13_SSMI.1991001.19H.A.SIR.CSU.v0.1.nc",
			    cetb->filename );
  status = cetb_file_open( cetb );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );

  
}

void tearDown( void ) {

}

void test_cetb_dimensions( void ) {

  int nc_fileid=0;
  int dim_id;
  int rows_var_id;
  int cols_var_id;
  size_t rows=cetb_grid_rows[ region_id ][ factor ];
  size_t cols=cetb_grid_cols[ region_id ][ factor ];
  size_t dim_len;
  size_t expected_rows=cetb_grid_rows[ region_id ][ factor ];
  size_t expected_cols=cetb_grid_cols[ region_id ][ factor ];
  double half_pixel_m = cetb_exact_scale_m[ region_id ][ factor ] / 2.D;
  double value;
  double expected_rows_valid_range[ 2 ] = {
    0.D - ( cetb_exact_scale_m[ region_id ][ factor ] * cetb_grid_rows[ region_id ][ factor ] / 2.D ),
    cetb_exact_scale_m[ region_id ][ factor ] * cetb_grid_rows[ region_id ][ factor ] / 2.D
  };
  /* center of any top row pixel */
  double expected_first_y=expected_rows_valid_range[ 1 ] - half_pixel_m;
  /* center of any bottom row pixel*/
  double expected_last_y=expected_rows_valid_range[ 0 ] + half_pixel_m;
  size_t index[] = { 0 };
  size_t att_len;
  char att_name[ 256 ];
  char att_str[ 256 ];
  double valid_range[ 2 ];

  status = cetb_file_set_dimensions( cetb, rows, cols );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "cetb_file_set_dimensions" );
  cetb_file_close( cetb );

  status = nc_open( "./test/EASE2_T25km.F13_SSMI.1991001.19H.A.SIR.CSU.v0.1.nc",
		    NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  /* Confirm the expected rows, cols, time dimensions are in the output file */
  status = nc_inq_dimid( nc_fileid, "rows", &dim_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  status = nc_inq_dimlen( nc_fileid, dim_id, &dim_len );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  TEST_ASSERT_TRUE( expected_rows == dim_len );

  /* Confirm the expected dimension values are in the output file */
  status = nc_inq_varid( nc_fileid, "rows", &rows_var_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  status = nc_get_var1_double( nc_fileid, rows_var_id, index, &value );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_first_y, value, "first_y_coordinate" );
  index[ 0 ] = expected_rows - 1;
  status = nc_get_var1_double( nc_fileid, rows_var_id, index, &value ); 
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_last_y, value, "last_y_coordinate" );

  /* Confirm the expected dimension attributes are in the output file */
  strcpy( att_name, "standard_name" );
  status = nc_inq_attlen( nc_fileid, rows_var_id, att_name, &att_len );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  status = nc_get_att_text( nc_fileid, rows_var_id, att_name, att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "projection_y_coordinate", att_str, "rows standard_name" );
  
  strcpy( att_name, "units" );
  status = nc_inq_attlen( nc_fileid, rows_var_id, att_name, &att_len );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  status = nc_get_att_text( nc_fileid, rows_var_id, att_name, att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "meters", att_str, "rows units" );

  strcpy( att_name, "axis" );
  status = nc_inq_attlen( nc_fileid, rows_var_id, att_name, &att_len );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  status = nc_get_att_text( nc_fileid, rows_var_id, att_name, att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "Y", att_str, "rows axis" );

  status = nc_get_att_double( nc_fileid, rows_var_id, "valid_range", valid_range );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_rows_valid_range[ 0 ], valid_range[ 0 ], "valid_range min" );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_rows_valid_range[ 1 ], valid_range[ 1 ], "valid_range max" );

  /* status = nc_inq_dimid( nc_fileid, "time", &dim_id ); */
  /* TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) ); */

  /* status = nc_inq_dimlen( nc_fileid, dim_id, &dim_len ); */
  /* TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) ); */
  /* TEST_ASSERT_TRUE( 1 == dim_len ); */

  nc_close( nc_fileid );
  
}


    
