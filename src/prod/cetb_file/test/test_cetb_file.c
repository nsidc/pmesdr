#include "unity.h"
#include "cetb_file.h"

/*
 * global variables used in multiple tests
 * (Not sure if I should initialize the strings in setUp)
 */
int status;
char filename[ FILENAME_MAX ] = "";
char dirname[ FILENAME_MAX ] = "/path_to_file";;

void setUp(void)
{
  status = 0;
}

void tearDown(void)
{
}

void test_bogus_region_id(void)
{
  cetb_region_id region_id = 311;
  int factor = 1;
  cetb_platform_id platform_id = CETB_F08;
  cetb_sensor_id sensor_id = CETB_SSMI;
  int year = 1991;
  int doy = 1;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_factor(void)
{
  cetb_region_id region_id = 308;
  int factor = 5;
  cetb_platform_id platform_id = CETB_F08;
  cetb_sensor_id sensor_id = CETB_SSMI;
  int year = 1991;
  int doy = 1;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_platform(void)
{
  cetb_region_id region_id = 308;
  int factor = 2;
  cetb_platform_id platform_id = CETB_NO_PLATFORM;
  cetb_sensor_id sensor_id = CETB_SSMI;
  int year = 1991;
  int doy = 1;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_sensor(void)
{
  cetb_region_id region_id = 308;
  int factor = 2;
  cetb_platform_id platform_id = CETB_F08;
  cetb_sensor_id sensor_id = CETB_NO_SENSOR;
  int year = 1991;
  int doy = 1;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_year(void)
{
  cetb_region_id region_id = 308;
  int factor = 2;
  cetb_platform_id platform_id = CETB_F08;
  cetb_sensor_id sensor_id = CETB_SSMI;
  int year = 1970;
  int doy = 1;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_FALSE( status );
}

void test_bogus_doy(void)
{
  cetb_region_id region_id = 308;
  int factor = 2;
  cetb_platform_id platform_id = CETB_F08;
  cetb_sensor_id sensor_id = CETB_SSMI;
  int year = 1987;
  int doy = 366;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_FALSE( status );
}

void test_cetb_north_filename(void)
{
  cetb_region_id region_id = 308;
  int factor = 0;
  cetb_platform_id platform_id = CETB_F13;
  cetb_sensor_id sensor_id = CETB_SSMI;
  int year = 1991;
  int doy = 1;
  
  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_TRUE( status );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_N25km.F13_SSMI.1991001.TEST_FILE.nc", filename );
  
}

void test_cetb_south_filename(void)
{
  cetb_region_id region_id = 309;
  int factor = 1;
  cetb_platform_id platform_id = CETB_AQUA;
  cetb_sensor_id sensor_id = CETB_AMSRE;
  int year = 1991;
  int doy = 31;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_TRUE( status );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_S12.5km.AQUA_AMSRE.1991031.TEST_FILE.nc", filename );
  
}

void test_cetb_temperate_filename(void)
{
  cetb_region_id region_id = 310;
  int factor = 2;
  cetb_platform_id platform_id = CETB_F13;
  cetb_sensor_id sensor_id = CETB_SSMI;
  int year = 1991;
  int doy = 365;

  status = cetb_filename( filename, FILENAME_MAX - 1, dirname, region_id, factor, platform_id, sensor_id, year, doy );
  TEST_ASSERT_TRUE( status );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/EASE2_T6.25km.F13_SSMI.1991365.TEST_FILE.nc", filename );
  
}
