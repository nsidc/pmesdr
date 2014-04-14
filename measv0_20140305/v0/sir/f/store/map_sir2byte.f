      program map_sir2byte
c
c     Writes map outline onto image and writes out as a byte
c
c     Input:  SIR image
c             map file
c     Output: sir images
c
c     Written by: David Long 11 Nov 2000
c
      character*180 fname1,fname2,line,mapfilename
c
      parameter (maxsize=35000000)
c
      dimension stval(maxsize)
      byte b(maxsize)
C
C	IMAGE FILE HEADER INFORMATION
C
	CHARACTER SENSOR*40,TITLE*80
        CHARACTER TAG*40,TYPE*138,CRPROC*100,CRTIME*28
	INTEGER   IXDEG_OFF2,IYDEG_OFF2,IDEG_SC2,ISCALE_SC2
	INTEGER   IA0_OFF2,IB0_OFF2,I0_OFF2
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
        integer argn,narg
C
        PRINT *,'BYU SIR map sir to byte Program'
C
C       CHECK FOR COMMAND LINE OPTIONS
C
        argn=0
        NARG=IARGC()            ! NUMBER OF COMMAND LINE ARGUMENTS
        IF (NARG.EQ.1) THEN
           CALL GETARG(1,LINE)
           IF (LINE(1:1).EQ.'?') THEN
              PRINT *
              PRINT *,'Command line options:'
              PRINT *,' 1st arg: SIR input file name'
              PRINT *,' 2th arg: map file name'
              STOP
           ENDIF
        ENDIF
C
        IF (NARG.GT.argn) THEN
           CALL GETARG(argn+1,FNAME1)
           PRINT *,'Reading from file "',FNAME1(1:length(FNAME1)),'"'
           argn=argn+1
        ELSE
           PRINT *
           PRINT *,'What SIR image file to read from?'
           READ(*,15) FNAME1
 15        FORMAT(A180)
        ENDIF
C
C	READ INPUT IMAGE FILE
C
        WRITE(*,16) FNAME1(1:LENGTH(FNAME1))
16      FORMAT(' Reading file "',A,'"')
	MAXDES=LEN(DESCRIP)
        iu=10
	CALL READSIRHEAD3(FNAME1,IU,IERR,NHEAD,NDES,NHTYPE,IDATATYPE,
     *       NSX,NSY,XDEG,YDEG,ASCALE,BSCALE,A0,B0,IOPT,IOFF,ISCALE,
     *       IXDEG_OFF2,IYDEG_OFF2,IDEG_SC2,ISCALE_SC2,IA0_OFF2,
     *       IB0_OFF2,I0_OFF2,
     *       IYEAR,ISDAY,ISMIN,IEDAY,IEMIN,IREGION,ITYPE,
     *       IPOL,IFREQHM,ISPARE1,ANODATA,VMIN,VMAX,
     *       SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *       MAXDES,DESCRIP,LDES,MAXI,IAOPT,NIA)
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
	CALL PRINTHEAD(NHEAD,NDES,NHTYPE,IDATATYPE,
     *     NSX,NSY,XDEG,YDEG,ASCALE,BSCALE,A0,B0,IOPT,IOFF,ISCALE,
     *     IYEAR,ISDAY,ISMIN,IEDAY,IEMIN,IREGION,ITYPE,
     *     IPOL,IFREQHM,ISPARE1,ANODATA,VMIN,VMAX,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     DESCRIP,LDES,IAOPT,NIA)
        PRINT *
C
C     GET SEED LOCATION
C
        IF (NARG.GT.argn) THEN
           CALL GETARG(argn+1,mapfilename)
           argn=argn+1
        ELSE
           PRINT *
           PRINT *,'What mapfile? '
           READ(*,15) mapfilename
        ENDIF
        PRINT *,'Using map ',mapfilename(1:length(mapfilename))
C
C     MAKE SURE IMAGE IS STRICTLY NEGATIVE
C
        DO I=1,NSX*NSY
           IF (STVAL(I).GE.0.0) STVAL(I)=-1.0
        END DO
C       
C	DETERMINE MAX/MIN OF DATA ARRAY
C
	SMIN=1.E25
	SMAX=-1.E25
        do I=1,NSX*NSY
           IF (STVAL(I).NE.ANODATA) THEN
              SMIN=MIN(SMIN,STVAL(I))
              SMAX=MAX(SMAX,STVAL(I))
           ENDIF
	END DO
C
	SMIN1=SMIN
	SMAX1=SMAX
	IF (ANINT(SMIN).EQ.ANINT(SMAX)) THEN
	   SMIN=AINT(SMIN)
	   SMAX=SMIN+1.0
	ENDIF
	SMIN=ANINT(SMIN)
	SMAX=ANINT(SMAX)
	PRINT *
	PRINT *,'Array min, max values:',SMIN1,SMAX1
        smin=-33
        smax=0
	PRINT *,'Will use Min, Max:',SMIN,SMAX
c    	PRINT 34,'Max?'
c34	FORMAT(1X,'New ',A,1X,$)
c	SMAX = READVAL1(SMAX,IERR)
c	PRINT 34,'Min?'
c	SMIN = READVAL1(SMIN,IERR)
c
c     drap land map onto image
c
        print *,'Processing map ',mapfilename
        call drawmap(stval,nsx,nsy,mapfilename,
     $       a0,b0,ascale,bscale,xdeg,ydeg,iopt,1.0)
	SMIN1=1.E25
	SMAX1=-1.E25
        do I=1,NSX*NSY
           IF (STVAL(I).NE.ANODATA) THEN
              SMIN1=MIN(SMIN1,STVAL(I))
              SMAX1=MAX(SMAX1,STVAL(I))
           ENDIF
	END DO
c	PRINT *,'new Min, Max:',SMIN1,SMAX1
C
C	SCALE ARRAY AND PUT INTO BYTE ARRAY
C
	SS=(SMAX-SMIN)/255.0
	IF (SS.GT.0.0) THEN
	   SS=1.0/SS
	ELSE
	   SS=1.0
	ENDIF
C
	DS=SS
C
        imin=999
        imax=-999
C
        DO NN1=0,NSY-1
           DO NN2=0,NSX-1
              INDEXV = NN1*NSX+NN2+1
              Z = STVAL(INDEXV)
              VAL=(Z-SMIN)*DS
              IVAL=ANINT(VAL)
              IF (IVAL.GT.255) IVAL=255
              IF (IVAL.LT.0)   IVAL=0
              IF (IVAL.GT.127) IVAL=IVAL-256
              I = (NSY-NN1)*NSX + NN2 + 1
              B(I)=IVAL
              imin=min(imin,ival)
              imax=max(imax,ival)
           END DO
        END DO
        print *,'min,max',imin,imax
c
c     strip path name off of input file
c
        i=index(fname1,'/')
        do while(i.gt.0)
           fname1=fname1(i+1:)
           i=index(fname1,'/')
        end do
c
c     write out sir file
c
        fname2=fname1(1:length(fname1))//'.raw'
C
C	NOW WRITE OUT BYTE FILE
C
	NS=NSX*NSY
C
	OPEN(UNIT=1,FILE=fname2,STATUS='NEW',FORM='UNFORMATTED',recl=ns,access='direct')
C
	WRITE (1,rec=1) (B(I),I=1,NS)
        CLOSE (1)
        WRITE(*,*) 'Output file: ',fname2(1:length(fname2))
	WRITE(*,*) 'Output Image Size: (X x Y)',NSX,NSY
c
        stop
        end
c
c
      subroutine drawmap(stval,nsx,nsy,mapname,
     $       a0,b0,ascale,bscale,xdeg,ydeg,iopt,value)
c
      dimension stval(*)
      character*(*) mapname
C
      parameter (MAX_IADD=10000)
      integer iadd(MAX_IADD)
c
      OPEN(UNIT=2,FILE=mapname,FORM='FORMATTED',STATUS='OLD',ERR=2031)
c
      do while(.true.)
         READ (2,2011,END=2030) ALAT,ALON,IPEN,IFLAG
 2011    FORMAT(1X,F7.3,1X,F8.3,1X,I1,1X,I2)
c         IF (IFLAG.EQ.1.OR.IFLAG.EQ.13) THEN 
         IF (IFLAG.ne.0)  THEN   ! draw all data 
            IF (ALON.GT.360) ALON=ALON-360.0
            IF (ALON.GT.180.0) ALON=ALON-360.0
            if (iopt.eq.1.or.iopt.eq.2) then ! lambert projection
c               IF (ALON.LT.0) ALON=ALON+360.0
            endif
C     
            IF (IPEN.EQ.2.AND.IPEN1.EQ.3) IPEN=3
            IF (IPEN.EQ.3) IPEN1=3
            IF (IPEN.EQ.3) THEN
               PREVLON=ALON
               PREVLAT=ALAT
            ENDIF
            IF (IPEN1.NE.3) THEN
               CALL LATLON2PIX(PREVLON,PREVLAT,X1,Y1,THELON1,THELAT1,
     $              IOPT,XDEG,YDEG,ASCALE,BSCALE,A0,B0)
               CALL LATLON2PIX(ALON,ALAT,X2,Y2,THELON1,THELAT1,
     $              IOPT,XDEG,YDEG,ASCALE,BSCALE,A0,B0)
               CALL lineoutf(X1,Y1,X2,Y2,5.0,NSX,NSY,
     $              IBNT,IADD,MAX_IADD)
               do i=1,ibnt
                  stval(iadd(i))=value
               end do
            ENDIF
            PREVLON=ALON
            PREVLAT=ALAT
            IPEN1=2
         ELSE
            IPEN1=3
         ENDIF
      end do
 2030 CONTINUE
      CLOSE(2)
      return
 2031 CONTINUE
      print *,'*** Error opening input map file ...'
      stop
      end      
C
C
	SUBROUTINE LINEOUTF(RX1,RY1,RX2,RY2,RES1,
     $		IXWIDE,IYWIDE,IBNT,IADD,MAX_IADD)
	DIMENSION IADD(1)
C
C	COMPUTE THE LIST OF PIXELS (LEXICOGRAPHIC ORDER) A ALONG THE
C	LINE FROM (RX1,RY1) TO (RX2,RY2) WITH ENHANCED RESOLUTION
C
C	RX1,RY1	(R)	START POINT OF LINE SEGMENT
C	RX2,RY2	(R)	STOP POINT OF LINE SEGMENT
C	RES	(R)	RESOLUTION ENHANCEMENT FACTOR
C	IXWIDE	(I)	X DIMENSION (0 <= IX <= IXWIDE-1)
C	IYWIDE	(I)	Y DIMENSION (0 <= IY <= IYWIDE-1)
C	IBNT	(I)	RETURNED NUMBER OF PIXELS
C	IADD	(I)	RETURNED ARRAY OF PIXEL INDEXES
C
C	LEXICOGRAPHIC INDEX IS: N=IY*IXWIDE+IX+1
C
	INTEGER DX,DY,X,Y,X1,Y1,X2,Y2,DIF,D
	LOGICAL INV
C
C        print *,rx1,ry1,rx2,ry2,res,ixwide,iywide
C
        RES=RES1
        IF (RES.LE.0.0) RES=1.0
        RESI=1.0/RES
C
	DX=NINT(ABS((RX1-RX2)*RES))
	DY=NINT(ABS((RY1-RY2)*RES))
	X1=NINT(RX1*RES)
	X2=NINT(RX2*RES)
	Y1=NINT(RY1*RES)
	Y2=NINT(RY2*RES)
	IX0=NINT(RX1*RES)
	IY0=NINT(RY1*RES)
	IX1=NINT(RX2*RES)
	IY1=NINT(RY2*RES)
	IF (X1.LT.0) X1=0
	IF (X2.LT.0) X2=0
	IF (Y1.LT.0) Y1=0
	IF (Y2.LT.0) Y2=0
	IF (X1.GE.IXWIDE*RES) X1=IXWIDE*RES-1
	IF (X2.GE.IXWIDE*RES) X2=IXWIDE*RES-1
	IF (Y1.GE.IYWIDE*RES) Y1=IYWIDE*RES-1
	IF (Y2.GE.IYWIDE*RES) Y2=IYWIDE*RES-1
	INV=.FALSE.
	IF (DX.LT.DY) THEN
		X=X1
		X1=Y1
		Y1=X
		X=X2
		X2=Y2
		Y2=X
		X=DY
		DY=DX
		DX=X
		INV=.TRUE.
		IX0=IY1
		IY0=IX1
	ENDIF
	IF (X2.LT.X1) THEN
		X=X1
		X1=X2
		X2=X
		Y=Y1
		Y1=Y2
		Y2=Y
	ENDIF
	DIF=1
	IF (Y2.LT.Y1) DIF=-1
	D=2*DY-DX
	I1=2*DY
	I2=2*(DY-DX)
	Y=Y1
	IYY=0
        LAST=-1
C
	IBNT=0
	DO 10 X=X1,X2
           if (ibnt.ge.MAX_IADD) then
              print *,'*** max size exceeded in lineout',MAX_IADD
              return
           endif

           IF (INV) THEN
              JX=NINT(float(X)*RESI)
              JY=NINT(float(Y)*RESI)
           ELSE
              JX=NINT(float(Y)*RESI)
              JY=NINT(float(X)*RESI)
           ENDIF
           if (jx.ge.0.and.jx.lt.IYWIDE.and.
     $         jy.ge.0.and.jy.lt.IXWIDE) then
               INDEX=JX*IXWIDE+JY+1
              if (index.gt.IXWIDE*IYWIDE)
     $          print *,'*** error in lineout ',index,ixwide,iywde,jx,jy
              IF (INDEX.NE.LAST) THEN
                 IBNT=IBNT+1
                 IADD(IBNT)=INDEX
              ENDIF
              LAST=INDEX
           endif
           IF (D.LT.0) THEN
              D=D+I1
           ELSE
              D=D+I2
              Y=Y+DIF
           ENDIF
10	CONTINUE
C        print *,'returning with ',ibnt
	RETURN
	END
