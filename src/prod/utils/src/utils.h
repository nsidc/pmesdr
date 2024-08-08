/*
 * utils.h - General utilities for Passive Microwave ESDR project
 *
 * 11-Nov-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#ifndef utils_H
#define utils_H

int utils_allocate_clean_aligned_memory( void **this, size_t size );
void utils_ease2_map_info(int iopt, int isc, int ind, 
		    double *map_equatorial_radius_m, double *map_eccentricity, 
		    double *e2, double *map_reference_latitude, 
		    double *map_reference_longitude, 
		    double *map_second_reference_latitude,double * sin_phi1, 
		    double *cos_phi1, double *kz,
		    double *map_scale, int *bcols, int *brows, 
		    double *r0, double *s0, double *epsilon);

#endif // utils_H
