PRO EASE2_MAP_INFO,iopt,isc,ind, $
         map_equatorial_radius_m, map_eccentricity, $
          e2, map_reference_latitude, map_reference_longitude, $
          map_second_reference_latitude, sin_phi1, cos_phi1, kz, $ 
          map_scale, bcols, brows, r0, s0, epsilon
;
; define EASE2 grid information 
;
;  inputs (integer)
;    iopt: projection type 8=EASE2 N, 9-EASE2 S, 10=EASE2 T/M
;    isc:  scale factor 0..5 grid size is (basesize(ind))/2^isc
;    ind:  base grid size index   (map units per cell in m
; 
;          NSIDC .grd file for isc=0
;           project type    ind=0     ind=1         ind=3
;	       N         EASE2_N25km EASE2_N30km EASE2_N36km  
;              S         EASE2_S25km EASE2_S30km EASE2_S36km 
;              T/M       EASE2_T25km EASE2_M25km EASE2_M36km 
;
;          cell size (m) for isc=0 (scale is reduced by 2**isc)
;           project type    ind=0     ind=1            ind=3
;	       N          25000.0     30000.0         36000.0
;              S          25000.0     30000.0         36000.0
;              T/M       T25025.26   M25025.2600081  M36032.220840584
;	      
;	  for a given base cell size isc is related to NSIDC .grd file names
;	     isc        N .grd name   S .grd name   T .grd name
;	      0	      EASE2_N25km     EASE2_S25km     EASE2_T25km  
;	      1	      EASE2_N12.5km   EASE2_S12.5km   EASE2_T12.5km  
;	      2	      EASE2_N6.25km   EASE2_S6.25km   EASE2_T6.25km  
;	      3	      EASE2_N3.125km  EASE2_S3.125km  EASE2_T3.125km  
;	      4	      EASE2_N1.5625km EASE2_S1.5625km EASE2_T1.5625km  
;
;  outputs (double precision)
;    map_equatorial_radius_m  EASE2 Earth equitorial radius (km) [WGS84]
;    map_eccentricity         EASE2 Earth eccentricity [WGS84]
;    map_reference_latitude   Reference latitude (deg) 
;    map_reference_longitude  Reference longitude (deg)
;    map_second_reference_latitude Secondary reference longitude* (deg)
;    sin_phi1, cos_phi1 kz    EASE2 Cylin parameters*
;    map_scale                EASE2 map projection pixel size (km)
;    bcols, brows,            EASE2 grid size in pixels
;    r0, s0                   EASE2 base projection size in pixels
;    epsilon                  EASE2 near-polar test factor
;
;    *these parameters only assigned values if projection is T
;
	DTR=0.01745329241994
	m=2**isc   ; compute power-law scale factor

	map_equatorial_radius_m = 6378137.0d0  ; WGS84
	map_eccentricity = 0.081819190843d0    ; WGS84
	e2 = map_eccentricity *  map_eccentricity
	map_reference_longitude = 0.0

	if (iopt eq 8) then begin         ; EASE2 grid north
	   map_reference_latitude = 90.0d0
	   if (ind eq 1) then begin            ; EASE2_N30km.gpd
	      base=30000.0d0
	      nx=600
	      ny=600	
           endif else if (ind eq 2) then begin ; EASE2_N36km.gpd
	      base=36000.0d0    
	      nx=500
	      ny=500	
           endif else begin                    ; EASE2_N25km.gpd
	      base=25000.0d0
	      nx=720
	      ny=720	
	   endelse
        endif else if (iopt eq 9) then    ; EASE2 grid south
	   map_reference_latitude = -90.0d0
	   if (ind eq 1) then begin            ; EASE2_S30km.gpd
	      base=30000.0d0
	      nx=600
	      ny=600	
	   endif else if (ind eq 2) then begin ; EASE2_S36km.gpd
	      base=36000.0d0    
	      nx=500
	      ny=500	
           endif else begin                    ; EASE2_S25km.gpd
	      base=25000.0d0
	      nx=720
	      ny=720	
	   endelse
        endif else if (iopt eq 10) then begin ; EASE2 cylindrical
	   map_reference_latitude = 0.0d0
	   map_second_reference_latitude = 30.0d0
	   sin_phi1 = sin( DTR * map_second_reference_latitude)
	   cos_phi1 = cos( DTR * map_second_reference_latitude)
	   kz = cos_phi1 / sqrt( 1.0d0 - e2*sin_phi1*sin_phi1)
 	   if (ind eq 1) then begin          ; EASE2_M25km.gpd
	      base=25025.2600081d0
	      nx=1388
	      ny=584	
	   endif else if (ind==2) then begin  ; EASE2_M36km.gpd
	      base=36032.220840584d0
	      nx=964
	      ny=406	
           endif else begin                  ; EASE2_T25km.gpd
	      base=25025.26000d0
	      nx=1388
	      ny=538	
	   endif
        endif else begin
	   base=25000.0d0
	   nx=10
	   ny=10
	endelse

;       grid info
	map_scale = base / m
        bcols = ceil(nx * m)
        brows = ceil(ny * m)
	r0 = (nx*m - 1) / 2.0d0
	s0 = (ny*m - 1) / 2.0d0

	END
