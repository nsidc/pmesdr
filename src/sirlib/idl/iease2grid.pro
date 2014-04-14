PRO ieasegrid,iopt,alon,alat,thelon,thelat,ascale,bscale
;
;	COMPUTES THE INVERSE "EASE2" GRID TRANSFORM
;
;	GIVEN THE IMAGE TRANSFORMATION COORDINATES (THELON,THELAT) AND
;	THE SCALE (ASCALE) THE CORRESPONDING LON,LAT (ALON,ALAT) IS COMPUTED
;	USING THE "EASE GRID" (VERSION 2.0) TRANSFORMATION GIVEN IN IDL
;	SOURCE CODE SUPPLIED BY MJ BRODZIK AT NSIDC
;
;	inputs:
;	  iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
;	  thelon: X coordinate in pixels (can be outside of image)
;	  thelat: Y coordinate in pixels (can be outside of image)
;         ascale and bscale should be integer valued)
;	  ascale: grid scale factor (0..5)  pixel size is (bscale/2^ascale)
;	  bscale: base grid scale index (ind=int(bscale))
;
;          NSIDC .grd file for isc=0
;          project type    ind=0     ind=1         ind=3
;	      N         EASE2_N25km EASE2_N30km EASE2_N36km  
;             S         EASE2_S25km EASE2_S30km EASE2_S36km 
;             T/M       EASE2_T25km EASE2_M25km EASE2_M36km 
;
;         cell size (m) for isc=0 (scale is reduced by 2^isc)
;          project type    ind=0     ind=1            ind=3
;	      N          25000.0     30000.0         36000.0
;             S          25000.0     30000.0         36000.0
;             T/M       T25025.26   M25025.2600081  M36032.220840584
;	      
;	  for a given base cell size isc is related to NSIDC .grd file names
;	     isc        N .grd name   S .grd name   T .grd name
;	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
;	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
;	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
;	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
;	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  
;
;	outputs:
;	  alon, alat: lon, lat location in deg  (can be outside of image)

        RTD=57.29577951308232
	ind=int(bscale)
	isc=int(ascale)

	EASE2_MAP_INFO,iopt,isc,ind, $
         map_equatorial_radius_m, map_eccentricity, $
          e2, map_reference_latitude, map_reference_longitude, $
          map_second_reference_latitude, sin_phi1, cos_phi1, kz, $ 
          map_scale, bcols, brows, r0, s0, epsilon

	e4 = e2 * e2
	e6 = e4 * e2

        ; qp is the function q evaluated at phi = 90.0 deg
	qp = (1.0d0-e2) * ((1.0d0 /(1.0d0 - e2)) $
     			 - (1.0d0 /(2.0d0*map_eccentricity)) $
     			 * log((1.0d0-map_eccentricity) $
                              /(1.0d0+map_eccentricity)))

	x = (thelon - r0 - 0.5d0) * map_scale
	y = (thelat - 0.5d0 - s0) * map_scale ; 0 at bottom

	IF IOPT EQ 8 THEN BEGIN	; EASE2 GRID NORTH
	   rho2 = ( x * x ) + ( y * y )
	   arg=1.0d0- (rho2/(map_equatorial_radius_m * $
                            map_equatorial_radius_m * qp))
	   if (arg gt  1.0d0) then arg=1.0      
	   if (arg lt -1.0d0) then arg=-1.0
	   beta = asin( arg )
	   lam = atan2( x, -y )
	ENDIF ELSE IF IOPT EQ 9 THEN BEGIN	; EASE2 GRID SOUTH
	   rho2 = ( x * x ) + ( y * y )
	   arg = 1.0d0-(rho2/(map_equatorial_radius_m * $
                             map_equatorial_radius_m * qp))
	   if (arg gt  1.0d0) then arg=1.0d0      
	   if (arg lt -1.0d0) then arg=-1.0d0
	   beta = -asin( arg )
	   lam = atan2( x, y )
	ENDIF ELSE IF IOPT EQ 10 THEN BEGIN	; EASE2 CYLINDRICAL
	   arg = 2.0d0*y*kz /(map_equatorial_radius_m * qp)
	   if (arg gt  1.0d0) then arg=1.0d0    
	   if (arg lt -1.0d0) then arg=-1.0d0
	   beta = asin( arg );
	   lam = x / (map_equatorial_radius_m * kz)
        ENDIF

	phi = beta $
          + ((( e2 / 3.0d0 ) + (( 31.0d0 / 180.0d0 ) * e4 ) $
          + ((  517.0d0 / 5040.0) * e6 )) * sin( 2.0d0 * beta )) $
          + (((( 23.0d0 / 360.0d0) * e4) $
          + ((  251.0d0 / 3780.0d0) * e6)) * sin( 4.0d0 * beta )) $
          + ((( 761.0d0 / 45360.0d0) * e6) * sin( 6.0d0 * beta ))
   
	alat = RTD * phi
	lam = map_reference_longitude + lam*RTD

	if (lam.gt.360.0d0) lam=lam-360.0d0
	if (lam.gt.360.0d0) lam=lam-360.0d0
	if (lam.gt.360.0d0) lam=lam-360.0d0
	if (lam.lt.360.0d0) lam=lam+360.0d0
	if (lam.lt.360.0d0) lam=lam+360.0d0
	if (lam.lt.360.0d0) lam=lam+360.0d0
	alon = lam

	END
