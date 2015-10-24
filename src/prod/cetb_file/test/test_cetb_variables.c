/*
 * test_cetb_variables - Unit tests for populating cetb_file objects with TB data
 *
 * 01-Sep-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <float.h>
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
char test_filename[ FILENAME_MAX ];
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


/* Helper functions */
static int allocate_clean_aligned_memory( void **this, size_t size );
static char *get_text_att( int fileid, int varid, const char *name );

void setUp( void ) {
  /*
   * Default values for globals
   * Tests only need to change the specific thing they're trying to find
   */
  cetb = NULL;
  status = 0;
  strcpy( dirname, getenv( "PMESDR_TOP_DIR" ) );
  strcat( dirname, "/src/prod/cetb_file/test" );
  strcpy( test_filename, dirname );
  strcat( test_filename, "/EASE2_T25km.F13_SSMI.1991153.19H.A.SIR.CSU.v0.1.nc" );
  region_id = CETB_EASE2_T;
  region_number = cetb_region_number[ region_id ];
  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 153;
  beam_id = 1;
  direction_id = CETB_ASC_PASSES;
  reconstruction_id = CETB_SIR;
  producer_id = CETB_CSU;

  cetb = cetb_file_init( dirname,
			 region_number, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_STRING( test_filename, cetb->filename );
  status = cetb_file_open( cetb );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  
}

void tearDown( void ) {

}

void test_cetb_tbs_wrong_dims( void ) {

  int status;
  size_t rows=2;
  size_t cols=3;
  float *data;

  status = allocate_clean_aligned_memory( ( void * )&data, sizeof( float ) * rows * cols );
  TEST_ASSERT_EQUAL_INT( 0, status );
  
  status = cetb_file_add_packed_floats( cetb, "TB", data,
					cols, rows,
					"brightness_temperature",
					"SIR TB",
					"kelvin",
					(unsigned short) CETB_FILE_TB_FILL_VALUE,
					(unsigned short) CETB_FILE_TB_MISSING_VALUE,
					(unsigned short) CETB_FILE_TB_MIN,
					(unsigned short) CETB_FILE_TB_MAX,
					(float) CETB_FILE_TB_SCALE_FACTOR,
					(float) CETB_FILE_TB_ADD_OFFSET
					);
  TEST_ASSERT_EQUAL_INT_MESSAGE( 1, status, "bad dimensions" );
  cetb_file_close( cetb );

}

void test_cetb_tbs( void ) {

  int nc_fileid=0;
  int dim_id;
  int tb_var_id, tb_stddev_var_id, tb_num_samples_var_id, tb_time_var_id;
  size_t rows=cetb_grid_rows[ region_id ][ factor ];
  size_t cols=cetb_grid_cols[ region_id ][ factor ];
  size_t dim_len;
  size_t expected_rows=cetb_grid_rows[ region_id ][ factor ];
  size_t expected_cols=cetb_grid_cols[ region_id ][ factor ];
  size_t expected_times=1;
  char *att_p;
  size_t att_len;
  float *data;
  unsigned short fill_value;
  unsigned short missing_value;
  unsigned short valid_range[ 2 ];
  unsigned short expected_tb_valid_range[ 2 ] = {
    CETB_FILE_TB_MIN,
    CETB_FILE_TB_MAX
  };
  float scale_factor;
  float add_offset;

  status = allocate_clean_aligned_memory( ( void * )&data, sizeof( float ) * rows * cols );
  TEST_ASSERT_EQUAL_INT( 0, status );
  
  status = cetb_file_add_packed_floats( cetb, "TB", data,
					cols, rows,
					"brightness_temperature",
					"SIR TB",
					"kelvin",
					(unsigned short) CETB_FILE_TB_FILL_VALUE,
					(unsigned short) CETB_FILE_TB_MISSING_VALUE,
					(unsigned short) CETB_FILE_TB_MIN,
					(unsigned short) CETB_FILE_TB_MAX,
					(float) CETB_FILE_TB_SCALE_FACTOR,
					(float) CETB_FILE_TB_ADD_OFFSET
					);
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, __FUNCTION__ );
  status = cetb_file_add_packed_floats( cetb, "TB_std_dev", data,
					cols, rows,
					NULL,
					"SIR TB Std Dev",
					"kelvin",
					(unsigned short) CETB_FILE_TB_STDDEV_FILL_VALUE,
					(unsigned short) CETB_FILE_TB_STDDEV_MISSING_VALUE,
					(unsigned short) CETB_FILE_TB_STDDEV_MIN,
					(unsigned short) CETB_FILE_TB_STDDEV_MAX,
					(float) CETB_FILE_TB_STDDEV_SCALE_FACTOR,
					(float) CETB_FILE_TB_STDDEV_ADD_OFFSET
					);
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, __FUNCTION__ );
  cetb_file_close( cetb );

  status = nc_open( test_filename, NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  /* Confirm the expected TB variable is in the output file */
  status = nc_inq_varid( nc_fileid, "TB", &tb_var_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );

  /* Confirm the expected dimension attributes are in the output file */
  att_p = get_text_att( nc_fileid, tb_var_id, "standard_name" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "brightness_temperature", att_p, "TB standard_name" );
  free( att_p );
  att_p = get_text_att( nc_fileid, tb_var_id, "long_name" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "SIR TB", att_p, "TB long_name" );
  free( att_p );
  att_p = get_text_att( nc_fileid, tb_var_id, "units" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "kelvin", att_p, "TB units" );
  free( att_p );

  /* _FillValue, missing_value and valid_range */
  status = nc_inq_var_fill( nc_fileid, tb_var_id, NULL, &fill_value );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_TB_FILL_VALUE, fill_value, "TB _FillValue" );
  status = nc_get_att_ushort( nc_fileid, tb_var_id, "missing_value", &missing_value );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_TB_MISSING_VALUE, missing_value,
				 "TB missing_value" );
  

  status = nc_get_att_ushort( nc_fileid, tb_var_id, "valid_range", valid_range );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_INT_MESSAGE( expected_tb_valid_range[ 0 ], valid_range[ 0 ],
				 "tb valid_range min" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( expected_tb_valid_range[ 1 ], valid_range[ 1 ],
				 "tb valid_range max" );

  /* scale_factor, add_offset, packing_convention, packing_convention_description */
  att_p = get_text_att( nc_fileid, tb_var_id, "packing_convention" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( CETB_FILE_PACKING_CONVENTION,
				    att_p, "TB packing_convention" );
  free( att_p );
  att_p = get_text_att( nc_fileid, tb_var_id, "packing_convention_description" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( CETB_FILE_PACKING_CONVENTION_DESC,
				    att_p, "TB packing_convention_description" );
  free( att_p );
  status = nc_get_att_float( nc_fileid, tb_var_id, "scale_factor", &scale_factor );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE( CETB_FILE_TB_SCALE_FACTOR, scale_factor,
				   "tb scale_factor" );
  status = nc_get_att_float( nc_fileid, tb_var_id, "add_offset", &add_offset );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE( CETB_FILE_TB_ADD_OFFSET, add_offset,
				   "tb add_offset" );

  att_p = get_text_att( nc_fileid, tb_var_id, "grid_mapping" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( CETB_FILE_GRID_MAPPING, att_p, "TB grid_mapping" );
  free( att_p );
  
  att_p = get_text_att( nc_fileid, tb_var_id, "coverage_content_type" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( CETB_FILE_COVERAGE_CONTENT_TYPE, att_p,
				    "TB coverage_content_type" );
  free( att_p );
  
  /* Confirm the expected TB_std_dev variable is in the output file */
  status = nc_inq_varid( nc_fileid, "TB_std_dev", &tb_stddev_var_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );

  /* There should not be a standard name for std dev */
  status = nc_inq_attlen( nc_fileid, tb_stddev_var_id, "standard_name", &att_len );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR != status, "unexpected TB_std_dev standard_name" );

  nc_close( nc_fileid );

}


/*
 * allocate_clean_aligned_memory - allocate aligned memory that is
 *                                 zeroed out.
 *
 * input:
 *   this : void ** address of pointer to new memory
 *   size : size_t size of memory to allocate
 *
 * output: n/a
 *
 * returns : STATUS_OK for success, or error message to stderr and STATUS_FAILURE
 * 
 */
int allocate_clean_aligned_memory( void **this, size_t size ) {

  if ( 0 != posix_memalign( this, CETB_FILE_ALIGNMENT, size ) ) {
    perror( __FUNCTION__ );
    return 1;
  }
  memset( *this, 0, size );
  return 0;

}

static char *get_text_att( int fileid, int varid, const char *name ) {

  int status;
  size_t att_len;
  char *p;

  status = nc_inq_attlen( fileid, varid, name, &att_len );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  status = posix_memalign( ( void * )&p, CETB_FILE_ALIGNMENT, att_len + 1 );
  status = nc_get_att_text( fileid, varid, name, p );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  p[ att_len ] = '\0';

  return p;
  
}
    
