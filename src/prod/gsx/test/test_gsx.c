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
  TEST_ASSERT_TRUE( 0 == status );
}

  
