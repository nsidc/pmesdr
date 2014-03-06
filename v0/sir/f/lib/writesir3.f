c
c *************************************************************************
c
c  standard SIR file write routine
c
      subroutine writesir3(fname,iu_in,ierr,nhead,ndes,nhtype,idatatype,
     *     nsx,nsy,xdeg,ydeg,ascale,bscale,a0,b0,iopt,ioff,iscale,
     *     ixdeg_off,iydeg_off,ideg_sc,iscale_sc,ia0_off,ib0_off,i0_sc,
     *     iyear,isday,ismin,ieday,iemin,iregion,itype,
     *     ipol,ifreqhm,ispare1,anodata,vmin,vmax,
     *     sensor,title,type,tag,crproc,crtime,
     *     stval,descrip,ldes,iaopt,nia)
c
c     Version 3 standard SIR image format write routine
c
c     written by D.G. Long  28 Oct. 2000
c
c
c inputs:
c     fname    file name to write to
c              note: may overwrite an existing file of the same name
c     iu_in    fortran logical unit to use
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
c     ispare1 spare integer
c     sensor  20 character string: describes sensor
c     title   80 character string: image title
c     type    80 character string: description of image type
c     tag     creator byline
c     crproc  100 character string: name and version of creating program
c     crtime  40 character string: date and time of creation
c     stval   image array (floats)
c     descrip variable length character string: description and annontation
c     ldes    number of characters in descrip
c     iaopt   array of optional integers
c       by convention, first value iaopt is a code telling how to interpret
c       the rest of the array if nia>0
c     nia     number of integers in extra iaopt header
c
c outputs:
c     ierr   file write error code
c             set to 0 for successful write
c             set to -1 for open error
c             set to -2 for write error
c             set to -3 for invalid image size
c     nhead  number of 512 byte blocks in header
c     nhtype file header type code
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
        integer         ixdeg_off,iydeg_off,ideg_sc,iscale_sc
        integer         ia0_off,ib0_off,i0_sc
	integer		ioff,iscale,iyear,isday,ismin,ieday,iemin
        integer		iregion,itype,ipol,ifreqhm,ispare1,ldes,nia
	real            anodata,vmin,vmax
	real            stval(*)
	character*(*)	sensor,title,type,tag,crproc,crtime,descrip
	integer		iaopt(*)
c
c     working variables
c
c	byte            tempb(512)    ! integer*1 (-127 .. 128)
	integer*1       tempb(512)    ! integer*1 (-127 .. 128)
	integer*2	temp(256)
	real            tempf(128)
	equivalence     (temp,tempb)  ! used only to save space
	equivalence     (temp,tempf)  ! used only to save space
	integer*2       i2(2)
	real            f2
	equivalence     (f2,i2)       ! used in i*2 to float mapping
	character	sens1*41,tit1*81,type1*139
        character       tag1*41,crp1*101,crt1*29
        integer         iu,minv,maxv,i,ii,j,k,ntot,nrec
        integer         irec,icnt,imax,iocnt,iucnt
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
           write (*,*) '*** writesir3 error: invalid iu, using ',iu
        endif
c
        if (nsx.lt.1.or.nsy.lt.1.or.nsx.gt.32767
     *       .or.nsy.gt.32767) then
           write (*,*) '*** writesir3 error: invalid nsx,nsy ',nsx,nsy
           ierr=-3
           return
        endif
c
c     check projection scale and offset parameters
c
        if (nhtype.lt.30.or.
     *      ideg_sc.le.0.or.iscale_sc.le.0.or.i0_sc.le.0) then
           if (nhtype.ge.30.and.
     *          (ideg_sc.le.0.or.iscale_sc.le.0.or.i0_sc.le.0))
     *       write(*,*) '*** writsir3 warning: projection scale error'
     *          ,ideg_sc,iscale_sc,i0_sc,ixdeg_off,iydeg_off,
     *          ia0_off,ib0_off
c
c     set parameters to header version 2.0 defaults
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
           else ! unknown default scaling
              ideg_sc=100
              iscale_sc=1000
              i0_sc=100
              ixdeg_off=0
              iydeg_off=0
              ia0_off=0
              ib0_off=0
           endif
           if (nhtype.lt.30) nhtype=30  ! can only write new headers
c           if (nhtype.ge.30.and.
c     *          (ideg_sc.le.0.0.or.iscale_sc.le.0.0.or.i0_sc.le.0)
c     *       write(*,*) '*** values used:',ideg_sc,iscale_sc,i0_sc,
c     *          ixdeg_off,iydeg_off,ia0_off,ib0_off
        endif
c
c	make first header block
c
        ierr = 0
	temp(1)	= nsx				! image dimensions
	temp(2) = nsy
        if (nhtype.lt.30) nhtype = 31 ! nhtype=30 reserved for version 2.0
	temp(5) = nhtype		       ! version 3.0 header code
c
c       scale projection variables based on transform option
c
        temp(3) = nint((xdeg + float(ixdeg_off)) * float(ideg_sc))
        temp(4) = nint((ydeg + float(iydeg_off)) * float(ideg_sc))
        temp(6) = nint(ascale * float(iscale_sc))
        temp(7) = nint(bscale * float(iscale_sc))
        temp(8) = nint((a0 + float(ia0_off)) * float(i0_sc))
        temp(9) = nint((b0 + float(ib0_off)) * float(i0_sc))
c
	if (iopt.eq.-1)	then			! image only
	else if (iopt.eq.0) then		! rectalinear lat/lon
	else if (iopt.eq.1.or.iopt.eq.2) then		! lambert
           temp(6) = nint(float(iscale_sc)/ascale)
           temp(7) = nint(float(iscale_sc)/bscale)
	else if (iopt.eq.5) then		! polar stereographic
	else if (iopt.eq.11.or.iopt.eq.12.or.iopt.eq.13) then ! EASE grid
           temp(6) = nint(float(iscale_sc)*anint(10.*ascale*
     *                         25.067525/6371.228)*0.05)
           temp(7) = nint(float(iscale_sc)*anint(10.*bscale/
     *                         25.067525)*0.05)
	else
           write (*,*) '*** Unrecognized SIR option in writesir3 ***'
	endif
c
c     store projection parameters offset and scale factors
c
        temp( 40) = iscale_sc
        temp(127) = ixdeg_off
        temp(128) = iydeg_off
        temp(169) = ideg_sc
        temp(190) = ia0_off
        temp(241) = ib0_off
        temp(256) = i0_sc
c
c	store scale factors, time, option, region, and image type
c
	temp(10) = ioff
	temp(11) = iscale
	if (iscale.eq.0) temp(11)=1
	temp(12) = iyear
	temp(13) = isday
	temp(14) = ismin
	temp(15) = ieday
	temp(16) = iemin
	temp(17) = iopt
	temp(18) = iregion
	temp(19) = itype
c
	nhead=1
	ndes=0
	if (ldes.gt.0) then
	   ndes=ldes/512
	   if (mod(ldes,512).gt.0) ndes=ndes+1
c	   write (*,*) 'extra descriptor',ldes,ndes
	   nhead=nhead+ndes
	endif
	if (nia.gt.0) then
c	   write (*,*) 'extra integers',nia
	   nhead=nhead+nia/256
	   if (mod(nia,256).gt.0) nhead=nhead+1
	endif
c
	temp(41) = nhead
	temp(42) = ndes
	temp(43) = ldes
	temp(44) = nia
	temp(45) = ipol
	temp(46) = ifreqhm
	temp(47) = ispare1
	temp(48) = idatatype
c
	if (idatatype.eq.1) then
	   maxv=127
	   minv=128
	else
	   maxv=32767
	   minv=32767
	endif
c
	if (idatatype.ne.4) then
	   k = nint(iscale*(anodata-ioff))-minv
	   if (k.gt.maxv) then
	      k=maxv
	      write (*,*) '*** WRITESIR: overflow in "anodata"',
     *             anodata,ioff,iscale
	   endif
	   if (k.lt.-minv) then
	      k=-minv
	      write (*,*) '*** WRITESIR: underflow in "anodata"',
     *             anodata,ioff,iscale
	   endif
	   temp(49) = k
	   k = nint(iscale*(vmin-ioff))-minv
	   if (k.gt.maxv) then
	      k=maxv
	      write (*,*) '*** WRITESIR: overflow in "vmin"',
     *             vmin,ioff,iscale
	   endif
	   if (k.lt.-minv) then
	      k=-minv
	      write (*,*) '*** WRITESIR: underflow in "vmin"',
     *             vmin,ioff,iscale
	   endif
	   temp(50) = k
	   k = nint(iscale*(vmax-ioff))-minv
	   if (k.gt.maxv) then
	      k=maxv
	      write (*,*) '*** WRITESIR: overflow in "vmax"',
     *             vmax,ioff,iscale
	   endif
	   if (k.lt.-minv) then
	      k=-minv
	      write (*,*) '*** WRITESIR: underflow in "vmax"',
     *             vmax,ioff,iscale
	   endif
	   temp(51) = k
	endif
c
	f2=anodata
	temp(52)=i2(1)
	temp(53)=i2(2)
	f2=vmin
	temp(54)=i2(1)
	temp(55)=i2(2)
	f2=vmax
	temp(56)=i2(1)
	temp(57)=i2(2)
c
c	write character variables
c
	sens1=' '  ! initially make them blank
	tit1 =' '
        type1=' '
        tag1 =' '
        crp1 =' '
        crt1 =' '
	sens1=sensor
	do i=1,20
		j=(i-1)*2+1
		temp(i+19)=ichar(sens1(j:j))+256*ichar(sens1(j+1:j+1))
	end do

	type1=type
	do i=1,69
		j=(i-1)*2+1
		temp(i+57)=ichar(type1(j:j))+256*ichar(type1(j+1:j+1))
	end do

	tit1=title
	do i=1,40
		j=(i-1)*2+1
		temp(i+128)=ichar(tit1(j:j))+256*ichar(tit1(j+1:j+1))
	end do

	tag1=tag
	do i=1,20
		j=(i-1)*2+1
		temp(i+169)=ichar(tag1(j:j))+256*ichar(tag1(j+1:j+1))
	end do

	crp1=crproc
	do i=1,50
		j=(i-1)*2+1
		temp(i+190)=ichar(crp1(j:j))+256*ichar(crp1(j+1:j+1))
	end do

	crt1=crtime
	do i=1,14
		j=(i-1)*2+1
		temp(i+241)=ichar(crt1(j:j))+256*ichar(crt1(j+1:j+1))
	end do
c
c	open output file
c
        if (VMS) then
	  open(unit=iu,file=fname,status='new',      ! for vax/vms
     *          form='formatted',err=999,
     *		recl=ISIRRECLENGTH)
c     *          ,recordtype='fixed',carriagecontrol='none')
        else
	  open(unit=iu,file=fname,status='unknown',  ! generally portable
     *          form='unformatted',err=999,          ! unix file open
     *		recl=ISIRRECLENGTH,access='direct')
        endif
c
c	write first header block
c
	irec=1
	if (BSWAP) call swapbuf(temp,256)
	if (VMS) then
          write(iu,414,err=1999) temp
        else
          write(iu,rec=1,err=1999) temp
        endif
c
c       write additional header records if needed
c
	if (ndes.gt.0) then
	   icnt=0
	   do ii=1,ndes
	      do i=1,256
		 k=0
		 icnt=icnt+1
		 if (icnt.le.ldes) then
		    k=ichar(descrip(icnt:icnt))
		    icnt=icnt+1
		    if (icnt.le.ldes) k=k+256*ichar(descrip(icnt:icnt))
		 endif
		 temp(i)=k
	      end do
	      if (BSWAP) call swapbuf(temp,256)
	      irec=irec+1
	      if (VMS) then
		 write(iu,414,err=1999) temp
	      else
		 write(iu,rec=irec,err=1999) temp
	      endif
	   end do
	endif
c
c	write extra header parameters
c
	if (nhead-ndes-1.gt.0) then
	   icnt=0
	   do ii=nhead-ndes+1,nhead
	      do i=1,256
		 icnt=icnt+1
		 if (icnt.le.nia) then
		    temp(i)=iaopt(icnt)
		 else
		    temp(i)=0
		 endif
	      end do
	      if (BSWAP) call swapbuf(temp,256)
	      irec=irec+1
	      if (VMS) then
		 write(iu,414,err=1999) temp
	      else
		 write(iu,rec=irec,err=1999) temp
	      endif
	   end do
	endif
c
c	write image data
c
	iucnt=0
	iocnt=0
	ntot = nsx*nsy
	nrec=256                      ! output values/record (I*2)
	if (idatatype.eq.1) nrec=512  ! output values/record (byte)
	if (idatatype.eq.4) nrec=128  ! output values/record (float)
	do j = 0,ntot,nrec
	   imax=min(nrec,ntot-j)
	   if (imax.gt.0) then
	      do i=1,imax
		 if (idatatype.eq.4) then
		    tempf(i)=stval(j+i)
		 else
		    k = nint(iscale*(stval(j+i)-ioff))-minv
		    if (k.gt.maxv) then
		       k=maxv
		       iocnt=iocnt+1
		    endif
		    if (k.lt.-minv) then
		       k=-minv
		       iucnt=iucnt+1
		    endif
		    if (idatatype.eq.1) then
		       tempb(i) = k
		    else
		       temp(i) = k
		    endif
		 endif
	      end do
	      if (imax.ne.nrec) then
		 do i=imax+1,nrec
		    if (idatatype.eq.1) then
		       tempb(i) = -minv
		    else if (idatatype.eq.4) then
		       tempf(i) = anodata
		    else
		       temp(i) = -minv
		    endif
		 end do
	      endif
	      irec=irec+1
	      if (idatatype.eq.1) then ! byte size
		 if (VMS) then
		    write(iu,413,err=1999) tempb
 413		    format(512a1)
		 else
		    write(iu,rec=irec,err=1999) tempb
		 endif
	      else if (idatatype.eq.4) then ! 4 byte float size
		 if (VMS) then
		    write(iu,412,err=1999) tempf
 412		    format(128a4)
		 else
		    write(iu,rec=irec,err=1999) tempf
		 endif
	      else		! integer*2 size
		 if (BSWAP) call swapbuf(temp,256)
		 if (VMS) then
		    write(iu,414,err=1999) temp
 414		    format(256a2)
		 else
		    write(iu,rec=irec,err=1999) temp
		 endif
	      endif
	   endif
	end do 
c
c	close file
c
	close(unit=iu)
c
c     summarize over/underflow stats
c
	if (iucnt.gt.0) write (*,50) iucnt,ntot
50	format(' *** WRITESIR3 Underflow count: ',I9,' of ',I9,' ***')
	if (iocnt.gt.0) write (*,60) iocnt,ntot
60	format(' *** WRITESIR3 Overflow count:  ',I9,' of ',I9,' ***')
	return
c
 999    continue
        write (*,5000)
 5000   format(' *** writesir3: ERROR OPENING OUTPUT FILE ***')
        ierr=-1
        return
 1999   continue
        write (*,5050)
 5050   format(' *** writesir3: ERROR WRITING OUTPUT FILE ***')
        ierr=-2
        return
	end
