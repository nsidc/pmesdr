/*
 * utils - General utilities for Passive Microwave ESDR project
 *
 * 11-Nov-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include "cetb.h"
#include "utils.h"

/*
 * utils_allocate_clean_aligned_memory - allocate aligned memory that is
 *                                       zeroed out.
 *
 * input:
 *   this : void ** address of pointer to new memory
 *   size : size_t size of memory to allocate
 *
 * output: n/a
 *
 * returns : 0 for success, or error message to stderr and return non-zero
 * 
 */
int utils_allocate_clean_aligned_memory( void **this, size_t size ) {

  if ( 0 != posix_memalign( this, CETB_MEM_ALIGNMENT, size ) ) {
    perror( __FUNCTION__ );
    return 1;
  }
  memset( *this, 0, size );
  return 0;

}
