	PROGRAM SIRMASK
C
C	THIS PROGRAM COMPUTES A LAND MASKED SIR IMAGE.  THE RESULT IS STORED
C       IN A NEW FILE
C
C	WRITTEN BY: DGL Feb 2002
C
C       This simple program reads two BYU sir-format input files.  The
C       first is masked (i.e. pixels with zero values in mask image
C       are set to the nodata value) using the second.  The result is
C       written to a new SIR file.
C
C       Uses command line arguments to a fortran program via IARGC and GETARG
C       Not all compilers/operating systems support this.  If not supported
C       Comment out the CALL GETARG lines and the NARG=IARGC() line
C
C       READVAL1 and IREADVAL1 are I/O routines to simplify input.
C       They can be replaced with READ (*,*) statements
C
       	CHARACTER*180	FNAME,FNAME2
	CHARACTER*190	NAME,LINE
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
C	IMAGE2 FILE HEADER INFORMATION
C
	CHARACTER SENSOR2*40,TITLE2*80
        CHARACTER TAG2*40,TYPE2*138,CRPROC2*100,CRTIME2*28
        INTEGER   NSX2,NSY2,IOPT2,IDATATYPE2,IOFF2,ISCALE2
        INTEGER   NHEAD2,NDES2,NHTYPE2,LDES2,NIA2,IPOL2,IFREQHM2,ISPARE12
        INTEGER   IREGION2,ITYPE2,IYEAR2,ISDAY2,ISMIN2,IEDAY2,IEMIN2
        REAL      XDEG2,YDEG2,ASCALE2,BSCALE2,A02,B02,ANODATA2,VMIN2,VMAX2
C
C     OPTIONAL HEADER INFO
C
        PARAMETER (MAXI=256)
        CHARACTER DESCRIP*1024,DESCRIP2*1024
	DIMENSION IAOPT(MAXI),IAOPT2(MAXI)
C
C	BEGIN PROGRAM
C
	PRINT 5
 5	FORMAT(/'SIRMASK -- BYU SIR land mask program'/)
	NARG=0
C
C       compiler dependent number of arguments routine
        NARG=IARGC()  ! GET NUMBER OF COMMAND LINE ARGUMENTS
C
        IF (NARG.LT.1) THEN
           PRINT *,'Cmd line: sirmask in mask <out>'
           PRINT *,'           in:   input sir file'
           PRINT *,'           mask: input sir mask file'
           PRINT *,'           out: (optional) landmasked output file'
        ENDIF
C
        IF (NARG.GT.0) THEN
C       compiler dependent routine to fill FNAME with first run argument
           CALL GETARG(1,FNAME)
        ELSE
           PRINT *,'File to read from?'
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
	CALL SIRUPDATE(STVAL,NHTYPE,ANODATA,VMIN,VMAX)
C
C	ECHO INPUT FILE HEADER TO USER
C
	PRINT *,'Input file header information:'
	CALL PRINTHEAD3(NHEAD,NDES,NHTYPE,IDATATYPE,
     *     NSX,NSY,XDEG,YDEG,ASCALE,BSCALE,A0,B0,IOPT,IOFF,ISCALE,
     *     IXDEG_OFF,IYDEG_OFF,IDEG_SC,ISCALE_SC,IA0_OFF,IB0_OFF,IO_SC,
     *     IYEAR,ISDAY,ISMIN,IEDAY,IEMIN,IREGION,ITYPE,
     *     IPOL,IFREQHM,ISPARE1,ANODATA,VMIN,VMAX,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     DESCRIP,LDES,IAOPT,NIA)
	PRINT *
C
        IF (NARG.GT.1) THEN
C       compiler dependent routine to fill FNAME with first run argument
           CALL GETARG(2,FNAME2)
        ELSE
           PRINT *,'Mask file?'
           READ(*,15) FNAME2
        ENDIF
C
C	READ SECOND INPUT IMAGE FILE
C
        WRITE(*,16) FNAME2(1:LENGTH1(FNAME2))
	MAXDES=LEN(DESCRIP2)
        IU=1
	CALL READSIRHEAD3(FNAME2,IU,IERR,NHEAD2,NDES2,NHTYPE2,IDATATYPE2,
     *     NSX2,NSY2,XDEG2,YDEG2,ASCALE2,BSCALE2,A02,B02,IOPT2,IOFF2,ISCALE2,
     *     IXDEG_OFF2,IYDEG_OFF2,IDEG_SC2,ISCALE_SC2,IA0_OFF2,IB0_OFF2,IO_SC2,
     *     IYEAR2,ISDAY2,ISMIN2,IEDAY2,IEMIN2,IREGION2,ITYPE2,
     *     IPOL2,IFREQHM2,ISPARE12,ANODATA2,VMIN2,VMAX2,
     *     SENSOR2,TITLE2,TYPE2,TAG2,CRPROC2,CRTIME2,
     *     MAXDES,DESCRIP2,LDES2,MAXI,IAOPT2,NIA2)
C
C     READ IMAGE CONTENTS
C
	IF (IERR.GE.0) THEN
	   CALL READSIRF(IU,IERR,NHEAD2,NHTYPE2,IDATATYPE2,STVAL2,NSX2,NSY2,
     *	         IOFF2,ISCALE2,SMIN2,SMAX2,NCNT2,ANODATA2,VMIN2,VMAX2)
	ELSE
	   WRITE (*,*) '*** ERROR OPENING FILE',IERR
	ENDIF
	IF (IERR.LT.0) STOP
C
C	ECHO INPUT FILE HEADER TO USER
C
	PRINT *,'Input file header information:'
	CALL PRINTHEAD3(NHEAD2,NDES2,NHTYPE2,IDATATYPE2,
     *     NSX2,NSY2,XDEG2,YDEG2,ASCALE2,BSCALE2,A02,B02,IOPT2,IOFF2,ISCALE2,
     *     IXDEG_OFF2,IYDEG_OFF2,IDEG_SC2,ISCALE_SC2,IA0_OFF2,IB0_OFF2,IO_SC2,
     *     IYEAR2,ISDAY2,ISMIN2,IEDAY2,IEMIN2,IREGION2,ITYPE2,
     *     IPOL2,IFREQHM2,ISPARE12,ANODATA2,VMIN2,VMAX2,
     *     SENSOR2,TITLE2,TYPE2,TAG2,CRPROC2,CRTIME2,
     *     DESCRIP2,LDES2,IAOPT2,NIA2)
C
C       CHECK COMPATIBLITY OF IMAGES
C
       IF (NSX.NE.NSX2.OR.NSY.NE.NSY2.OR.A0.NE.A02.OR.B0.NE.B02.OR.
     *	   IOPT.NE.IOPT2.OR.XDEG.NE.XDEG2.OR.YDEG.NE.YDEG2.OR.
     *     ASCALE.NE.ASCALE2.OR.BSCALE.NE.BSCALE2) THEN  
           PRINT *,'*** Incompatible images ***'
           STOP
        ENDIF
C
	DO I=1,NSX*NSY
	   IF (STVAL2(I).LT.0.5) STVAL(I)=ANODATA
	END DO
C
C	OUTPUT FILE NAME
C
	IF (NARG.GT.2) THEN
	   CALL GETARG(3,NAME)
	ELSE
 533       PRINT 255,FNAME(1:LENGTH1(FNAME))
 255       FORMAT(' Output File Name: [def=',A,'.lmsk] ',$)
           READ(*,260) NAME
        ENDIF
        IF (NAME.EQ.' ') NAME=FNAME(1:LENGTH1(FNAME))//'.lmsk'
260	FORMAT(A80)
C
	IF (NAME(1:4).EQ.'NONE') STOP
C
C     WRITE OUT SIR FILE
C
        PRINT *,'Writing to ',NAME(1:LENGTH1(NAME))
	CALL WRITESIR3(NAME,40,IERR,NHEAD,NDES,NHTYPE,IDATATYPE,
     *     NSX,NSY,XDEG,YDEG,ASCALE,BSCALE,A0,B0,IOPT,IOFF,ISCALE,
     *     IXDEG_OFF,IYDEG_OFF,IDEG_SC,ISCALE_SC,IA0_OFF,IB0_OFF,IO_SC,
     *     IYEAR,ISDAY,ISMIN,IEDAY,IEMIN,IREGION,ITYPE,
     *     IPOL,IFREQHM,ISPARE1,ANODATA,VMIN,VMAX,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     STVAL,DESCRIP,LDES,IAOPT,NIA)
C
	IF (IERR.LT.0) THEN
	   PRINT *,'*** ERROR WRITING OUTPUT FILE ***'
	ENDIF
	STOP
	END