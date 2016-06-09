/*
 * sir_geom.h - Utilities for Calibrated Enhanced-Resolution coordinate conversions
 *
 * 09-Jun-2016 M. A. Hardman mhardman@nsidc.org 303-492-2969
 * Copyright (C) 2016 Regents of the University of Colorado and Brigham Young University
 */
#ifndef sir_geom_H
#define sir_geom_H

/* Include standard headers for things like printf and __FUNCTION__ */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void pixtolatlon(float x, float y, float *alon, float *alat,
		 int iopt, float xdeg, float ydeg,
		 float ascale, float bscale, float a0, float b0);
void latlon2pix(float alon, float alat, float *x, float *y, 
		 int iopt, float xdeg, float ydeg,
		float ascale, float bscale, float a0, float b0);
void ease2_map_info(int iopt, int isc, int ind, 
		    double *map_equatorial_radius_m, double *map_eccentricity, 
		    double *e2, double *map_reference_latitude, 
		    double *map_reference_longitude, 
		    double *map_second_reference_latitude,double * sin_phi1, 
		    double *cos_phi1, double *kz,
		    double *map_scale, int *bcols, int *brows, 
		    double *r0, double *s0, double *epsilon);
void f2ipix(float x, float y, int *ix, int *iy, int nsx, int nsy);

#endif // sir_geom_H


