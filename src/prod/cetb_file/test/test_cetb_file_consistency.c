/*
 * test_cetb_file_consistency - Unit tests for testing possible OOR values in TB data
 *
 * 22-Jun-2016 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2016 Regents of the University of Colorado and Brigham Young University
 */
#include <float.h>
#include <libgen.h>
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
char dir[ FILENAME_MAX ];
cetb_region_id region_id;
int region_number;
int factor;
cetb_platform_id platform_id;
cetb_sensor_id sensor_id;
int year;
int doy;
char epoch_date_str[ 256 ];
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
  strcpy( dir, getenv( "PMESDR_TOP_DIR" ) );
  strcat( dir, "/src/prod/cetb_file/test" );
  strcpy( test_filename, dir );
  strcat( test_filename, "/NSIDC-0630_EASE2_T25km.F13_SSMI.1991153.19H.A.SIR.CSU.v0.1.nc" );
  region_id = CETB_EASE2_T;
  region_number = cetb_region_number[ region_id ];
  factor = 0;
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  year = 1991;
  doy = 153; 
  sprintf( epoch_date_str, "Epoch date for data in this file: %04d-%02d-%02d 00:00:00Z",
	   1991, 6, 2);
  beam_id = 1;
  direction_id = CETB_ASC_PASSES;
  reconstruction_id = CETB_SIR;
  producer_id = CETB_CSU;
  char progname[256] = "/my/path/test_program_name";

  cetb = cetb_file_init( dir,
			 region_number, factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id,
			 basename( progname ) );
  TEST_ASSERT_NOT_NULL( cetb );
  TEST_ASSERT_EQUAL_STRING( test_filename, cetb->filename );
  status = cetb_file_open( cetb );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );
  
}

void tearDown( void ) {

}
/*
 * This function is to test the file_consistency function of cetb_file
 *
 * Insert temperatures that are OOR and ensure that they come back as set to missing
 *
 * Also check that TB_std_dev has the corresponding value set to missing
 *
 */
void test_cetb_file_consistency( void ) {

  int i;
  int nc_fileid=0;
  int dim_id;
  int tb_var_id, tb_stddev_var_id, tb_num_samples_var_id;
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
  int int_fill_value=CETB_NCATTS_TB_FILL_VALUE;
  int int_valid_range[ 2 ] = {
    CETB_NCATTS_TB_MIN,
    CETB_NCATTS_TB_MAX
  };
  float sample_tb_time0 = (float)(CETB_NCATTS_TB_TIME_FILL_VALUE*CETB_NCATTS_TB_TIME_SCALE_FACTOR);
  float sample_tb_time1 = 1440.0;
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
  float sample_tb2 = 365.05;
  float sample_tb_std_dev0 = 0.9;
  float sample_tb_std_dev1 = 2.0;
  float sample_tb_std_dev2 = 3.1;
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
  float_data[ (rows*cols) - 1 ] = sample_tb2; // Last element of array 
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
  
  float_data[ 0 ] = sample_tb_std_dev0;     // First element of array (row=0)
  float_data[ cols ] = sample_tb_std_dev1;  // First column of array (row=1)
  float_data[ (rows*cols) - 1 ] = sample_tb_std_dev2; // Last element of array 
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

  cetb_file_close( cetb );

  status = cetb_file_check_consistency( "testing" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( -1, status, "checking consistency" );

  status = cetb_file_check_consistency( test_filename );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "checking consistency" );

  status = nc_open( test_filename, NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  /* Confirm that the data date is the value of the comment attribute */
  att_p = get_text_att( nc_fileid, NC_GLOBAL, "comment" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( epoch_date_str, att_p, "comment" );
  free( att_p );

  /* Confirm that the history is the name of the program used to create it */
  att_p = get_text_att( nc_fileid, NC_GLOBAL, "history" );
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "test_program_name", att_p, "history" );
  free( att_p );

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
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_PACK_DATA( CETB_NCATTS_TB_SCALE_FACTOR,
						      CETB_NCATTS_TB_ADD_OFFSET,
						      600.0 ),
				 tb_data[ cols-1 ],     // Last element of first row array
				 "sample2 tb_data element" );
  
  /* Confirm the expected TB_std_dev variable is in the output file */
  status = nc_inq_varid( nc_fileid, "TB_std_dev", &tb_stddev_var_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );

  /* Read the actual TB_std_dev data */
  status = nc_inq_var( nc_fileid, tb_stddev_var_id, 0, &xtype, &ndims, dim_ids, &natts );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, nc_strerror( status ) );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_USHORT, xtype, "unexpected TB_std_dev data type" );
  
  /* Zero out the ushort_data values */
  for ( i = 0; i < ( rows * cols ); i++ ) {
    *( tb_data + i ) = 0;
  }
  status = nc_get_var_ushort( nc_fileid, tb_stddev_var_id, tb_data );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "reading tb_std_dev data" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_PACK_DATA( CETB_NCATTS_TB_STDDEV_SCALE_FACTOR,
						      CETB_NCATTS_TB_STDDEV_ADD_OFFSET,
						      sample_tb_std_dev0 ),
				 tb_data[ cols * ( rows - 1 ) ], // First element of last row
				 "sample0 tb_std_dev element" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_FILE_PACK_DATA( CETB_NCATTS_TB_STDDEV_SCALE_FACTOR,
						      CETB_NCATTS_TB_STDDEV_ADD_OFFSET,
						      sample_tb_std_dev1 ),
				 tb_data[ cols * (rows - 2 ) ], // First element of second-to-last row
				 "sample1 tb_std_dev element" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( CETB_NCATTS_TB_STDDEV_MISSING_VALUE,
				 tb_data[ cols-1 ], // Last element at end of first row
				 "sample2 tb_std_dev element" );
  
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

/* Test function for udunits date conversions */
void test_time_functions( void ) {

  double my_time, second, resolution;
  int year, month, day, hour, minute;

  my_time = ut_encode_time( 2016, 8, 23, 0, -5, 0 );
  ut_decode_time( my_time, &year, &month, &day, &hour, &minute, &second, &resolution );
  fprintf( stderr, "%s: %4d-%02d-%02dT%02d:%02d:%05.2lfZ\n",
	  __FUNCTION__, year, month, day, hour, minute, second );
  
}
    
