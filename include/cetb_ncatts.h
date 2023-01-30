/*
 * cetb_ncatts.h - Your one-stop shop for #DEFINEs for Calibrated Enhanced-Resolution TB files
 *
 * 20-Jun-2016 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2016 Regents of the University of Colorado and Brigham Young University
 */
#ifndef cetb_ncatts_H
#define cetb_ncatts_H

/* Include standard headers for things like printf and __FUNCTION__ */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* TB values for output files */
#define CETB_NCATTS_TB_FILL_VALUE 0
#define CETB_NCATTS_STOKES_FILL_VALUE 5000
#define CETB_NCATTS_TB_SCALE_FACTOR (0.01)
#define CETB_NCATTS_STOKES_SCALE_FACTOR (0.01)
#define CETB_NCATTS_TB_ADD_OFFSET (0.0)
#define CETB_NCATTS_STOKES_ADD_OFFSET (-100.0)
#define CETB_NCATTS_TB_MIN 5000
#define CETB_NCATTS_STOKES_MIN 5000
#define CETB_NCATTS_TB_MAX 35000
#define CETB_NCATTS_STOKES_MAX 15000
#define CETB_NCATTS_TB_SCALED_MIN ( (CETB_NCATTS_TB_MIN) * (CETB_NCATTS_TB_SCALE_FACTOR) )
#define CETB_NCATTS_STOKES_SCALED_MIN ( ((CETB_NCATTS_STOKES_MIN) * (CETB_NCATTS_STOKES_SCALE_FACTOR)) \
					+ (CETB_NCATTS_STOKES_ADD_OFFSET) )
#define CETB_NCATTS_TB_SCALED_MAX ( (CETB_NCATTS_TB_MAX) * (CETB_NCATTS_TB_SCALE_FACTOR) )
#define CETB_NCATTS_STOKES_SCALED_MAX ( ((CETB_NCATTS_STOKES_MAX) * (CETB_NCATTS_STOKES_SCALE_FACTOR)) \
					+ (CETB_NCATTS_STOKES_ADD_OFFSET) )
#define CETB_NCATTS_TB_SIR_LONG_NAME "SIR TB"
#define CETB_NCATTS_TB_GRD_LONG_NAME "GRD TB"
#define CETB_NCATTS_STOKES_SIR_LONG_NAME "SIR 4th Stokes parameter at the surface after RFI filtering"
#define CETB_NCATTS_STOKES_GRD_LONG_NAME "GRD 4th Stokes parameter at the surface after RFI filtering"

/* Other constants needed for output files */
/* TB std dev values for output files */
#define CETB_NCATTS_TB_STDDEV_FILL_VALUE ( NC_MAX_USHORT )
#define CETB_NCATTS_STOKES_STDDEV_FILL_VALUE CETB_NCATTS_TB_STDDEV_FILL_VALUE
#define CETB_NCATTS_TB_STDDEV_SCALE_FACTOR (0.01)
#define CETB_NCATTS_STOKES_STDDEV_SCALE_FACTOR CETB_NCATTS_TB_STDDEV_SCALE_FACTOR
#define CETB_NCATTS_TB_STDDEV_ADD_OFFSET (0.0)
#define CETB_NCATTS_STOKES_STDDEV_ADD_OFFSET (-100.0)
#define CETB_NCATTS_TB_STDDEV_MIN 0
#define CETB_NCATTS_TB_STDDEV_MAX ( (NC_MAX_USHORT) - 2 )

/* TB number of samples values for output files */
#define CETB_NCATTS_TB_NUM_SAMPLES_FILL_VALUE 0
#define CETB_NCATTS_TB_NUM_SAMPLES_MIN 1
#define CETB_NCATTS_TB_NUM_SAMPLES_MAX NC_MAX_CHAR

/* TB time values for output files */
#define CETB_NCATTS_TB_TIME_FILL_VALUE NC_MIN_SHORT
#define CETB_NCATTS_TB_TIME_SCALE_FACTOR (1.0)
#define CETB_NCATTS_TB_TIME_ADD_OFFSET (0.0)
#define CETB_NCATTS_TB_TIME_MIN ( (NC_MIN_SHORT) + 1 )
#define CETB_NCATTS_TB_TIME_MAX NC_MAX_SHORT

/* Incidence angle values for output files */
#define CETB_NCATTS_THETA_FILL_VALUE (-1)
#define CETB_NCATTS_THETA_SCALE_FACTOR (0.01)
#define CETB_NCATTS_THETA_ADD_OFFSET (0.0)
#define CETB_NCATTS_THETA_MIN 0
#define CETB_NCATTS_THETA_MAX 9000


#endif // cetb_ncatts_H

