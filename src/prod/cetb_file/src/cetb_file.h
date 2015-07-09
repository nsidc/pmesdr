/*
 * cetb_file.h - Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef cetb_file_H
#define cetb_file_H

#define CETB_FILE_FORMAT_VERSION "v0.1"

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
  CETB_UNKNOWN_PRODUCER=-1,
  CETB_CSU,
  CETB_RSS
} cetb_swath_producer_id;

/*
 * Data producer string
 */
static const char *cetb_swath_producer_id_name[] = {
  "CSU",
  "RSS"
};

int cetb_valid_region_id( cetb_region_id region_id );
int cetb_valid_resolution_factor( int factor );
int cetb_valid_platform_id( cetb_platform_id platform_id );
int cetb_valid_sensor_id( cetb_sensor_id sensor_id );
int cetb_valid_date( int year, int doy );
int cetb_valid_pass_direction( cetb_region_id region_id, cetb_direction_id direction_id );
int cetb_valid_reconstruction_id( cetb_reconstruction_id reconstruction_id );
int cetb_valid_swath_producer_id( cetb_swath_producer_id producer_id );
int cetb_filename( char *filename, int max_length, char *dirname,
		   cetb_region_id region_id,
		   int factor,
		   cetb_platform_id platform_id,
		   cetb_sensor_id sensor_id,
		   int year,
		   int doy,
		   int beam_id,
		   cetb_direction_id direction_id,
		   cetb_reconstruction_id reconstruction_id,
		   cetb_swath_producer_id producer_id );

#endif // cetb_file_H
