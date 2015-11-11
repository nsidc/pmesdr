/*
 * cetb_file.h - Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef cetb_file_H
#define cetb_file_H

#include "cetb.h"

#include <netcdf.h>

#define CETB_FILE_FORMAT_VERSION "v0.1"
#define CETB_FILE_ALIGNMENT 64
#define CETB_PACK 1
#define CETB_NO_PACK 0

/* Values of fill/missing/valid_range are recorded in packed form */
#define CETB_FILE_PACKING_CONVENTION "netCDF"
#define CETB_FILE_PACKING_CONVENTION_DESC "unpacked = scale_factor*packed + add_offset"
#define CETB_FILE_UNPACK_DATA( factor, offset, packed ) ( ( (factor) * (packed) ) + (offset) )
#define CETB_FILE_PACK_DATA( factor, offset, unpacked ) ( ( ( (unpacked) - (offset) ) / (factor) ) + 0.5 )
#define CETB_FILE_GRID_MAPPING "crs"
#define CETB_FILE_TB_STANDARD_NAME "brightness_temperature"
#define CETB_FILE_TB_UNIT "kelvin"
#define CETB_FILE_ANGULAR_UNIT "degree"

/* TB values for output files */
#define CETB_FILE_TB_FILL_VALUE 0
#define CETB_FILE_TB_MISSING_VALUE 60000
#define CETB_FILE_TB_SCALE_FACTOR 0.01
#define CETB_FILE_TB_ADD_OFFSET 0.0
#define CETB_FILE_TB_MIN 5000
#define CETB_FILE_TB_MAX 35000

/* TB std dev values for output files */
#define CETB_FILE_TB_STDDEV_FILL_VALUE ( NC_MAX_USHORT )
#define CETB_FILE_TB_STDDEV_MISSING_VALUE ( (NC_MAX_USHORT) - 1 )
#define CETB_FILE_TB_STDDEV_SCALE_FACTOR 0.01
#define CETB_FILE_TB_STDDEV_ADD_OFFSET 0.0
#define CETB_FILE_TB_STDDEV_MIN 0
#define CETB_FILE_TB_STDDEV_MAX ( (NC_MAX_USHORT) - 2 )

/* TB number of samples values for output files */
#define CETB_FILE_TB_NUM_SAMPLES_FILL_VALUE 0
#define CETB_FILE_TB_NUM_SAMPLES_MIN 1
#define CETB_FILE_TB_NUM_SAMPLES_MAX NC_MAX_CHAR

/* TB time values for output files */
#define CETB_FILE_TB_TIME_FILL_VALUE NC_MIN_SHORT
#define CETB_FILE_TB_TIME_SCALE_FACTOR 1.0
#define CETB_FILE_TB_TIME_ADD_OFFSET 0.0
#define CETB_FILE_TB_TIME_MIN ( (NC_MIN_SHORT) + 1 )
#define CETB_FILE_TB_TIME_MAX NC_MAX_SHORT

/* Incidence angle values for output files */
#define CETB_FILE_THETA_FILL_VALUE -1
#define CETB_FILE_THETA_SCALE_FACTOR 0.01
#define CETB_FILE_THETA_ADD_OFFSET 0.0
#define CETB_FILE_THETA_MIN 0
#define CETB_FILE_THETA_MAX 9000

/* Needs to hold "minutes since yyyy-dd-mm 00:00:00" */
#define CETB_FILE_EPOCH_STRING_LENGTH 34

/*
 * ESIP-recommended attribute to assist with mapping to ISO code
 * for the source of the data.
 */
#define CETB_FILE_COVERAGE_CONTENT_TYPE "image"

/*
 * These 2 functions are temporary, and should not be needed once
 * we start passing this information along to bgi/sir in setup files
 */
cetb_direction_id cetb_get_direction_id_from_info_name( const char *info_name );
cetb_swath_producer_id cetb_get_swath_producer_id_from_outpath( const char *outpath,
								const cetb_reconstruction_id reconstruction_id );

/*
 * The public interface definition of a cetb object
 */
typedef struct {
  int fid;
  char *filename;
  int year;
  int doy;
  char epoch_string[CETB_FILE_EPOCH_STRING_LENGTH];
  cetb_platform_id platform_id;
  cetb_region_id region_id;
  int factor;
  cetb_sensor_id sensor_id;
  cetb_reconstruction_id reconstruction_id;
  long int cols;
  long int rows;
  int cols_dim_id;
  int rows_dim_id;
  int time_dim_id;
} cetb_file_class;

cetb_file_class *cetb_file_init( char *dirname,
				 int region_number,
				 int factor,
				 cetb_platform_id platform_id,
				 cetb_sensor_id sensor_id,
				 int year,
				 int doy,
				 int beam_id,
				 cetb_direction_id direction_id,
				 cetb_reconstruction_id reconstruction_id,
				 cetb_swath_producer_id producer_id );
int cetb_file_open( cetb_file_class *this );
int cetb_file_add_var( cetb_file_class *this,
		       char *var_name,
		       nc_type xtype,
		       void *data,
		       long int cols,
		       long int rows,
		       char *standard_name,
		       char *long_name,
		       char *units,
		       void *fill_value_p,
		       void *missing_value_p,
		       void *valid_range_p,
		       int do_pack,
		       float scale_factor,
		       float add_offset,
		       char *calendar );
int cetb_file_add_bgi_parameters( cetb_file_class *this,
				  double gamma,
				  float dimensional_tuning_parameter,
				  float noise_variance,
				  float db_threshold,
				  float diff_threshold,
				  int median_flag );
int cetb_file_add_sir_parameters( cetb_file_class *this,
				  int number_of_iterations,
				  int median_flag );
int cetb_file_add_grd_parameters( cetb_file_class *this,
				  int median_flag );
void cetb_file_close( cetb_file_class *this );

#endif // cetb_file_H
