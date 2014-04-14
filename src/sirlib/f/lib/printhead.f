C
C **************************************************************************
C
	SUBROUTINE PRINTHEAD(nhead,ndes,nhtype,idatatype,
     *     nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *     iyear,isday,ismin,ieday,iemin,iregion,itype,
     *     ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     DESCRIP,LDES,IAOPT,NIA)
C
C	PRINTS OUT SIR FILE HEADER (version 3)
C
	CHARACTER*(*) SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,DESCRIP
	INTEGER IAOPT(1)
c
	call PRINTHEAD3(nhead,ndes,nhtype,idatatype,
     *     nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *     ixdeg_off,iydeg_off,ideg_sc,iscale_sc,ia0_off,ib0_off,i0_sc,
     *     iyear,isday,ismin,ieday,iemin,iregion,itype,
     *     ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *     SENSOR,TITLE,TYPE,TAG,CRPROC,CRTIME,
     *     DESCRIP,LDES,IAOPT,NIA)
	RETURN
	END
