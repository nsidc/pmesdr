C
C *********************************************************************
C
C	PROJECTION TRANSFORMATION ROUTINES
C
C *********************************************************************
C
	SUBROUTINE LAMBERT1(LAT,LON,X,Y,ORGLAT,ORGLON,IOPT)
C
C	COMPUTES THE TRANSFORMATION FROM LAT/LON TO X/Y FOR THE
C	LAMBERT AZIMUTHAL EQUAL-AREA PROJECTION
C
C	SEE "MAP PROJECTIONS USED BY THE U.S. GEOLOGICAL SURVEY"
C	GEOLOGICAL SURVEY BULLETIN 1532, PGS 157-173
C
C	FOR THIS ROUTINE, A SPHERICAL EARTH IS ASSUMED FOR THE PROJECTION.
C	THE ERROR WILL BE SMALL FOR SMALL-SCALE MAPS.  
C	FOR IOPT=1 A FIXED, NOMINAL EARTH RADIUS IS USED.
C	FOR IOPT=2 THE LOCAL RADIUS OF THE EARTH IS USED BASED ON
C	THE 1972 WGS ELLIPSOID MODEL (BULLETIN PG 15).
C
C	WRITTEN BY: DGL 7 FEB 1992
C	MODIFIED:   DGL 4 MAR 1995
C	+ INCLUDED RADIUS OF THE EARTH COMPUTED AT LOCAL REFERENCE POINT
C
C	INPUTS:
C	 LAT	(R): LATITUDE +90 TO -90 DEG WITH NORTH POSITIVE
C	 LON	(R): LONGITUDE 0 TO +360 DEG WITH EAST POSITIVE
C			OR -180 TO +180 WITH EAST MORE POSITIVE
C	 ORGLAT	(R): ORIGIN PARALLEL +90 TO -90 DEG WITH NORTH POSITIVE
C	 ORGLON	(R): CENTRAL MERIDIAN (LONGITUDE) 0 TO +360 DEG
C			OR -180 TO +180 WITH EAST MORE POSITIVE
C	 IOPT	(I): EARTH RADIUS OPTION
C
C	OUTPUTS:
C	 X,Y	(R): RECTANGULAR COORDINATES IN KM
C
	IMPLICIT NONE
	REAL LAT, LON, X, Y, ORGLAT, ORGLON
	INTEGER  IOPT
	REAL LON1, ORGLON1, DENOM, AK
	REAL F, ERADEARTH, RADEARTH, ERA
C
	DATA RADEARTH/6378.135/, F/298.26/	! EQUITORIAL EARTH RADIUS, 1/F
C						! WGS 72 MODEL VALUES
	LON1=MOD(LON+720.0,360.0)
	ORGLON1=MOD(ORGLON+720.0,360.0)
C
C	COMPUTE LOCAL RADIUS OF THE EARTH AT CENTER OF IMAGE
C
	ERADEARTH=6378.0			! USE FIXED NOMINAL VALUE
	IF (IOPT.EQ.2) THEN			! LOCAL RADIUS
	  ERA=(1.-1./F)
	  ERADEARTH=RADEARTH*ERA/SQRT(ERA*ERA*COSD(ORGLAT)**2+
     *         SIND(ORGLAT)**2)
	ENDIF
C
	DENOM=1.0+SIND(ORGLAT)*SIND(LAT)+
     1		COSD(ORGLAT)*COSD(LAT)*COSD(LON1-ORGLON1)
	IF (DENOM.GT.0.0) THEN
		AK=SQRT(2.0/DENOM)
	ELSE
		WRITE(*,*) '*** DIVISION ERROR IN LAMBERT1 ROUTINE ***'
		AK=1.0
	ENDIF
	X=AK*COSD(LAT)*SIND(LON1-ORGLON1)
	Y=AK*(COSD(ORGLAT)*SIND(LAT)-
     1		SIND(ORGLAT)*COSD(LAT)*COSD(LON1-ORGLON1))
	X=X*ERADEARTH
	Y=Y*ERADEARTH
	RETURN
	END