C
C
	SUBROUTINE LATLON2PIX(ALON,ALAT,X,Y,THELON,THELAT,
     $     IOPT,XDEG,YDEG,ASCALE,BSCALE,A0,B0)
C
C	CONVERT A LAT,LON COORDINATE (ALON,ALAT) TO AN IMAGE PIXEL LOCATION
C	(X,Y) (IN FLOATING POINT) AND IMAGE X,Y COORDINATE (THELON,THELAT).
C	TO COMPUTE INTEGER PIXEL INDICES (IX,IY): CHECK TO INSURE
C	1 <= X < NSX+1 AND 1 <= X < NSX+1 THEN IX=IFIX(X) IY=IFIX(Y)
C
C	WRITTEN BY:	DGL MARCH 1997
C
        REAL X,Y,THELON,THELAT,ALON,ALAT,XDEG,YDEG,ASCALE,BSCALE,A0,B0
        INTEGER IOPT
C
	IF (IOPT.EQ.-1) THEN		! IMAGE ONLY (CAN'T TRANSFORM!)
		X=ASCALE*(ALON-A0)+1.0
		Y=BSCALE*(ALAT-B0)+1.0
		THELON=ALON
		THELAT=ALAT
	ELSE IF (IOPT.EQ.0) THEN	! LAT/LONG (ASCALE AND BSCALE > 0)
		THELON=ALON
		THELAT=ALAT
		X=ASCALE*(THELON-A0)+1.0
		Y=BSCALE*(THELAT-B0)+1.0
	ELSE IF (IOPT.EQ.1.OR.IOPT.EQ.2) THEN	! LAMBERT (ASCALE AND BSCALE < 0)
		CALL LAMBERT1(ALAT,ALON,THELON,THELAT,YDEG,XDEG,IOPT)
		X=(THELON-A0)*ASCALE+1.0
		Y=(THELAT-B0)*BSCALE+1.0
	ELSE IF (IOPT.EQ.5) THEN	! POLAR STEREOGRAPHIC
		CALL POLSTER(ALON,ALAT,THELON,THELAT,XDEG,YDEG)
		X=(THELON-A0)/ASCALE+1.0
		Y=(THELAT-B0)/BSCALE+1.0
	ELSE IF (IOPT.EQ.11.OR.IOPT.EQ.12.OR.IOPT.EQ.13) THEN	! EASE GRID
		CALL EASEGRID(IOPT,ALAT,ALON,THELON,THELAT,ASCALE)
		THELON=THELON+XDEG
		THELAT=THELAT+YDEG
		X=THELON+1.0-(XDEG+A0)
		Y=THELAT+1.0-(YDEG+B0)
	ELSE				! UNKNOWN TRANSFORMATION TYPE
	ENDIF
C
	RETURN
	END