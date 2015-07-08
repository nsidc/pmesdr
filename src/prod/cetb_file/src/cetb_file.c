/*
 * cetb_files- Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include <stdio.h>
#include <string.h>

#include "cetb_file.h"

/*
 * cetb_filename - returns a pointer to an output filename with the requested characteristics
 *
 *  input :
 *
 *  result : CETB filename, according to convention in ATBD
 *
 */
void cetb_filename( char *filename, int max_length, char *dirname, int region ) {

  /*
   * Needs heck for exceeding max length?
   */
  snprintf( filename, max_length, "%s/%d%s", dirname, region, "TEST_FILE.nc" );
  
  return;
  
}
