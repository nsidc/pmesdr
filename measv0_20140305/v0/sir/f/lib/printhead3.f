C
C **************************************************************************
C
	SUBROUTINE PRINTHEAD3(nhead,ndes,nhtype,idatatype,
     *     nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *     ixdeg_off,iydeg_off,ideg_sc,iscale_sc,ia0_off,ib0_off,i0_sc,
     *     iyear,isday,ismin,ieday,iemin,iregion,itype,
     *     ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     DESCRIP,LDES,IAOPT,NIA)
C
C	PRINTS OUT SIR FILE HEADER (version 3)
C
	CHARACTER*(*) SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,DESCRIP
	INTEGER IAOPT(1)
C
	IF (NHTYPE.LT.16) WRITE (*,*) 'Old style header',nhtype
C
 	WRITE (*,*) 'Title:  "',TITLE(1:LENGTH1(TITLE)),'"'
	WRITE (*,*) 'Sensor: "',SENSOR(1:LENGTH1(SENSOR)),'"'
	IF (NHTYPE.GT.16) THEN
	   WRITE (*,*) 'Type:   "',TYPE(1:LENGTH1(TYPE)),'"'
	   WRITE (*,*) 'Tag:    "',TAG(1:LENGTH1(TAG)),'"'
	   WRITE (*,*) 'Creator:"',CRPROC(1:LENGTH1(CRPROC)),'"'
	   WRITE (*,*) 'Created:"',CRTIME(1:LENGTH1(CRTIME)),'"'
	ENDIF
	IF (IOPT.EQ.-1) THEN		! IMAGE ONLY
	 WRITE (*,*) 'Rectangular image-only form: ',ioff,iscale,iopt
	 WRITE (*,*) ' x,y scale   ',ascale,bscale
	 WRITE (*,*) ' x,y offsets:',a0,b0
	 WRITE (*,*) ' x,y span:   ',xdeg,ydeg
	ELSE IF (IOPT.EQ.0) THEN	! LAT/LONG
	 WRITE (*,*) 'Lat/Lon Rectangular form: ',ioff,iscale,iopt
	 WRITE (*,*) ' X and Y scales (in pixels/deg)',ascale,bscale
	 WRITE (*,*) ' Offset values:',a0,b0
	 WRITE (*,*) ' Degrees:      ',xdeg,ydeg
	ELSE IF (IOPT.EQ.1.OR.IOPT.EQ.2) THEN	! LAMBERT
	 WRITE (*,*) 'Lambert form: ',ioff,iscale,iopt
	 WRITE (*,*) ' X and Y scales (in km/pixel)',1./ascale,1./bscale
	 WRITE (*,*) ' Lower Left Corner:',a0,b0
	 WRITE (*,*) ' Center Point:     ',xdeg,ydeg
	 IF (IOPT.EQ.2) WRITE (*,*) ' Local radius used'
	ELSE IF (IOPT.EQ.5) THEN	! POLAR STEREOGRAPHIC
	 WRITE (*,*) 'Polar Sterographic form:     ',ioff,iscale,iopt
	 WRITE (*,*) ' X and Y scales (in km/pixel)',ascale,bscale
	 WRITE (*,*) ' Lower Left Corner: (km)     ',a0,b0
	 WRITE (*,*) ' Center Lon, Lat:            ',xdeg,ydeg
	ELSE IF (IOPT.EQ.11.OR.IOPT.EQ.12) THEN	! EASE NORTH OR SOUTH
	 WRITE (*,*) 'EASE polar azimuthal form: ',ioff,iscale,iopt
	 WRITE (*,*) ' A, B scales:',ascale,bscale
	 WRITE (*,*) ' Map origin (col/row):',a0,b0
	 WRITE (*,*) ' Map center (col/row):',xdeg,ydeg
	ELSE IF (IOPT.EQ.13) THEN	! EASE CYLINDRICAL
	 WRITE (*,*) 'EASE cylindrical form: ',ioff,iscale,iopt
	 WRITE (*,*) ' A, B scales:',ascale,bscale
	 WRITE (*,*) ' Map origin (col/row):',a0,b0
	 WRITE (*,*) ' Map center (col/row):',xdeg,ydeg
	ELSE				! UNKNOWN TRANSFORMATION
	 WRITE (*,*) '*** Unrecognized SIR file form ***',ioff,iscale,iopt
	 WRITE (*,*) ' a,b scale   ',ascale,bscale
	 WRITE (*,*) ' a,b offsets:',a0,b0
	 WRITE (*,*) ' x,y span:   ',xdeg,ydeg
	ENDIF
	WRITE (*,*) ' Pixels:       ',nsx,' x ',nsy
	WRITE (*,*) ' Year:  ',iyear,'  Region: ',iregion
	WRITE (*,*) ' Start: ',isday,ismin,' End:   ',ieday,iemin
	WRITE (*,*) ' Type:  ',itype,'  Transform option: ',iopt
	IF (NHTYPE.GT.16) THEN
	   IF (IPOL.EQ.1) THEN
	      WRITE (*,*) ' Frequency:  ',ifreqhm*0.1,' GHz  H Pol '
	   ELSE IF (IPOL.EQ.2) THEN
	      WRITE (*,*) ' Frequency:  ',ifreqhm*0.1,' GHz  V Pol '
	   ELSE
	      WRITE (*,*) ' Frequency:  ',ifreqhm*0.1,' GHz  Pol: ',ipol
	   ENDIF
	   WRITE (*,*) ' Data:  ',idatatype,'  Header: ',nhead,nhtype,ndes
	   WRITE (*,*) ' Offset:',ioff,'  Scale:',iscale
	   WRITE (*,*) ' Head scale factors:',ixdeg_off,iydeg_off,ideg_sc,
     $		iscale_sc,ia0_off,ib0_off,i0_sc
	   WRITE (*,*) ' Nodata:',anodata,'  Vn: ',vmin,'  Vx:',vmax
	   IF (LDES.GT.0) WRITE (*,*) 'Description: "',DESCRIP(1:LDES),'"'
	   IF (NIA.GT.0) THEN
	      DO I=1,NIA
		 WRITE (*,*) ' Aopt: ',i,' ',iaopt(i)
	      END DO
	   ENDIF
	ENDIF
	RETURN
	END
