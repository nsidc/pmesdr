#include "unity.h"
#include "utils.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_module_generator_needs_to_be_implemented(void) {

  int status;
  float *data;

  status = utils_allocate_clean_aligned_memory( ( void * )&data, sizeof( float ) * 2 );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, status, "unexpected failure from allocate_clean_aligned_memory" );
  
}
