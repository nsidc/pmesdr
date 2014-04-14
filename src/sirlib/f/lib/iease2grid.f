	SUBROUTINE IEASEGRID(IOPT,ALON,ALAT,THELON,THELAT,ASCALE,BSCALE)
C
C	COMPUTE THE INVERSE "EASE2" GRID TRANSFORM
C
C	GIVEN THE IMAGE TRANSFORMATION COORDINATES (THELON,THELAT) AND
C	THE CORRESPONDING LON,LAT (ALON,ALAT) IS COMPUTED
C	USING THE "EASE GRID" (VERSION 2.0) TRANSFORMATION GIVEN IN IDL
C	SOURCE CODE SUPPLIED BY MJ BRODZIK
C
C	inputs:
C	  iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
C	  thelon: X coordinate in pixels (can be outside of image)
C	  thelat: Y coordinate in pixels (can be outside of image)
C         ascale and bscale should be integer valued)
C	  ascale: grid scale factor (0..5)  pixel size is (bscale/2^ascale)
C	  bscale: base grid scale index (ind=int(bscale))
C
C          NSIDC .grd file for isc=0
C          project type    ind=0     ind=1         ind=3
C	      N         EASE2_N25km EASE2_N30km EASE2_N36km  
C             S         EASE2_S25km EASE2_S30km EASE2_S36km 
C             T/M       EASE2_T25km EASE2_M25km EASE2_M36km 
C
C         cell size (m) for isc=0 (scale is reduced by 2^isc)
C          project type    ind=0     ind=1            ind=3
C	      N          25000.0     30000.0         36000.0
C             S          25000.0     30000.0         36000.0
C             T/M       T25025.26   M25025.2600081  M36032.220840584
C	      
C	  for a given base cell size isc is related to NSIDC .grd file names
C	     isc        N .grd name   S .grd name   T .grd name
C	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
C	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
C	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
C	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
C	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  
C
C	outputs:
C	  alon, alat: lon, lat location in deg  (can be outside of image)
C
C	WRITTEN BY: DGL 12 MAR 2014
C
	integer iopt
	real alat,alon,thelon,thelat,ascale,bscale

	integer isc,ind
	double precision map_equatorial_radius_m, map_eccentricity
	double precision e2, map_reference_latitude
	double precision map_reference_longitude
	double precision map_second_reference_latitude
	double precision sin_phi1, cos_phi1, kz
	double precision map_scale, r0, s0, epsilon 
	integer bcols, brows

	double precision lam, arg, phi, beta, qp, rho2, x, y, e4, e6

	double precision RTD
	parameter (RTD=57.29577951308232)


	ind=int(bscale)
	isc=int(ascale)

	call EASE2_MAP_INFO(iopt,isc,ind,
     1    map_equatorial_radius_m, map_eccentricity,
     2     e2, map_reference_latitude, map_reference_longitude,
     3     map_second_reference_latitude, sin_phi1, cos_phi1, kz, 
     4     map_scale, bcols, brows, r0, s0, epsilon)

    
	e4 = e2 * e2
	e6 = e4 * e2

C       qp is the function q evaluated at phi = 90.0 deg
	qp = (1.0d0-e2) * ((1.0d0 /(1.0d0 - e2)) 
     1			 - (1.0d0 /(2.0d0*map_eccentricity)) 
     2			 * log((1.0d0-map_eccentricity) 
     3                         /(1.0d0+map_eccentricity)))

	x = (thelon - r0 - 0.5d0) * map_scale
	y = (thelat - 0.5d0 - s0) * map_scale ! 0 at bottom

	if (iopt==8) then          ! EASE2 grid north
	   rho2 = ( x * x ) + ( y * y )
	   arg=1.0d0- (rho2/(map_equatorial_radius_m * 
     1                       map_equatorial_radius_m * qp))
	   if (arg .gt.  1.0d0) arg=1.0      
	   if (arg .lt. -1.0d0) arg=-1.0
	   beta = asin( arg )
	   lam = atan2( x, -y )

	else if (iopt==9) then     ! EASE2 grid south
	   rho2 = ( x * x ) + ( y * y )
	   arg = 1.0d0-(rho2/(map_equatorial_radius_m * 
     1                        map_equatorial_radius_m * qp))
	   if (arg .gt.  1.0d0) arg=1.0d0      
	   if (arg .lt. -1.0d0) arg=-1.0d0
	   beta = -asin( arg )
	   lam = atan2( x, y )

	else if (iopt==10) then    ! EASE2 cylindrical
	   arg = 2.0d0*y*kz /(map_equatorial_radius_m * qp)
	   if (arg .gt.  1.0d0) arg=1.0d0    
	   if (arg .lt. -1.0d0) arg=-1.0d0
	   beta = asin( arg )
	   lam = x / (map_equatorial_radius_m * kz)
	else
	   write (*,*) '*** invalid EASE2 projection in iease2grid',iopt
	endif

	phi = beta 
     1     + ((( e2 / 3.0d0 ) + (( 31.0d0 / 180.0d0 ) * e4 ) 
     2	   + ((  517.0d0 / 5040.0) * e6 )) * sin( 2.0d0 * beta )) 
     3     + (((( 23.0d0 / 360.0d0) * e4) 
     4	   + ((  251.0d0 / 3780.0d0) * e6)) * sin( 4.0d0 * beta )) 
     5     + ((( 761.0d0 / 45360.0d0) * e6) * sin( 6.0d0 * beta ))
   
	alat = RTD * phi
	lam = map_reference_longitude + lam*RTD
	if (lam.gt.360.0d0) lam=lam-360.0d0
	if (lam.gt.360.0d0) lam=lam-360.0d0
	if (lam.gt.360.0d0) lam=lam-360.0d0
	if (lam.lt.360.0d0) lam=lam+360.0d0
	if (lam.lt.360.0d0) lam=lam+360.0d0
	if (lam.lt.360.0d0) lam=lam+360.0d0
	alon = lam

	RETURN
	END
