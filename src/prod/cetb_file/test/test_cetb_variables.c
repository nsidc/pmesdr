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
#include "cetb_ncatts.h"
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
  strcat( test_filename, "/NSIDC-0630-EASE2_T25km-F13_SSMI-1991153-19H-A-SIR-CSU-v1.0.nc" );
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
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_STRING( test_filename, cetb->filename );
  status = cetb_file_open( cetb );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  
}

void tearDown( void ) {

}

void test_cetb_tbs_unpacking( void ) {

  int status;
  int i; 
  size_t size=3;
  unsigned short *packed;
  float *unpacked;
  float expected[ 3 ] = { 50., 220., 600. };

  cetb_file_close( cetb );

  status = allocate_clean_aligned_memory( ( void * )&packed, sizeof( unsigned short ) * size );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "allocating memory for packed array" );
  status = allocate_clean_aligned_memory( ( void * )&unpacked, sizeof( float ) * size );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "allocating mamory for unpacked array" );

  packed[ 0 ] = 5000;
  packed[ 1 ] = 22000;
  packed[ 2 ] = 60000;

  for ( i = 0; i < size; i++ ) {
    *( unpacked + i ) = CETB_FILE_UNPACK_DATA( 0.01, 0.0, *( packed + i ) );
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE( expected[ i ], unpacked[ i ], "bad unpack" );
  }

}

void test_cetb_tbs_packing( void ) {

  int status;
  int i; 
  size_t size=3;
  unsigned short *packed;
  float *unpacked;
  unsigned short expected[ 9 ] = { 20000, 20001, 20001 };

  cetb_file_close( cetb );

  status = allocate_clean_aligned_memory( ( void * )&packed, sizeof( unsigned short ) * size );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "allocating memory for packed array" );
  status = allocate_clean_aligned_memory( ( void * )&unpacked, sizeof( float ) * size );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "allocating mamory for unpacked array" );

  unpacked[ 0 ] = 200.0049;
  unpacked[ 1 ] = 200.0050;
  unpacked[ 2 ] = 200.0051;

  for ( i = 0; i < size; i++ ) {
    *( packed + i ) = CETB_FILE_PACK_DATA( 0.01, 0.0, *( unpacked + i ) );
    TEST_ASSERT_EQUAL_INT_MESSAGE( expected[ i ], packed[ i ], "bad pack" );
  }

}

void test_cetb_tbs_wrong_dims( void ) {

  int status;
  size_t rows=2;
  size_t cols=3;
  float *data;
  unsigned short fill_value=CETB_NCATTS_TB_FILL_VALUE;
  unsigned short missing_value=CETB_NCATTS_TB_MISSING_VALUE;
  unsigned short valid_range[ 2 ] = { CETB_NCATTS_TB_MIN, CETB_NCATTS_TB_MAX };
  status = allocate_clean_aligned_memory( ( void * )&data, sizeof( float ) * rows * cols );
  TEST_ASSERT_EQUAL_INT( 0, status );
  
  status = cetb_file_add_var( cetb, "TB",
			      NC_USHORT,
			      data,
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
  TEST_ASSERT_EQUAL_INT_MESSAGE( 1, status, "bad dimensions" );
  cetb_file_close( cetb );

}

void test_cetb_tbs( void ) {

  int i;
  int nc_fileid=0;
  int dim_id;
  int tb_var_id, tb_stddev_var_id, tb_num_samples_var_id, tb_time_var_id;
  size_t rows=cetb_grid_rows[ region_id ][ factor ];
  size_t cols=cetb_grid_cols[ region_id ][ factor ];
  size_t dim_len;
  nc_type xtype;
  int ndims;
  int natts;
  int dim_ids[ 3 ];
  size_t expected_rows=cetb_grid_rows[ region_id ][ factor ];
  size_t expected_cols=cetb_grid_cols[ region_id ][ factor ];
  size_t expected_times=1;
  char *att_p;
  size_t att_len;
  float *float_data;
  unsigned short *tb_data;
  unsigned short fill_value=CETB_NCATTS_TB_FILL_VALUE;
  unsigned short missing_value=CETB_NCATTS_TB_MISSING_VALUE;
  unsigned short valid_range[ 2 ] = {
    CETB_NCATTS_TB_MIN,
    CETB_NCATTS_TB_MAX
  };
  unsigned short expected_tb_valid_range[ 2 ] = {
    CETB_NCATTS_TB_MIN,
    CETB_NCATTS_TB_MAX
  };
  float scale_factor;
  float add_offset;
  unsigned char *ubyte_data;
  unsigned char ubyte_fill_value=CETB_NCATTS_TB_NUM_SAMPLES_FILL_VALUE;
  unsigned char ubyte_valid_range[ 2 ] = {
    CETB_NCATTS_TB_NUM_SAMPLES_MIN,
    CETB_NCATTS_TB_NUM_SAMPLES_MAX
  };
  unsigned char ubyte_expected_tb_num_samples_valid_range[ 2 ] = {
    CETB_NCATTS_TB_NUM_SAMPLES_MIN,
    CETB_NCATTS_TB_NUM_SAMPLES_MAX
  };
  short *time_data;
  int int_fill_value=CETB_NCATTS_TB_FILL_VALUE;
  int int_valid_range[ 2 ] = {
    CETB_NCATTS_TB_MIN,
    CETB_NCATTS_TB_MAX
  };
  short short_fill_value=CETB_NCATTS_TB_TIME_FILL_VALUE;
  short short_valid_range[ 2 ] = {
    CETB_NCATTS_TB_TIME_MIN,
    CETB_NCATTS_TB_TIME_MAX
  };
  int int_expected_tb_time_valid_range[ 2 ] = {
    CETB_NCATTS_TB_TIME_MIN,
    CETB_NCATTS_TB_TIME_MAX
  };
  float sample_tb0 = 50.002;
  float sample_tb1 = 100.008;
  float sample_tb_time0 = (float)(CETB_NCATTS_TB_TIME_FILL_VALUE*CETB_NCATTS_TB_TIME_SCALE_FACTOR);
  float sample_tb_time1 = 1440.0;
  float sample_num_samples0 = 254;
  float sample_num_samples1 = 100;
  float float_fill_value=-1.;
  float float_missing_value=-2.;
  float float_valid_range[ 2 ] = {
    0.0,
    10000.
  };

  status = allocate_clean_aligned_memory( ( void * )&float_data, sizeof( float ) * rows * cols );
  TEST_ASSERT_EQUAL_INT( 0, status );

  float_data[ 0 ] = sample_tb0;     // First element of array (row=0)
  float_data[ cols ] = sample_tb1;  // First column of array (row=1)
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
  
  fill_value = CETB_NCATTS_TB_STDDEV_FILL_VALUE;
  missing_value = CETB_NCATTS_TB_STDDEV_MISSING_VALUE;
  valid_range[ 0 ] = CETB_NCATTS_TB_STDDEV_MIN;
  valid_range[ 1 ] = CETB_NCATTS_TB_STDDEV_MAX;
  status = cetb_file_add_var( cetb, "TB_std_dev",
			      NC_USHORT,
			      float_data,
			      cols, rows,
			      NULL,
			      "SIR TB Std Dev",
			      CETB_FILE_TB_UNIT,
			      &fill_value,
			      &missing_value,
			      &valid_range,
			      CETB_PACK,
			      (float) CETB_NCATTS_TB_STDDEV_SCALE_FACTOR,
			      (float) CETB_NCATTS_TB_STDDEV_ADD_OFFSET,
			      NULL );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "adding TB_std_dev" );

  status = cetb_file_add_var( cetb, "TB_dump",
			      NC_FLOAT,
			      float_data,
			      cols, rows,
			      NULL,
			      "SIR TB Dump Variable",
			      CETB_FILE_TB_UNIT,
			      &float_fill_value,
			      &float_missing_value,
			      &float_valid_range,
			      CETB_NO_PACK,
			      0.0,
			      0.0,
			      NULL );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "adding TB_dump" );

  status = allocate_clean_aligned_memory( ( void * )&ubyte_data, sizeof( unsigned char ) * rows * cols );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "allocating memory for ubyte_data" );
  ubyte_data[ 0 ] = sample_num_samples0;
  ubyte_data[ cols ] = sample_num_samples1;
  status = cetb_file_add_var( cetb, "TB_num_samples",
			      NC_UBYTE,
			      ubyte_data,
			      cols, rows,
			      NULL,
			      "SIR TB Number of Measurements",
			      "count",
			      &ubyte_fill_value,
			      NULL,
			      &ubyte_valid_range,
			      CETB_NO_PACK,
			      0.0,
			      0.0,
			      NULL );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "adding TB_num_samples" );

  status = allocate_clean_aligned_memory( ( void * )&float_data, sizeof( float ) * rows * cols );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "allocating memory for TB time float_data" );
  float_data[ 0 ] = sample_tb_time0;
  float_data[ cols ] = sample_tb_time1;
  status = cetb_file_add_var( cetb, "TB_time",
			      NC_SHORT,
			      float_data,
			      cols, rows,
			      NULL,
			      "SIR TB Time of Day",
			      "minutes since 1987-01-01 00:00:00",
			      &short_fill_value,
			      NULL,
			      &short_valid_range,
			      CETB_PACK,
			      (float) CETB_NCATTS_TB_TIME_SCALE_FACTOR,
			      (float) CETB_NCATTS_TB_TIME_ADD_OFFSET,
			      "gregorian" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "adding TB_time" );

  cetb_file_close( cetb );

  status = nc_open( test_filename, NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  /* Confirm the expected TB variable is in the output file */
  status = nc_inq_varid( nc_fileid, "TB", &tb_var_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );

  /* Read the actual TB data */
  status = nc_inq_var( nc_fileid, tb_var_id, 0, &xtype, &ndims, dim_ids, &natts );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_USHORT, xtype, "unexpected TB data type" );

  status = allocate_clean_aligned_memory( ( void * )&tb_data, sizeof( unsigned short ) * rows * cols );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "allocating memory for ushort_data" );

  status = nc_get_var_ushort( nc_fileid, tb_var_id, tb_data );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "reading tb data" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_PACK_DATA( CETB_NCATTS_TB_SCALE_FACTOR,
						      CETB_NCATTS_TB_ADD_OFFSET,
						      sample_tb0 ),
				 tb_data[ cols * ( rows - 1 ) ],     // First element of last row
				 "sample0 tb_data element" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_PACK_DATA( CETB_NCATTS_TB_SCALE_FACTOR,
						      CETB_NCATTS_TB_ADD_OFFSET,
						      sample_tb1 ),
				 tb_data[ cols * ( rows - 2 ) ],  // First element of second-to-last row
				 "sample1 tb_data element" );


  /* Confirm the expected variable attributes are in the output file */
  att_p = get_text_att( nc_fileid, tb_var_id, "standard_name" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( CETB_FILE_TB_STANDARD_NAME, att_p, "TB standard_name" );
  free( att_p );
  att_p = get_text_att( nc_fileid, tb_var_id, "long_name" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "SIR TB", att_p, "TB long_name" );
  free( att_p );
  att_p = get_text_att( nc_fileid, tb_var_id, "units" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( CETB_FILE_TB_UNIT, att_p, "TB units" );
  free( att_p );

  /* _FillValue, missing_value and valid_range */
  status = nc_inq_var_fill( nc_fileid, tb_var_id, NULL, &fill_value );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_NCATTS_TB_FILL_VALUE, fill_value, "TB _FillValue" );
  status = nc_get_att_ushort( nc_fileid, tb_var_id, "missing_value", &missing_value );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_NCATTS_TB_MISSING_VALUE, missing_value,
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
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE( CETB_NCATTS_TB_SCALE_FACTOR, scale_factor,
				   "tb scale_factor" );
  status = nc_get_att_float( nc_fileid, tb_var_id, "add_offset", &add_offset );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE( CETB_NCATTS_TB_ADD_OFFSET, add_offset,
				   "tb add_offset" );

  att_p = get_text_att( nc_fileid, tb_var_id, "grid_mapping" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( CETB_FILE_GRID_MAPPING, att_p, "TB grid_mapping" );
  free( att_p );
  
  att_p = get_text_att( nc_fileid, tb_var_id, "coverage_content_type" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( CETB_FILE_COVERAGE_CONTENT_TYPE_IMAGE, att_p,
				    "TB coverage_content_type" );
  free( att_p );
  
  /* Confirm the expected TB_std_dev variable is in the output file */
  status = nc_inq_varid( nc_fileid, "TB_std_dev", &tb_stddev_var_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );

  /* There should not be a standard name for std dev */
  status = nc_inq_attlen( nc_fileid, tb_stddev_var_id, "standard_name", &att_len );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR != status, "unexpected TB_std_dev standard_name" );

  /* Confirm the expected TB_num_samples variable is in the output file */
  status = nc_inq_varid( nc_fileid, "TB_num_samples", &tb_num_samples_var_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );

  /* There should be a flag_values attribute for TB_num_samples */
  status = nc_inq_attlen( nc_fileid, tb_num_samples_var_id, "flag_values", &att_len );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status, "expected TB_num_samples flag_values" );

  /* There should be a flag_meanings attribute for TB_num_samples */
  status = nc_inq_attlen( nc_fileid, tb_num_samples_var_id, "flag_meanings", &att_len );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status, "expected TB_num_samples flag_meanings" );

  /* There should not be a standard name for num samples */
  status = nc_inq_attlen( nc_fileid, tb_num_samples_var_id, "standard_name", &att_len );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR != status, "unexpected TB_num_samples standard_name" );

  status = nc_get_att_ubyte( nc_fileid, tb_num_samples_var_id, "valid_range", ubyte_valid_range );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_INT_MESSAGE( ubyte_expected_tb_num_samples_valid_range[ 0 ], ubyte_valid_range[ 0 ],
				 "tb_num_samples valid_range min" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( ubyte_expected_tb_num_samples_valid_range[ 1 ], ubyte_valid_range[ 1 ],
				 "tb_num_samples valid_range max" );

  /* Read the actual TB_num_samples data */
  status = nc_inq_var( nc_fileid, tb_num_samples_var_id, 0, &xtype, &ndims, dim_ids, &natts );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_UBYTE, xtype, "unexpected TB_num_samples data type" );
  
  /* Zero out the ubyte_data values */
  for ( i = 0; i < ( rows * cols ); i++ ) {
    *( ubyte_data + i ) = 255;
  }
  status = nc_get_var_ubyte( nc_fileid, tb_num_samples_var_id, ubyte_data );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "reading tb_num_samples data" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( sample_num_samples0,
				 ubyte_data[ cols * ( rows - 1 ) ], // First elements of last row
				 "sample0 tb_num_samples element" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( sample_num_samples1,
				 ubyte_data[ cols * (rows - 2 ) ], // First element of second-to-last row
				 "sample1 tb_num_samples element" );

  /* Confirm the expected TB_time variable is in the output file */
  status = nc_inq_varid( nc_fileid, "TB_time", &tb_time_var_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );

  /* There should not be a standard name for time */
  status = nc_inq_attlen( nc_fileid, tb_time_var_id, "standard_name", &att_len );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR != status, "unexpected TB_time standard_name" );
  att_p = get_text_att( nc_fileid, NC_GLOBAL, "time_coverage_start" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "1991-06-02T00:00:00.00Z", att_p, "time coverage start" );
  free( att_p );
  att_p = get_text_att( nc_fileid, NC_GLOBAL, "time_coverage_end" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "1991-06-03T00:00:00.00Z", att_p, "time coverage end" );
  free( att_p );
  att_p = get_text_att( nc_fileid, NC_GLOBAL, "time_coverage_duration" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "P01T00:00:00.00", att_p, "time coverage duration" );
  free( att_p );

  status = nc_get_att_int( nc_fileid, tb_time_var_id, "valid_range", int_valid_range );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_INT_MESSAGE( int_expected_tb_time_valid_range[ 0 ], int_valid_range[ 0 ],
				 "tb_time valid_range min" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( int_expected_tb_time_valid_range[ 1 ], int_valid_range[ 1 ],
				 "tb_time valid_range max" );

  status = allocate_clean_aligned_memory( ( void * )&time_data, sizeof( short ) * rows * cols );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "allocating memory for short_data" );

  status = nc_get_var_short( nc_fileid, tb_time_var_id, time_data );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "reading time data" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_PACK_DATA( CETB_NCATTS_TB_TIME_SCALE_FACTOR,
						      CETB_NCATTS_TB_TIME_ADD_OFFSET,
						      NC_MIN_SHORT ),
				 time_data[ cols * ( rows - 1 ) ],     // First element of last row
				 "sample0 tb_time element" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_PACK_DATA( CETB_NCATTS_TB_TIME_SCALE_FACTOR,
						      CETB_NCATTS_TB_TIME_ADD_OFFSET,
						      sample_tb_time1 ),
				 time_data[ cols * ( rows - 2 ) ],  // First element of second-to-last row
				 "sample1 tb_time element" );

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
    
