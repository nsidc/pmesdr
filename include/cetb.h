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

/* alignment is needed for memory allocations across applications in this system */
#define CETB_MEM_ALIGNMENT 64

/* Problem with AMSRE input data files requires this first measurement parameter to be used in setup */
#define CETB_AMSRE_FIRST_MEASUREMENT 14

/* Max number of input files that could go into a daily output file */
#define CETB_MAX_INPUT_FILES 100 /* maximum number of input files to process onto a single grid */

typedef enum {
  CETB_NO_REGION=-1,
  CETB_EASE2_N,
  CETB_EASE2_S,
  CETB_EASE2_T,
  CETB_NUM_REGIONS
} cetb_region_id;

/*
 * CETB Region Numbers to match meas_meta convetions
 */
static const int cetb_region_number[] = {
  308,
  309,
  310,
};

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
 * lat and lon extents are determined by the region number - use the region ID
 * number to index into the array
 */
static double cetb_latitude_extent[CETB_NUM_REGIONS][2] = {
  { 0.000000, 90.000000 }, /* latitude min and max indexed by EASE2_N */
  { -90.000000, 0.000000 }, /* latitude min and max indexed by EASE2_S */
  { -67.057541, 67.057541 }  /* latitude min and max indexed by EASE2_T */
};
static double cetb_longitude_extent[CETB_NUM_REGIONS][2] = {
  { -180.00000, 180.00000 }, /* longitude min and max indexed by EASE2_N */
  { -180.00000, 180.00000 }, /* longitude min and max indexed by EASE2_S */
  { -180.00000, 180.00000 }  /* longitude min and max indexed by EASE2_T */
};
static const char *cetb_geospatial_bounds[] = {
  "EPSG:3475",
  "EPSG:3474",
  "POLYGON((-67.057541 -180.000000, -67.057541 180.000000, 67.057541 180.000000, 67.057541 -180.000000, -67.057541 -180.000000))"
};
static const char *cetb_geospatial_bounds_crs[] = {
  "EPSG:6931",
  "EPSG:6932",
  "EPSG:6933"
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
 * In the case of N or S projections, these are exact.
 * For T projections, they are nominal (but used tin the gpd names, nonetheless).
 */
static const char *cetb_resolution_name[] = {
  "25km",
  "12.5km",
  "6.25km",
  "3.125km",
  "1.5625km"
};

/*
 * Exact scale is a function of projection (N, S, T) and resolution factor
 * N,S grids are exact divisors of 25.0 km, T grids are slightly different
 */
static double cetb_exact_scale_m[CETB_NUM_REGIONS][CETB_MAX_RESOLUTION_FACTOR+1] = {
  { 25000.00000, 12500.00000, 6250.00000, 3125.00000, 1562.50000 }, /* row indexed by EASE2_N */
  { 25000.00000, 12500.00000, 6250.00000, 3125.00000, 1562.50000 }, /* row indexed by EASE2_S */
  { 25025.26000, 12512.63000, 6256.31500, 3128.15750, 1564.07875 }  /* row indexed by EASE2_T */
};

/*
 * Number of rows is a function of projection (N, S, T) and resolution factor
 */
static long int cetb_grid_rows[CETB_NUM_REGIONS][CETB_MAX_RESOLUTION_FACTOR+1] = {
  { 720, 1440, 2880, 5760, 11510 }, /* row indexed by EASE2_N */
  { 720, 1440, 2880, 5760, 11510 }, /* row indexed by EASE2_S */
  { 540, 1080, 2160, 4320,  8640 }  /* row indexed by EASE2_T */
};

/*
 * Number of cols is a function of projection (N, S, T) and resolution factor
 */
static long int cetb_grid_cols[CETB_NUM_REGIONS][CETB_MAX_RESOLUTION_FACTOR+1] = {
  {  720, 1440, 2880,  5760, 11510 }, /* row indexed by EASE2_N */
  {  720, 1440, 2880,  5760, 11510 }, /* row indexed by EASE2_S */
  { 1388, 2776, 5552, 11104, 22208 }  /* row indexed by EASE2_T */
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

/* LTOD split times are a function of sensor and projection
 * the projection list is as described in the variable above */
static float cetb_ltod_split_times[CETB_NUM_PLATFORMS][CETB_NUM_REGIONS][2] = {
  { {6.0, 18.0}, {6.0, 18.0}, {-1.0, -1.0} }, /* CETB_NIMBUS7 platform, 3 regions, -1.0 for undefined (T projection) */
  { {5.0, 17.0}, {8.0, 20.0}, {-1.0, -1.0} }, /* CETB_AQUA platform, 3 regions, -1.0 for undefined (T projection) */
  { {0.0, 12.0}, {0.0, 12.0}, {-1.0, -1.0} }, /* CETB_F08 platform, 3 regions, -1.0 for undefined (T projection) */
  { {4.0, 16.0}, {4.0, 16.0}, {-1.0, -1.0} }, /* CETB_F10 platform, 3 regions, -1.0 for undefined (T projection) */
  { {2.0, 14.0}, {2.0, 14.0}, {-1.0, -1.0} }, /* CETB_F11 platform, 3 regions, -1.0 for undefined (T projection) */
  { {0.0, 12.0}, {0.0, 12.0}, {-1.0, -1.0} }, /* CETB_F13 platform, 3 regions, -1.0 for undefined (T projection) */
  { {3.0, 15.0}, {3.0, 15.0}, {-1.0, -1.0} }, /* CETB_F15 platform, 3 regions, -1.0 for undefined (T projection) */
  { {3.0, 15.0}, {3.0, 15.0}, {-1.0, -1.0} }, /* CETB_F16 platform, 3 regions, -1.0 for undefined (T projection) */
  { {0.0, 12.0}, {0.0, 12.0}, {-1.0, -1.0} }, /* CETB_F17 platform, 3 regions, -1.0 for undefined (T projection) */
  { {0.0, 12.0}, {0.0, 12.0}, {-1.0, -1.0} }  /* CETB_F18 platform, 3 regions, -1.0 for undefined (T projection) */  
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

/* need to be able to map platform ID to sensor ID in meas_meta_setup */
static const cetb_sensor_id cetb_platform_to_sensor[] = {
  CETB_SMMR,
  CETB_AMSRE,
  CETB_SSMI,
  CETB_SSMI,
  CETB_SSMI,
  CETB_SSMI,
  CETB_SSMIS,
  CETB_SSMIS,
  CETB_SSMIS,
  CETB_SSMIS
};

/* Need to be able to map sensor ID to NSIDC dataset ID */
static const char *cetb_NSIDC_dataset_id[] = {
  "NSIDC-0630",
  "NSIDC-0630",
  "NSIDC-0630",
  "NSIDC-0630"
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
typedef enum {
  CETB_NO_PRODUCER=-1,
  CETB_CSU,
  CETB_RSS,
  CETB_JPL,
  CETB_NUM_PRODUCERS
} cetb_swath_producer_id;

/*
 * Data producer string
 */
static const char *cetb_swath_producer_id_name[] = {
  "CSU",
  "RSS",
  "JPL"
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

#endif // cetb_H

