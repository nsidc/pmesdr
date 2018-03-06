/*
 * test_cetb_calcalc - Unit tests for validating calcalc epoch conversions
 *
 * 11-Feb-2018 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2018 Regents of the University of Colorado and Brigham Young University
 */
#include <netcdf.h>
#include <stdlib.h>
#include <string.h>
#include <udunits2.h>

#include "unity.h"
#include "calcalcs.h"
#include "cetb_file.h"
#include "utCalendar2_cal.h"


/*
 * global variables used in multiple tests
 */
calcalcs_cal* calendar;
ut_system* unitSystem;
char *unitString;
double seconds_per_day;
int year, outYear;
int month, outMonth;
int day, outDay;
int status;

void setUp( void ) {
  /*
   * Default values for globals
   * Tests only need to change the specific thing they're trying to find
   */
  calendar = ccs_init_calendar( "Standard" );
  /*  ut_set_error_message_handler(ut_ignore); */
  unitSystem = ut_read_xml( NULL );
  /* ut_set_error_message_handler( NULL ); */
  seconds_per_day = 60. * 60. * 24.;
  
}

void tearDown( void ) {

  ut_free_system( unitSystem );
  ccs_free_calendar( calendar );
  
}

void test_date_arithmetic( void ) {

  unitString = strdup("seconds since 1987-01-01 00:00:00");
  ut_unit* binUnits = ut_parse( unitSystem, unitString, UT_ISO_8859_1 );
  if (NULL == binUnits) {
    fprintf( stderr, "%s: unable to parse unit string\n", __FUNCTION__ );
  }

  /* Test date arithmetic where units are seconds and we add/subtract days */
  year = 1997;
  month = 3;
  day = 2;
  fprintf( stderr, "%s: Test on: %4d-%02d-%02d\n",
	   __FUNCTION__, year, month, day );

  /* Find 1 day earlier */
  status = ccs_dayssince( calendar, year, month, day, -1,
			  calendar, &outYear, &outMonth, &outDay);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to find 1 day earlier\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: 1 day earlier: %4d-%02d-%02d\n",
	     __FUNCTION__, outYear, outMonth, outDay);
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( year, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( month, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( day - 1, outDay, "Day" );
  
  /* Find 2 days later */
  status = ccs_dayssince( calendar, year, month, day, 2,
			  calendar, &outYear, &outMonth, &outDay);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to find 2 days later\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: 2 days later: %4d-%02d-%02d\n",
	     __FUNCTION__, outYear, outMonth, outDay);
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( year, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( month, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( day + 2, outDay, "Day" );
  
  /* Check month crossover */
  year = 1997;
  month = 1;
  day = 31;
  fprintf( stderr, "%s: Test on: %4d-%02d-%02d\n",
	   __FUNCTION__, year, month, day );

  /* Find 1 day earlier */
  status = ccs_dayssince( calendar, year, month, day, -1,
			  calendar, &outYear, &outMonth, &outDay);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to find 1 day earlier\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: 1 day earlier: %4d-%02d-%02d\n",
	     __FUNCTION__, outYear, outMonth, outDay);
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( year, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( month, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( day - 1, outDay, "Day" );
  
  /* Find 2 days later */
  status = ccs_dayssince( calendar, year, month, day, 2,
			  calendar, &outYear, &outMonth, &outDay);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to find 2 days later\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: 2 days later: %4d-%02d-%02d\n",
	     __FUNCTION__, outYear, outMonth, outDay);
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( year, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( month + 1, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 2, outDay, "Day" );

  /* Check year crossover */
  year = 1996;
  month = 12;
  day = 31;
  fprintf( stderr, "%s: Test on: %4d-%02d-%02d\n",
	   __FUNCTION__, year, month, day );

  /* Find 1 day earlier */
  status = ccs_dayssince( calendar, year, month, day, -1,
			  calendar, &outYear, &outMonth, &outDay);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to find 1 day earlier\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: 1 day earlier: %4d-%02d-%02d\n",
	     __FUNCTION__, outYear, outMonth, outDay);
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( year, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( month, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( day - 1, outDay, "Day" );
  
  /* Find 2 days later */
  status = ccs_dayssince( calendar, year, month, day, 2,
			  calendar, &outYear, &outMonth, &outDay);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to find 2 days later\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: 2 days later: %4d-%02d-%02d\n",
	     __FUNCTION__, outYear, outMonth, outDay);
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( year + 1, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 1, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 2, outDay, "Day" );
  
  free( binUnits );
  return;
  
}

void test_new( void ) {

  fprintf( stderr, "%s: calname = %s\n", __FUNCTION__, calendar->name );

}
