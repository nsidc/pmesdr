/*
 * gsx.h - Utilities for extended generic swath
 *
 * 03-Aug-2015 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef gsx_H
#define gsx_H

#define GSX_MAX_DIMS 3 // max number of positional variables in file, i.e. _loc1, _loc2, _loc3
#define GSX_MAX_CHANNELS 20 // max expected number of channels per platform
#define SENSOR_MAX 10 // max number of characters in the short sensor name
#define PLATFORM_MAX 10 // max number of characters in the short platform name

static const char *gsx_variable_attributes[] = {
  "standard_name",
  "long_name",
  "units",
  "valid_range",
  "_FillValue",
  "coordinates",
  "gsx_field_of_view",
  "gsx_azimuth_angle",
  "gsx_incidence_angle",
  "origin_method",
  "gsx_total_counts",
  "gsx_out_of_bounds"
};

static const char *gsx_latitudes[] = {
  "latitude_loc1",
  "latitude_loc2",
  "latitude_loc3"
};

static const char *gsx_longitudes[] = {
  "longitude_loc1",
  "longitude_loc2",
  "longitude_loc3"
};

static const char *gsx_eias[] = {
  "earth_incidence_angle_loc1",
  "earth_incidence_angle_loc2",
  "earth_incidence_angle_loc3"
};

static const char *gsx_eazs[] = {
  "earth_azimuth_angle_loc1",
  "earth_azimuth_angle_loc2",
  "earth_azimuth_angle_loc3"
};

static const char *gsx_sc_latitudes[] = {
  "spacecraft_latitude_loc1",
  "spacecraft_latitude_loc2",
  "spacecraft_latitude_loc3"
};

static const char *gsx_sc_longitudes[] = {
  "spacecraft_longitude_loc1",
  "spacecraft_longitude_loc2",
  "spacecraft_longitude_loc3"
};

static const char *gsx_scantime[] = {
  "scan_time_loc1",
  "scan_time_loc2",
  "scan_time_loc3"
};

static const char *gsx_ssmi_channel_name[] = {
  "brightness_temperature_19H",
  "brightness_temperature_19V",
  "brightness_temperature_22V",
  "brightness_temperature_37H",
  "brightness_temperature_37V",
  "brightness_temperature_85H",
  "brightness_temperature_85V"
};

static const char *gsx_ssmis_channel_name[] = {
  "brightness_temperature_19H",
  "brightness_temperature_19V",
  "brightness_temperature_22V",
  "brightness_temperature_37H",
  "brightness_temperature_37V",
  "brightness_temperature_91H",
  "brightness_temperature_91V"
};

static const char *gsx_amsre_channel_name[] = {
  "brightness_temperature_7H",
  "brightness_temperature_7V",
  "brightness_temperature_10.7H",
  "brightness_temperature_10.7V",
  "brightness_temperature_18H",
  "brightness_temperature_18V",
  "brightness_temperature_23H",
  "brightness_temperature_23V",
  "brightness_temperature_37H",
  "brightness_temperature_37V",
  "brightness_temperature_89H_A",
  "brightness_temperature_89V_A",
  "brightness_temperature_89H_B",
  "brightness_temperature_89V_B"
};

static const char *gsx_smmr_channel_name[] = {
  "brightness_temperature_7H",
  "brightness_temperature_7V",
  "brightness_temperature_10H",
  "brightness_temperature_10V",
  "brightness_temperature_18H",
  "brightness_temperature_18V",
  "brightness_temperature_21H",
  "brightness_temperature_21V",
  "brightness_temperature_37H",
  "brightness_temperature_37V"
};


typedef enum {
  CETB_NOLOC=-1,
  CETB_LOC1,
  CETB_LOC2,
  CETB_LOC3,
  CETB_NUM_LOCS
} cetb_loc_id;

/*
 * Sensor names
 */
static const char *cetb_loc_id_name[] = {
  "_loc1",
  "_loc2",
  "_loc3"
};

typedef struct {
  int fileid;
  char* gsx_version;
  int dims;
  int vars;
  int atts;
  int unlimdims;
  int scans[GSX_MAX_DIMS];
  int measurements[GSX_MAX_DIMS];
  char *source_file;
  cetb_sensor_id short_sensor;
  cetb_platform_id short_platform;
  cetb_swath_producer_id input_provider;
  char *channel_names[GSX_MAX_CHANNELS];
  cetb_loc_id channel_dims[GSX_MAX_CHANNELS];
  int channel_number;
  float fillvalue[GSX_MAX_CHANNELS];
  float *validRange[GSX_MAX_CHANNELS];
  float *efov[GSX_MAX_CHANNELS];
  float *latitude[GSX_MAX_DIMS];
  float fill_latitude[GSX_MAX_DIMS];
  float *longitude[GSX_MAX_DIMS];
  float fill_longitude[GSX_MAX_DIMS];
  float *sc_latitude[GSX_MAX_DIMS];
  float fill_sc_latitude[GSX_MAX_DIMS];
  float *sc_longitude[GSX_MAX_DIMS];
  float fill_sc_longitude[GSX_MAX_DIMS];
  double *scantime[GSX_MAX_DIMS];
  double fill_scantime[GSX_MAX_DIMS];
  float *eia[GSX_MAX_DIMS];
  float fill_eia[GSX_MAX_DIMS];
  float *eaz[GSX_MAX_DIMS];
  float fill_eaz[GSX_MAX_DIMS];
  float *brightness_temps[GSX_MAX_CHANNELS];
  int orbit;
  int fill_orbit;
  cetb_direction_id pass_direction;
} gsx_class;

/*
 * public functions
 */
int gsx_version ( gsx_class *this );
gsx_class *gsx_init ( char *filename );
void gsx_close ( gsx_class *this );

#endif // gsx_H

