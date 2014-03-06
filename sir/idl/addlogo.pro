function addlogo,array,logo_size,logo_corner,logo_value
;
; add a MERS logo the array
;
; written by dgl 6 May 1997
;
if n_params() lt 4 then begin
  print," addlogo SYNTAX:  out=addlogo(array, logo_size, logo_corner, logo_value)"
  print,"  logo_size: 0,1,2,3   logo_corner: 0=ll,1=lr,2=ur,3=ul"
endif

case logo_size of
 0: begin
	xs=68
	ys=79	
	openr,1,'/mers0/long/logos/MLogo68x79'
    end
 1: begin
	xs=91
	ys=104	
	openr,1,'/mers0/long/logos/MLogo91x104'
    end
 2: begin
	xs=110
	ys=128	
	openr,1,'/mers0/long/logos/MLogo110x128'
    end
 3: begin
	xs=141
	ys=161
	openr,1,'/mers0/long/logos/MLogo141x161'
    end
 else: begin
	message,'*** error in selection of logo size in addlogo'
	goto, STOP
       end
endcase

logo=bytarr(xs,ys,/Nozero)
readu,1,logo
close,1
logo=rotate(logo,7)

b=size(array)
xsize=b(1)
ysize=b(2)
mask=bytarr(xsize,ysize)

case logo_corner of
 0: mask(0:xs-1,0:ys-1)=logo
 1: mask(xsize-xs:xsize-1,0:ys-1)=logo
 2: mask(xsize-xs:xsize-1,ysize-ys:ysize-1)=logo
 3: mask(0:xs-1,ysize-ys:ysize-1)=logo
 else: message,'*** error in selection of logo corner in addlogo'
endcase

where=mask eq 0

return, where*array+mask*logo_value

STOP:
end





