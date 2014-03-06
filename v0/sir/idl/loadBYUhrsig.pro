PRO loadBYUhrsig,filename,outarray,nx,ny,code
;
; load a test BYU high res swath sigma-0 file
; D.G. Long 15 Dec. 2000
;
if n_elements(filename) eq 0 then begin
   print,"SYNTAX:    loadBYUhrsig,'filename',outarray,nx,ny,code"
   print,"            code: 1=H,forward; 2=H,aft; 3=V,forward; 4=V,aft; 5=lat; 6=long; 7=landflag"
   print," To view:  window,xsize=nx,ysize=ny"
   print,"           tv,bytscl(outarray,min=-60,max=0)  [for sigma-0]"
   print,"           tv,bytscl(outarray,min=-90,max=90) [for latitude]"
   print,"           tv,bytscl(outarray,min=0,max=360)  [for longitude]"
   print,"           tv,bytscl(outarray,min=0,max=1)  [for land]"
   print," or use: slide_image,bytscl(outarray,min=-60,max=0),xvisible=500,yvisible=760[for sigma-0]"
   goto, STOP
endif

; open input file

openr,1,filename

buff=intarr(760,/Nozero)

; read header record

readu,1,buff

version=buff(1)*0.1;
other_params=buff(2);
ngrid=buff(3);
wctgrid=buff(4);
watgrid=buff(5);
ctgrid=buff(6);
atgrid=buff(7);
natstart=buff(8);
natend=buff(9);
nctstart=buff(10);
nctend=buff(11);
irev=buff(12);
year=buff(13);
day=buff(14);
ascnode=buff(15)/180.0;
cnt_arr=buff(16);
inc_corr=buff(17);
angle_ref_inner=buff(18)/200.0;
angle_ref_outer=buff(19)/200.0;
az_ang_ex=buff(20)/200.0;
nits=buff(21);
its=buff(22);

;print,nctstart,nctend,natstart,natend,ngrid,atgrid,ctgrid

ny=nctend-nctstart+1;
nx=natend-natstart+1;

;print,'nx,ny,code',nx,ny,code

outarray=fltarr(nx,ny,/Nozero)

for i=0,nx-1 do begin
 readu,1,buff  ; lat
 if (code eq 5 or code eq 7) then begin
  for j=0,ny-1 do begin
   if (code eq 5) then begin
     outarray(i,j)=(abs(buff(j))-18001)/200.0  ; latitude
   endif else begin
     outarray(i,j)=0  ; ocean
     if (buff(j) lt 0) then outarray(i,j)=1  ; land
   endelse
   endfor
 endif

 readu,1,buff ; lon
 if (code eq 6) then begin
  for j=0,ny-1 do begin
   outarray(i,j)=buff(j)/180.0  ; longitude
   endfor
 endif

 for k=0,3 do begin
  readu,1,buff ; sigma-0 in dB
  if (code eq k+1) then begin
   for j=0,ny-1 do begin
    outarray(i,j)=buff(j)/300.0  ; sigma0s
    endfor
  endif

  if (other_params eq 1) then begin
   if (cnt_arr eq 1) then begin
    readu,1,buff ; skip cnt record
   endif
   readu,1,buff ; skip az angle record /180
   readu,1,buff ; skip inc angle record /200
  endif
 endfor
endfor

close,1

STOP:
end



