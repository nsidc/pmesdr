/*
 * utils - General utilities for Passive Microwave ESDR project
 *
 * 11-Nov-2015 M. J. Brodzik brodzik@nsidc.org 303-492-8263
 * Copyright (C) 2015 Regents of the University of Colorado and Brigham Young University
 */
#include "cetb.h"
#include "utils.h"
#include <math.h>

static int dceil( double r );

/*
 * utils_allocate_clean_aligned_memory - allocate aligned memory that is
 *                                       zeroed out.
 *
 * input:
 *   this : void ** address of pointer to new memory
 *   size : size_t size of memory to allocate
 *
 * output: n/a
 *
 * returns : 0 for success, or error message to stderr and return non-zero
 * 
 */
int utils_allocate_clean_aligned_memory( void **this, size_t size ) {

  if ( 0 != posix_memalign( this, CETB_MEM_ALIGNMENT, size ) ) {
    perror( __FUNCTION__ );
    return 1;
  }
  memset( *this, 0, size );
  return 0;

}

/*
 * utils_ease2_map_info - defines EASE2 grid information 
 *
 * input:
 *   iopt: int, projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
 *   isc: int, scale factor 0..5 grid size is (basesize(ind))/2^isc
 *   ind: int,  base grid size index   (map units per cell in NSIDC .grd file for isc=0
 *         project type    ind=0     ind=1         ind=2
 *            N         EASE2_N25km EASE2_N36km EASE2_N24km  
 *            S         EASE2_S25km EASE2_S36km EASE2_S24km 
 *            T/M       EASE2_T25km EASE2_M36km EASE2_M24km 
 *
 *        cell size (m) for isc=0 (scale is reduced by 2^isc)
 *         project type    ind=0     ind=1            ind=2
 *	      N          25000.0     36000.0         24000.0
 *            S          25000.0     36000.0         24000.0
 *            T/M       T25025.26  M36032.220840584  M24021.4805603
 *	      
 *	  for a given base cell size isc is related to NSIDC .grd file names
 *	     isc        N .grd name   S .grd name   T .grd name
 *	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
 *	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
 *	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
 *	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
 *	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  
 *
 *  outputs:
 *    map_equatorial_radius_m  EASE2 Earth equitorial radius (km) [WGS84]
 *    map_eccentricity         EASE2 Earth eccentricity [WGS84]
 *    e2                       square of eccentricity
 *    map_reference_latitude   Reference latitude (deg) 
 *    map_reference_longitude  Reference longitude (deg)
 *    map_second_reference_latitude Secondary reference longitude* (deg)
 *    sin_phi1, cos_phi1, kz   EASE2 Cylin parameters*
 *    map_scale                EASE2 map projection pixel size (km)
 *    bcols, brows,            EASE2 grid size in pixels
 *    r0, s0                   EASE2 base projection size in pixels
 *    epsilon                  EASE2 near-polar test factor
 *
 *  *these parameters only assigned values if projection is T
 *
 * returns : n/a
 *
 */
void utils_ease2_map_info(int iopt, int isc, int ind,
			  double *map_equatorial_radius_m, double *map_eccentricity, 
			  double *e2, double *map_reference_latitude, 
			  double *map_reference_longitude, 
			  double *map_second_reference_latitude,double * sin_phi1, 
			  double *cos_phi1, double *kz,
			  double *map_scale, int *bcols, int *brows, 
			  double *r0, double *s0, double *epsilon) {

  double base;  
  int m, nx, ny;
  int region_index;

  *map_equatorial_radius_m = 6378137.0 ; /* WGS84 */
  *map_eccentricity = 0.081819190843 ;   /* WGS84 */
  *e2 = *map_eccentricity *  *map_eccentricity;
  *map_reference_longitude = 0.0;
  *epsilon = 1.e-6;

  /* map-specific parameters  - these all come from cetb.h */
  switch (iopt) {
    case 8:   /* EASE2 grid north */
      *map_reference_latitude = 90.0;
      switch(ind) {
      case CETB_36KM:  /* EASE2_N36km.gpd */
	region_index = CETB_36KM*CETB_NUMBER_PROJECTIONS;
	base=cetb_exact_scale_m[region_index][0]; //36000.0;      
	nx=cetb_grid_cols[region_index][0]; //500;
	ny=cetb_grid_rows[region_index][0]; //500;	
	break;
      case CETB_24KM:  /* EASE2_N24km.gpd */
	region_index = CETB_24KM*CETB_NUMBER_PROJECTIONS;
	base=cetb_exact_scale_m[region_index][0]; //24000.0;
	nx=cetb_grid_cols[region_index][0]; //750;
	ny=cetb_grid_rows[region_index][0]; //750;	
	break;
      default: /* EASE2_N25km.gpd */
	region_index = CETB_25KM*CETB_NUMBER_PROJECTIONS;
	base=cetb_exact_scale_m[region_index][0]; //25000.0;
	nx=cetb_grid_cols[region_index][0]; //720;
	ny=cetb_grid_rows[region_index][0]; //720;	
      }
      break;
    case 9:   /* EASE2 grid south */
      *map_reference_latitude = -90.0;
      switch(ind) {
      case CETB_36KM:  /* EASE2_S36km.gpd */
	region_index = (CETB_36KM*CETB_NUMBER_PROJECTIONS)+1;
	base=cetb_exact_scale_m[region_index][0]; //36000.0;      
	nx=cetb_grid_cols[region_index][0]; //500;
	ny=cetb_grid_rows[region_index][0]; //500;	
	break;
      case CETB_24KM:  /* EASE2_S24km.gpd */
	region_index = (CETB_24KM*CETB_NUMBER_PROJECTIONS)+1;
	base=cetb_exact_scale_m[region_index][0]; //24000.0;
	nx=cetb_grid_cols[region_index][0]; //750;
	ny=cetb_grid_rows[region_index][0]; //750;	
	break;
      default: /* EASE2_S25km.gpd */
	region_index = (CETB_25KM*CETB_NUMBER_PROJECTIONS)+1;
	base=cetb_exact_scale_m[region_index][0]; //25000.0;
	nx=cetb_grid_cols[region_index][0]; //720;
	ny=cetb_grid_rows[region_index][0]; //720;	
      }
      break;
    case 10:  /* EASE2 cylindrical */
      *map_reference_latitude = 0.0;
      *map_second_reference_latitude = 30.0;
      *sin_phi1 = sin( UTILS_DTR * *map_second_reference_latitude );
      *cos_phi1 = cos( UTILS_DTR * *map_second_reference_latitude );
      *kz = *cos_phi1 / sqrt( 1.0 - *e2 * *sin_phi1 * *sin_phi1 );
      switch(ind) {
      case CETB_36KM:  /* EASE2_M36km.gpd */
	region_index = (CETB_36KM*CETB_NUMBER_PROJECTIONS)+2;
	base=cetb_exact_scale_m[region_index][0]; //36032.220840584;
	nx=cetb_grid_cols[region_index][0]; //964;
	ny=cetb_grid_rows[region_index][0]; //406;	
	break;
      case CETB_24KM:  /* EASE2_M24km.gpd */
	region_index = (CETB_24KM*CETB_NUMBER_PROJECTIONS)+2;
	base=cetb_exact_scale_m[region_index][0]; //24021.480560389347;  
	nx=cetb_grid_cols[region_index][0]; //1446;
	ny=cetb_grid_rows[region_index][0]; //609;	
	break;
      default: /* EASE2_T25km.gpd */
	region_index = (CETB_25KM*CETB_NUMBER_PROJECTIONS)+2;
	base=cetb_exact_scale_m[region_index][0]; //25025.26000;
	nx=cetb_grid_cols[region_index][0]; //1388;
	ny=cetb_grid_rows[region_index][0]; //540;  /* originally was 538 */	
      }
   }
  
  /* grid info */
  if (isc>=0) {    
    for (m=1; isc>0; isc--) m *= 2;  /* compute power-law scale factor */
    *map_scale = base / (double) m;
    *bcols = nx * m;
    *brows = ny * m;
    *r0 = ((double) (*bcols - 1)) / 2.0;
    *s0 = ((double) (*brows - 1)) / 2.0;
  } else {
    for (m=1; isc<0; isc++) m *= 2;  /* compute power-law scale factor */
    *map_scale = base * (double) m;
    *bcols = dceil( nx / (double) m);    
    *brows = dceil( ny / (double) m);
    /* note: the following computation ensures that the lower-left corner
       remains at the same location even if ny/m is a non-integer. */
    *r0 = (nx / (double) m - 1.0) / 2.0;
    *s0 = (ny / (double) m - 1.0) / 2.0;    
  }
  
}

/*
 * dceil - calculates numeric ceiling for double value
 *
 * input :
 *  r : double value to find ceiling for, works for negative or positive values
 *
 * output : n/a
 *
 * returns : nearest integer ceiling of r
 *
 */
static int dceil(double r)
{
  int ret_val = (int)r;
  if (ret_val - r > 0.0) ret_val++;
  return(ret_val);
}
