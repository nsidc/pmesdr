	SUBROUTINE EASE2GRID(IOPT,ALAT,ALON,THELON,THELAT,ASCALE,BSCALE)
C
C	COMPUTE THE FORWARD "EASE2" GRID TRANSFORMATION
C
C	GIVEN THE IMAGE TRANSFORMATION COORDINATES (THELON,THELAT) AND
C	THE CORRESPONDING LON,LAT (ALON,ALAT) IS COMPUTED
C	USING THE "EASE2 GRID" (VERSION 2.0) TRANSFORMATION GIVEN IN IDL
C	SOURCE CODE SUPPLIED BY MJ BRODZIK
C	RADIUS EARTH=6378.137 KM (WGS 84)
C	MAP ECCENTRICITY=0.081819190843 (WGS84)
C
C	inputs:
C	  iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
C	  alon, alat: lon, lat (deg) to convert (can be outside of image)
C          ascale and bscale should be integer valued)
C	  ascale: grid scale factor (0..5)  pixel size is (bscale/2^ascale)
C	  bscale: base grid scale index (ind=int(bscale))
C
C          NSIDC .grd file for isc=0
C           project type    ind=0     ind=1         ind=3
C	      N         EASE2_N25km EASE2_N30km EASE2_N36km  
C             S         EASE2_S25km EASE2_S30km EASE2_S36km 
C             T/M       EASE2_T25km EASE2_M25km EASE2_M36km 
C
C          cell size (m) for isc=0 (scale is reduced by 2^isc)
C           project type    ind=0     ind=1            ind=3
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
C	  thelon: X coordinate in pixels (can be outside of image)
C	  thelat: Y coordinate in pixels (can be outside of image)
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

	double precision dlon,phi,lam,sin_phi,q,qp,rho,x,y

	double precision DTR
	parameter (DTR=0.01745329241994)

	ind=int(bscale)
	isc=int(ascale)
	dlon=alon
	phi= DTR * alat

	call EASE2_MAP_INFO(iopt,isc,ind,
     1    map_equatorial_radius_m, map_eccentricity,
     2     e2, map_reference_latitude, map_reference_longitude,
     3     map_second_reference_latitude, sin_phi1, cos_phi1, kz, 
     4     map_scale, bcols, brows, r0, s0, epsilon)

	dlon = dlon - map_reference_longitude   
	dlon = easeconv_normalize_degrees( dlon )
	lam = DTR * dlon
    
	sin_phi=sin(phi)
	q=(1.0d0-e2)*((sin_phi/(1.0d0-e2*sin_phi*sin_phi)) 
     1        - (1.0d0/(2.0d0*map_eccentricity)) 
     2        * log((1.0d0-map_eccentricity*sin_phi) 
     3        / ( 1.0d0+map_eccentricity*sin_phi)))

	if (iopt==8) then          ! EASE2 grid north
	   qp = 1.0d0-((1.0d0-e2)/(2.0d0*map_eccentricity) 
     1		   * log((1.0d0-map_eccentricity) 
     2             / (1.0d0 + map_eccentricity)))
	   if (abs(qp-q).lt.epsilon) then 
	      rho = 0.0
	   else
	      rho = map_equatorial_radius_m * sqrt( qp - q )
	   endif
	   x =  rho * sin( lam )
	   y = -rho * cos( lam )

	else if (iopt==9) then     ! EASE2 grid south
	   qp = 1.0d0-((1.0d0-e2)/( 2.0d0*map_eccentricity) 
     1		   * log((1.0d0 - map_eccentricity) 
     2			  / (1.0d0 + map_eccentricity)))
	   if ( abs( qp + q ) < epsilon ) then
	     rho = 0.0
	   else
	     rho = map_equatorial_radius_m * sqrt( qp + q )
	   endif
	   x = rho * sin( lam )
	   y = rho * cos( lam )

	else if (iopt==10) then    ! EASE2 cylindrical
	   x =   map_equatorial_radius_m * kz * lam
	   y = ( map_equatorial_radius_m * q ) / ( 2.0D0 * kz )
	else
	   write (*,*) '*** invalid EASE2 projection in ease2grid',iopt
	endif

	thelon = r0 + ( x / map_scale ) + 0.5
	thelat = s0 + ( y / map_scale ) + 0.5 ! 0 at bottom

	RETURN
	END