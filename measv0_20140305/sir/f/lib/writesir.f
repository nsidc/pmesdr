c
c *************************************************************************
c
c  header version 2.0 interface to version 3.0 standard sir write routine
c
      subroutine writesir(fname,iu_in,ierr,nhead,ndes,nhtype,idatatype,
     *     nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *     iyear,isday,ismin,ieday,iemin,iregion,itype,
     *     ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *     sensor,title,type,tag,crproc,crtime,
     *     stval,descrip,ldes,iaopt,nia)
c
        implicit none
c
	character*(*) 	fname
	integer		iu_in,ierr,nhead,ndes,nhtype,idatatype
	integer 	nsx,nsy,iopt
	real		xdeg,ydeg,ascale,bscale,a0,b0
	integer		ioff,iscale,iyear,isday,ismin,ieday,iemin
        integer		iregion,itype,ipol,ifreqhm,ispare1,ldes,nia
	real            anodata,vmin,vmax
	real            stval(*)
	character*(*)	sensor,title,type,tag,crproc,crtime,descrip
	integer		iaopt(*)
c
        integer         ixdeg_off,iydeg_off,ideg_sc,iscale_sc
        integer         ia0_off,ib0_off,i0_sc
c
c     set version 3.0 parameters to header version 2.0 defaults
c
        if (iopt.eq.-1) then    ! image only
           ideg_sc=10
           iscale_sc=1000
           i0_sc=100
           ixdeg_off=0
           iydeg_off=0
           ia0_off=0
           ib0_off=0
        else if (iopt.eq.0) then ! rectalinear lat/lon
           ideg_sc=100
           iscale_sc=1000
           i0_sc=100
           ixdeg_off=-100
           iydeg_off=0
           ia0_off=0
           ib0_off=0
        else if (iopt.eq.1.or.iopt.eq.2) then ! lambert
           ideg_sc=100
           iscale_sc=1000
           i0_sc=1
           ixdeg_off=0
           iydeg_off=0
           ia0_off=0
           ib0_off=0
        else if (iopt.eq.5) then ! polar stereographic
           ideg_sc=100
           iscale_sc=100
           i0_sc=1
           ixdeg_off=-100
           iydeg_off=0
           ia0_off=0
           ib0_off=0
        else if (iopt.eq.11.or.iopt.eq.12.or.iopt.eq.13) then ! EASE grid
           ideg_sc=10
           iscale_sc=1000
           i0_sc=10
           ixdeg_off=0
           iydeg_off=0
           ia0_off=0
           ib0_off=0
        else                    ! unknown default scaling
           ideg_sc=100
           iscale_sc=1000
           i0_sc=100
           ixdeg_off=0
           iydeg_off=0
           ia0_off=0
           ib0_off=0
        endif
c
c     call version 3.0 write sir routine
c
        call writesir3(fname,iu_in,ierr,nhead,ndes,nhtype,idatatype,
     *     nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *     ixdeg_off,iydeg_off,ideg_sc,iscale_sc,ia0_off,ib0_off,i0_sc,
     *     iyear,isday,ismin,ieday,iemin,iregion,itype,
     *     ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *     sensor,title,type,tag,crproc,crtime,
     *     stval,descrip,ldes,iaopt,nia)
c
        return
        end
