C
C **************************************************************************
C
C STANDARD SIR IMAGE FORMAT READ/WRITE ROUTINES
C
C **************************************************************************
C
      subroutine readsirhead3(fname,iu_in,ierr,nhead,ndes,nhtype,
     *     idatatype,
     *     nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *     ixdeg_off,iydeg_off,ideg_sc,iscale_sc,ia0_off,ib0_off,
     *     i0_sc,
     *     iyear,isday,ismin,ieday,iemin,iregion,itype,
     *     ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *     sensor,title,type,tag,crproc,crtime,
     *     maxdes,descrip,ldes,maxi,iaopt,nia)
c
c     Version 3.0 standard SIR image format header read routine
c
c     written by D.G. Long  28 Oct 2000
c     revised to initialize ierr
c
c inputs:
c     fname    file name to read from
c     iu_in    fortran logical unit to use
c     maxdes   maximum number of characters in descrip
c     maxi     maximum number of integers in iaopt
c
c outputs:
c     ierr   file read error code
c             set to 0 for successful read of header
c             set to -1 for open error
c             set to -2 for header read error
c     nhead  number of 512 byte blocks in header
c     nhtype file header type code from header
c     idatatype file data type code (0,2=i*2,1=i*1,4=i*4) def=i*2
c     nsx,nsy image dimensions
c     xdeg..b0 projection factors
c     iopt    projection type
c     ioff,iscale image storage scale factors
c     ixdeg_off..i0_sc projection offset and scale storage factors (see below)
c     iyear,isday,ismin  year,day,minute of start of data used in image
c     ieday,iemin day,minute of end of data used in image
c     iregion study region id number
c     itype   image type code (=0 none or unknown)
c             typically: 1's digit is polarization (0=H, 1=V)
c             10's digit and higher is frequency in hundreds of MHz
c     ispare* spare integers
c     sensor  20 character string: describes sensor
c     title   80 character string: image title
c     type    80 character string: description of image type
c     tag     creator byline
c     crproc  100 character string: name and version of creating program
c     crtime  40 character string: date and time of creation
c     descrip variable length character string: description and annontation
c     ldes    number of characters in descrip
c     iaopt   optional integers
c       by convention, first value iaopt is a code telling how to interpret
c       the rest of the array if nia>0
c     nia     number of integers
c
c     SIR files have headers which are multiples of 512 bytes followed by
c     image data stored as 2 byte integers zero padded to be a multiple
c     of 512 bytes.
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
c     working variables
c
	integer*2       i2(2)
	real            f2
	equivalence (i2,f2)      ! used in i*2 to real mapping
	integer*2	temp(256)
	character	sens1*41,tit1*81,type1*139
        character       tag1*41,crp1*101,crt1*29
        integer         iu,minv,i,j,icnt,irec,iv
        real            soff

        integer*2       i256  ! this awkward definition avoids some compiler
        parameter (i256=256)  ! variable typing problems for the mod function
c
c	header mapping (see code for scaling of <= variables)
c       note that character strings do not have to be null terminated
c
c	temp(1)	= nsx			! pixels in x direction
c	temp(2) = nsy			! pixels in y direction
c       temp(3) <= xdeg			! span of x
c	temp(4) <= ydeg			! span of y
c	temp(5) = nhtype                ! header type (old<15,20,30)
c	temp(6) <= ascale               ! x scaling
c	temp(7) <= bscale               ! y scaling
c	temp(8) <= a0                   ! x (or lon) origin
c         note: longitudes should be in the range -180 to 180
c	temp(9) <= b0                   ! y (or lat) origin
c         note: latitudes should be in the range -90 to 90
c
c     scaling for prorjection parameters are generally:
c
c        temp(3) = nint((xdeg + float(ixdeg_off)) * float(ideg_sc))
c        temp(4) = nint((ydeg + float(iydeg_off)) * float(ideg_sc))
c        temp(6) = nint(ascale * float(iscale_sc))
c        temp(7) = nint(bscale * float(iscale_sc))
c        temp(8) = nint((a0 + float(ia0_off)) * float(i0_sc))
c        temp(9) = nint((b0 + float(ib0_off)) * float(i0_sc))
c
c     with the following projection specific exceptions:
c
c	if (iopt.eq.1.or.iopt.eq.2) then		! lambert
c           temp(6) = nint(float(iscale_sc)/ascale)
c           temp(7) = nint(float(iscale_sc)/bscale)
c	if (iopt.eq.11.or.iopt.eq.12.or.iopt.eq.13) then ! EASE grid
c           temp(6) = nint(float(iscale_sc)*anint(10.*ascale*
c     *                         25.067525/6371.228)*0.05)
c           temp(7) = nint(float(iscale_sc)*anint(10.*bscale/
c     *                         25.067525)*0.05)
c
c	temp(10) = ioff			! offset to be added to scale val
c	temp(11) = iscale		! scale factor ival=(val-ioff)/iscale
c	temp(12) = iyear		! year for data used
c	temp(13) = isday		! starting JD
c	temp(14) = ismin		! time of day for first data (in min)
c	temp(15) = ieday		! ending JD
c	temp(16) = iemin		! time of day for last data (in min)
c	temp(17) = iopt			! projection type
c					!  -1 = no projection, image only
c					!   0 = rectalinear lat/lon
c					!   1 = lambert equal area
c					!   2 = lambert equal area (local rad)
c					!   5 = polar stereographic
c					!  11 = EASE north equal area grid
c					!  12 = EASE south equal area grid
c					!  13 = EASE cylindrical grid
c	temp(18) = iregion		! region id code
c	temp(19) = itype		! image type code
c                                       ! some standard values: (0=unknown)
c                                       ! 1 = scatterometer A (dB)
c                                       ! 2 = scatterometer B (dB/deg)
c                                       ! 3 = radiometer Tb (K)
c                                       ! 9 = topography (m)
c	temp(20)-temp(39) 40 chars of sensor
c       temp(40) = iscale_sc            ! ascale/bscale scale factor
c       temp(41) = nhead                ! number of 512 byte header blocks
c       temp(42) = ndes                 ! number of 512 byte blocks description
c       temp(43) = ldes                 ! number of bytes of description
c       temp(44) = nia                  ! number of optional integers
c       temp(45) = ipol                 ! polarization (0=n/a,1=H,2=V)
c       temp(46) = ifreqhm              ! frequency in 100's MHz (0 if n/a)
c       temp(47) = ispare1              ! spare
c       temp(48) = idatatype            ! data type code 0,2=i*2,1=i*1,4=float
c
c       the value of idata type determines how data is stored and how
c       anodata, vmin, and vmax are stored.
c
c       if idatatype = 1 data is stored as bytes (minv=128)
c       if idatatype = 2 data is stored as 2 byte integers (minv=32766)
c       if idatatype = 4 data is stored as IEEE floating point
c
c       if idatatype = 1,2 anodata,vmin,vmax are stored as 2 byte integers
c         in temp(49)..temp(51)  minv, ioff and iscal used to convert
c         integers or bytes into floating point values
c         nodata, vmin, and vmax must be representable with ioff and iscale
c            temp(*) = (value-ioff)*iscale-minv
c            value = float(temp(*)+minv)/float(iscale)+ioff
c       idatatype=2 is considered the SIR standard format
c
c       if idatatype = f anodata,vmin,vmax are stored as floating points
c         in temp(42)..temp(57) and minv, ioff and iscale are ignored here
c         and when reading the file.
c         floating point numbers are NOT standard across platforms and
c         are therefore not recommended
c
c       temp(49) <= anodata           ! value representing no data
c       temp(50) <= vmin              ! minimum useful value from creator prg
c       temp(51) <= vmax              ! maximum useful value from creator prg
c       temp(52,53) = anodata         ! IEEE floating value of no data
c       temp(54,55) = vmin            ! IEEE floating minimum useful value
c       temp(56,57) = vmax            ! IEEE floating maximum useful value
c
c	temp(58)-temp(126) 150 chars of type
c       temp(127) = ixdeg_off         ! xdeg offset
c       temp(128) = iydeg_off         ! ydeg offset
c	temp(129)-temp(168) 80 chars of title
c       temp(169) = ideg_sc           ! xdeg,ydeg scale factor
c	temp(170)-temp(189) 40 chars of tag
c       temp(190) = ia0_off           ! b0 offset 
c	temp(191)-temp(240) 100 chars of crproc
c       temp(241) = ib0_off           ! b0 offset 
c	temp(242)-temp(255) 28 chars of crtime
c       temp(256) = i0_sc             ! a0,b0 scale factor
c
c     optional header blocks:
c
c	ndes header blocks of 512 bytes: chars of description
c	nhead-ndes-1 header blocks of 512 bytes: values of iaopt
c       by convention, first value iaopt is a code telling how to interpret
c       the rest of the array if nia>0.  Usage of additional blocks is
c       user dependent and non-standard.
c
c       remainder of file is image data in a multiple of 512 byte blocks
c
c       one,two byte integer scaling (idatatype=1,2) is
c          intval = (fvalue-ioff)*iscale-minv
c          fvalue = float(intval+minv)/float(iscale)+ioff
c       no scaling of float values for (idatatype=4)
c
c
c *******************************************************************
c
c     get platform-dependent parameters
c
        include 'SIR.inc'
c
c *******************************************************************
c
c     error check other input values
c
        iu=iu_in
        if (iu.lt.1) then
           iu=2
           write (*,*) '*** readsirhead3 error: invalid iu, using ',iu
        endif
        ierr=0
c
c	open input file
c
	if (VMS) then
	  open(unit=iu,file=fname,status='old',form='formatted',
     *		recl=ISIRRECLENGTH,err=99) !,
c     *          readonly,recordtype='fixed',carriagecontrol='none')
	else
	  open(unit=iu,file=fname,status='old',form='unformatted',
     *          recl=ISIRRECLENGTH,access='direct',err=99)
	endif
c
c	read first header block
c
	if (VMS) then
		read (iu,414) temp
 414		format(256a2)
		call swapbuf(temp,256)
	else
		read (iu,rec=1) temp
                if (BSWAP) call swapbuf(temp,256)
	endif
c
c	decode first header block
c
	nsx = temp(1)			! dimension
	nsy = temp(2)
	iopt = temp(17)			! transformation option
        nhtype = temp(5)                ! header type
        if (nhtype.lt.20) nhtype=1
c
        if (nhtype.lt.30) then
c
c     set version 3.0 parameters to header version 2.0 defaults
c     
           if (iopt.eq.-1) then ! image only
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
           else                 ! unknown default scaling
              ideg_sc=100
              iscale_sc=1000
              i0_sc=100
              ixdeg_off=0
              iydeg_off=0
              ia0_off=0
              ib0_off=0
           endif
        else
c
c     get projection parameters offset and scale factors
c
           iscale_sc = temp( 40)
           ixdeg_off = temp(127)
           iydeg_off = temp(128)
           ideg_sc   = temp(169)
           ia0_off   = temp(190)
           ib0_off   = temp(241)
           i0_sc     = temp(256)
        endif
c
c     decode default projection parameters
c
        xdeg   = temp(3)/float(ideg_sc) - ixdeg_off
        ydeg   = temp(4)/float(ideg_sc) - iydeg_off
        ascale = temp(6)/float(iscale_sc)
        bscale = temp(7)/float(iscale_sc)
        a0     = temp(8)/float(i0_sc) - ia0_off
        b0     = temp(9)/float(i0_sc) - ib0_off
c
c	handle special cases which depend on transformation option
c
	if (iopt.eq.-1) then				! image only
	else if (iopt.eq.0) then			! rectalinear
	else if (iopt.eq.1.or.iopt.eq.2) then		! lambert
		ascale=float(iscale_sc)/float(temp(6))
		bscale=float(iscale_sc)/float(temp(7))
	else if (iopt.eq.5) then			! polar stereographic
	else if (iopt.eq.11.or.iopt.eq.12.or.iopt.eq.13) then ! EASE grid
		ascale = 2.0d0*(float(temp(6))/float(iscale_sc))
     *               *6371.228d0/25.067525d0
		bscale = 2.0d0*(float(temp(7))/float(iscale_sc))
     *               *25.067525d0
	else
		write (*,*)'*** Unrecognized SIR option in READSIRHEAD ***'
	endif
c
c	decode scaling, data time, region, and type information
c
	ioff = temp(10)
	iscale = temp(11)
	if (iscale.eq.0) iscale=1
	iyear = temp(12)
	isday = temp(13)
	ismin = temp(14)
	ieday = temp(15)
	iemin = temp(16)
	iregion = temp(18)
	itype = temp(19)
        nhead = temp(41)        ! total number of header blocks
        if (nhead.eq.0) nhead=1
        if (nhtype.eq.1) nhead=1
        ndes = temp(42)         ! number of description blocks
        ldes = temp(43)
        nia = temp(44)
        ipol = temp(45)
        ifreqhm = temp(46)
        ispare1 = temp(47)
        idatatype = temp(48)    ! data type code
	if (idatatype.eq.0) idatatype=2
        if (nhtype.eq.1) then
           ndes=0
           ldes=0
           nia=0
           idatatype=2
        endif
c
	if (idatatype.eq.1) then
	   minv=128
	else
	   minv=32767
	endif
c
	soff=float(minv)/float(iscale)
	if (idatatype.eq.1) soff=float(minv)/float(iscale)
c
        anodata = temp(49)/float(iscale)+ioff+soff  ! int type values
        vmin = temp(50)/float(iscale)+ioff+soff
        vmax = temp(51)/float(iscale)+ioff+soff
c
	if (idatatype.eq.4) then  ! float type values
	   i2(1)=temp(52)
	   i2(2)=temp(53)
	   anodata=f2
	   i2(1)=temp(54)
	   i2(2)=temp(55)
	   vmin=f2
	   i2(1)=temp(56)
	   i2(2)=temp(57)
	   vmax=f2
	endif
c
c	get character variables
c
	do i=1,20
		j=(i-1)*2+1
		sens1(j:j)=char(mod(temp(i+19),i256))
		sens1(j+1:j+1)=char(temp(i+19)/256)
	end do
	sens1(41:41)=char(0)  ! null terminate string
	sensor=sens1
	do i=1,40
		j=(i-1)*2+1
		tit1(j:j)=char(mod(temp(i+128),i256))
		tit1(j+1:j+1)=char(temp(i+128)/256)
	end do
	tit1(81:81)=char(0)  ! null terminate string
	title=tit1
        if (nhtype.gt.1) then
           do i=1,69
              j=(i-1)*2+1
              type1(j:j)=char(mod(temp(i+57),i256))
              type1(j+1:j+1)=char(temp(i+57)/256)
           end do
	   type1(139:139)=char(0)  ! null terminate string
           do i=1,20
              j=(i-1)*2+1
              tag1(j:j)=char(mod(temp(i+169),i256))
              tag1(j+1:j+1)=char(temp(i+169)/256)
           end do
	   tag1(41:41)=char(0)  ! null terminate string
           do i=1,50
              j=(i-1)*2+1
              crp1(j:j)=char(mod(temp(i+190),i256))
              crp1(j+1:j+1)=char(temp(i+190)/256)
           end do
	   crp1(101:101)=char(0)  ! null terminate string
           do i=1,14
              j=(i-1)*2+1
              crt1(j:j)=char(mod(temp(i+241),i256))
              crt1(j+1:j+1)=char(temp(i+241)/256)
           end do
	   crt1(29:29)=char(0)  ! null terminate string
	else
	   type1=' '
	   tag1=' '
	   crp1=' '
	   crt1=' '
           nhead=1
           nhtype=20  ! upgrade header version upon return
        endif
        type=type1
        tag=tag1
        crproc=crp1
        crtime=crt1
        descrip=' '
c
c	get extra storage information
c
        if (nhead.gt.1) then
           if (ndes.gt.0) then
c
c	read next header blocks
c
              icnt=0
              do irec=2,ndes+1
                 if (VMS) then
                    read (iu,414) temp
                    call swapbuf(temp,256)
                 else
                    read (iu,rec=irec) temp
                    if (BSWAP) call swapbuf(temp,256)
                 endif
                 do j=1,512
                    icnt=icnt+1
                    if (icnt.le.ldes.and.icnt.le.maxdes) then
                       i=(j-1)/2+1
                       iv=temp(i)
                       if (mod(j,2).eq.0) then
                          iv=iv/256
                       else
                          iv=mod(iv,256)
                       endif
                       descrip(icnt:icnt)=char(iv)
                    endif
                 end do                
              end do
              if (maxdes.lt.ldes) then
                 write (*,*) '*** readsirhead3 warning: extra char',
     *                ' head overflow ',nia,maxi
                 ldes=maxdes
              endif
           endif
           if (nhead-ndes-1.gt.0) then
              icnt=0
              do irec=ndes+2,nhead
                 if (VMS) then
                    read (iu,414) temp
                    call swapbuf(temp,256)
                 else
                    read (iu,rec=irec) temp
                    if (BSWAP) call swapbuf(temp,256)
                 endif
                 do i=1,256
                    icnt=icnt+1
                    if (icnt.le.nia.and.icnt.le.maxi) 
     *                 iaopt(icnt) = temp(i)
                 end do
              end do
              if (nia.gt.maxi) then
                 write (*,*) '*** readsirhead3 warning: extra ints',
     *                ' head overflow ',nia,maxi
                 nia=maxi
              endif
           endif
        endif
	return
c     
 300    continue
	write (*,398)
398	format(' *** readsirhead3 ERROR: read header of SIR file ***')
c
c	close file on header read error
c
	close(unit=iu)
c
	ierr=-2
	return

99	continue
	write (*,198)
198	format(' *** readsirhead3 ERROR: opening input SIR file ***')
	ierr=-1
	return
	end
