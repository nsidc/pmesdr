/*
 * gsx.h - Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 03-Aug-2015 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef gsx_H
#define gsx_H

#define ALIGNMENT 64
#define GSX_MAX_DIMS 10

#define GSX_VERSION "v0.1"

typedef struct {
  int fileid;
  int dims;
  int vars;
  int atts;
  int unlimdims;
  int scans_loc1;
  int scans_loc2;
  int measurements_loc1;
  int measurements_loc2;
} gsx_class;

int gsx_version ( void );
gsx_class *gsx_init ( char *filename );
void gsx_close ( gsx_class *this );
#endif // gsx_H

