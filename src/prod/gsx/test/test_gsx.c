#include <string.h>
#include "unity.h"
#include "cetb.h"
#include "gsx.h"


char *file_name;

void setUp(void)
{
  file_name = strdup( "/projects/PMESDR/vagrant/mhardman/GSX_CSU_SSMI_FCDR_V01R00_F13_D19970302_S0351_E0533_R10006.nc" );
  //file_name = strdup( "/projects/PMESDR/vagrant/mhardman/GSX_AMSR_E_L2A_BrightnessTemperatures_V12_200308080121_D.nc" );
}

void tearDown(void)
{
}

void test_gsx_init( void )
{
  gsx_class *gsx;

  fprintf( stderr, "\n%s: bogus file test\n", __FUNCTION__ );
  gsx = gsx_init( "bogus_file_name" );
  TEST_ASSERT_FALSE( gsx );
}

void test_gsx_literal_file ( void )
{
  gsx_class *gsx;

  gsx = gsx_init( "/home/vagrant/measures-byu/python/test_cetb_utilities_data/bgi_data/test_bgi.nc" );
  TEST_ASSERT_FALSE( gsx );
  fprintf( stderr, "\n%s: netcdf file 'test_bgi.nc' is not a gsx file \n", __FUNCTION__ );
  gsx_close( gsx );
}

void test_gsx_valid_file ( void )
{
  gsx_class *gsx;

  gsx = gsx_init( file_name );
  TEST_ASSERT_TRUE( gsx );
  fprintf( stderr, "\n%s: netcdf file %s returns id %d\n\n", __FUNCTION__, file_name, gsx->fileid );
  gsx_close( gsx );
}

void test_gsx_ndims ( void )
{
  gsx_class *gsx;

  gsx = gsx_init ( file_name );
  fflush( stderr );
  fprintf( stderr, "\n%s: netcdf file %s with id %d :\n\t: ndims=%d\n\t: nvars=%d\n\t: natts=%d\n\t: nunlim=%d\n\n", \
	   __FUNCTION__, file_name, gsx->fileid, gsx->dims, gsx->vars, gsx->atts, gsx->unlimdims );
  TEST_ASSERT_TRUE( 5 == gsx->dims );
  gsx_close( gsx );
}

void test_gsx_version(void)
{
  gsx_class *gsx;

  fprintf( stderr, "\n%s: gsx version test\n", __FUNCTION__ );
  gsx = gsx_init( file_name ); 
  TEST_ASSERT_TRUE( NULL != gsx->gsx_version );
  gsx_close( gsx );
}

void test_gsx_dim_names ( void )
{
  gsx_class *gsx;
  int status;

  gsx = gsx_init ( file_name );
  fprintf( stderr, "\n%s: netcdf file %s with id %d has returned %d dimension values\n",\
	   __FUNCTION__, file_name, gsx->fileid, gsx->dims );
  TEST_ASSERT_TRUE( 0 != gsx->measurements_loc1 );
  gsx_close( gsx );
}


void test_gsx_dim_scans_loc1 ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  fprintf( stderr, "\n%s: netcdf file %s with id %d has %d lo res scans\n",\
	   __FUNCTION__, file_name, gsx->fileid, gsx->scans_loc1 );
  switch (gsx->short_sensor) {
  case CETB_SSMI:
    TEST_ASSERT_TRUE( gsx->scans_loc1 > 1000 );
    break;
  case CETB_AMSRE:
    TEST_ASSERT_TRUE( gsx->scans_loc1 > 1000 );
    break;
  }
  gsx_close( gsx );
}

void test_gsx_dim_scans_loc2 ( void ) {
  gsx_class *gsx;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file %s with id %d has %d hi/loc2 res scans\n", \
	     __FUNCTION__, file_name, gsx->fileid, gsx->scans_loc2 );
  } 
  switch (gsx->short_sensor) {
  case CETB_SSMI:
    TEST_ASSERT_TRUE( gsx->scans_loc2 > 3000 );
    break;
  case CETB_AMSRE:
    TEST_ASSERT_TRUE( gsx->scans_loc2 > 1000 );
    break;
  }
  gsx_close( gsx );
}

void test_gsx_dim_scans_loc3 ( void ) {
  gsx_class *gsx;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file %s with id %d has %d hi/loc3 res scans\n", \
	     __FUNCTION__, file_name, gsx->fileid, gsx->scans_loc3 );
  }
  switch (gsx->short_sensor) {
  case CETB_SSMI:
    TEST_ASSERT_TRUE( gsx->scans_loc3 == 0 );
    break;
  case CETB_AMSRE:
    TEST_ASSERT_TRUE( gsx->scans_loc3 > 1000 );
    break;
  }
  gsx_close( gsx );
}
void test_gsx_dim_measurements_loc1 ( void ) {
  gsx_class *gsx;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file %s with id %d has %d loc1 res measurements per scan\n", \
	     __FUNCTION__, file_name, gsx->fileid, gsx->measurements_loc1 );
  }
  switch (gsx->short_sensor) {
  case CETB_SSMI:
    TEST_ASSERT_TRUE( gsx->measurements_loc1 == 64 );
    break;
  case CETB_AMSRE:
    TEST_ASSERT_TRUE( gsx->measurements_loc1 > 100 );
    break;
  }
  gsx_close( gsx );
}
  
void test_gsx_dim_measurements_loc3 ( void ) {
  gsx_class *gsx;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file %s with id %d has %d loc3 res measurements per scans\n", \
	   __FUNCTION__, file_name, gsx->fileid, gsx->measurements_loc3 );
  } 
  switch (gsx->short_sensor) {
  case CETB_SSMI:
    TEST_ASSERT_TRUE( gsx->measurements_loc3 == 0 );
    break;
  case CETB_AMSRE:
    TEST_ASSERT_TRUE( gsx->measurements_loc3 > 200 );
    break;
  }
  gsx_close( gsx );
}

void test_gsx_dim_measurements_loc2 ( void ) {
  gsx_class *gsx;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file %s with id %d has %d loc2 res measurements per scans\n", \
	   __FUNCTION__, file_name, gsx->fileid, gsx->measurements_loc2 );
  } 
  switch (gsx->short_sensor) {
  case CETB_SSMI:
    TEST_ASSERT_TRUE( gsx->measurements_loc2 == 128 );
    break;
  case CETB_AMSRE:
    TEST_ASSERT_TRUE( gsx->measurements_loc2 > 200 );
    break;
  }
  gsx_close( gsx );
}

void test_gsx_source_file ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file '%s' with id %d was derived from source file '%s'\n", \
	   __FUNCTION__, file_name, gsx->fileid, gsx->source_file );
  }
  TEST_ASSERT_TRUE( NULL != gsx->source_file );
  gsx_close( gsx );
}

void test_gsx_short_platform ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file '%s' with id %d is from '%s' platform\n", \
	   __FUNCTION__, gsx->source_file, gsx->fileid, cetb_platform_id_name[gsx->short_platform] );
  }
  TEST_ASSERT_TRUE( CETB_NO_PLATFORM != gsx->short_platform );
  gsx_close( gsx );
}

void test_gsx_short_sensor ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "%s: netcdf file '%s' with id %d has '%s' sensor\n",	\
	     __FUNCTION__, gsx->source_file, gsx->fileid, cetb_sensor_id_name[gsx->short_sensor] );
  }
  TEST_ASSERT_TRUE( CETB_NO_SENSOR != gsx->short_sensor );
  gsx_close( gsx );
}

void test_gsx_input_provider ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file '%s' with id %d was provided by '%s'\n",	\
	     __FUNCTION__, gsx->source_file, gsx->fileid, cetb_swath_producer_id_name[gsx->input_provider] );
  }
  TEST_ASSERT_TRUE( CETB_NO_PRODUCER != gsx->input_provider );
  gsx_close( gsx );
}

void test_gsx_channel_number ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file '%s' has %d channels\n",	\
	     __FUNCTION__, gsx->source_file, gsx->channel_number );
  }
  TEST_ASSERT_TRUE( 7 == gsx->channel_number );
  gsx_close( gsx );
}

void test_gsx_channel_names ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file '%s', %dth channel name is '%s'\n",	\
	     __FUNCTION__, gsx->source_file, gsx->channel_number, \
	     gsx->channel_names[gsx->channel_number-1] );
  }
  TEST_ASSERT_TRUE( NULL != gsx->channel_names[gsx->channel_number-1] );
  gsx_close( gsx );
}

void test_gsx_fill_value ( void ) {
  gsx_class *gsx;
  int status;
  int counter;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    for ( counter=0; counter<gsx->channel_number; counter++ ) {
      TEST_ASSERT_TRUE( -100.0 == gsx->fillvalue[counter] );
      fprintf( stderr, "%s: fill value for '%s' is %f\n", \
	       __FUNCTION__, gsx->channel_names[counter], gsx->fillvalue[counter] );
    }
  }
  gsx_close( gsx );
}

void test_gsx_temperature ( void ) {
  gsx_class *gsx;
  int counter;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "\n%s: netcdf file '%s' has these temps\n", __FUNCTION__, gsx->source_file ); 
    for ( counter=0; counter<gsx->channel_number; counter++ ) {
      fprintf( stderr, "\t on channel '%s', temperature data %f and %f\n", \
	       gsx->channel_names[counter], \
	       *(gsx->brightness_temps[counter]+1000),	\
	       *(gsx->brightness_temps[counter]+1001) );
      TEST_ASSERT_TRUE( NULL != gsx->brightness_temps[counter] );
    }
  }

  gsx_close( gsx );
}

void test_gsx_efov ( void ) {
  gsx_class *gsx;
  int status;
  int counter;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    for ( counter=0; counter<gsx->channel_number; counter++ ) {
      fprintf( stderr, "\n%s: efov for %s is %f by %f\n", \
	       __FUNCTION__, gsx->channel_names[counter], *gsx->efov[counter], *(gsx->efov[counter]+1) );
      TEST_ASSERT_TRUE( 0.0 != *gsx->efov[counter] );
      TEST_ASSERT_TRUE( 0.0 != *(gsx->efov[counter]+1) );
    }
  }
  gsx_close( gsx );
}

