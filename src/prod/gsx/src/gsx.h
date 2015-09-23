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
  char *channel_names[GSX_MAX_CHANNELS];
  int channel_number;
  float fillvalue;
  float *efov[GSX_MAX_CHANNELS];
  float *latitude_loc1;
  float *latitude_loc2;
  float *latitude_loc3;
  float *longitude_loc1;
  float *longitude_loc2;
  float *longitude_loc3;
  float *sc_latitude_loc1;
  float *sc_latitude_loc2;
  float *sc_latitude_loc3;
  float *sc_longitude_loc1;
  float *sc_longitude_loc2;
  float *sc_longitude_loc3;
  double *scantime_loc1;
  double *scantime_loc2;
  double *scantime_loc3;
  float *eia_loc1;
  float *eia_loc2;
  float *eia_loc3;
  float *eaz_loc1;
  float *eaz_loc2;
  float *eaz_loc3;
  float *brightness_temps[GSX_MAX_CHANNELS];
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

static const char *gsx_variable_attributes[] = {
  "standard_name",
  "long_name",
  "units",
  "valid_range",
  "_FillValue",
  "coordinates",
  "gsx_field_of_view",
  "gsx_azimuth_angle",
  "gsx_incidence_angle"
};

int gsx_version ( void );
gsx_class *gsx_init ( char *filename );
void gsx_close ( gsx_class *this );
#endif // gsx_H

