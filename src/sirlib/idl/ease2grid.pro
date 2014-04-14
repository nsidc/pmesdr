PRO easegrid,iopt,alat,alon,thelon,thelat,ascale,bscale
;
;	COMPUTES THE FORWARD "EASE2" GRID TRANSFORM
;
;	GIVEN A LAT,LON (ALAT,ALON) AND THE SCALE (ASCALE) THE IMAGE
;	TRANSFORMATION COORDINATES (THELON,THELAT) ARE COMUTED
;	USING THE "EASE GRID" (VERSION 2.0) TRANSFORMATION GIVEN IN FORTRAN
;	SOURCE CODE SUPPLIED BY MJ BRODZIK AT NSIDC.
;	RADIUS EARTH=6378.137 KM (WGS 84)
;	MAP ECCENTRICITY=0.081819190843 (WGS84)
;
;	inputs:
;	  iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
;	  alon, alat: lon, lat (deg) to convert (can be outside of image)
;          ascale and bscale should be integer valued)
;	  ascale: grid scale factor (0..5)  pixel size is (bscale/2^ascale)
;	  bscale: base grid scale index (ind=int(bscale))
;
;          NSIDC .grd file for isc=0
;           project type    ind=0     ind=1         ind=3
;	      N         EASE2_N25km EASE2_N30km EASE2_N36km  
;             S         EASE2_S25km EASE2_S30km EASE2_S36km 
;             T/M       EASE2_T25km EASE2_M25km EASE2_M36km 
;
;          cell size (m) for isc=0 (scale is reduced by 2^isc)
;           project type    ind=0     ind=1            ind=3
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
;	  thelon: X coordinate in pixels (can be outside of image)
;	  thelat: Y coordinate in pixels (can be outside of image)
;
        DTR=0.01745329241994

	ind=int(bscale)
	isc=int(ascale)
	dlon=alon
	phi= DTR * alat

        EASE2_MAP_INFO(iopt,isc,ind, $
         map_equatorial_radius_m, map_eccentricity, $
          e2, map_reference_latitude, map_reference_longitude, $
          map_second_reference_latitude, sin_phi1, cos_phi1, kz, $ 
          map_scale, bcols, brows, r0, s0, epsilon

	dlon = dlon - map_reference_longitude
	dlon = easeconv_normalize_degrees( dlon )
	lam = DTR * dlon
    
	sin_phi=sin(phi)
	q=(1.0d0-e2)*((sin_phi/(1.0d0-e2*sin_phi*sin_phi)) $
             - (1.0d0/(2.0d0*map_eccentricity)) $
             * log((1.0d0-map_eccentricity*sin_phi) $
             / ( 1.0d0+map_eccentricity*sin_phi)))

	IF IOPT EQ 8 THEN BEGIN		; EASE2 GRID NORTH
	   qp = 1.0d0-((1.0d0-e2)/(2.0d0*map_eccentricity) $
     		   * log((1.0d0-map_eccentricity) $
                  / (1.0d0 + map_eccentricity))) $
	   if (abs(qp-q) lt epsilon) then $
	      rho = 0.0
	   else $
	      rho = map_equatorial_radius_m * sqrt( qp - q )
	   x =  rho * sin( lam )
	   y = -rho * cos( lam )
	ENDIF ELSE IF IOPT EQ 9 THEN BEGIN	; EASE2 GRID SOUTH
	   qp = 1.0d0-((1.0d0-e2)/( 2.0d0*map_eccentricity) $
     		   * log((1.0d0 - map_eccentricity) $
     			  / (1.0d0 + map_eccentricity)))
	   if ( abs( qp + q ) lt epsilon ) then $
	     rho = 0.0
	   else $
	     rho = map_equatorial_radius_m * sqrt( qp + q )
	   x = rho * sin( lam )
	   y = rho * cos( lam )
	ENDIF ELSE IF IOPT EQ 10 THEN BEGIN	; EASE2 CYLINDRICAL
	   x =   map_equatorial_radius_m * kz * lam
	   y = ( map_equatorial_radius_m * q ) / ( 2.0D0 * kz )
        ENDIF

	thelon = r0 + ( x / map_scale ) + 0.5
	thelat = s0 + ( y / map_scale ) + 0.5 ; 0 at bottom

	END
