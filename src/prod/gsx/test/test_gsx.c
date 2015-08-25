#include <string.h>
#include "unity.h"
#include "gsx.h"


char file_name[FILENAME_MAX];

void setUp(void)
{
  strcpy( file_name, "/home/vagrant/measures-byu/python/test_cetb_utilities_data/bgi_data/test_bgi.nc" );
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

  //  gsx = gsx_init( file_name );
  gsx = gsx_init( "/projects/PMESDR/vagrant/savoie/rss-2003/GSX_RSS_SSMI_FCDR_V07R00_F13_D20031231_S2309_E2343_R45267.nc" );
  TEST_ASSERT_TRUE( gsx );
  fprintf( stderr, "%s: netcdf file id %d \n", __FUNCTION__, *(gsx->fileid) );
  //  gsx_close( gsx );
}

  
