pro xsir_idl,sir,head,mn=mn,mx=mx,dispzoom=dispzoom,dispvals=dispvals,disphist=disphist,fixhr=fixhr,maxdim=maxdim,ns=ns
; The xsir routine allows a user to interactively view a sir
;   format image.  The original image is read in using loadsir.pro. 
;   The routine displays four windows:
;     1 - Full sir image (size reduced if needed)
;     2 - Zoomed image around a mouse selected pixel
;     3 - x,y location, lat, lon, and value of selected pixel
;     5 - Histogram of the values in the zoom window
;
; Required inputs:
;    sir = a 2-D sir image (use loadsir.pro  to read from disk)
;    head = the sir header returned from loadsir.pro
;
; Optional inputs:
;    mn = min value for image display (default is min of sir image)
;    mx = max value for image display (default is max of sir image)
;    dispzoom = zoom window display flag: 0=no, 1=yes (default)
;    dispvals = vals window display flag: 0=no, 1=yes (default)
;    disphist = histogram window display flag: 0=no, 1=yes (default)
;    fixhr = fix histogram range flag: 0=use zoom min and max (default), 1=use mn and mx
;    maxdim = define max dimension for image resizing (default is 970 if the max dimension
;             of the sir image is larger than 970)
;
; Called routines:
;    pixtolatlon.pro
;
; (c) BYU MERS August 1999
;     Written by Quinn Remund
;

print
print,'Running xsir...'
print,'Right click to exit.'
print
!x.style=1
!y.style=1

; check the routine arguments
if (n_elements(fixhr) eq 0) then fixhr=0
if (n_elements(maxdim) eq 0) then maxdim=970
if (n_elements(mn) eq 0) then mn=min(sir)
if (n_elements(mx) eq 0) then mx=max(sir)
if (n_elements(ns) eq 0) then ns=251
if (n_elements(dispzoom) eq 0) then dispzoom=1
if (n_elements(dispvals) eq 0) then dispvals=1
if (n_elements(disphist) eq 0) then disphist=1

; error checking
if ((ns mod 2) eq 0) then begin
  ns=ns+1
endif

; compute some important parameters
sz=size(sir)
mn=float(mn)
mx=float(mx)

; determine the display image size
if ((sz(1) lt maxdim)and(sz(2) lt maxdim)) then maxdim=max([sz(1),sz(2)])
if (sz(1) eq sz(2)) then begin
  wx=maxdim
  wy=maxdim
endif
if (sz(1) gt sz(2)) then begin
  wx=maxdim
  wy=fix(float(sz(2))*float(maxdim)/float(sz(1)))
endif
if (sz(2) gt sz(1)) then begin
  wx=fix(float(sz(1))*float(maxdim)/float(sz(2)))
  wy=maxdim
endif

; open the window and display the image
window,xs=wx,ys=wy,28,xpos=2000,ypos=2000
tv,bytscl(congrid(sir,wx,wy),min=mn,max=mx)

; define some important constants
quit=0
pxsz=4. ; number of display pixels per image pixel in zoom window
ns2=fix(float(ns)/(2.0*pxsz)) 
timg=fltarr(2*ns2+1,2*ns2+1)

; open the zoom, data value, and histogram windows   
ds=get_screen_size()
yp=ds(1)-ns-5
if (dispzoom ne 0) then window,xs=ns,ys=ns,29,xpos=10,ypos=yp,title='Zoom'
yp=yp-ns/2-35
if (dispvals ne 0) then window,xs=ns,ys=ns/2,30,xpos=10,ypos=yp,title='Pixel Parameters'
yp=yp-fix(ns*.8)-35
if (disphist ne 0) then window,xs=ns,ys=ns*.8,31,xpos=10,ypos=yp,title='Zoom Window Histogram'

; begin the loop in which the user clicks on image pixels
oldx=-1
oldy=-1
repeat begin
   wset,28
   cursor,x,y,/device            ; get x,y location of pixel click
   if (!Err ne 1) then begin     ; check for right click
     quit=1
   endif else begin
     x=fix(float(x)*float(sz(1))/float(wx))     ; convert to original image pixel
     y=fix(float(y)*float(sz(2))/float(wy))
     if ((x ne oldx)or(y ne oldy)) then begin  ; do nothing if pixel is same
       oldx=x
       oldy=y
       lx=x-ns2                                 ; compute boundaries for zoom
       ux=x+ns2
       ly=y-ns2
       uy=y+ns2
       timg=timg*0.+mn
       tlx=0
       tux=2*ns2
       tly=0
       tuy=2*ns2
       if (lx lt 0) then begin                  ; check for pixel overflow
         tlx=-lx
         lx=0
       endif
       if (ly lt 0) then begin
         tly=-ly
         ly=0
       endif
       if (ux ge sz(1)) then begin
         tux=2*ns2-(ux-sz(1)-1)-2
         ux=sz(1)-1
       endif
       if (uy ge sz(2)) then begin
         tuy=2*ns2-(uy-sz(2)-1)-2
         uy=sz(2)-1
       endif
       
       timg(tlx:tux,tly:tuy)=sir(lx:ux,ly:uy)   ; define the zoom region

       if (dispzoom ne 0) then begin            ; display the zoom region
         wset,29
         timgb=congrid(timg,ns,ns,interp=0)
         cs=.45
         timgb(fix(ns/2),ns*cs:ns-ns*cs-1)=mx     ; cross hairs
         timgb(ns*cs:ns-ns*cs-1,fix(ns/2))=mx
         tv,bytscl(timgb,min=mn,max=mx)           ; display the zoomed image
       endif

       if (dispvals ne 0) then begin
         pixtolatlon,alon,alat,x+1,y+1,head
         wset,30                                  ; now display the x,y,lat,lon,pixel values
         erase
         xyouts,ns/10,ns*.75/2,/device,'(x,y): '+string(x)+string(y)
         xyouts,ns/10,ns*.5/2,/device,'(lon, lat): '+string(alon,format="(F8.3)")+string(alat,format="(F8.3)")
         xyouts,ns/10,ns*.25/2,/device,'Value: '+string(sir(x,y),format="(F7.3)")  
       endif ; dispvals ne 0

       if (disphist ne 0) then begin
         wset,31                                  ; now display the histogram
         if (fixhr eq 0) then begin 
           mnh=min(timg)
           mxh=max(timg)
	   nh=50
         endif else begin
           mnh=mn
           mxh=mx
           nh=100
         endelse
         bnh=(mxh-mnh)/nh
         if (mxh gt mnh) then begin
           hist=histogram(timg,min=mnh,max=mxh,binsize=bnh)
           szh=size(hist)
           ox=findgen(szh(1))*bnh+mnh
           plot,ox,hist,xticks=4,xtitle='Value',ytitle='Population'
         endif else begin
           erase
         endelse
       endif  ; plothist ne 0
     endif  ; end if that checks if pixel is different
   endelse  ; end else that implies a left mouse click
endrep until (quit eq 1)

if (dispzoom ne 0) then wdelete,29
if (dispvals ne 0) then wdelete,30
if (disphist ne 0) then wdelete,31

end




