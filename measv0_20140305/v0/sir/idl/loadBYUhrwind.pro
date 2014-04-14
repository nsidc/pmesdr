PRO loadBYUhrwind,filename,speed,dir,lat,lon,land,nat,nct
;
; load a test BYU high res wind field file
;
if n_elements(filename) eq 0 then begin
   print,"SYNTAX:    loadBYUhrwind,'filename',speed,dir,lat,lon,land,nat,nct
   print," To view:  window,xsize=nat,ysize=nct"
   print,"           tv,bytscl(speed,min=0,max=30)   [speed sel. amb]"
   print,"           tv,bytscl(dir,min=-180,max=1800)    [dir sel. amb]"
   print,"           tv,bytscl(lat,min=-90,max=90)   [latitude]"
   print,"           tv,bytscl(lon,min=-180,max=180) [longitude]"
   print,"           tv,bytscl(land,min=0,max=1)     [land flag]"
   print," or: slide_image,bytscl(speed,min=0,max=30),order=1,yvisible=760,xvisible=500"
   goto, STOP
endif

; open input file

openr,1,filename

; read file header info  
; (note: There dummy I*4's at start, end of records since file wa
;        written from fortran with unformatted variable length records.
;        These are ignored.)

head1=lonarr(7,/Nozero) 
head2=lonarr(6,/Nozero) 
head3=lonarr(6,/Nozero) 

readu,1,head1 ; ngrid,wctgrid,watgrid,ctgrid,atgrid
readu,1,head2 ; natstart,natend,nctstart,nctend
readu,1,head3 ; iastart,iaend,icstart,icend

iastart=head3(1);
iaend=head3(2);
icstart=head3(3);
icend=head3(4);

ascnode=fltarr(1,/Nozero)
readu,1,dummy
readu,1,ascnode
temp=lonarr(4,/Nozero)
readu,1,temp ; irev,iyear,iday

nat=iaend-iastart+1 ; along-track size of useful data
nct=icend-icstart+1 ; cross-track size of useful data

nchoice=intarr(nat,nct,/Nozero) ; ambiguity choice

speed=fltarr(nat,nct)  ; selected ambiguity speed
dir=fltarr(nat,nct)    ; selected ambiguity speed

speeds=fltarr(nat,nct,4) ; ambiguity speeds
dirs=fltarr(nat,nct,4)   ; ambiguity directions

land=bytarr(nat,nct) ; land flag (ocean=0,land=1)
lat=fltarr(nat,nct)  ; latitude
lon=fltarr(nat,nct)  ; longitude

; read remainder of file

dummy=intarr(2)  ; dummy integer*4
a=intarr(8)      ; array to extract wind speed and direction
b=intarr(1)      ; array to extract ambiguity selection choice
loc=intarr(2)    ; location array

for i=0,nat-1 do begin
 readu,1,dummy
 for j=0,nct-1 do begin
  readu,1,b  ; read ambiguity selection flag
  nchoice(i,j)=b
  readu,1,a  ; read ambiguity speeds and directions

  for k=0,3 do begin
   speeds(i,j,k)=a(2*k)/200.0
   dirs(i,j,k)=a(2*k+1)/180.0
  endfor

  speed(i,j)=speeds(i,j,1)  ; default first alias
  dir(i,j)=dirs(i,j,1)

  if (nchoice(i,j) gt 0) then begin
    speed(i,j) = speeds(i,j,nchoice(i,j)-1)
    dir(i,j) = dirs(i,j,nchoice(i,j)-1)
  endif

;  print,i,j,nchoice(i,j),speed(i,j),dir(i,j)

 endfor

 for j=0,nct-1 do begin
  readu,1,loc ; read locations

  land(i,j)=0 ; ocean
  if (loc(0) lt 0) then land(i,j)=1 ; land

  lat(i,j)=(abs(loc(0))-18001)/200.0  ; latitude
  lon(i,j)=loc(1)/180.0          ; longitude

 endfor

 readu,1,dummy
endfor

close,1

STOP:
end


