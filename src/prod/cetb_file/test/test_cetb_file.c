#include "unity.h"
#include "cetb_file.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_bogus(void)
{
  TEST_IGNORE_MESSAGE("Implement me!");
}

void test_cetb_filename(void)
{
  char filename[ FILENAME_MAX ] = "";
  char dirname[ FILENAME_MAX ] = "/path_to_file";
  int region = 308;

  cetb_filename( filename, FILENAME_MAX - 1, dirname, region );
  TEST_ASSERT_EQUAL_STRING( "/path_to_file/308TEST_FILE.nc", filename );
  
}
