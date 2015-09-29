/*
 * gsx.h - Utilities for extended generic swath
 *
 * 03-Aug-2015 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef gsx_H
#define gsx_H

#define ALIGNMENT 64
#define GSX_MAX_DIMS 10 // max number of dimension variables in file
#define GSX_MAX_CHANNELS 20 // max expected number of channels per platform
#define SENSOR_MAX 10 // max number of characters in the short sensor name
#define PLATFORM_MAX 10 // max number of characters in the short platform name

typedef struct {
  int fileid;
  char* gsx_version;
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
  //char *short_sensor;
  cetb_sensor_id short_sensor;
  cetb_platform_id short_platform;
  cetb_swath_producer_id input_provider;
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

/*
 * public functions
 */
int gsx_version ( gsx_class *this );
gsx_class *gsx_init ( char *filename );
void gsx_close ( gsx_class *this );

#endif // gsx_H

