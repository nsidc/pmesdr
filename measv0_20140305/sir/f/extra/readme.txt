Routines in this directory are not essential to SIR programs but
can be used to support user interaction with fortran utilities.  
The essential routines are readval1 and ireadval1 which are functions
returning a REAL or INTEGER value.  Given a default value, these
functions return either a user input value or. if the user input
is blank (i.e. a carriage return), the default value.  This can be
useful for user input.

A typical call is 

       VALUE=READVAL1(DEFAULT_VALUE,IERR)

The returned value is VALUE.  If a i/o error end of input is encountered
IERR is set to a negative value.

This utility function call can be replaced with

       READ (*,*) VALUE

Last revised: 14 Feb 2002