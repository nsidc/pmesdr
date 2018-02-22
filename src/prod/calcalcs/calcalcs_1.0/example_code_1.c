#include <stdio.h>
#include <stdlib.h>

#include <udunits2.h>
#include "calcalcs.h"
#include "utCalendar2_cal.h"

/* Exercise the udunits calls */

int main( int argc, char *argv[] ) 
{
	static	char *units    = "seconds since 2010-01-06 09:20";
	static	char *calendar = "Standard";

	ut_system	*u_system;
	ut_unit		*u_1;
	double		tval, sec, tval_inv;
	int		ierr, yr, mo, day, hr, min;

	/* Initialize the udunits-2 library */
	if( (u_system = ut_read_xml( NULL )) == NULL ) {
		fprintf( stderr, "Error initializing udunits-2 unit system\n" );
		exit(-1);
		}

	/* Parse the units string */
	if( (u_1 = ut_parse( u_system, units, UT_ASCII )) == NULL ) {
		fprintf( stderr, "Error parsing units string \"%s\"\n", units );
		exit(-1);
		}

	/* Make the Calendar calls */
	tval = 86460.0;		/* in seconds, this is 1 day and 1 minute */

	if( (ierr = utCalendar2_cal( tval, u_1, &yr, &mo, &day, &hr, &min, &sec, calendar )) != 0 ) {
		fprintf( stderr, "Error on utCalendar2_cal call: %s\n", ccs_err_str(ierr) );
		exit(-1);
		}
	printf( "%lf %s in the %s calendar equals date %04d-%02d-%02d %02d:%02d:%06.3lf\n", 
		tval, units, calendar, yr, mo, day, hr, min, sec );

	/* For a test, convert back from the date to a value and see if it matches */
	if( (ierr = utInvCalendar2_cal( yr, mo, day, hr, min, sec, u_1, &tval_inv, calendar )) != 0 ) {
		fprintf( stderr, "Error on utCalendar2_cal call: %s\n", ccs_err_str(ierr) );
		exit(-1);
		}
	printf( "Test: %04d-%02d-%02d %02d:%02d:%06.3lf is %lf %s in the %s calendar\n",
		yr, mo, day, hr, min, sec, tval_inv, units, calendar );

	return(0);
}

