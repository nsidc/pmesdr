pro xsir,sir,head,mn=mn,mx=mx,dispzoom=dispzoom,dispvals=dispvals,disphist=disphist,fixhr=fixhr,maxdim=maxdim,ns=ns
; The xsir routine allows a user to interactively view a sir
;   format image.  The original image is read in using loadsir.pro. 
;   The routine displays four windows:
;     1 - Full sir image (size reduced if needed)
;     2 - Zoomed image around a mouse selected pixel
;     3 - x,y location, lat, lon, and value of selected pixel
;     5 - Histogram of the values in the zoom window
;
; Required inputs:
;    sir = a 2-D sir image (use loadsir.pro to read from disk: loadsir,'file_name.sir',sir,head )
;    head = the sir header returned from loadsir.pro
;
; Optional inputs:
;    mn = min value for image display (default is min of sir image)
;    mx = max value for image display (default is max of sir image)
;    dispzoom = zoom window display flag: 0=no, 1=yes (default)
;    dispvals = vals window display flag: 0=no, 1=yes (default)
;    disphist = histogram window display flag: 0=no (default), 1=yes
;    fixhr = fix histogram range flag: 0=use zoom min and max (default), 1=use mn and mx
;    maxdim = define max dimension for image resizing (default is 970 if the max dimension
;             of the sir image is larger than 970)
;    ns = zoom window size in pixels (odd)
;
; Called routines:
;    pixtolatlon.pro 
;      which, depending on sir file projection, calls 
;        ilambert1.pro
;        ipolster.pro
;        polster.pro
;        ieasegrid.pro
;
; (c) copyright BYU MERS 1999, 2000, 2002
;     Written by Quinn Remund, Aug. 1999
;     Revised by David Long, Sept. 2000 + use IDL-tuned widgets to add extra features
;     Revised by David Long, 30 April 2002 + help output,
;

if n_elements(sir) eq 0 then begin
   print,"SYNTAX:    xsir,sir,head, mn=,mx=,dispzoom=,dispvals=,disphist=,fixhr=,maxdim=,ns=>
   print,"   sir = a 2-D sir image array (use loadsir.pro to read from disk)"
   print,"   head = the sir header returned from loadsir.pro"
   print," optional args:"
   print,"   mn = min value for image display (default is min of sir image)"
   print,"   mx = max value for image display (default is max of sir image)"
   print,"   dispzoom = zoom window display flag: 0=no, 1=yes (default)"
   print,"   dispvals = vals window display flag: 0=no, 1=yes (default)"
   print,"   disphist = histogram window display flag: 0=no (default), 1=yes"
   print,"   fixhr = fix histogram range flag: 0=use zoom min and max (default), 1=use mn and mx"
   print,"   maxdim = define max dimension for image resizing (default is 970 if the max dimension"
   print,"             of the sir image is larger than 970)"
   print,"   ns = zoom window size in pixels (odd)"
   goto, STOP
endif

if (n_elements(head) eq 0) then begin
   print,"*** ihead array must be specified..."
   goto, STOP
endif

print
print,'Running xsir_idl...'
print,'Left click=pixel info, center=resolution switch, right=exit.'
print
!x.style=1
!y.style=1

; check the routine arguments
if (n_elements(fixhr) eq 0) then fixhr=0
if (n_elements(maxdim) eq 0) then maxdim=970
if (n_elements(mn) eq 0) then mn=min(sir)
if (n_elements(mx) eq 0) then mx=max(sir)
if (n_elements(ns) eq 0) then ns=251   ; zoom window size
if (n_elements(dispzoom) eq 0) then dispzoom=1  ; default zoom window on
if (n_elements(dispvals) eq 0) then dispvals=1  ; default display values window on
if (n_elements(disphist) eq 0) then disphist=1  ; default display histogram window on

res=0    ; start in reduce resolution mode
;res=1    ; start in full resolution mode

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


; define some important constants

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

quit=0
repeat begin  ; for resolution selection

; open the window and display the image

 base=widget_base()
 if (res eq 1) then begin
  draw=widget_draw(base,xsize=sz(1),ysize=sz(2),x_scroll_size=wx,y_scroll_size=wy,button_events=1,motion_events=1)
 endif else begin
  draw=widget_draw(base,xsize=wx,ysize=wy,button_events=1,motion_events=1)
 endelse
 widget_control,/realize,base
 widget_control,get_value=index,draw
 wset,index
 if (res eq 1) then begin
  tv,bytscl(sir,min=mn,max=mx)
 endif else begin
  tv,bytscl(congrid(sir,wx,wy),min=mn,max=mx)
 endelse

 oldx=-1
 oldy=-1
 button1_down=0

; begin the loop in which the user clicks on image pixels

 repeat begin
   wset,index
   event=widget_event(base)
   if (event.release eq 4) then quit=1  ; right button press
   if (event.release eq 2) then quit=1  ; center button press
   if (event.press eq 1) then button1_down=1  ; left button press
   if (event.release eq 1) then button1_down=0
   x=event.x
   y=event.y
   ;print,'event',event.x,event.y
   if (button1_down) then begin
     if (res ne 1) then begin
       x=fix(float(x)*float(sz(1))/float(wx))     ; convert to original image pixel
       y=fix(float(y)*float(sz(2))/float(wy))
     endif
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

       if ((ux ge lx) and (uy ge ly) and (tux ge tlx) and (tuy ge tly)) then begin       
          timg(tlx:tux,tly:tuy)=sir(lx:ux,ly:uy)   ; define the zoom region
       endif

       if (dispzoom ne 0) then begin            ; display the zoom region
         wset,29
         timgb=congrid(timg,ns,ns,interp=0)
         cs=.45
         timgb(fix(ns/2),ns*cs:ns-ns*cs-1)=mx     ; cross hairs
         timgb(ns*cs:ns-ns*cs-1,fix(ns/2))=mx
         tv,bytscl(timgb,min=mn,max=mx)           ; display the zoomed image
       endif

       if (dispvals ne 0) then begin              ; display pixel info
         pixtolatlon,alon,alat,x+1,y+1,head
         wset,30                                  ; now display the x,y,lat,lon,pixel values
         erase
         xyouts,ns/10,ns*.75/2,/device,'x,y:      '+string(x)+string(y)
         xyouts,ns/10,ns*.5/2,/device,'lon, lat: ('+string(alon,format="(F8.3)")+','+string(alat,format="(F8.3)")+')'
	 alon1=fix(alon)
	 alon11=fix((abs(alon)-abs(alon1))*60)
	 alat1=fix(alat)
	 alat11=fix((abs(alat)-abs(alat1))*60)
         xyouts,ns/10,ns*.4/2,/device,'         ('+string(alon1,format="(I4)")+string(alon11,format="(I3)")+''', '+string(alat1,format="(I3)")+string(alat11,format="(I3)")+''')'
	 dval=0
	 if ((x gt 0) and (x lt sz(1)) and (y gt 0) and (y lt sz(2))) then begin
	   dval=sir(x,y)
	 endif	
         xyouts,ns/10,ns*.25/2,/device,'Value:    '+string(dval,format="(F7.3)")  
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
   endif
 endrep until (quit eq 1) ; end mouse event loop

 if (event.release eq 2) then begin ; change resolution
  widget_control, base, /destroy
  if (res eq 1) then begin
   res=0
  endif else begin
   if (res eq 0) then res=1
  endelse
  quit=0
 endif

endrep until (quit eq 1)  ; end resolution repeat

; clear up windows

widget_control, base, /destroy
if (dispzoom ne 0) then wdelete,29
if (dispvals ne 0) then wdelete,30
if (disphist ne 0) then wdelete,31

STOP:
end




