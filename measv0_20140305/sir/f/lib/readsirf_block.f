c
c
	subroutine readsirf_block(iu,ierr,nhead,nhtype,idatatype,stval,nsx,nsy,
     *       ioff,iscale,smin,smax,ncnt,anodata,vmin,vmax,lx,ly,nx,ny,krec)
c
c	read part (a rectangulatr block of data) of a SIR file image
c
c inputs:
c     iu       fortran logical unit of previously opened file
c     ierr     error code from header read
c     nhead    number of 512 byte blocks in header
c     nhtype   header type
c     nsx,nsy  image dimensions
c     ioff,iscale image storage scale factors
c     anodata   specify nodata value
c     vmin,vmax specify min, max valid range
c     lx,ly    lower-left corner of image block (0<lx<=nsx, 0<ly<=nsy)
c     nx,ny    upper-right corner of image block (lx<=nx<=nsx,ly<=ny<=nsy)
c     krec     record counter for most recent read (on first call set to nhead)
c
c outputs:
c     ierr   file read error code
c             set to 0 for successful read of file
c             set to -3 for data read error
c     stval  output image array (nx-lx+1 by ny-ly+1)
c             indexed as ind=(y-1)*(nx-lx+1)+x where 0 < y <= (ny-ly+1) and
c              0 < x <= (nx-lx+1) and 1 <= ind<= (nx-lx+1) * (ny-ly+1).
c     smin,max min and max of image within valid range 
c              valid range = [vmin..vmax] and .ne. anodata
c     ncnt   count of pixels in valid range
c     krec   record counter for most recent read
c
	integer		iu,nhead,nhtype
	real		stval(*)
	integer 	nsx,nsy,ncnt
	integer		ioff,iscale
	real            anodata,vmin,vmax,smin,smax
	integer         lx,ly,nx,ny,krec
c
c     working variables
c
	byte            tempb(512)  ! integer*1 (-127 .. 128)
	integer*2	temp(256)
	real            tempf(128)
	integer         ntot,jj,irec
	real            s,soff
c
c       platform-dependent parameters
c
	include 'SIR.inc'
c
	if (ierr.lt.0) return   ! return if file open previously failed
c
c       check block specification
c
	if (lx.lt.1 .or.lx.gt.nsx.or.ly.lt.1 .or.ly.gt.nsy .or.
     $      nx.lt.lx.or.nx.gt.nsx.or.ny.lt.ly.or.ny.gt.nsy) then
	   print *,'*** readsirf_block error: invalid block',lx,ly,nx,ny,nsx,nsy
	   ierr=-1
	   return
	endif
c
	ntot =(nx-lx+1)*(ny-ly+1)  ! total number of pixels to be read
	if (iscale.eq.0) iscale=1
	s=1./float(iscale)
	soff=32767.0/float(iscale)
	nrec=256                   ! pixel values per 512 byte record
	if (idatatype.eq.1) then
	   soff=128.0/float(iscale)
	   nrec=512
	endif
	if (idatatype.eq.4) nrec=128
c
	jj=0                       ! output array index
	i=((ly-1)*nsx+lx-1)        ! index of (lx,ly) in file
	irec=i/nrec                ! initial 512 byte record in image
	ii=i-irec*nrec+1           ! initial value offset in record
	irec=nhead+irec            ! first 512 byte record to read
	smin=1.e25
	smax=-1.e25
	ncnt=0
200	continue
	    irec=irec+1
	    if (VMS) then	! sequential i/o
	       if (krec.ge.irec) then
		  rewind(iu)
		  krec=0
	       endif
	       if (krec.lt.irec-1) then
		  do i=krec,irec-1
		     read (iu,414,end=299) temp
		  end do
		  krec=irec-1
	       endif
	       krec=krec+1
	       if (idatatype.eq.1) then
		  read (iu,413,end=299) tempb
 413		  format(512a1)
	       else if (idatatype.eq.4) then
		  read (iu,412,end=299) tempf
 412		  format(128a4)
	       else
		  read (iu,414,end=299) temp
 414		  format(256a2)
		  call swapbuf(temp,256)
	       endif
	    else		! direct record-based i/o
	       if (idatatype.eq.1) then
		  read(iu,rec=irec,err=298) tempb
	       else if (idatatype.eq.4) then
		  read(iu,rec=irec,err=298) tempf
	       else
		  read(iu,rec=irec,err=298) temp
		  if (BSWAP) call swapbuf(temp,256)
	       endif
	    endif
	    do i=ii,nrec
	       jj=jj+1
	       if (idatatype.eq.1) then
		  k=tempb(i)
		  stval(jj)=s*k+soff+ioff
	       else if (idatatype.eq.4) then
		  stval(jj)=tempf(i)
	       else
		  stval(jj)=s*temp(i)+soff+ioff
	       endif
	       if (stval(jj).ne.anodata.and.
     *		   stval(jj).ge.vmin.and.
     *		   stval(jj).le.vmax) then
		  smin=min(smin,stval(jj))
		  smax=max(smax,stval(jj))
		  ncnt=ncnt+1
	       endif
	       if (jj.ge.ntot) goto 299
	    end do
	    ii=1
	    goto 200
c
298     continue      ! error return
	write (*,98) irec
98	format(' *** ERROR reading input SIR file at ',I8)
c
c	close file on read error
c
	close(unit=iu)
	ierr=-3
	return
c
299	continue      ! successful completion
c
c	file is NOT closed on successful return
c
	krec=irec
	ierr=0
	return
	end




