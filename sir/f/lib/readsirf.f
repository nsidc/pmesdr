c
c standard version 3.0 header SIR read data routine
c
c
	subroutine readsirf(iu,ierr,nhead,nhtype,idatatype,stval,nsx,nsy,
     *       ioff,iscale,smin,smax,ncnt,anodata,vmin,vmax)
c
c	read SIR file image data
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
c
c outputs:
c     ierr   file read error code
c             set to 0 for successful read of file
c             set to -3 for data read error
c     stval  output image array
c     smin,max min and max of image within valid range 
c              valid range = [vmin..vmax] and .ne. anodata
c     ncnt   count of pixels in valid range
c
	integer		iu,nhead,nhtype
	real		stval(*)
	integer 	nsx,nsy,ncnt
	integer		ioff,iscale
	real            anodata,vmin,vmax,smin,smax
c
c     working variables
c
c	byte            tempb(512)  ! integer*1 (-127 .. 128)
	integer*1       tempb(512)  ! integer*1 (-127 .. 128)
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
	ntot = nsx*nsy
	if (iscale.eq.0) iscale=1
	s=1./float(iscale)
	soff=32767.0/float(iscale)
	nrec=256
	if (idatatype.eq.1) then
	   soff=128.0/float(iscale)
	   nrec=512
	endif
	if (idatatype.eq.4) nrec=128
c
	jj=0
	irec=nhead
	smin=1.e25
	smax=-1.e25
	ncnt=0
200	continue
	        irec=irec+1
		if (VMS) then
		   if (idatatype.eq.1) then
		      read (iu,413,end=299) tempb
 413		      format(512a1)
		   else if (idatatype.eq.4) then
		      read (iu,412,end=299) tempf
 412		      format(128a4)
		   else
		      read (iu,414,end=299) temp
 414		      format(256a2)
		      call swapbuf(temp,256)
		   endif
		else
		   if (idatatype.eq.1) then
		      read(iu,rec=irec,err=298) tempb
		   else if (idatatype.eq.4) then
		      read(iu,rec=irec,err=298) tempf
		   else
		      read(iu,rec=irec,err=298) temp
		      if (BSWAP) call swapbuf(temp,256)
		   endif
		endif
		do i=1,nrec
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
     *			     stval(jj).ge.vmin.and.
     *			     stval(jj).le.vmax) then
			   smin=min(smin,stval(jj))
			   smax=max(smax,stval(jj))
			   ncnt=ncnt+1
			endif
			if (jj.ge.ntot) goto 299
		end do
		goto 200
298     continue
	write (*,98)
98	format(' *** ERROR reading input SIR file ***')
c
c	close file on read error
c
	close(unit=iu)
	ierr=-3
	return
299	continue
c
c	close file on successful completion
c
	close(unit=iu)
	ierr=0
	return
	end

