C
C **************************************************************************
C
C header version 2.0 interface to version 3.0 standard sir read routine
C
C **************************************************************************
C
      subroutine readsirhead(fname,iu_in,ierr,nhead,ndes,nhtype,
     *     idatatype,
     *     nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *     iyear,isday,ismin,ieday,iemin,iregion,itype,
     *     ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *     sensor,title,type,tag,crproc,crtime,
     *     maxdes,descrip,ldes,maxi,iaopt,nia)
c
        implicit none
c
	character*(*) 	fname
	integer		iu_in,ierr,nhead,ndes,nhtype,idatatype
	integer 	nsx,nsy,iopt
	real		xdeg,ydeg,ascale,bscale,a0,b0
	integer		ioff,iscale,iyear,isday,ismin,ieday,iemin
        integer         ixdeg_off,iydeg_off,ideg_sc,iscale_sc
        integer         ia0_off,ib0_off,i0_sc,maxi,maxdes
        integer		iregion,itype,ipol,ifreqhm,ispare1,ldes,nia
	real            anodata,vmin,vmax
	character*(*)	sensor,title,type,tag,crproc,crtime,descrip
	integer		iaopt(*)
c
c     call version 3.0 read sir routine
c
        call readsirhead3(fname,iu_in,ierr,nhead,ndes,nhtype,
     *       idatatype,
     *       nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *       ixdeg_off,iydeg_off,ideg_sc,iscale_sc,ia0_off,ib0_off,
     *       i0_sc,
     *       iyear,isday,ismin,ieday,iemin,iregion,itype,
     *       ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *       sensor,title,type,tag,crproc,crtime,
     *       maxdes,descrip,ldes,maxi,iaopt,nia)
c
        if (nhtype.gt.30) then
           write (*,*) '*** readsirhead warning: file has version 3.0',
     *          ' head'
           write (*,*) '*** use readsirhead3 instead'
        endif
c
        return
        end

