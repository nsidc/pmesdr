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
  region_number = cetb_region_number[ CETB_EASE2_N ];
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
  status = cetb_file_open( cetb );
  TEST_ASSERT_TRUE_MESSAGE( 0 == status, "cetb_file_open" );
  
}

void tearDown( void ) {

}

void test_cetb_dimensions( void ) {

  int nc_fileid=0;
  int dim_id;

  status = cetb_file_write_dimensions( cetb );
  cetb_file_close( cetb );

  /* Confirm the expected dimensions are in the output file */
  status = nc_open( "./test/EASE2_N25km.F13_SSMI.1991001.19H.M.SIR.CSU.v0.1.nc",
		    NC_NOWRITE, &nc_fileid );
  TEST_ASSERT_TRUE( NC_NOERR == status );

  status = nc_inq_dimid( nc_fileid, "rows", &dim_id );
  TEST_ASSERT_TRUE_MESSAGE( NC_NOERR == status, nc_strerr( status ) );

  nc_close( nc_fileid );
  
}


    
