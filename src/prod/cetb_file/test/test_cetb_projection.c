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

/*
 * global variables used in multiple tests
  */
cetb_file_class *cetb;
int status;
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
  status = 0;
  strcpy( dirname, "./test" );
  platform_id = CETB_F13;
  sensor_id = CETB_SSMI;
  base_resolution = CETB_25KM;
  year = 1991;
  doy = 1;
  beam_id = 1;
  direction_id = CETB_MORNING_PASSES;
  reconstruction_id = CETB_SIR;
  producer_id = CETB_CSU;

}

void tearDown( void ) {

}

void test_cetb_N_parameters( void ) {

  int nc_fileid=0;
  int crs_id;
  size_t att_len;
  nc_type att_type;
  char att_name[ 256 ];
  char att_str[ 256 ];
  double att_double;
  double expected_double;
  
  region_number = cetb_region_number[ 0 ][ CETB_EASE2_N ];
  factor = 3;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_N3.125km_F13_SSMI_M_19H_19910101_v%.1f.nc",
	   dirname, CETB_VERSION_ID );

  TEST_ASSERT_EQUAL_STRING( filename, cetb->filename );
  
  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_open" );
  cetb_file_close( cetb );

  /* Confirm the expected values are in the output file */
  status = nc_open( filename, NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_inq_varid( nc_fileid, "crs", &crs_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );

  strcpy( att_name, "long_name" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "EASE2_N3.125km", att_str, att_name );

  strcpy( att_name, "grid_mapping_name" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "lambert_azimuthal_equal_area", att_str, att_name );

  strcpy( att_name, "longitude_of_projection_origin" );
  expected_double = 0.0;
  status = nc_get_att_double( nc_fileid, crs_id, att_name, &att_double );
	  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_double, att_double, att_name );
  
  strcpy( att_name, "latitude_of_projection_origin" );
  expected_double = 90.0;
  status = nc_get_att_double( nc_fileid, crs_id, att_name, &att_double );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_double, att_double, att_name );
  
  strcpy( att_name, "false_easting" );
  expected_double = 0.0;
  status = nc_get_att_double( nc_fileid, crs_id, att_name, &att_double );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_double, att_double, att_name );

  strcpy( att_name, "proj4text" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "+proj=laea +lat_0=90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m",
				    att_str, att_name );

  strcpy( att_name, "srid" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "urn:ogc:def:crs:EPSG::6931", att_str, att_name );

  nc_close( nc_fileid );
  
}

void test_cetb_S_parameters( void ) {

  int nc_fileid=0;
  int crs_id;
  size_t att_len;
  nc_type att_type;
  char att_name[ 256 ];
  char att_str[ 256 ];
  double att_double;
  double expected_double;
  
  region_number = cetb_region_number[ 0 ][ CETB_EASE2_S ];
  factor = 2;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_S6.25km_F13_SSMI_M_19H_19910101_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename, cetb->filename );
  
  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_open" );
  cetb_file_close( cetb );

  /* Confirm the expected values are in the output file */
  status = nc_open( filename, NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_inq_varid( nc_fileid, "crs", &crs_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );

  strcpy( att_name, "long_name" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "EASE2_S6.25km", att_str, att_name );

  strcpy( att_name, "grid_mapping_name" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "lambert_azimuthal_equal_area", att_str, att_name );

  strcpy( att_name, "longitude_of_projection_origin" );
  expected_double = 0.0;
  status = nc_get_att_double( nc_fileid, crs_id, att_name, &att_double );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_double, att_double, att_name );
  
  strcpy( att_name, "latitude_of_projection_origin" );
  expected_double = -90.0;
  status = nc_get_att_double( nc_fileid, crs_id, att_name, &att_double );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_double, att_double, att_name );
  
  strcpy( att_name, "false_easting" );
  expected_double = 0.0;
  status = nc_get_att_double( nc_fileid, crs_id, att_name, &att_double );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_double, att_double, att_name );

  strcpy( att_name, "proj4text" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "+proj=laea +lat_0=-90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m",
				    att_str, att_name );

  strcpy( att_name, "srid" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "urn:ogc:def:crs:EPSG::6932", att_str, att_name );

  nc_close( nc_fileid );
  
}

void test_cetb_T_parameters( void ) {

  int nc_fileid=0;
  int crs_id;
  size_t att_len;
  nc_type att_type;
  char att_name[ 256 ];
  char att_str[ 256 ];
  double att_double;
  double expected_double;

  direction_id = CETB_ASC_PASSES;
  region_number = cetb_region_number[ 0 ][ CETB_EASE2_T ];
  factor = 1;

  cetb = cetb_file_init( dirname,
			 region_number, base_resolution,
			 factor, platform_id, sensor_id, year, doy, beam_id,
			 direction_id, reconstruction_id, producer_id, "test" );
  TEST_ASSERT_NOT_NULL( cetb );
  sprintf( filename, "%s/NSIDC0630_SIR_EASE2_T12.5km_F13_SSMI_A_19H_19910101_v%.1f.nc",
	   dirname, CETB_VERSION_ID );
  TEST_ASSERT_EQUAL_STRING( filename, cetb->filename );
  
  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_open" );
  cetb_file_close( cetb );

  /* Confirm the expected values are in the output file */
  status = nc_open( filename, NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_inq_varid( nc_fileid, "crs", &crs_id );
  TEST_ASSERT_EQUAL_INT_MESSAGE( NC_NOERR, status, nc_strerror( status ) );

  strcpy( att_name, "long_name" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "EASE2_T12.5km", att_str, att_name );

  strcpy( att_name, "grid_mapping_name" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "lambert_cylindrical_equal_area", att_str, att_name );

  strcpy( att_name, "longitude_of_central_meridian" );
  expected_double = 0.0;
  status = nc_get_att_double( nc_fileid, crs_id, att_name, &att_double );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_double, att_double, att_name );
  
  strcpy( att_name, "standard_parallel" );
  expected_double = 30.0;
  status = nc_get_att_double( nc_fileid, crs_id, att_name, &att_double );
  TEST_ASSERT_EQUAL_DOUBLE_MESSAGE( expected_double, att_double, att_name );

  strcpy( att_name, "proj4text" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "+proj=cea +lat_0=0 +lon_0=0 +lat_ts=30 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m",
				    att_str, att_name );

  strcpy( att_name, "srid" );
  status = nc_inq_attlen( nc_fileid, crs_id, att_name, &att_len );
  status = nc_get_att_text( nc_fileid, crs_id, att_name,  att_str );
  att_str[ att_len ] = '\0';
  TEST_ASSERT_EQUAL_STRING_MESSAGE( "urn:ogc:def:crs:EPSG::6933", att_str, att_name );

  nc_close( nc_fileid );
  
}

    
