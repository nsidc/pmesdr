c easy BYU SIR file format F include file
c
c This include file defines an easy record based fortran interface for the
c BYU SIR file format.  A fortran data structure which stores the key
c SIR header information is defined in this file to simplify transfer
c of the SIR information to the various routines.  Unfortunately, data
c structures are not standard to fortran77 and so may not be available
c for use on all platforms.
c
c written 4 Nov 2000 by DGL at BYU
c
c (c) 2000 by BYU MERS
c
c ***************************************************************
c
c	define a data structure to hold the standard SIR file header info
c
	structure /sirhead/
c
c     the following variables are defined in the standard SIR file
c     header (version 3.0) and are fixed length
c
	integer nhead            ! number of 512 byte header records
	integer nhtype	         ! version number of SIR file header
	integer nsx, nsy	 ! image size
	integer iopt	         ! projection option
	real  xdeg, ydeg, ascale, bscale, a0, b0	! projection parameters
c
	integer ixdeg_off, iydeg_off, ideg_sc, iscale_sc ! projection st scale
	integer ia0_off, ib0_off, i0_sc	                 ! factors 
c
	integer idatatype	 ! data storage type
	integer ioff, iscale	 ! data storage scale factor and offset value
	real  anodata, vmin, vmax	     ! no data value, visible range
	integer iyear, isday, ismin, ieday, iemin ! file time codes
	integer iregion, itype, ipol, ifreqhm	  ! region id number, etc
	integer ispare1	         ! spare int  
c
	character title*100      ! title for image
	character sensor*40	 ! sensor description
	character type*138	 ! image type description
	character tag*100	 ! file tag
	character crproc*100	 ! file creation process name
	character crtime*28	 ! file createion time/date
c
c     After the first 512 byte header of variables defined above,
c     the SIR file format permits optional additional 512 byte headers
c     containing an optional user-defined string 'descrip' and an 
c     array of integer*2 'iaopt'.  These may be arbitrarily long, 
c     though if present are typically short.  Since fortran does
c     not do character pointers well, this can make this simple interface
c     more complicated.  Here, we have choosen to store a maximum
c     of MAXDES characters and MAXI integers.  
c
	integer ndes		! number of chars of extra description string
	integer ldes	        ! number of 512 records devoted to descript
	integer nia	        ! number of extra integers
c
	integer MAXDES, MAXI    ! define max chars and ints
	parameter (MAXDES=512)	! maximum chars in extra description string 
	parameter (MAXI=128)    ! maximum number of extra integers
c
	character*512 descrip	! size should match MAXDES above
	integer*2 iaopt(MAXI)   ! integer storage
c
	end structure  ! end of /sirhead/ data structure (record) definition
c
c ***************************************************************
c








