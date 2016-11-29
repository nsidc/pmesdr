#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cetb.h"
#include "gsx.h"
#include "unity.h"

char *file_name;

char *get_pathname(const char *envvar, const char *filename) {
  char *dirname, *pathname;
  if (!(dirname = getenv(envvar))) {
    fprintf(stderr, "%s%s%s\n", "Environment variable '", envvar, "' is not set");
    exit(1);
  }
  pathname = malloc(strlen(dirname) + strlen(filename) + 2);
  sprintf(pathname, "%s/%s", dirname, filename);
  return pathname;
}

void setUp(void)
{
  const char *fn = "GSX_CSU_SSMIS_FCDR_V01R00_F16_D20061231_S0103_E0245_R16519.nc";
  file_name = get_pathname("PMESDR_TESTDATA_DIR", fn);
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

  char *fn = get_pathname("PMESDR_TOP_DIR", "python/test_cetb_utilities_data/bgi_data/test_bgi.nc");
  gsx = gsx_init(fn);
  TEST_ASSERT_FALSE( gsx );
  fprintf( stderr, "\n%s: netcdf file '%s' is not a gsx file \n", __FUNCTION__, fn );
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
  fprintf( stderr, "\n%s: netcdf file %s with id %d :\n\t: ndims=%d\n\t: nvars=%d\n\t: natts=%d\n\t: nunlim=%d\n\n",
	   __FUNCTION__, file_name, gsx->fileid, gsx->dims, gsx->vars, gsx->atts, gsx->unlimdims ); 
  TEST_ASSERT_TRUE( 7 == gsx->dims );
  
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

/* /\* void test_orbit_number(void) *\/ */
/* /\* { *\/ */
/* /\*   gsx_class *gsx; *\/ */

/* /\*   fprintf( stderr, "\n%s: gsx orbit number test\n", __FUNCTION__ ); *\/ */
/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   fprintf( stderr, "\t: orbit number %d\n", gsx->orbit ); *\/ */
/* /\*   TEST_ASSERT_TRUE( 0 != gsx->orbit ); *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_dim_names ( void ) *\/ */
/* /\* { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */

/* /\*   gsx = gsx_init ( file_name ); *\/ */
/* /\*   fprintf( stderr, "\n%s: netcdf file %s with id %d has returned %d dimension values\n",\ *\/ */
/* /\* 	   __FUNCTION__, file_name, gsx->fileid, gsx->dims ); *\/ */
/* /\*   TEST_ASSERT_TRUE( 0 != gsx->measurements[0] ); *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */


/* /\* void test_gsx_dim_scans_loc1 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int loc=0; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   fprintf( stderr, "\n%s: netcdf file %s with id %d has %d lo res scans\n",\ *\/ */
/* /\* 	   __FUNCTION__, file_name, gsx->fileid, gsx->scans[loc] ); *\/ */
/* /\*   switch (gsx->short_sensor) { *\/ */
/* /\*   case CETB_SSMI: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->scans[loc] > 1000 ); *\/ */
/* /\*     break; *\/ */
/* /\*   case CETB_AMSRE: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->scans[loc] > 1000 ); *\/ */
/* /\*     break; *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_dim_scans_loc2 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int loc=1; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file %s with id %d has %d hi/loc2 res scans\n", \ *\/ */
/* /\* 	     __FUNCTION__, file_name, gsx->fileid, gsx->scans[loc] ); *\/ */
/* /\*   }  *\/ */
/* /\*   switch (gsx->short_sensor) { *\/ */
/* /\*   case CETB_SSMI: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->scans[loc] > 3000 ); *\/ */
/* /\*     break; *\/ */
/* /\*   case CETB_AMSRE: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->scans[loc] > 1000 ); *\/ */
/* /\*     break; *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_dim_scans_loc3 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int loc=2; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file %s with id %d has %d hi/loc3 res scans\n", \ *\/ */
/* /\* 	     __FUNCTION__, file_name, gsx->fileid, gsx->scans[loc] ); *\/ */
/* /\*   } *\/ */
/* /\*   switch (gsx->short_sensor) { *\/ */
/* /\*   case CETB_SSMI: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->scans[loc] == 0 ); *\/ */
/* /\*     break; *\/ */
/* /\*   case CETB_AMSRE: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->scans[loc] > 1000 ); *\/ */
/* /\*     break; *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */
/* /\* void test_gsx_dim_measurements_loc1 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int loc=0; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file %s with id %d has %d loc1 res measurements per scan\n", \ *\/ */
/* /\* 	     __FUNCTION__, file_name, gsx->fileid, gsx->measurements[loc] ); *\/ */
/* /\*   } *\/ */
/* /\*   switch (gsx->short_sensor) { *\/ */
/* /\*   case CETB_SSMI: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->measurements[loc] == 64 ); *\/ */
/* /\*     break; *\/ */
/* /\*   case CETB_AMSRE: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->measurements[loc] > 100 ); *\/ */
/* /\*     break; *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */
  
/* /\* void test_gsx_dim_measurements_loc3 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int loc=2; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file %s with id %d has %d loc3 res measurements per scans\n", \ *\/ */
/* /\* 	   __FUNCTION__, file_name, gsx->fileid, gsx->measurements[loc] ); *\/ */
/* /\*   }  *\/ */
/* /\*   switch (gsx->short_sensor) { *\/ */
/* /\*   case CETB_SSMI: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->measurements[loc] == 0 ); *\/ */
/* /\*     break; *\/ */
/* /\*   case CETB_AMSRE: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->measurements[loc] > 200 ); *\/ */
/* /\*     break; *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_dim_measurements_loc2 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int loc=1; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file %s with id %d has %d loc2 res measurements per scans\n", \ *\/ */
/* /\* 	   __FUNCTION__, file_name, gsx->fileid, gsx->measurements[loc] ); *\/ */
/* /\*   }  *\/ */
/* /\*   switch (gsx->short_sensor) { *\/ */
/* /\*   case CETB_SSMI: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->measurements[loc] == 128 ); *\/ */
/* /\*     break; *\/ */
/* /\*   case CETB_AMSRE: *\/ */
/* /\*     TEST_ASSERT_TRUE( gsx->measurements[loc] > 200 ); *\/ */
/* /\*     break; *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_source_file ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file '%s' with id %d was derived from source file '%s'\n", \ *\/ */
/* /\* 	   __FUNCTION__, file_name, gsx->fileid, gsx->source_file ); *\/ */
/* /\*   } *\/ */
/* /\*   TEST_ASSERT_TRUE( NULL != gsx->source_file ); *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_short_platform ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file '%s' with id %d is from '%s' platform\n", \ *\/ */
/* /\* 	   __FUNCTION__, gsx->source_file, gsx->fileid, cetb_platform_id_name[gsx->short_platform] ); *\/ */
/* /\*   } *\/ */
/* /\*   TEST_ASSERT_TRUE( CETB_NO_PLATFORM != gsx->short_platform ); *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_short_sensor ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "%s: netcdf file '%s' with id %d has '%s' sensor\n",	\ *\/ */
/* /\* 	     __FUNCTION__, gsx->source_file, gsx->fileid, cetb_sensor_id_name[gsx->short_sensor] ); *\/ */
/* /\*   } *\/ */
/* /\*   TEST_ASSERT_TRUE( CETB_NO_SENSOR != gsx->short_sensor ); *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_input_provider ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file '%s' with id %d was provided by '%s'\n",	\ *\/ */
/* /\* 	     __FUNCTION__, gsx->source_file, gsx->fileid, cetb_swath_producer_id_name[gsx->input_provider] ); *\/ */
/* /\*   } *\/ */
/* /\*   TEST_ASSERT_TRUE( CETB_NO_PRODUCER != gsx->input_provider ); *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_channel_number ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file '%s' has %d channels\n",	\ *\/ */
/* /\* 	     __FUNCTION__, gsx->source_file, gsx->channel_number ); *\/ */
/* /\*   } *\/ */
/* /\*   TEST_ASSERT_TRUE( 14 == gsx->channel_number ); *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_channel_names ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file '%s', %dth channel name is '%s'\n",	\ *\/ */
/* /\* 	     __FUNCTION__, gsx->source_file, gsx->channel_number, \ *\/ */
/* /\* 	     gsx->channel_names[gsx->channel_number-1] ); *\/ */
/* /\*   } *\/ */
/* /\*   TEST_ASSERT_TRUE( NULL != gsx->channel_names[gsx->channel_number-1] ); *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_fill_value ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */
/* /\*   int counter; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     for ( counter=0; counter<gsx->channel_number; counter++ ) { *\/ */
/* /\*       TEST_ASSERT_TRUE( -100.0 == gsx->fillvalue[counter] ); *\/ */
/* /\*       fprintf( stderr, "%s: fill value for '%s' is %f\n", \ *\/ */
/* /\* 	       __FUNCTION__, gsx->channel_names[counter], gsx->fillvalue[counter] ); *\/ */
/* /\*     } *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_temperature ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int counter; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file '%s' has these temps\n", __FUNCTION__, gsx->source_file );  *\/ */
/* /\*     for ( counter=0; counter<gsx->channel_number; counter++ ) { *\/ */
/* /\*       fprintf( stderr, "\t on channel '%s', temperature data %f and %f\n", \ *\/ */
/* /\* 	       gsx->channel_names[counter], \ *\/ */
/* /\* 	       *(gsx->brightness_temps[counter]+1000),	\ *\/ */
/* /\* 	       *(gsx->brightness_temps[counter]+1001) ); *\/ */
/* /\*       TEST_ASSERT_TRUE( NULL != gsx->brightness_temps[counter] ); *\/ */
/* /\*     } *\/ */
/* /\*   } *\/ */

/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_efov ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */
/* /\*   int counter; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     for ( counter=0; counter<gsx->channel_number; counter++ ) { *\/ */
/* /\*       fprintf( stderr, "\n%s: efov for %s is %f by %f\n", \ *\/ */
/* /\* 	       __FUNCTION__, gsx->channel_names[counter], *gsx->efov[counter], *(gsx->efov[counter]+1) ); *\/ */
/* /\*       TEST_ASSERT_TRUE( 0.0 != *gsx->efov[counter] ); *\/ */
/* /\*       TEST_ASSERT_TRUE( 0.0 != *(gsx->efov[counter]+1) ); *\/ */
/* /\*     } *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_validRange ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int status; *\/ */
/* /\*   int counter; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ){ *\/ */
/* /\*     for ( counter=0; counter<gsx->channel_number; counter++ ) { *\/ */
/* /\*       if ( NULL != gsx->validRange[counter] ) fprintf( stderr, "\n%s: valid range for %s is %f by %f\n",	\ *\/ */
/* /\* 						       __FUNCTION__, gsx->channel_names[counter], *\/ */
/* /\* 						       *gsx->validRange[counter], *(gsx->validRange[counter]+1) ); *\/ */
/* /\*       TEST_ASSERT_TRUE( NULL != gsx->validRange[counter] ); *\/ */
/* /\*       TEST_ASSERT_TRUE( NULL != (gsx->validRange[counter]+1) ); *\/ */
/* /\*     } *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_fill_values_loc1 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int i=0; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ) { *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_latitude[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_longitude[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_eia[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_eaz[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -1.e+30 == gsx->fill_scantime[i] ); *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_fill_values_loc2 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int i=1; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( ( NULL != gsx ) && ( gsx->scans[i] != 0 ) ) { *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_latitude[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_longitude[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_eia[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_eaz[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -1.e+30 == gsx->fill_scantime[i] ); *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_fill_values_loc3 ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int i=2; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( ( NULL != gsx ) && ( gsx->scans[i] != 0 ) ) { *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_latitude[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_longitude[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_eia[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -9999.0 == gsx->fill_eaz[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( -1.e+30 == gsx->fill_scantime[i] ); *\/ */
/* /\*   } *\/ */
/* /\*   gsx_close( gsx ); *\/ */
/* /\* } *\/ */

/* /\* void test_gsx_loc1_variables ( void ) { *\/ */
/* /\*   gsx_class *gsx; *\/ */
/* /\*   int i=0; *\/ */

/* /\*   gsx = gsx_init( file_name ); *\/ */
/* /\*   if ( NULL != gsx ) { *\/ */
/* /\*     fprintf( stderr, "\n%s: netcdf file '%s' has these loc1 variables\n", __FUNCTION__, gsx->source_file );  *\/ */
/* /\*     fprintf( stderr, "\t on latitude_loc1, longitude_loc1 eia_loc1, eaz_loc1, scantime" ); *\/ */
/* /\*     if ( CETB_AQUA != gsx->short_sensor ) *\/ */
/* /\*       fprintf( stderr, ", sc_lat_loc1, sc_lon_loc1" ); *\/ */
/* /\*     fprintf( stderr, "\n"); *\/ */
/* /\*     fprintf( stderr, "\t %f \t %f \t %f \t %f \t %f \t ",	\ *\/ */
/* /\* 	     *(gsx->latitude[i]+1000), \ *\/ */
/* /\* 	     *(gsx->longitude[i]+1000), \ *\/ */
/* /\* 	     *(gsx->eia[i]+1000),	\ *\/ */
/* /\* 	     *(gsx->eaz[i]+1000), \ *\/ */
/* /\* 	     *(gsx->scantime[i]+(1000/gsx->measurements[i])) ); *\/ */
/* /\*     if ( CETB_AQUA != gsx->short_sensor ) *\/ */
/* /\*       fprintf( stderr, "%f \t %f",  \ *\/ */
/* /\* 	       *(gsx->sc_latitude[i]+(1000/gsx->measurements[i])), \ *\/ */
/* /\* 	       *(gsx->sc_longitude[i]+(1000/gsx->measurements[i])) ); *\/ */
/* /\*     fprintf( stderr, "\n" ); *\/ */

/* /\*     fprintf( stderr, "\t %f \t %f \t %f \t %f \t %f \t ", \ *\/ */
/* /\* 	     *(gsx->latitude[i]+1001), \ *\/ */
/* /\* 	     *(gsx->longitude[i]+1001), \ *\/ */
/* /\* 	     *(gsx->eia[i]+1001),	\ *\/ */
/* /\* 	     *(gsx->eaz[i]+1001),  \ *\/ */
/* /\* 	     *(gsx->scantime[i]+(1001/gsx->measurements[i])) ); *\/ */
/* /\*     if ( CETB_AQUA != gsx->short_sensor ) *\/ */
/* /\*       fprintf( stderr, "%f \t %f",  \ *\/ */
/* /\* 	       *(gsx->sc_latitude[i]+(1001/gsx->measurements[i])), \ *\/ */
/* /\* 	       *(gsx->sc_longitude[i]+(1001/gsx->measurements[i])) ); *\/ */
/* /\*     fprintf( stderr, "\n" ); *\/ */


/* /\*     TEST_ASSERT_TRUE( NULL != gsx->latitude[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( NULL != gsx->longitude[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( NULL != gsx->eia[i] ); *\/ */
/* /\*     TEST_ASSERT_TRUE( NULL != gsx->eaz[i] ); *\/ */
/* /\*     if ( CETB_AQUA != gsx->short_sensor ) { *\/ */
/* /\*       TEST_ASSERT_TRUE( NULL != gsx->sc_latitude[i] ); *\/ */
/* /\*       TEST_ASSERT_TRUE( NULL != gsx->sc_longitude[i] ); *\/ */
/* /\*     } *\/ */
/* /\*     TEST_ASSERT_TRUE( NULL != gsx->scantime[i] ); *\/ */
/* /\*   } *\/ */

/* /\*   gsx_close( gsx ); */
/* } */

/* void test_gsx_loc2_variables ( void ) { */
/*   gsx_class *gsx; */
/*   int i=1; */

/*   gsx = gsx_init( file_name ); */
/*   if ( NULL != gsx ) { */
/*     fprintf( stderr, "\n%s: netcdf file '%s' has these loc2 variables\n", __FUNCTION__, gsx->source_file );  */
/*     fprintf( stderr, "\t on latitude_loc2, longitude_loc2 eia_loc2, eaz_loc2\n" ); */
/*     fprintf( stderr, "\t %f \t %f \t %f \t %f \n",	\ */
/* 	     *(gsx->latitude[i]+1000), \ */
/* 	     *(gsx->longitude[i]+1000), \ */
/* 	     *(gsx->eia[i]+1000),	\ */
/* 	     *(gsx->eaz[i]+1000) ); */
/*     fprintf( stderr, "\t %f \t %f \t %f \t %f \n", \ */
/* 	     *(gsx->latitude[i]+1001), \ */
/* 	     *(gsx->longitude[i]+1001), \ */
/* 	     *(gsx->eia[i]+1001),	\ */
/* 	     *(gsx->eaz[i]+1001) ); */

/*     TEST_ASSERT_TRUE( NULL != gsx->latitude[i] ); */
/*     TEST_ASSERT_TRUE( NULL != gsx->longitude[i] ); */
/*     TEST_ASSERT_TRUE( NULL != gsx->eia[i] ); */
/*     TEST_ASSERT_TRUE( NULL != gsx->eaz[i] ); */
/*   } */

/*   gsx_close( gsx ); */
/* } */

/* void test_gsx_loc3_variables ( void ) { */
/*   gsx_class *gsx; */
/*   int i=2; */

/*   gsx = gsx_init( file_name ); */
/*   if ( NULL != gsx ) { */
/*     if ( 0 != gsx->scans[i] ) { */
/*       fprintf( stderr, "\n%s: netcdf file '%s' has these loc3 variables\n", __FUNCTION__, gsx->source_file );  */
/*       fprintf( stderr, "\t on latitude_loc3, longitude_loc3 eia_loc3, eaz_loc3\n" ); */
/*       fprintf( stderr, "\t %f \t %f \t %f \t %f \n",	\ */
/* 	       *(gsx->latitude[i]+1000), \ */
/* 	       *(gsx->longitude[i]+1000), \ */
/* 	       *(gsx->eia[i]+1000),	\ */
/* 	       *(gsx->eaz[i]+1000) ); */
/*       fprintf( stderr, "\t %f \t %f \t %f \t %f \n", \ */
/* 	       *(gsx->latitude[i]+1001), \ */
/* 	       *(gsx->longitude[i]+1001), \ */
/* 	       *(gsx->eia[i]+1001),	\ */
/* 	       *(gsx->eaz[i]+1001) ); */

/*       TEST_ASSERT_TRUE( NULL != gsx->latitude[i] ); */
/*       TEST_ASSERT_TRUE( NULL != gsx->longitude[i] ); */
/*       TEST_ASSERT_TRUE( NULL != gsx->eia[i] ); */
/*       TEST_ASSERT_TRUE( NULL != gsx->eaz[i] ); */
/*     } else { */
/*       fprintf( stderr, "%s: There are no loc3 variables in this file %s\n", \ */
/* 	       __FUNCTION__, gsx->source_file ); */
/*     } */
/*   } */

/*   gsx_close( gsx ); */
/* } */

/* void test_gsx_orbit_direction ( void ) { */
/*   gsx_class *gsx; */
/*   int status; */

/*   gsx = gsx_init( file_name ); */
/*   if ( NULL != gsx ){ */
/*     if ( CETB_AQUA == gsx->short_sensor ) { */
/*       TEST_ASSERT_TRUE( CETB_DES_PASSES == gsx->pass_direction ); */
/*     } else { */
/*       TEST_ASSERT_TRUE( CETB_NO_DIRECTION == gsx->pass_direction ); */
/*     } */
/*   } */
/*   gsx_close( gsx ); */
/* } */

/* void test_gsx_channel_coordinates ( void ) { */
/*   gsx_class *gsx; */
/*   int status; */
/*   int counter; */

/*   gsx = gsx_init( file_name ); */
/*   if ( NULL != gsx ) { */
/*     for ( counter=0; counter<GSX_MAX_CHANNELS; counter++ ) { */
/*       fprintf( stderr, "%s: channel %s has locations %s and %d\n", __FUNCTION__, \ */
/* 	       gsx->channel_names[counter], cetb_loc_id_name[gsx->channel_dims[counter]], \ */
/* 	       gsx->channel_dims[counter] ); */
/*       //TEST_ASSERT_TRUE( 0 == gsx->channel_dims[counter] ); */
/*     } */
/*   } */
/*   gsx_close( gsx ); */
/* } */
