/*
 * cetb.h - Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 25-Sep-2015 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef cetb_H
#define cetb_H

/* Include standard headers for things like printf and __FUNCTION__ */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* alignment is needed for memory allocations across applications in this system */
#define CETB_MEM_ALIGNMENT 64

/* Problem with AMSRE input data files requires this first measurement parameter to be used in setup */
#define CETB_AMSRE_FIRST_MEASUREMENT 14

/* Max number of input files that could go into a daily output file */
#define CETB_MAX_INPUT_FILES 250 /* maximum number of input files to process onto a single grid */

/*
 * Grid resolution factor: power of 2 to divide into base resolution of 25 km
 * factor = 0 : 25/2**0 = 25
 *          1 : 25/2**1 = 12.5
 */
#define CETB_BASE_25_RESOLUTION 25
#define CETB_BASE_36_RESOLUTION 36
#define CETB_BASE_24_RESOLUTION 24
#define CETB_MIN_RESOLUTION_FACTOR 0
#define CETB_MAX_RESOLUTION_FACTOR 4

typedef enum {
  CETB_NO_RESOLUTION = -1,
  CETB_25KM,
  CETB_36KM,
  CETB_24KM,
  CETB_NUMBER_BASE_RESOLUTIONS
} cetb_resolution_id;

/* Hardcoded definition of regions from regionsdat.def file */
typedef enum {
  CETB_NO_PROJECTION = 307,
  CETB_NORTH_PROJECTION,
  CETB_SOUTH_PROJECTION,
  CETB_CYLINDRICAL_PROJECTION,
  CETB_ALL_PROJECTIONS
} cetb_projection_number;

#define CETB_PROJECTION_BASE_NUMBER (CETB_NO_PROJECTION+1)

/* Number of allowed projections - N, S, T/M */
#define CETB_NUMBER_PROJECTIONS (CETB_ALL_PROJECTIONS - CETB_PROJECTION_BASE_NUMBER)

typedef enum {
  CETB_NO_REGION=-1,
  CETB_EASE2_N,
  CETB_EASE2_S,
  CETB_EASE2_T,
  CETB_EASE2_N36,
  CETB_EASE2_S36,
  CETB_EASE2_M36,
  CETB_EASE2_N24,
  CETB_EASE2_S24,
  CETB_EASE2_M24,
  CETB_NUM_REGIONS
} cetb_region_id;

/*
 * CETB Region Numbers to match meas_meta convetions
 */
static const int cetb_region_number[CETB_NUMBER_BASE_RESOLUTIONS]
                                   [CETB_NUMBER_PROJECTIONS] = {
  { 308, 309, 310 },
  { 308, 309, 310 },
  { 308, 309, 310 }
};

/*
 * CETB Region Names
 */
#define CETB_MAX_LEN_REGION_ID_NAME 10
static const char *cetb_region_id_name[CETB_NUMBER_BASE_RESOLUTIONS]
                                      [CETB_NUMBER_PROJECTIONS] = {
  { "EASE2_N", "EASE2_S", "EASE2_T" },
  { "EASE2_N", "EASE2_S", "EASE2_M" },
  { "EASE2_N", "EASE2_S", "EASE2_M" }
};

/*
 * lat and lon extents are determined by the region number - use the region ID
 * number to index into the array
 */
static double cetb_latitude_extent[CETB_NUMBER_BASE_RESOLUTIONS]
                                  [CETB_NUMBER_PROJECTIONS][2] = {
  { {0.000000, 90.000000}, {-90.000000, 0.000000}, {-67.057541, 67.057541} }, 
  { {0.000000, 90.000000}, {-90.000000, 0.000000}, {-85.0445664, 85.0445664} }, 
  { {0.000000, 90.000000}, {-90.000000, 0.000000}, {-85.0445664, 85.0445664} }
};
static double cetb_longitude_extent[CETB_NUMBER_BASE_RESOLUTIONS]
                                   [CETB_NUMBER_PROJECTIONS][2] = {
  { {-180.00000, 180.00000}, {-180.00000, 180.00000}, {-180.00000, 180.00000} },
  { {-180.00000, 180.00000}, {-180.00000, 180.00000}, {-180.00000, 180.00000} },
  { {-180.00000, 180.00000}, {-180.00000, 180.00000}, {-180.00000, 180.00000} } 
};

static const char *cetb_geospatial_bounds[CETB_NUMBER_BASE_RESOLUTIONS]
                                         [CETB_NUMBER_PROJECTIONS] = {
  { "EPSG:3475", "EPSG:3474",
    "POLYGON((-67.057541 -180.000000, -67.057541 180.000000, 67.057541 180.000000,"
    "67.057541 -180.000000, -67.057541 -180.000000))" },
  { "EPSG:3475", "EPSG:3474",
    "POLYGON((-85.0445664 -180.000000, -85.0445664 180.000000, "
    "85.0445664 180.000000, 85.0445664 -180.000000, -85.0445664 -180.000000))" },
  { "EPSG:3475", "EPSG:3474",
    "POLYGON((-85.0445664 -180.000000, -85.0445664 180.000000, "
    "85.0445664 180.000000, 85.0445664 -180.000000, -85.0445664 -180.000000))" }
};

static const char *cetb_geospatial_bounds_crs[CETB_NUMBER_BASE_RESOLUTIONS]
                                             [CETB_NUMBER_PROJECTIONS] = {
  { "EPSG:6931", "EPSG:6932", "EPSG:6933" },
  { "EPSG:6931", "EPSG:6932", "EPSG:6933" },
  { "EPSG:6931", "EPSG:6932", "EPSG:6933" }
};

/*
 * Grid resolution strings: string to match the names specified on the ATBD,
 * which in turn match the authoritative gpd filenames
 * In the case of N or S projections, these are exact.
 * For T projections, they are nominal (but used tin the gpd names, nonetheless).
 */
static const char
*cetb_resolution_name[CETB_NUMBER_BASE_RESOLUTIONS][CETB_MAX_RESOLUTION_FACTOR+1] = {
  { "25km", "12.5km", "6.25km", "3.125km", "1.5625km" },
  { "36km", "18km",   "09km",    "4.5km",   "2.25km" },
  { "24km", "12km",   "06km",    "03km",     "1.5km" }
};

/*
 * Exact scale is a function of projection (N, S, T/M) and resolution factor
 * N,S grids are exact divisors of 25.0 or 36.0 km, T/M grids are slightly different
 */
static double cetb_exact_scale_m[CETB_NUM_REGIONS]
                                [CETB_MAX_RESOLUTION_FACTOR+1] = {
  { 25000.00000, 12500.00000, 6250.00000, 3125.00000, 1562.50000 }, /* row indexed by EASE2_N 25 km*/
  { 25000.00000, 12500.00000, 6250.00000, 3125.00000, 1562.50000 }, /* row indexed by EASE2_S 25 km */
  { 25025.26000, 12512.63000, 6256.31500, 3128.15750, 1564.07875 }, /* row indexed by EASE2_T 25 km */
  { 36000.00000, 18000.00000, 9000.00000, 4500.00000, 2250.00000 }, /* row indexed by EASE2_N 36 km*/
  { 36000.00000, 18000.00000, 9000.00000, 4500.00000, 2250.0000  }, /* row indexed by EASE2_S 36 km */
  { 36032.220840584, 18016.110420292, 9008.055210146, 4504.027605073,
      2252.0138025365 },  /* row indexed by EASE2_M 36 km */
  { 24000.00000, 12000.00000, 6000.00000, 3000.00000, 1500.00000 }, /* row indexed by EASE2_N 24 km */
  { 24000.00000, 12000.00000, 6000.00000, 3000.00000, 1500.00000 }, /* row indexed by EASE2_S 24 km */
  { 24021.480560389347, 12010.740280194674, 6005.370140097337,
    3002.685070048668, 1501.342535024334 } /* row indexed by EASE2_M 24 km */
};

/*
 * Number of rows is a function of projection (N, S, T) and resolution factor
 */
static long int cetb_grid_rows[CETB_NUM_REGIONS]
                              [CETB_MAX_RESOLUTION_FACTOR+1] = {
  { 720, 1440, 2880, 5760, 11510 }, /* row indexed by EASE2_N 25 km */
  { 720, 1440, 2880, 5760, 11510 }, /* row indexed by EASE2_S 25 km */
  { 540, 1080, 2160, 4320,  8640 },  /* row indexed by EASE2_T 25 km */
  { 500, 1000, 2000, 4000,  8000 }, /* row indexed by EASE2_N 36 km */
  { 500, 1000, 2000, 4000,  8000 }, /* row indexed by EASE2_S 36 km */
  { 406,  812, 1624, 3248,  6496 },  /* row indexed by EASE2_M 36 km */
  { 750, 1500, 3000, 6000, 12000 }, /* row indexed by EASE2_N 24 km */
  { 750, 1500, 3000, 6000, 12000 }, /* row indexed by EASE2_S 24 km */
  { 609, 1218, 2436, 4872,  9744 } /* row indexed by EASE2_M 24 km */
};

/*
 * Number of cols is a function of projection (N, S, T) and resolution factor
 */
static long int cetb_grid_cols[CETB_NUM_REGIONS]
                              [CETB_MAX_RESOLUTION_FACTOR+1] = {
  { 720, 1440, 2880,  5760, 11510 }, /* row indexed by EASE2_N 25 km */
  { 720, 1440, 2880,  5760, 11510 }, /* row indexed by EASE2_S 25 km */
  { 1388, 2776, 5552, 11104, 22208 },  /* row indexed by EASE2_T 25 km */
  { 500, 1000, 2000,  4000,  8000 }, /* row indexed by EASE2_N 36 km */
  { 500, 1000, 2000,  4000,  8000 }, /* row indexed by EASE2_S 36 km */
  { 964, 1928, 3856,  7712, 15424 }, /* row indexed by EASE2_M 36 km */
  { 750, 1500, 3000,  6000, 12000 }, /* row indexed by EASE2_N 24 km */
  { 750, 1500, 3000,  6000, 12000 }, /* row indexed by EASE2_S 24 km */
  { 1446, 2892, 5784, 11568, 23136 }  /* row indexed by EASE2_M 24 km */
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
  CETB_F14,
  CETB_F15,
  CETB_F16,
  CETB_F17,
  CETB_F18,
  CETB_F19,
  CETB_SMAP,
  CETB_GCOMW1,
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
  "F14",
  "F15",
  "F16",
  "F17",
  "F18",
  "F19",
  "SMAP",
  "GCOMW1"
};

/*
 * GCMD platform keywords
 * Ref: http://gcmdservices.gsfc.nasa.gov/static/kms/platforms/platforms.csv
 *
 * Note that generate_premetandspatial.py parses this string to the the GCMD SHORT_NAME
 * for the platform field and that this short_name must be an exact match to GCMD
 * entry for short_name in order for ECS to ingest the data
 */
static const char *cetb_gcmd_platform_keyword[] = {
  "NIMBUS > Nimbus-7",
  "AQUA > Earth Observing System, AQUA",
  "DMSP 5D-2/F8 > Defense Meteorological Satellite Program-F8",
  "DMSP 5D-2/F10 > Defense Meteorological Satellite Program-F10",
  "DMSP 5D-2/F11 > Defense Meteorological Satellite Program-F11",
  "DMSP 5D-2/F13 > Defense Meteorological Satellite Program-F13",
  "DMSP 5D-2/F14 > Defense Meteorological Satellite Program-F14",
  "DMSP 5D-3/F15 > Defense Meteorological Satellite Program-F15",
  "DMSP 5D-3/F16 > Defense Meteorological Satellite Program-F16",
  "DMSP 5D-3/F17 > Defense Meteorological Satellite Program-F17",
  "DMSP 5D-3/F18 > Defense Meteorological Satellite Program-F18",
  "DMSP 5D-3/F19 > Defense Meteorological Satellite Program-F19",
  "SMAP > Soil Moisture Active and Passive Observatory",
  "GCOM-W1 > Global Change Observation Mission 1st-Water"
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
  CETB_SMAP_RADIOMETER,
  CETB_AMSR2,
  CETB_NUM_SENSORS
} cetb_sensor_id;

/*
 * Sensor names
 */
static const char *cetb_sensor_id_name[] = {
  "SMMR",
  "AMSRE",
  "SSMI",
  "SSMIS",
  "LRM",
  "AMSR2"
};

/* need to be able to map platform ID to sensor ID in meas_meta_setup */
static const cetb_sensor_id cetb_platform_to_sensor[] = {
  CETB_SMMR,
  CETB_AMSRE,
  CETB_SSMI,
  CETB_SSMI,
  CETB_SSMI,
  CETB_SSMI,
  CETB_SSMI,
  CETB_SSMI,
  CETB_SSMIS,
  CETB_SSMIS,
  CETB_SSMIS,
  CETB_SSMIS,
  CETB_SMAP_RADIOMETER,
  CETB_AMSR2
};

/* Need to be able to map sensor ID and producer ID to NSIDC dataset ID */
static const char *cetb_NSIDC_dataset_id[] = {
  "NSIDC0630",
  "NSIDC-0763",
  "NSIDC-0738"
};

/* enum to index into the cetb_NSIDC_dataset_id array */
typedef enum {
  CETB_NSIDC_0000=-1,
  CETB_NSIDC_0630,
  CETB_NSIDC_0763,
  CETB_NSIDC_0738
} cetb_dataset_id;

/*
 * GCMD sensor name keywords
 * Ref: http://gcmdservices.gsfc.nasa.gov/static/kms/instruments/instruments.csv
 *
 * Note that generate_premetandspatial.py parses this string to the the GCMD SHORT_NAME
 * for the sensor field and that this short_name must be an exact match to GCMD
 * entry for short_name in order for ECS to ingest the data
 */
static const char *cetb_gcmd_sensor_keyword[] = {
  "SMMR > Scanning Multichannel Microwave Radiometer",
  "AMSR-E > Advanced Microwave Scanning Radiometer-EOS",
  "SSM/I > Special Sensor Microwave/Imager",
  "SSMIS > Special Sensor Microwave Imager/Sounder",
  "SMAP L-BAND RADIOMETER > SMAP L-Band Radiometer",
  "AMSR2 > Advanced Microwave Scanning Radiometer 2"
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
 * Direction names
 */
static const char *cetb_direction_id_name[] = {
  "B",
  "A",
  "D",
  "M",
  "E"
};

/*
 * Full Direction names
 */
static const char *cetb_direction_id_name_full[] = {
  "Both",
  "Ascending",
  "Descending",
  "Morning",
  "Evening"
};

/*
 * Image reconstruction technique
 */
typedef enum {
  CETB_UNKNOWN_RECONSTRUCTION=-1,
  CETB_SIR,
  CETB_BGI,
  CETB_GRD,
  CETB_NUM_RECONSTRUCTIONS

} cetb_reconstruction_id;

/*
 * Image reconstruction strings
 */
static const char *cetb_reconstruction_id_name[] = {
  "SIR",
  "BGI",
  "GRD"
};

/*
 * Input swath data producer
 */
typedef enum { CETB_NO_PRODUCER=-1,
  CETB_CSU,
  CETB_RSS,
  CETB_JPL,
  CETB_CSU_ICDR,
  CETB_PPS_XCAL,
  CETB_NUM_PRODUCERS
} cetb_swath_producer_id;

/*
 * Data producer string
 */
static const char *cetb_swath_producer_id_name[] = {
  "CSU",
  "RSS",
  "JPL",
  "CSU_ICDR",
  "PPS_XCAL"
};

/*
 * SSM/I channel IDs
 */
typedef enum {
  SSMI_NO_CHANNEL=-1,
  SSMI_19H,
  SSMI_19V,
  SSMI_22V,
  SSMI_37H,
  SSMI_37V,
  SSMI_85H,
  SSMI_85V,
  SSMI_NUM_CHANNELS
} cetb_ssmi_channel_id;
  
static const char *cetb_ssmi_channel_name[] = {
  "19H",
  "19V",
  "22V",
  "37H",
  "37V",
  "85H",
  "85V"
};

/*
 * this mapping is set to match the way SSMI beams are defined in meas_meta_setup and
 * in meas_meta_make
 *
 * in those routines, ibeam = 1 is set for 19H and on up to ibeam = 7 for 85V
 *
 * starting at -1 allows the code to have normal C arrays that begin at 0, i.e.
 * ibeam = 1 will have the mapping 0 in the C code
 * 
 * Example:
 *  cetb_ibeam_to_cetb_ssmi_channel[1] returns the value of SSMI_19H
 *  and
 *  cetb_ibeam_to_cetb_ssmi_channel[3] returns the value of SSMI_22V
 *
 */
static const cetb_ssmi_channel_id cetb_ibeam_to_cetb_ssmi_channel[] = {
  SSMI_NO_CHANNEL,
  SSMI_19H,
  SSMI_19V,
  SSMI_22V,
  SSMI_37H,
  SSMI_37V,
  SSMI_85H,
  SSMI_85V
};

/*
 * SSMIS channel IDs
 */
typedef enum {
  SSMIS_NO_CHANNEL=-1,
  SSMIS_19H,
  SSMIS_19V,
  SSMIS_22V,
  SSMIS_37H,
  SSMIS_37V,
  SSMIS_91H,
  SSMIS_91V,
  SSMIS_NUM_CHANNELS
} cetb_ssmis_channel_id;
  
static const char *cetb_ssmis_channel_name[] = {
  "19H",
  "19V",
  "22V",
  "37H",
  "37V",
  "91H",
  "91V"
};

/*
 * this mapping is set to match the way SSMIS beams are defined in meas_meta_setup and
 * in meas_meta_make
 *
 * see explanation above
 *
 */
static const cetb_ssmis_channel_id cetb_ibeam_to_cetb_ssmis_channel[] = {
  SSMIS_NO_CHANNEL,
  SSMIS_19H,
  SSMIS_19V,
  SSMIS_22V,
  SSMIS_37H,
  SSMIS_37V,
  SSMIS_91H,
  SSMIS_91V
};

/*
 * AMSR-E channel IDs
 */
typedef enum {
  AMSRE_NO_CHANNEL=-1,
  AMSRE_06H,
  AMSRE_06V,
  AMSRE_10H,
  AMSRE_10V,
  AMSRE_18H,
  AMSRE_18V,
  AMSRE_23H,
  AMSRE_23V,
  AMSRE_36H,
  AMSRE_36V,
  AMSRE_89H_A,
  AMSRE_89V_A,
  AMSRE_89H_B,
  AMSRE_89V_B,
  AMSRE_NUM_CHANNELS
} cetb_amsre_channel_id;
  
/*
 * AMSR-E channel ID names
 * these are only used for file naming - note that the A and B channels
 * for 89 GHz map into the same output grid
 */
static const char *cetb_amsre_channel_name[] = {
  "06H",
  "06V",
  "10.7H",
  "10.7V",
  "18H",
  "18V",
  "23H",
  "23V",
  "36H",
  "36V",
  "89H",
  "89V",
  "89H",
  "89V"
};

/*
 * See notes for using cetb_ibeam_to_cetb_ssmi_channel, above.
 */
static const cetb_amsre_channel_id cetb_ibeam_to_cetb_amsre_channel[] = {
  AMSRE_NO_CHANNEL,
  AMSRE_06H,
  AMSRE_06V,
  AMSRE_10H,
  AMSRE_10V,
  AMSRE_18H,
  AMSRE_18V,
  AMSRE_23H,
  AMSRE_23V,
  AMSRE_36H,
  AMSRE_36V,
  AMSRE_89H_A,
  AMSRE_89V_A,
  AMSRE_89H_B,
  AMSRE_89V_B
};

/*
 * SMMR channel IDs
 */
typedef enum {
  SMMR_NO_CHANNEL=-1,
  SMMR_06H,
  SMMR_06V,
  SMMR_10H,
  SMMR_10V,
  SMMR_18H,
  SMMR_18V,
  SMMR_21H,
  SMMR_21V,
  SMMR_37H,
  SMMR_37V,
  SMMR_NUM_CHANNELS
} cetb_smmr_channel_id;
  
/*
 * SMMR channel ID names
 * these are only used for file naming  
 */
static const char *cetb_smmr_channel_name[] = {
  "06H",
  "06V",
  "10H",
  "10V",
  "18H",
  "18V",
  "21H",
  "21V",
  "37H",
  "37V"
};

/*
 * See notes for using cetb_ibeam_to_cetb_ssmi_channel, above.
 */
static const cetb_smmr_channel_id cetb_ibeam_to_cetb_smmr_channel[] = {
  SMMR_NO_CHANNEL,
  SMMR_06H,
  SMMR_06V,
  SMMR_10H,
  SMMR_10V,
  SMMR_18H,
  SMMR_18V,
  SMMR_21H,
  SMMR_21V,
  SMMR_37H,
  SMMR_37V
};

/*
 * SMAP channel IDs
 */
typedef enum {
  SMAP_NO_CHANNEL=-1,
  SMAP_1d41H,
  SMAP_1d41V,
  SMAP_1d41F,
  SMAP_NUM_CHANNELS
} cetb_smap_channel_id;
  
static const char *cetb_smap_channel_name[] = {
  "1.4H",
  "1.4V",
  "1.4F"
};

/*
 * this mapping is set to match the way SMAP beams are defined in meas_meta_setup and
 * in meas_meta_make
 *
 * see explanation above
 *
 */
static const cetb_smap_channel_id cetb_ibeam_to_cetb_smap_channel[] = {
  SMAP_NO_CHANNEL,
  SMAP_1d41H,
  SMAP_1d41V,
  SMAP_1d41F
};

/*
 * AMSR2 channel IDs
 */
typedef enum {
  AMSR2_NO_CHANNEL=-1,
  AMSR2_10H,
  AMSR2_10V,
  AMSR2_18H,
  AMSR2_18V,
  AMSR2_23H,
  AMSR2_23V,
  AMSR2_36H,
  AMSR2_36V,
  AMSR2_89H_A,
  AMSR2_89V_A,
  AMSR2_89H_B,
  AMSR2_89V_B,
  AMSR2_NUM_CHANNELS
} cetb_amsr2_channel_id;
  
/*
 * AMSR2 channel ID names
 * these are only used for file naming - note that the A and B channels
 * for 89 GHz map into the same output grid
 */
static const char *cetb_amsr2_channel_name[] = {
  "10.7H",
  "10.7V",
  "18H",
  "18V",
  "23H",
  "23V",
  "36H",
  "36V",
  "89H",
  "89V",
  "89H",
  "89V"
};

/*
 * See notes for using cetb_ibeam_to_cetb_ssmi_channel, above.
 */
static const cetb_amsr2_channel_id cetb_ibeam_to_cetb_amsr2_channel[] = {
  AMSR2_NO_CHANNEL,
  AMSR2_10H,
  AMSR2_10V,
  AMSR2_18H,
  AMSR2_18V,
  AMSR2_23H,
  AMSR2_23V,
  AMSR2_36H,
  AMSR2_36V,
  AMSR2_89H_A,
  AMSR2_89V_A,
  AMSR2_89H_B,
  AMSR2_89V_B
};

#endif // cetb_H

