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
#include "utCalendar2_cal.h"


/*
 * global variables used in multiple tests
  */

void setUp( void ) {
  /*
   * Default values for globals
   * Tests only need to change the specific thing they're trying to find
   */
}

void tearDown( void ) {
}

void test_cetb_epoch_conversion( void ) {

  char* calendar = "Standard";
  ut_system* unitSystem = ut_read_xml( NULL );
  char *unitString;
  
  int year, outYear;
  int month, outMonth;
  int day, outDay;
  int hour, outHour;
  int minute, outMinute;
  double second, outSecond;
  double value;
  int status;

  unitString = strdup("days since 1901-01-01 00:00");
  ut_unit* binUnits = ut_parse( unitSystem, unitString, UT_ISO_8859_1 );
  if (NULL == binUnits) {
    fprintf( stderr, "%s: unable to parse unit string\n", __FUNCTION__ );
  }

  year = 1901;
  month = 1;
  day = 3;
  hour = 12;
  minute = 0;
  second = 0.0;
  fprintf( stderr, "%s: Test on: %4d-%02d-%02d %02d:%02d:%lf\n",
	   __FUNCTION__, year, month, day, hour, minute, second );

  /* yyyy/mm/dd hh:mm:ss to epoch time */
  status = utInvCalendar2_cal( year, month, day,
			       hour, minute, second,
			       binUnits, &value, calendar);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to convert Gregorian to epoch\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: Greg2Epoch: epochTime=%lf %s\n",
	     __FUNCTION__, value, unitString );
  }

  /* epoch time back to Gregorian */
  status = utCalendar2_cal( value, binUnits,
			    &outYear, &outMonth, &outDay,
			    &outHour, &outMinute, &outSecond,
			    calendar);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to convert epochTime to Gregorian\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: Epoch2Greg: %4d-%02d-%02d %02d:%02d:%lf\n",
	     __FUNCTION__, outYear, outMonth, outDay,
	     outHour, outMinute, outSecond );
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( year, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( month, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( day, outDay, "Day" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( hour, outHour, "Hour" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( minute, outMinute, "Minute" );
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE( second, outSecond, "Second" );
  
  /* Test value from GSX file */
  free(unitString);
  unitString = strdup("seconds since 1987-01-01 00:00:00");
  binUnits = ut_parse( unitSystem, unitString, UT_ISO_8859_1 );
  if (NULL == binUnits) {
    fprintf( stderr, "%s: unable to parse unit string\n", __FUNCTION__ );
  }
  value = 599612803.264000;
  status = utCalendar2_cal( value, binUnits,
  			    &outYear, &outMonth, &outDay,
  			    &outHour, &outMinute, &outSecond,
  			    calendar);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to convert epochTime to Gregorian\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: epochTime=%lf %s\n",
	     __FUNCTION__, value, unitString );
    fprintf( stderr, "%s: Epoch2Greg: %4d-%02d-%02d %02d:%02d:%lf\n",
  	     __FUNCTION__, outYear, outMonth, outDay,
  	     outHour, outMinute, outSecond );
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( 2005, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 12, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 31, outDay, "Day" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 23, outHour, "Hour" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 6, outMinute, "Minute" );
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE( 43.264, outSecond, "Second" );
  
  value = 599619534.175000;
  status = utCalendar2_cal( value, binUnits,
  			    &outYear, &outMonth, &outDay,
  			    &outHour, &outMinute, &outSecond,
  			    calendar);
  if ( 0 != status ) {
    fprintf( stderr, "%s: unable to convert epochTime to Gregorian\n", __FUNCTION__ );
  } else {
    fprintf( stderr, "%s: epochTime=%lf %s\n",
	     __FUNCTION__, value, unitString );
    fprintf( stderr, "%s: Epoch2Greg: %4d-%02d-%02d %02d:%02d:%lf\n",
  	     __FUNCTION__, outYear, outMonth, outDay,
  	     outHour, outMinute, outSecond );
  }
  TEST_ASSERT_EQUAL_INT_MESSAGE( 2006, outYear, "Year" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 1, outMonth, "Month" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 1, outDay, "Day" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 0, outHour, "Hour" );
  TEST_ASSERT_EQUAL_INT_MESSAGE( 58, outMinute, "Minute" );
  TEST_ASSERT_EQUAL_FLOAT_MESSAGE( 54.175, outSecond, "Second" );

  return;
  
}

