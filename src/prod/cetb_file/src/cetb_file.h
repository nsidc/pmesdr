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
