#include <string.h>
#include "unity.h"
#include "gsx.h"


char *file_name;

void setUp(void)
{
  file_name = strdup( "/home/vagrant/testing/GSX_SHORT.nc" );
}

void tearDown(void)
{
}

void test_gsx_version(void)
{
  int status;

  status = gsx_version( );
  TEST_ASSERT_TRUE( status );
}

void test_gsx_init( void )
{
  gsx_class *gsx;

  gsx = gsx_init( "bogus_file_name" );
  TEST_ASSERT_FALSE( gsx );
}

void test_gsx_valid_file ( void )
{
  gsx_class *gsx;

  gsx = gsx_init( file_name );
  TEST_ASSERT_TRUE( gsx );
  fprintf( stderr, "%s: netcdf file id %d \n", __FUNCTION__, gsx->fileid );
  fprintf( stderr, "%s: netcdf dimensions %d\n", __FUNCTION__, gsx->dims );
  gsx_close( gsx );
}

void test_gsx_literal_file ( void )
{
  gsx_class *gsx;

  gsx = gsx_init( "/home/vagrant/measures-byu/python/test_cetb_utilities_data/bgi_data/test_bgi.nc" );
  TEST_ASSERT_TRUE( gsx );
  fprintf( stderr, "%s: netcdf file id %d \n", __FUNCTION__, gsx->fileid );
  gsx_close( gsx );
}

void test_gsx_ndims ( void )
{
  gsx_class *gsx;
  int status;

  gsx = gsx_init ( file_name );
  fprintf( stderr, "%s: netcdf file %s with id %d :\n\t: ndims=%d\n\t: nvars=%d\n\t: natts=%d\n\t: nunlim=%d\n",\
	   __FUNCTION__, file_name, gsx->fileid, gsx->dims, gsx->vars, gsx->atts, gsx->unlimdims );
  TEST_ASSERT_TRUE( 5 == gsx->dims );
  gsx_close( gsx );
}

void test_gsx_dim_names ( void )
{
  gsx_class *gsx;
  int status;

  gsx = gsx_init ( file_name );
  if ( NULL != gsx ){
    status = gsx_inq_dims( gsx );
  } else {
    status = 1;
  }
  fprintf( stderr, "%s: netcdf file %s with id %d has returned %d dimension names\n",\
	   __FUNCTION__, file_name, gsx->fileid, gsx->dims );
  TEST_ASSERT_TRUE( 0 == status );
}

void test_gsx_dim_scans_loc1 ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    status = gsx_inq_dims( gsx );
  } else {
    status = 1;
  }
  fprintf( stderr, "%s: netcdf file %s with id %d has %d lo res scans\n",\
	   __FUNCTION__, file_name, gsx->fileid, gsx->scans_loc1 );
  TEST_ASSERT_TRUE( gsx->scans_loc1 == 1605 );
}

void test_gsx_dim_scans_loc2 ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "%s: netcdf file %s with id %d has %d hi/loc2 res scans\n", \
	     __FUNCTION__, file_name, gsx->fileid, gsx->scans_loc2 );
  } else {
    status = 1;
  }
  TEST_ASSERT_TRUE( gsx->scans_loc2 == 3210 );
  gsx_close( gsx );
}

void test_gsx_dim_measurements_loc1 ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "%s: netcdf file %s with id %d has %d loc1 res measurements per scan\n", \
	     __FUNCTION__, file_name, gsx->fileid, gsx->measurements_loc1 );
  } else {
    status = 1;
  }
  TEST_ASSERT_TRUE( gsx->measurements_loc1 == 64 );
  gsx_close( gsx );
}
  
void test_gsx_dim_measurements_loc2 ( void ) {
  gsx_class *gsx;
  int status;

  gsx = gsx_init( file_name );
  if ( NULL != gsx ){
    fprintf( stderr, "%s: netcdf file %s with id %d has %d loc2 res measurements per scans\n", \
	   __FUNCTION__, file_name, gsx->fileid, gsx->measurements_loc2 );
  } else {
    status = 1;
  }
  TEST_ASSERT_TRUE( gsx->measurements_loc2 == 128 );
}
