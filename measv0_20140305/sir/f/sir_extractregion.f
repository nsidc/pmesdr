	PROGRAM SIR_EXTRACTREGION
C
C	THIS PROGRAM EXTRACTS A SUB REGION FROM A SIR FILE AND SAVES
C	THE RESULTING SUB IMAGE IN A NEW SIR FILE
C
C	WRITTEN BY: DGL MAR 1995
C	REVISED BY: DGL Jan 2000 + new header
C	REVISED BY: DGL Feb 2002 + version 3 header
C	REVISED BY: DGL Jul 2005 + updated ease grid offset
C
C	IMAGE FILE READ INFORMATION
C
C       Uses command line arguments to a fortran program via IARGC and GETARG
C       Not all compilers/operating systems support this.  If not supported
C       Comment out the CALL GETARG lines and the NARG=IARGC() line
C
C       READVAL1 and IREADVAL1 are I/O routines to simplify input.
C       They can be replaced with READ (*,*) statements
C
       	CHARACTER*180	FNAME
	CHARACTER*190	NAME,LINE
	CHARACTER TITLE2*80
        LOGICAL LATLON
C
C	IMAGE DISPLAY VARIABLES
C
	PARAMETER (MAXSIZE=16000000)
	REAL 		STVAL(MAXSIZE)
	REAL 		STVAL2(MAXSIZE)
C
C	IMAGE FILE HEADER INFORMATION
C
	CHARACTER SENSOR*40,TITLE*80
        CHARACTER TAG*40,TYPE*138,CRPROC*100,CRTIME*28
        INTEGER   NSX,NSY,IOPT,IDATATYPE,IOFF,ISCALE
        INTEGER   NHEAD,NDES,NHTYPE,LDES,NIA,IPOL,IFREQHM,ISPARE1
        INTEGER   IREGION,ITYPE,IYEAR,ISDAY,ISMIN,IEDAY,IEMIN
        REAL      XDEG,YDEG,ASCALE,BSCALE,A0,B0,ANODATA,VMIN,VMAX
C
C     OPTIONAL HEADER INFO
C
        PARAMETER (MAXI=256)
        CHARACTER DESCRIP*1024
	DIMENSION IAOPT(MAXI)
C
C	BEGIN PROGRAM
C
	PRINT 5
 5	FORMAT(/'SIR_ExtractRegion -- extract subregion from SIR file '/)
	NARG=0
C
C       compiler dependent number of arguments routine
        NARG=IARGC()  ! GET NUMBER OF COMMAND LINE ARGUMENTS
C
        IF (NARG.LT.1) THEN
           PRINT *,'Command line: sirextregion in_file P/L LLx LLy URx URy out_file'
           PRINT *,'               in_file:  input sir file'
	   print *,'               P/L: pixel (P) or lat/lon (L) coordinates'
	   print *,'               LLx,LLy: lower-left corner (L=lon,lat)'
	   print *,'               URx,URy: upper-right corner (L=lon,lat)'
           PRINT *,'               out_file: output sir file'
        ENDIF
C
        IF (NARG.GT.0) THEN
C       compiler dependent routine to fill FNAME with first run argument
           CALL GETARG(1,FNAME)
        ELSE
           PRINT *,'Source file to read from?'
           READ(*,15) FNAME
 15        FORMAT(A80)
        ENDIF
C
C	READ INPUT IMAGE FILE
C
        WRITE(*,16) FNAME(1:LENGTH1(FNAME))
16      FORMAT(' Reading file "',A,'"')
	MAXDES=LEN(DESCRIP)
        IU=1
	CALL READSIRHEAD3(FNAME,IU,IERR,NHEAD,NDES,NHTYPE,IDATATYPE,
     *     NSX,NSY,XDEG,YDEG,ASCALE,BSCALE,A0,B0,IOPT,IOFF,ISCALE,
     *     IXDEG_OFF,IYDEG_OFF,IDEG_SC,ISCALE_SC,IA0_OFF,IB0_OFF,IO_SC,
     *     IYEAR,ISDAY,ISMIN,IEDAY,IEMIN,IREGION,ITYPE,
     *     IPOL,IFREQHM,ISPARE1,ANODATA,VMIN,VMAX,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     MAXDES,DESCRIP,LDES,MAXI,IAOPT,NIA)
C
C     READ IMAGE CONTENTS
C
	IF (IERR.GE.0) THEN
	   CALL READSIRF(IU,IERR,NHEAD,NHTYPE,IDATATYPE,STVAL,NSX,NSY,
     *	         IOFF,ISCALE,SMIN,SMAX,NCNT,ANODATA,VMIN,VMAX)
	ELSE
	   WRITE (*,*) '*** ERROR OPENING FILE',IERR
	ENDIF
	IF (IERR.LT.0) STOP
C
C	ECHO INPUT FILE HEADER TO USER
C
	PRINT *,'Input file header information:',NFILE
	CALL PRINTHEAD3(NHEAD,NDES,NHTYPE,IDATATYPE,
     *     NSX,NSY,XDEG,YDEG,ASCALE,BSCALE,A0,B0,IOPT,IOFF,ISCALE,
     *     IXDEG_OFF,IYDEG_OFF,IDEG_SC,ISCALE_SC,IA0_OFF,IB0_OFF,IO_SC,
     *     IYEAR,ISDAY,ISMIN,IEDAY,IEMIN,IREGION,ITYPE,
     *     IPOL,IFREQHM,ISPARE1,ANODATA,VMIN,VMAX,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     DESCRIP,LDES,IAOPT,NIA)
	PRINT *
C
C	OUTPUT HEADER
C
	IF (NHTYPE.EQ.1) THEN   ! OLD STYLE HEADER
C
C	DETERMINE MAX/MIN OF DATA ARRAY
C
           SMIN=1.E25
           SMAX=-1.E25
           DO NN1=0,NSY-1
              DO NN2=1,NSX
                 INDEX = NN1*NSX + NN2
                 SMIN=MIN(SMIN,STVAL(INDEX))
                 SMAX=MAX(SMAX,STVAL(INDEX))
              END DO
           END DO
	   ANODATA=SMIN
	   VMIN=SMIN
	   VMAX=SMAX
	   IF (ABS(SMIN+32.0).LT.2) THEN
	      ANODATA=-32.0
	      VMIN=-32.0
	   ELSE IF (ABS(SMIN+3.2).LT.2) THEN
	      ANODATA=-3.2
	      VMIN=-3.0
	   ENDIF
	   IF (ABS(SMAX).LT.2) THEN
	      VMAX=0.0
	   ENDIF
	ENDIF
C
        LATLON=.TRUE.
	IF (NARG.GT.2) THEN
	   CALL GETARG(2,LINE)
           IF (LINE(1:1).EQ.'P'.OR.LINE(1:1).EQ.'p') LATLON=.FALSE.
	ELSE
	   PRINT 514
 514	   FORMAT('Pixel (P) or Lat/Lon (L [def]) coordinates: ',$)
           READ(*,15) LINE
           IF (LINE(1:1).EQ.'P'.OR.LINE(1:1).EQ.'p') LATLON=.FALSE.
	ENDIF
C
        IF (LATLON) THEN
C     
           IF (NARG.GT.3) THEN
              CALL GETARG(3,LINE)
              READ(LINE,'(F)') ALON1
           ELSE
              PRINT 516
 516          FORMAT('Lower-left corner longitude (X) ',$)
C following can be replaced with READ(*,*) ALON1
              ALON1=READVAL1(ALON1,IEND)
              IF (IEND.LT.0) STOP
           ENDIF
C     
           IF (NARG.GT.4) THEN
              CALL GETARG(4,LINE)
              READ(LINE,'(F)') ALAT1
           ELSE
              PRINT 515
 515          FORMAT('Lower-left corner latitude (Y) ',$)
C following can be replaced with READ(*,*) ALA1
              ALAT1=READVAL1(ALAT1,IEND)
              IF (IEND.LT.0) STOP
           ENDIF
C     
           IF (NARG.GT.5) THEN
              CALL GETARG(5,LINE)
              READ(LINE,'(F)') ALON2
           ELSE
              PRINT 518
 518          FORMAT('Upper-right corner longitude (X) ',$)
C following can be replaced with READ(*,*) ALON2
              ALON2=READVAL1(ALON2,IEND)
              IF (IEND.LT.0) STOP
           ENDIF
C
           IF (NARG.GT.6) THEN
              CALL GETARG(6,LINE)
              READ(LINE,'(F)') ALAT2
           ELSE
              PRINT 517
 517          FORMAT('Upper-right corner latitude (Y) ',$)
C following can be replaced with READ(*,*) ALAT2
              ALAT2=READVAL1(ALAT2,IEND)
              IF (IEND.LT.0) STOP
           ENDIF
C
C	CONVERT LAT, LON CORNERS TO IMAGE PIXELS
C
           CALL LATLON2PIX(ALON1,ALAT1,X1,Y1,THELON,THELAT,
     $          IOPT,XDEG,YDEG,ASCALE,BSCALE,A0,B0)
           CALL F2IPIX(X1,Y1,IX1,IY1,NSX,NSY)
           PRINT *,'Input lon,lat: ',ALON1,ALAT1,' -> ',IX1,IY1
           IF (IX1.EQ.0.OR.IY1.EQ.0) THEN
              PRINT *,'** POINT OUTSIDE OF IMAGE ***'
              IX1=MIN(1,IX1)
              IY1=MIN(1,IY1)
           ENDIF
C     
           CALL LATLON2PIX(ALON2,ALAT2,X2,Y2,THELON,THELAT,
     $          IOPT,XDEG,YDEG,ASCALE,BSCALE,A0,B0)
           CALL F2IPIX(X2,Y2,IX2,IY2,NSX,NSY)
           PRINT *,'Input lon,lat: ',ALON2,ALAT2,' -> ',IX2,IY2
           IF (IX2.EQ.0.OR.IY2.EQ.0) THEN
              PRINT *,'** POINT OUTSIDE OF IMAGE ***'
              IX2=MIN(1,IX2)
              IY2=MIN(1,IY2)
           ENDIF
C
        ELSE
C     
           IF (NARG.GT.3) THEN
              CALL GETARG(3,LINE)
              READ(LINE,'(I)') IX1
           ELSE
              PRINT 1516
 1516         FORMAT('Lower-left corner pixel (X) ',$)
C following can be replaced with READ(*,*) IX1
              IX1=IREADVAL1(1,IEND)
              IF (IEND.LT.0) STOP
           ENDIF
C
           IF (NARG.GT.4) THEN
              CALL GETARG(4,LINE)
              READ(LINE,'(I)') IY1
           ELSE
              PRINT 1515
 1515         FORMAT('Lower-left corner pixel (Y) ',$)
C following can be replaced with READ(*,*) IY1
              IY1=IREADVAL1(1,IEND)
              IF (IEND.LT.0) STOP
           ENDIF
C     
           IF (NARG.GT.5) THEN
              CALL GETARG(5,LINE)
              READ(LINE,'(I)') IX2
           ELSE
              PRINT 1518
 1518         FORMAT('Upper-right corner pixel (X) ',$)
C following can be replaced with READ(*,*) IX2
              IX2=IREADVAL1(NSX,IEND)
              IF (IEND.LT.0) STOP
           ENDIF
C     
           IF (NARG.GT.6) THEN
              CALL GETARG(6,LINE)
              READ(LINE,'(I)') IY2
           ELSE
              PRINT 1517
 1517         FORMAT('Upper-right corner pixel (Y) ',$)
C following can be replaced with READ(*,*) IY2
              IY2=IREADVAL1(NSY,IEND)
              IF (IEND.LT.0) STOP
           ENDIF
        ENDIF
C
C       CLIP SUBREGION TO BE WITHIN IMAGE AND SORT CORNERS
C
	IF (IX1.LT.1) IX1=1
	IF (IY1.LT.1) IY1=1
	IF (IX2.LT.1) IX2=1
	IF (IY2.LT.1) IY2=1
C
	IF (IX1.GT.NSX) IX1=NSX
	IF (IY1.GT.NSY) IY1=NSY
	IF (IX2.GT.NSX) IX2=NSX
	IF (IY2.GT.NSY) IY2=NSY
C
	IX=MIN(IX1,IX2)
	IX2=MAX(IX1,IX2)
	IX1=IX
	IY=MIN(IY1,IY2)
	IY2=MAX(IY1,IY2)
	IY1=IY
	NSX2=IX2-IX1+1
	NSY2=IY2-IY1+1
C
	X=IX1
	Y=IY1
	CALL PIX2LATLON(X,Y,THELON,THELAT,ALON,ALAT,
     $     IOPT,XDEG,YDEG,ASCALE,BSCALE,A0,B0)
C
	X=IX2
	Y=IY2
	CALL PIX2LATLON(X,Y,THELON,THELAT,ALON2,ALAT2,
     $     IOPT,XDEG,YDEG,ASCALE,BSCALE,A0,B0)
C
	PRINT *,'Image sizes:	    (in)  ',NSX,NSY,'  (out) ',NSX2,NSY2
	PRINT *,'Lower-Left pixel location: ',IX1,IY1,' Lon, Lat: ',ALON,ALAT
	PRINT *,'Upper-Right pixel location:',IX2,IY2,' Lon, Lat: ',ALON2,ALAT2
C
	IF (NSX2.LT.1.OR.NSY2.LT.1) THEN
	   PRINT*,'*** Error Extracting SubRegion: ZERO SIZE ',NSX2,NSY2
	   STOP
	ENDIF
C
C	COMPUTE IMAGE PROJECTION HEADER INFORMATION FOR SUB IMAGE
C
        XDEG2=XDEG
        YDEG2=YDEG
	IF (IOPT.EQ.1.OR.IOPT.EQ.2) THEN ! LAMBERT
	   A02=(IX1-1)/ASCALE+A0
	   B02=(IY1-1)/BSCALE+B0
	ELSE IF (IOPT.EQ.11.OR.IOPT.EQ.12.OR.IOPT.EQ.13) THEN ! EASE GRID
	   A02=A0+(IX1-1)
	   B02=B0+(IY1-1)
	ELSE IF (IOPT.EQ.5) THEN ! POLAR STEREOGRAPHIC
	   A02=(IX1-1)*ASCALE+A0
	   B02=(IY1-1)*BSCALE+B0
	ELSE			! IMAGE ONLY, LAT/LON, UNKNOWN
           A02=(IX1-1)*XDEG/FLOAT(NSX)+A0
           B02=(IY1-1)*YDEG/FLOAT(NSY)+B0
           XDEG2=FLOAT(NSX2)*XDEG/FLOAT(NSX)
           YDEG2=FLOAT(NSY2)*YDEG/FLOAT(NSY)
	ENDIF
	TITLE2='SubReg: '//TITLE(1:LENGTH1(TITLE))
	TITLE=TITLE2
C
C	COPY DATA FROM SUB REGION INTO STORAGE AREA TO CREATE NEW IMAGE
C
	DO IY=1,NSY2
	   DO IX=1,NSX2
	      I1=(IY-1)*NSX2+IX
	      I2=(IY-1+IY1-1)*NSX+(IX+IX1-1)
	      STVAL2(I1)=STVAL(I2)
	   END DO
	END DO
C
C	OUTPUT FILE NAME
C
	IF (NARG.GE.7) THEN
	   CALL GETARG(7,NAME)
	ELSE
 533       PRINT 255,FNAME(1:LENGTH1(FNAME))
 255       FORMAT(' Output File Name: [def=',A,'.SUB] ',$)
           READ(*,260) NAME
        ENDIF
        IF (NAME.EQ.' ') NAME=FNAME(1:LENGTH1(FNAME))//'.SUB'
260	FORMAT(A80)
C
C       NEW HEADER
C
        NHTYPE=20       ! SET HEADER TYPE
        IDATATYPE=2     ! MAKE SURE OUTPUT IMAGE IS IN STANDARD I*2 FORM
C
	NSX=NSX2
	NSY=NSY2
	A0=A02
	B0=B02
	XDEG=XDEG2
	YDEG=YDEG2
C
C     WRITE OUT SIR FILE
C
        PRINT *,'Writing output SIR file to ',NAME(1:LENGTH1(NAME))
	CALL WRITESIR3(NAME,40,IERR,NHEAD,NDES,NHTYPE,IDATATYPE,
     *     NSX,NSY,XDEG,YDEG,ASCALE,BSCALE,A0,B0,IOPT,IOFF,ISCALE,
     *     IXDEG_OFF,IYDEG_OFF,IDEG_SC,ISCALE_SC,IA0_OFF,IB0_OFF,IO_SC,
     *     IYEAR,ISDAY,ISMIN,IEDAY,IEMIN,IREGION,ITYPE,
     *     IPOL,IFREQHM,ISPARE1,ANODATA,VMIN,VMAX,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     STVAL2,DESCRIP,LDES,IAOPT,NIA)
C
	IF (IERR.LT.0) THEN
	   PRINT *,'*** ERROR WRITING OUTPUT FILE ***'
	ENDIF
	STOP
	END
