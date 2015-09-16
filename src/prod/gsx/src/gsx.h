/*
 * gsx.h - Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 03-Aug-2015 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef gsx_H
#define gsx_H

#define ALIGNMENT 64
#define GSX_MAX_DIMS 10 // max number of dimension variables in file
#define GSX_MAX_CHANNELS 30 // max expected number of channels per platform
#define SENSOR_MAX 10 // max number of characters in the short sensor name
#define PLATFORM_MAX 10 // max number of characters in the short platform name
#define GSX_VERSION "v0.1"

typedef struct {
  int fileid;
  int dims;
  int vars;
  int atts;
  int unlimdims;
  int scans_loc1;
  int scans_loc2;
  int scans_loc3;
  int measurements_loc1;
  int measurements_loc2;
  int measurements_loc3;
  char *source_file;
  char *short_sensor;
  char *short_platform;
  char *input_provider;
  int channel_number;
  char *channel_names[GSX_MAX_CHANNELS];
} gsx_class;

typedef enum ssmi_channel {
  GSX_SSMI_19H=1,
  GSX_SSMI_19V,
  GSX_SSMI_22V,
  GSX_SSMI_37H,
  GSX_SSMI_37V,
  GSX_SSMI_85H,
  GSX_SSMI_85V
} gsx_ssmi_channel;

int gsx_version ( void );
gsx_class *gsx_init ( char *filename );
void gsx_close ( gsx_class *this );
#endif // gsx_H

