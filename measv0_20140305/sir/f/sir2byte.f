	PROGRAM SIR2BYTE
C
C	THIS PROGRAM ILLUSTRATES THE READING OF A FILE IN THE BYU MERS SIR
C       FILE FORMAT.  IT READS THE FILE AND
C       WRITES OUT A BYTE-ARRAY COPY OF THE IMAGE
C
C	WRITTEN BY D.LONG: MARCH 1997
C       REVIZED BY D.LONG: Nov. 2000  +Version 3.0 header
C
C       Link with SIR fortran library routines
C
        CHARACTER*70 FNAME,ONAME
C
C	IMAGE STORAGE VARIABLES
C
	PARAMETER (MAXSIZE=4500000)       ! MAXIMUM PIXELS
	REAL 	STVAL(MAXSIZE)            ! READ ARRAY
	BYTE	B(MAXSIZE)                ! OUTPUT BYTE ARRAY
C
C	IMAGE FILE HEADER INFORMATION
C
	CHARACTER SENSOR*40,TITLE*80
        CHARACTER TAG*40,TYPE*138,CRPROC*100,CRTIME*28
        INTEGER   NSX,NSY,IOPT,IDATATYPE,IOFF,ISCALE
        INTEGER   IXDEG_OFF,IYDEG_OFF,IDEG_SC,ISCALE_SC,IA0_OFF,IB0_OFF,IO_SC
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
C	PROMPT USER FOR FILE NAME INPUT
C
	WRITE(*,*)
	WRITE(*,15)
15	FORMAT(' Enter Input SIR File Name: ',$)
	READ(*,20) FNAME
20	FORMAT(A70)
C
40	CONTINUE
	WRITE(*,*) 'SIR In= "',FNAME(1:LENGTH1(FNAME)),'"'
C
C	READ SIR FILE HEADER
C
	MAXDES=LEN(DESCRIP)
	IU = 10
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
C	WRITE OUT IMAGE HEADER INFORMATION
C
	WRITE (*,*)
	WRITE (*,*) 'SIR File Header Information:'
	CALL PRINTHEAD3(NHEAD,NDES,NHTYPE,IDATATYPE,
     *     NSX,NSY,XDEG,YDEG,ASCALE,BSCALE,A0,B0,IOPT,IOFF,ISCALE,
     *     IXDEG_OFF,IYDEG_OFF,IDEG_SC,ISCALE_SC,IA0_OFF,IB0_OFF,IO_SC,
     *     IYEAR,ISDAY,ISMIN,IEDAY,IEMIN,IREGION,ITYPE,
     *     IPOL,IFREQHM,ISPARE1,ANODATA,VMIN,VMAX,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     DESCRIP,LDES,IAOPT,NIA)
	WRITE (*,*)
C
C	DETERMINE MAX/MIN OF DATA ARRAY
C
	SMIN=1.E25
	SMAX=-1.E25
	DO NN1=0,NSY-1
		DO NN2=1,NSX
			INDEX = NN1*NSX + NN2
			IF (STVAL(INDEX).NE.ANODATA) THEN
				SMIN=MIN(SMIN,STVAL(INDEX))
				SMAX=MAX(SMAX,STVAL(INDEX))
			ENDIF
		END DO
	END DO
C
	SMIN1=SMIN
	SMAX1=SMAX
C
	WRITE (*,*)
	WRITE (*,*) 'Array min, max values:',SMIN1,SMAX1
C
    	WRITE (*,34) 'Saturation Max?'
34	FORMAT(X,'New ',A,X,$)
        READ (*,*) SMAX
	WRITE (*,34) 'Saturation Min?'
        READ (*,*) SMIN
C
C	SCALE ARRAY AND PUT INTO BYTE ARRAY
C
C     NOTE THAT SIR IMAGE DATA HAS THE ORIGIN AT THE LOWER-LEFT OF THE
C     IMAGE.  THE BYTE ARRAY HAS THE ORIGIN AT THE UPPER-LEFT.
C
	SS=(SMAX-SMIN)/255.0
	IF (SS.GT.0.0) THEN
		SS=1.0/SS
	ELSE
		SS=1.0
	ENDIF
	DO NN1=0,NSY-1
		DO NN2=1,NSX
			INDEX = NN1*NSX + NN2
			INDEX2= (NSY-NN1)*NSX + NN2
			VAL=STVAL(INDEX)
			IVAL=(VAL-SMIN)*SS
			IF (IVAL.LT.0) IVAL=0
			IF (IVAL.GT.255) IVAL=255
			IF (IVAL.GT.127) IVAL=IVAL-256
			B(INDEX2)=IVAL
		END DO
	END DO
C
C     GET OUTPUT FILE NAME FOR BYTE FILE
C
	WRITE(*,25)
25	FORMAT(' Enter BYTE Output File Name: ',$)
	READ(*,20) ONAME
	WRITE(*,*) 'BYTE Out="',ONAME(1:LENGTH1(ONAME)),'"'
C
C	NOW WRITE OUT BYTE FILE
C
	WRITE(*,*) 'Output Image Size: (X x Y)',NSX,NSY
	NS=NSX*NSY
C
	OPEN(UNIT=1,FILE=ONAME,STATUS='NEW',FORM='UNFORMATTED')
C
	WRITE (1) (B(I),I=1,NS)
        CLOSE (1)
C
360	STOP
	END