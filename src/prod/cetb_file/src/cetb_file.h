/*
 * cetb_file.h - Utilities for Calibrated Enhanced-Resolution TB files
 *
 * 06-Jul-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef cetb_file_H
#define cetb_file_H

#include "cetb.h"

#define CETB_FILE_FORMAT_VERSION "v0.1"
#define CETB_FILE_ALIGNMENT 64

/* Values of fill/missing/valid_range are recorded in packed form */
#define CETB_FILE_PACKING_CONVENTION "netCDF"
#define CETB_FILE_PACKING_CONVENTION_DESC "unpacked = scale_factor*packed + add_offset"
#define CETB_FILE_GRID_MAPPING "crs"

/* TB values for output files */
#define CETB_FILE_TB_FILL_VALUE 0
#define CETB_FILE_TB_MISSING_VALUE 60000
#define CETB_FILE_TB_SCALE_FACTOR 0.01
#define CETB_FILE_TB_ADD_OFFSET 0.0
#define CETB_FILE_TB_MIN 5000
#define CETB_FILE_TB_MAX 35000

/* TB std dev values for output files */
#define CETB_FILE_TB_STDDEV_FILL_VALUE ( NC_MAX_USHORT )
#define CETB_FILE_TB_STDDEV_MISSING_VALUE ( NC_MAX_USHORT - 1 )
#define CETB_FILE_TB_STDDEV_SCALE_FACTOR 0.01
#define CETB_FILE_TB_STDDEV_ADD_OFFSET 0.0
#define CETB_FILE_TB_STDDEV_MIN 0
#define CETB_FILE_TB_STDDEV_MAX ( NC_MAX_USHORT - 2 )

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
int cetb_file_add_uchars( cetb_file_class *this,
			  char *var_name,
			  unsigned char *data,
			  long int cols,
			  long int rows,
			  char *standard_name,
			  char *long_name,
			  char *units,
			  unsigned char fill_value,
			  unsigned char missing_value,
			  unsigned char min_value,
			  unsigned char max_value );
int cetb_file_add_packed_floats( cetb_file_class *this,
				 char *var_name,
				 float *data,
				 long int cols,
				 long int rows,
				 char *standard_name,
				 char *long_name,
				 char *units,
				 unsigned short fill_value,
				 unsigned short missing_value,
				 unsigned short min_value,
				 unsigned short max_value,
				 float scale_factor,
				 float add_offset );
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
