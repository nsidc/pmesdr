#include <stdio.h>
#include <stdlib.h>

#include "calcalcs.h"

/*=======================================================================*/
void do_tests( const char *calendar_name, int year, int month, int day ) 
{
	calcalcs_cal	*cal;
	int		ierr, jday, leap, tyear, tmonth, tday, doy, days_since;

	/* Initialize the calendar */
	if( (cal = ccs_init_calendar( calendar_name )) == NULL ) {
		fprintf( stderr, "Error trying to initialize calendar %s\n",
			calendar_name );
		exit(-1);
		}

	/* Is the passed year a leap year? */
	if( (ierr = ccs_isleap( cal, year, &leap )) != 0 ) 
		printf( "Determining whether %d is a leap year in the %s calendar returned an error: %s\n",
			year, calendar_name, ccs_err_str(ierr) );
	else
		printf( "Year %d %s a leap year in the %s calendar\n",
			year, ((leap==1)?"IS":"is NOT"), calendar_name );

	/* Convert the passed Y/M/D date to a Julian day number */
	if( (ierr = ccs_date2jday( cal, year, month, day, &jday )) != 0 ) 
		printf( "Turning %04d-%02d-%02d into a Julian day returned an error: %s\n",
			year, month, day, ccs_err_str(ierr) );
	else
		printf( "%04d-%02d-%02d in the %s calendar is Julian day %d\n",
			year, month, day, calendar_name, jday );

	/* Convert the julian day we just calcalculated back into a date, it should match! */
	if( (ierr = ccs_jday2date( cal, jday, &tyear, &tmonth, &tday )) != 0 ) 
		printf( "Turning Julian day %d into a date returned an error: %s\n",
			jday, ccs_err_str(ierr) );
	else
		printf( "Julian day %d is date %04d-%02d-%02d in the %s calendar\n",
			jday, tyear, tmonth, tday, calendar_name );

	/* Get the day number of the year */
	if( (ierr = ccs_date2doy( cal, year, month, day, &doy )) != 0 ) 
		printf( "Turning %04d-%02d-%02d into a day-of-year returned an error: %s\n",
			year, month, day, ccs_err_str(ierr) );
	else
		printf( "%04d-%02d-%02d in the %s calendar is day-of-year number %d\n",
			year, month, day, calendar_name, doy );

	/* Convert the day-of-year we just calculated back into a date, it should match */
	if( (ierr = ccs_doy2date( cal, year, doy, &tmonth, &tday )) != 0 ) 
		printf( "Turning day %d of year %d into a date in the %s calendar returned an error: %s\n",
			doy, year, calendar_name, ccs_err_str(ierr) );
	else
		printf( "day-of-year %d of year %d is date %04d-%02d-%02d in the %s calendar\n",
			doy, year, year, tmonth, tday, calendar_name );

	/* Do a couple "days since" calls */
	days_since = 5;
	if( (ierr = ccs_dayssince( cal, year, month, day, days_since, cal, &tyear, &tmonth, &tday )) != 0 )
		printf( "Calculating %d days since  %04d-%02d-%02d in the %s calendar gave error %s\n",
			days_since, year, month, day, calendar_name, ccs_err_str(ierr));
	else
		printf( "%d days since  %04d-%02d-%02d in the %s calendar is %04d-%02d-%02d\n",
			days_since,  year, month, day, calendar_name, tyear, tmonth, tday );

	days_since = -5;
	if( (ierr = ccs_dayssince( cal, year, month, day, days_since, cal, &tyear, &tmonth, &tday )) != 0 )
		printf( "Calculating %d days since  %04d-%02d-%02d in the %s calendar gave error %s\n",
			days_since, year, month, day, calendar_name, ccs_err_str(ierr));
	else
		printf( "%d days since  %04d-%02d-%02d in the %s calendar is %04d-%02d-%02d\n",
			days_since,  year, month, day, calendar_name, tyear, tmonth, tday );
		
		
	/* Free the calendar */
	ccs_free_calendar( cal );

	printf( "\n" );
}

/*=======================================================================*/
int main( int argc, char *argv[] )
{
	do_tests( "Standard", 2010,  1, 6 );	/* Day I'm writing this */
	do_tests( "Standard", 1858, 11, 16);	/* Wikipedia: Jul Day # 2,400,000 */
	do_tests( "Standard", 2132,  8, 31);	/* Wikipedia: Jul Day # 2,500,000 */

	/* Try some dates around the calendar transition dates for fun */
	do_tests( "Standard", 1582, 10,  4 );	/* Day before transition in Std cal */
	do_tests( "Standard", 1582, 10, 15 );	/* Day after  transition in Std cal */
	do_tests( "Standard", 1582, 10,  9 );	/* Day DOES NOT EXIST in Std cal, should give errors */

	return(0);
}

