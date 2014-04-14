PRO pixtolatlon,alon,alat,x,y,info
;
;	GIVEN AN IMAGE PIXEL LOCATION (X,Y) (1..NSX,1..NSY)
;	COMPUTES THE LAT,LON COORDINATES (ALON,ALAT).   THE LAT,LON RETURNED 
;	CORRESPONDS TO THE LOWER-LEFT CORNER OF THE PIXEL.  IF LAT,LON
;	OF PIXEL CENTER IS DESIRED USE (X+0.5,Y+0.5) WHERE X,Y ARE INTEGER
;	VALUED PIXELS
;
;	NOTE:  WHILE ROUTINE WILL ATTEMPT TO CONVERT ANY (X,Y)
;	VALUES, ONLY (X,Y) VALUES WITH 1 <= X <= NSX+1 AND 1 <= Y <= NSY+1
;	ARE CONTAINED WITHIN IMAGE.
;
; INPUTS:
;   x,y - input pixel
;   info - image transformation info array from loadsir
;
; OUTPUTS:
;   alon,alat - longitude, latitude
;

; revised by DGL 15 Sept 2005 + corrected EASE

nsx=info(0)
nsy=info(1)
iopt=info(2)
xdeg=info(3)
ydeg=info(4)
ascale=info(5)
bscale=info(6)
a0=info(7)
b0=info(8)

if iopt eq -1 then begin		; IMAGE ONLY (CAN'T TRANSFORM!)
 THELON=(X-1.0)/ASCALE+A0
 THELAT=(Y-1.0)/BSCALE+B0
 ALON=THELON
 ALAT=THELAT
;
endif else if iopt eq 0 then begin      ; RECTALINEAR LAT/LONG
 THELON=(X-1.0)/ASCALE+A0
 THELAT=(Y-1.0)/BSCALE+B0
 ALON=THELON
 ALAT=THELAT
;
endif else if (iopt eq 1) or (iopt eq 2) then begin ; LAMBERT
 THELON=(X-1.0)/ASCALE+A0
 THELAT=(Y-1.0)/BSCALE+B0
 ilambert1,alat,alon,thelon,thelat,ydeg,xdeg,iopt
;
endif else if iopt eq 5 then begin	; polar stereographic
 THELON=(X-1.0)*ASCALE+A0
 THELAT=(Y-1.0)*BSCALE+B0
 ipolster,alon,alat,thelon,thelat,xdeg,ydeg
;
endif else if (iopt eq 11) or (iopt eq 12) or (iopt eq 13) then begin ; EASE
 THELON=X-XDEG-1.0+(XDEG+A0)
 THELAT=Y-YDEG-1.0+(YDEG+B0)
 ieasegrid,iopt,alon,alat,thelon,thelat,ascale
;
endif else print,'*** unknown SIR transformation'

;print,alon,alat

return
end






