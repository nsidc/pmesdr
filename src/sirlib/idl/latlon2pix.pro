PRO latlon2pix,alon,alat,x,y,info
;
;	CONVERT A LAT,LON COORDINATE (ALON,ALAT) TO AN IMAGE PIXEL LOCATION
;	(X,Y) (IN FLOATING POINT) AND IMAGE X,Y COORDINATE (THELON,THELAT).
;	TO COMPUTE INTEGER PIXEL INDICES (IX,IY): CHECK TO INSURE
;	1 <= X < NSX+1 AND 1 <= X < NSX+1 THEN IX=IFIX(X) IY=IFIX(Y)
;
; INPUTS:
;   alon,alat - longitude, latitude
;   info - image transformation info array from loadsir
;
; OUTPUTS:
;   x,y - input pixel values in floating point
;

; revised by DGL 12 Mar 2014 + added EASE2

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
 X=ASCALE*(ALON-A0)+1.0
 Y=BSCALE*(ALAT-B0)+1.0
;
endif else if iopt eq 0 then begin	; RECTALINEAR LAT/LONG
 THELON=ALON
 THELAT=ALAT
 X=ASCALE*(THELON-A0)+1.0
 Y=BSCALE*(THELAT-B0)+1.0
;
endif else if (iopt eq 1) or (iopt eq 2) then begin ; LAMBERT
 lambert1,alat,alon,thelon,thelat,ydeg,xdeg,iopt
 X=(THELON-A0)*ASCALE+1.0
 Y=(THELAT-B0)*BSCALE+1.0
;
endif else if iopt eq 5 then begin	; polar stereographic
 polster,alon,alat,thelon,thelat,xdeg,ydeg
 X=(THELON-A0)/ASCALE+1.0
 Y=(THELAT-B0)/BSCALE+1.0
;
endif else if (iopt eq 8) or (iopt eq 9) or (iopt eq 10) then begin ; EASE2
 ease2grid,iopt,alat,alon,thelon,thelat,ascale,ascale
 THELON=THELON+XDEG
 THELAT=THELAT+YDEG
 X=THELON+1.0-(XDEG+A0)
 Y=THELAT+1.0-(YDEG+B0)
;
endif else if (iopt eq 11) or (iopt eq 12) or (iopt eq 13) then begin ; EASE1
 easegrid,iopt,alat,alon,thelon,thelat,ascale
 THELON=THELON+XDEG
 THELAT=THELAT+YDEG
 X=THELON+1.0-(XDEG+A0)
 Y=THELAT+1.0-(YDEG+B0)
;
endif else print,'*** unknown SIR transformation'

;print,x,y

return
end



