c easy BYU SIR file format F interface code
c
c These routines define an easy-to-use fortran programing interface
c for the BYU SIR file format.  See SIREZ.inc for data structure definitions.
c not all fortran compilers support the extensions used in this code.
c
c written 4 Nov 2000 by DGL at BYU
c
c (c) 2000 by BYU MERS 
c
c ***************************************************************
c
      subroutine getsirhead(fname,lu,head,ierr)
c
c     read sir header information from file named fname into
c     data structure head using file unit lu.  Leave file open.
c
c     returned error code ierr:
c     ierr   file read error code
c             set to 0 for successful read of header
c             set to -1 for open error
c             set to -2 for header read error
c
      include 'SIREZ.inc'
c
      character*(*) fname
      integer lu,ierr
      record /sirhead/ head
c
      call readsirhead3(fname,lu,ierr,head.nhead,head.ndes,head.nhtype,
     *     head.idatatype,head.nsx,head.nsy,head.xdeg,head.ydeg,
     *     head.ascale,head.bscale,head.a0,head.b0,head.iopt,
     *     head.ioff,head.iscale,
     *     head.ixdeg_off,head.iydeg_off,head.ideg_sc,head.iscale_sc,
     *     head.ia0_off,head.ib0_off,head.i0_sc,
     *     head.iyear,head.isday,head.ismin,head.ieday,head.iemin,
     *     head.iregion,head.itype,head.ipol,head.ifreqhm,head.ispare1,
     *     head.anodata,head.vmin,head.vmax,
     *     head.sensor,head.title,head.type,head.tag,head.crproc,
     *     head.crtime,head.MAXDES,head.descrip,head.ldes,
     *     head.MAXI,head.iaopt,head.nia)
c
      return
      end
c
c
      subroutine getsirdata(lu,head,stval,ierr,smin,smax)
c
c     read sir image data information from file attached to 
c     file unit lu into real array stval using sir header
c     data structure head.  Closes file on successful completion.
c     smin and smax return the min,max of the data greater than the
c     nodata value.
c
c     returned error code ierr:
c     ierr   file read error code
c             set to 0 for successful read of file
c             set to -3 for data read error
c
      include 'SIREZ.inc'
c
      integer lu,ierr
      record /sirhead/ head
      real stval(*),smin,smax
c
      integer ncnt
c
      call readsirf(lu,ierr,head.nhead,head.nhtype,head.idatatype,
     *     stval,head.nsx,head.nsy,head.ioff,head.iscale,
     *     smin,smax,ncnt,head.anodata,head.vmin,head.vmax)
c
      return
      end
c
c
      subroutine printsirhead(head)
c
c     print out summary contents of sir file header information
c     contained in sir header data structure head.  Output is to
c     standard out
c
      include 'SIREZ.inc'
c
      record /sirhead/ head
c
      call printhead3(head.nhead,head.ndes,head.nhtype,head.idatatype,
     *     head.nsx,head.nsy,head.xdeg,head.ydeg,
     *     head.ascale,head.bscale,head.a0,head.b0,head.iopt,
     *     head.ioff,head.iscale,
     *     head.ixdeg_off,head.iydeg_off,head.ideg_sc,head.iscale_sc,
     *     head.ia0_off,head.ib0_off,head.i0_sc,
     *     head.iyear,head.isday,head.ismin,head.ieday,head.iemin,
     *     head.iregion,head.itype,head.ipol,head.ifreqhm,head.ispare1,
     *     head.anodata,head.vmin,head.vmax,
     *     head.SENSOR,head.TITLE,head.TYPE,head.TAG,head.CRPROC,
     *     head.CRTIME,head.DESCRIP,head.LDES,head.IAOPT,head.NIA)
c
      return
      end
c
c
      subroutine putsirfile(fname,lu,head,stval,ierr)
c
c     write sir image data and header to file
c     data structure head using file unit lu
c
c     returned error code ierr:
c     ierr   file write error code
c             set to 0 for successful write
c             set to -1 for open error
c             set to -2 for write error
c             set to -3 for invalid image size
c
      include 'SIREZ.inc'
c
      character*(*) fname
      integer lu,ierr
      record /sirhead/ head
      real stval(*)
c
      call writesir3(fname,lu,ierr,head.nhead,head.ndes,head.nhtype,
     *     head.idatatype,head.nsx,head.nsy,head.xdeg,head.ydeg,
     *     head.ascale,head.bscale,head.a0,head.b0,head.iopt,
     *     head.ioff,head.iscale,
     *     head.ixdeg_off,head.iydeg_off,head.ideg_sc,head.iscale_sc,
     *     head.ia0_off,head.ib0_off,head.i0_sc,
     *     head.iyear,head.isday,head.ismin,head.ieday,head.iemin,
     *     head.iregion,head.itype,head.ipol,head.ifreqhm,head.ispare1,
     *     head.anodata,head.vmin,head.vmax,
     *     head.sensor,head.title,head.type,head.tag,head.crproc,
     *     head.crtime,stval,head.descrip,head.ldes,
     *     head.iaopt,head.nia)
c
      return
      end
c
c
      subroutine sirpix2latlon(x, y, alon, alat, head)
c
c     compute lat,lon corresponding to pixel x,y given info in sir
c     header data structure head.  x,y can be outside of the image
c     but this may cause an invalid lat,lon to be returned or an algorithm
c     error
c
      include 'SIREZ.inc'
c
      real x,y,alat,alon
      record /sirhead/ head
c
      real thelon, thelat
c
      call pix2latlon(x, y, thelon, thelat, alon, alat,
     *     head.iopt, head.xdeg, head.ydeg, head.ascale, head.bscale,
     *     head.a0, head.b0)
c
      return
      end
c
c
      integer function isirlatlon2pix(alon, alat, x, y, head)
c
c     compute pixel coordinates corresponding to lat,lon given info in sir
c     header data structure head.  Returns lexicographic index to
c     corresponding pixel (only valid if > 0).  The image coordinate
c     range is defined as 1 <= x <= head.nsx and 0 <= y <= head.nsy
c     with the pixel (1,1) at index 1 and (nsx,nsy) at index nsx*nsy
c     if x or y out of image, an invalid index of 0 is returned.
c
      include 'SIREZ.inc'
c
      real alon, alat, x, y
      record /sirhead/ head
c
      real thelon, thelat
      integer ix, iy
c
      call latlon2pix(alon, alat, x, y, thelon, thelat, 
     *     head.iopt, head.xdeg, head.ydeg, head.ascale, head.bscale,
     *     head.a0, head.b0)
      call f2ipix(x, y, ix, iy, head.nsx, head.nsy)
      if (ix.ne.0.and.iy.ne.0) then
         isirlatlon2pix=(iy-1)*head.nsx+ix
      else
         isirlatlon2pix=0
      endif
c
      return
      end
c
c
      integer function isirpix(x, y, ix, iy, head)
c
c     compute pixel index from the coordinates given info in sir
c     header data structure head.  Returns lexicographic index to
c     corresponding pixel (only valid if > 0)  The image coordinate
c     range is defined as 1 <= x <= head.nsx and 0 <= y <= head.nsy
c     with the pixel (1,1) at index 1 and (nsx,nsy) at index nsx*nsy
c     if x or y out of image, an invalid index of 0 is returned and
c     one of ix and iy are set to zero.
c
      include 'SIREZ.inc'
c
      real x, y
      integer ix, iy
      record /sirhead/ head
c
      call f2ipix(x, y, ix, iy, head.nsx, head.nsy)
      if (ix.ne.0.and.iy.ne.0) then
         isirpix=(iy-1)*head.nsx+ix
      else
         isirpix=0
      endif
c
      return
      end

c
c
      integer function isirlex(ix, iy, head)
c
c     compute pixel coordinates corresponding to lat,lon given info in sir
c     header data structure head.  Returns lexicographic index to
c     corresponding pixel (only valid if > 0).  The image coordinate
c     range is defined as 1 <= x <= head.nsx and 0 <= y <= head.nsy
c     with the pixel (1,1) at index 1 and (nsx,nsy) at index nsx*nsy
c     if ix or iy out of image, an invalid index of 0 is returned
c
      include 'SIREZ.inc'
c
      integer ix,iy
      record /sirhead/ head
c
      if (ix.gt.0.and.ix.le.head.nsx.and.
     *    iy.gt.0.and.iy.le.head.nsy) then
         isirlex=(iy-1)*head.nsx+ix
      else
         isirlex=0
      endif
c
      return
      end



