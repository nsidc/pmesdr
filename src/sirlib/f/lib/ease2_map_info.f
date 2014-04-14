	SUBROUTINE EASE2_MAP_INFO(iopt,isc,ind,
     1    map_equatorial_radius_m, map_eccentricity,
     2     e2, map_reference_latitude, map_reference_longitude,
     3     map_second_reference_latitude, sin_phi1, cos_phi1, kz, 
     4     map_scale, bcols, brows, r0, s0, epsilon)
C
C define EASE2 grid information 
C
C  inputs (integer)
C    iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
C    isc:  scale factor 0..5 grid size is (basesize(ind))/2^isc
C    ind:  base grid size index   (map units per cell in m
C 
C          NSIDC .grd file for isc=0
C           project type    ind=0     ind=1         ind=3
C	       N         EASE2_N25km EASE2_N30km EASE2_N36km  
C              S         EASE2_S25km EASE2_S30km EASE2_S36km 
C              T/M       EASE2_T25km EASE2_M25km EASE2_M36km 
C
C          cell size (m) for isc=0 (scale is reduced by 2**isc)
C           project type    ind=0     ind=1            ind=3
C	       N          25000.0     30000.0         36000.0
C              S          25000.0     30000.0         36000.0
C              T/M       T25025.26   M25025.2600081  M36032.220840584
C	      
C	  for a given base cell size isc is related to NSIDC .grd file names
C	     isc        N .grd name   S .grd name   T .grd name
C	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
C	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
C	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
C	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
C	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  
C
C  outputs (double precision)
C    map_equatorial_radius_m  EASE2 Earth equitorial radius (km) [WGS84]
C    map_eccentricity         EASE2 Earth eccentricity [WGS84]
C    map_reference_latitude   Reference latitude (deg) 
C    map_reference_longitude  Reference longitude (deg)
C    map_second_reference_latitude Secondary reference longitude* (deg)
C    sin_phi1, cos_phi1 kz    EASE2 Cylin parameters*
C    map_scale                EASE2 map projection pixel size (km)
C    bcols, brows,            EASE2 grid size in pixels
C    r0, s0                   EASE2 base projection size in pixels
C    epsilon                  EASE2 near-polar test factor
C
C    *these parameters only assigned values if projection is T
C
	integer iopt,isc,ind
	double precision map_equatorial_radius_m, map_eccentricity
	double precision e2, map_reference_latitude
	double precision map_reference_longitude
	double precision map_second_reference_latitude
	double precision sin_phi1, cos_phi1, kz
	double precision map_scale, r0, s0, epsilon 
	integer bcols, brows

	double precision DTR
	parameter (DTR=0.01745329241994)

	double precision base
	integer m, nx, ny

	m=2**isc   ! compute power-law scale factor

	map_equatorial_radius_m = 6378137.0d0  ! WGS84
	map_eccentricity = 0.081819190843d0    ! WGS84
	e2 = map_eccentricity *  map_eccentricity
	map_reference_longitude = 0.0

	if (iopt==8) then          ! EASE2 grid north
	   map_reference_latitude = 90.0d0
	   if (ind==1) then           ! EASE2_N30km.gpd
	      base=30000.0d0
	      nx=600
	      ny=600	
	   else if (ind==2) then      ! EASE2_N36km.gpd
	      base=36000.0d0    
	      nx=500
	      ny=500	
           else                       ! EASE2_N25km.gpd
	      base=25000.0d0
	      nx=720
	      ny=720	
	   endif
	else if (iopt==9) then     ! EASE2 grid south
	   map_reference_latitude = -90.0d0
	   if (ind==1) then           ! EASE2_S30km.gpd
	      base=30000.0d0
	      nx=600
	      ny=600	
	   else if (ind==2) then      ! EASE2_S36km.gpd
	      base=36000.0d0    
	      nx=500
	      ny=500	
           else                       ! EASE2_S25km.gpd
	      base=25000.0d0
	      nx=720
	      ny=720	
	   endif
	else if (iopt==10) then    ! EASE2 cylindrical
	   map_reference_latitude = 0.0d0
	   map_second_reference_latitude = 30.0d0
	   sin_phi1 = sin( DTR * map_second_reference_latitude)
	   cos_phi1 = cos( DTR * map_second_reference_latitude)
	   kz = cos_phi1 / sqrt( 1.0d0 - e2*sin_phi1*sin_phi1)
 	   if (ind==1) then           ! EASE2_M25km.gpd
	      base=25025.2600081d0
	      nx=1388
	      ny=584	
	   else if (ind==2) then      ! EASE2_M36km.gpd
	      base=36032.220840584d0
	      nx=964
	      ny=406	
           else                       ! EASE2_T25km.gpd
	      base=25025.26000d0
	      nx=1388
	      ny=538	
	   endif
	else
	   base=25000.0d0
	   nx=10
	   ny=10
	endif

C       grid info
	map_scale = base / m
        bcols = ceil(nx * m)
        brows = ceil(ny * m)
	r0 = (nx * m - 1) / 2.0d0
	s0 = (ny * m - 1) / 2.0d0

	RETURN
	END
