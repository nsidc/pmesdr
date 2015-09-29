/*
 * cetb.h - Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 25-Sep-2015 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */

typedef enum {
  CETB_NO_REGION=-1,
  CETB_EASE2_N=308,
  CETB_EASE2_S,
  CETB_EASE2_T
} cetb_region_id;

/*
 * CETB Region Names
 * Use the region ID number to index into this array like this:
 * cetb_region_id_name[ num-CETB_EASE2_N ]
 */
static const char *cetb_region_id_name[] = {
  "EASE2_N",
  "EASE2_S",
  "EASE2_T"
};

/*
 * Grid resolution factor: power of 2 to divide into base resolution of 25 km
 * factor = 0 : 25/2**0 = 25
 *          1 : 25/2**1 = 12.5
 */
#define CETB_BASE_RESOLUTION 25.0
#define CETB_MIN_RESOLUTION_FACTOR 0
#define CETB_MAX_RESOLUTION_FACTOR 4

/*
 * Grid resolution strings: string to match the names specified on the ATBD,
 * which in turn match the authoritative gpd filenames
 */
static const char *cetb_resolution_name[] = {
  "25km",
  "12.5km",
  "6.25km",
  "3.125km",
  "1.5625km"
};

/*
 * Satellite platform IDs
 */
typedef enum {
  CETB_NO_PLATFORM=-1,
  CETB_NIMBUS7,
  CETB_AQUA,
  CETB_F08,
  CETB_F10,
  CETB_F11,
  CETB_F13,
  CETB_F15,
  CETB_F16,
  CETB_F17,
  CETB_F18,
  CETB_NUM_PLATFORMS
} cetb_platform_id;

/*
 * Satellite platform names
 */
static const char *cetb_platform_id_name[] = {
  "NIMBUS7",
  "AQUA",
  "F08",
  "F10",
  "F11",
  "F13",
  "F15",
  "F16",
  "F17",
  "F18"
};

/*
 * GCMD platform keywords
 * Ref: http://gcmdservices.gsfc.nasa.gov/static/kms/platforms/platforms.csv
 */
static const char *cetb_gcmd_platform_keyword[] = {
  "NIMBUS > Nimbus-7",
  "AQUA > Earth Observing System, AQUA",
  "DMSP 5D-2/F8 > Defense Meteorological Satellite Program-F8",
  "DMSP 5D-2/F10 > Defense Meteorological Satellite Program-F10",
  "DMSP 5D-2/F11 > Defense Meteorological Satellite Program-F11",
  "DMSP 5D-2/F13 > Defense Meteorological Satellite Program-F13",
  "DMSP 5D-2/F15 > Defense Meteorological Satellite Program-F15",
  "DMSP 5D-3/F16 > Defense Meteorological Satellite Program-F16",
  "DMSP 5D-3/F17 > Defense Meteorological Satellite Program-F17",
  "DMSP 5D-3/F18 > Defense Meteorological Satellite Program-F18"
};


/*
 * Sensor IDs
 */
typedef enum {
  CETB_NO_SENSOR=-1,
  CETB_SMMR,
  CETB_AMSRE,
  CETB_SSMI,
  CETB_SSMIS,
  CETB_NUM_SENSORS
} cetb_sensor_id;

/*
 * Sensor names
 */
static const char *cetb_sensor_id_name[] = {
  "SMMR",
  "AMSRE",
  "SSMI",
  "SSMIS"
};

/*
 * GCMD sensor name keywords
 * Ref: http://gcmdservices.gsfc.nasa.gov/static/kms/instruments/instruments.csv
 */
static const char *cetb_gcmd_sensor_keyword[] = {
  "SMMR > Scanning Multichannel Microwave Radiometer",
  "AMSR-E > Advanced Microwave Scanning Radiometer-EOS",
  "SSM/I > Special Sensor Microwave/Imager",
  "SSMIS > Special Sensor Microwave Imager/Sounder"
};

/*
 * Valid starting year for crude date checking
 */
#define CETB_YEAR_START 1978

/*
 * Pass direction IDs
 */
typedef enum {
  CETB_NO_DIRECTION=-1,
  CETB_ALL_PASSES,
  CETB_ASC_PASSES,
  CETB_DES_PASSES,
  CETB_MORNING_PASSES,
  CETB_EVENING_PASSES,
  CETB_NUM_PASS_DIRECTIONS
} cetb_direction_id;

/*
 * Sensor names
 */
static const char *cetb_direction_id_name[] = {
  "B",
  "A",
  "D",
  "M",
  "E"
};

/*
 * Image reconstruction technique
 */
typedef enum {
  CETB_UNKNOWN_RECONSTRUCTION=-1,
  CETB_SIR,
  CETB_BGI
} cetb_reconstruction_id;

/*
 * Image reconstruction strings
 */
static const char *cetb_reconstruction_id_name[] = {
  "SIR",
  "BGI"
};

/*
 * Input swath data producer
 */
typedef enum {
  CETB_NO_PRODUCER=-1,
  CETB_CSU,
  CETB_RSS,
  CETB_NUM_PRODUCERS
} cetb_swath_producer_id;

/*
 * Data producer string
 */
static const char *cetb_swath_producer_id_name[] = {
  "CSU",
  "RSS"
};

/*
 * Maximum lengths of channel strings, including null char
 */
#define CHANNEL_STR_LENGTH 4

/* Maximum generic string length */
#define MAX_STR_LENGTH 100

/*
 * SSM/I channel IDs
 */
typedef enum {
  SSMI_19H=1,
  SSMI_19V,
  SSMI_22V,
  SSMI_37H,
  SSMI_37V,
  SSMI_85H,
  SSMI_85V
} ssmi_channel_id;
  
static const char *ssmi_channel_name[] = {
  "XXX",
  "19H",
  "19V",
  "22V",
  "37H",
  "37V",
  "85H",
  "85V"
};

/*
 * AMSR-E channel IDs
 */
typedef enum {
  AMSRE_06H=1,
  AMSRE_06V,
  AMSRE_10H,
  AMSRE_10V,
  AMSRE_18H,
  AMSRE_18V,
  AMSRE_23H,
  AMSRE_23V,
  AMSRE_36H,
  AMSRE_36V,
  AMSRE_89H,
  AMSRE_89V
} amsre_channel_id;
  
/*
 * AMSR-E channel ID names
 */
static const char *amsre_channel_name[] = {
  "XXX",
  "06H",
  "06V",
  "10H",
  "10V",
  "18H",
  "18V",
  "23H",
  "23V",
  "36H",
  "36V",
  "89H",
  "89V"
};

