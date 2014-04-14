PRO loadpartsir,filename,xll0,yll0,xur0,yur0,outarray,info,printflag,sensor,type,title,tag,cproc,ctime,des,iaopt
;
; load part of an image from a BYU SIR file format v3.0
;
; written by dgl 1 Jan 2001 at BYU
; revised by dgl 7 Nov 2001 at BYU + fixed lambert scale factor printout
; revised by dgl 9 Sep 2003 at BYU + added more comments
;
; info, printflag, and the rest are optional. sensor...ctime are byte arrays
;  standard header values are printed if printflag=1
;
; note this code is written for big_endian machines (e.g., HP, SUN)
; use of /swap_if_little_endian in openr statement ensures proper
; byte decoding on little endian (intel) machines
;
; This routine assumes image origin in lower left corner (SIR
; convention and IDL default) 
;
if n_elements(filename) eq 0 then begin
   print,"SYNTAX:    loadpartsir,filename,xll,yll,xur,yur,out,<info,pflag,sensor,type,title,tag,cproc,ctime,des,iaopt>"
   print,"  xll,yll,xur,yur=lower-left,upper-right corners of sub image (SIR convention)"
   print,"    (0,0,0,0 will read entire image)"
   print,"  out=returned float image"
   print,"  info=projection info from file header (see code for details)"
   print,"  pflag=1 prints SIR file header, other values suppress print"
   print,"  sensor...ctime=returned header strings"
   print,"  optional SIR header parameters: des=header str, iaop=head ints"
   print
   print," To view output image:"
   print,"      window,xsize=info(0),ysize=info(1)"
   print,"      tv,order=0,bytscl(out,min=info(17),max=info(18))"
   goto, STOP
endif

; test for invalid arguments
xll=long(xll0)
yll=long(yll0)
xur=long(xur0)
yur=long(yur0)

if xll eq 0 and yll eq 0 and xur eq 0 and yur eq 0 then begin
endif else begin
    if xll eq 0 or yll eq 0 or xur eq 0 or yur eq 0 then begin
        print,"*** error in loadpartsir: region corner must be in SIR 1-based convention"
        return
    endif
endelse


;
; contents of returned info array (summary of SIR header info)
;
; info(0) nsx        number of X dimension pixels
; info(1) nsy        number of Y dimension pixels
; info(2) iopt       transformation type
; info(3) xdeg       X dimension span or center point
; info(4) ydeg       Y dimension span or center point
; info(5) ascale     X dimension scale factor
; info(6) bscale     Y dimension scale factor
; info(7) a0         X dimension offset or origin
; info(8) b0         Y dimension offset or origin
; info(9)            spare
; info(11) iyear     year of start data
; info(12) isday     day of start data
; info(13) ismin     minute of day of start data
; info(14) ieday     day of end data
; info(15) iemin     minute of day of end data
; info(16) anodata   no data value
; info(17) vmin      minimum useful value
; info(18) vmax      maximum useful value
; info(19) idatatype data type code (1=byte, 2=i*2 [preferred], 4=float)
; info(20) ioff      data value offset  
; info(21) iscale    data value scale
; info(22) iregion   region id code
; info(23) itype     data type id code  standard values: 0=unknown or n/a
;                                       1=scatt A, 2=scatt B, 3=Tb, 9=topo
; info(24) ipol      polarization code: 0=n/a, 1=H, 2=V
; info(25) ifreqhm   frequency in GHz
; info(26) ixdeg_off xdeg offset factor
; info(27) iydeg_off ydeg offset factor
; info(28) ideg_sc   xdeg/ydeg scale factor
; info(29) iscale_sc ascale/bscale scale factor
; info(30) ia0_off   a0 offset factor
; info(31) ib0_off   b0 offset factor
; info(32) i0_sc     a0/b0 scale factor
; info(33) nhtype    header version number
; info(34) ldes      number of extra description characters
; info(35) nia       number of extra integers in header

; open input file (SIR files are stored in big_endian order)

openr,1,filename,/swap_if_little_endian

; read first header record

head=intarr(256,/Nozero)
readu,1,head

;
; This is the description of the version 3.0 SIR file header
; The SIR file header consists of one or more 512 byte blocks.
; The first gives key transformation, the number of additional blocks,
; and other information.  The additional blocks are optional.
;
; While the contents of the SIR file are always represented by
; floating point values, version 3.0 permits storage as bytes, i*2, or
; floats.  Floats are not fully portable and are not encouraged.
;
; First header block is defined as
;
;  head(0) is x dimension size in pixels
;  head(1) is y dimension size in pixels     
;  head(2-3,5-9,16) are the projection transformation information
;  head(2) is xdeg
;  head(3) is ydeg
;  head(4) is the header type
;  head(5) is ascale
;  head(6) is bscale
;  head(7) is a0
;  head(8) is b0
;  head(9) is the image offset factor
;  head(10) is the image scale factor
;  head(11) is the data year
;  head(12) is the data starting day or year
;  head(13) is the data starting minute of day
;  head(14) is the data ending day or year
;  head(15) is the data ending minute of day
;  head(16) is the image projection type  (-1 = none, 
;       0 = rectalinear lat, lon
;	1 or 2 = lambert equal area
;	5 = polar stereographic
;	11,12,13 = EASE grids)
;  head(17) is the region id number
;  head(18) is the type code (0=n/a, 1=A, 2=B, 3=Tb, etc.)
;  head(19-38) contain 40 characters of sensor description
;  head(39) is iscale_sc
;  head(40) is the total number of 512 byte header blocks
;  head(41) is the number of 512 byte desccription header blocks
;  head(42) is the number of bytes of extra description
;  head(43) is the number of extra i*2 data points
;  head(44) is the polarization code: 0=n/a, 1=H, 2=V
;  head(45) is the frequency in hundreds of MHz
;  head(46) is spare
;  head(47) is the data type 0,2=i*2,1=i*1,4=i*4
;  head(48) is the integer no data value
;  head(49) is the integer minimum useful value from creator program
;  head(50) is the integer maximum useful value from creator program
;  head(51,52) is the float no data value
;  head(53,54) is the float minimum useful value from creator program
;  head(55,56) is the float maximum useful value from creator program
;  head(57-125) contain 138 characters of data type description
;  head(126) is ixdeg_off
;  head(127) is iydeg_off
;  head(128-167) contain 80 characters of title
;  head(168) is ideg_sc
;  head(169-188) contain 40 characters of tag
;  head(189) is ia0_off
;  head(190-239) contain 100 characters describing creator process
;  head(240) is ib0_off
;  head(241-254) contain 28 characters containing creation time
;  head(255) is i0_sc
;
; there are ndes additional header blocks containing additional
; character data and nhead-hdes-1 additional blocks containing
; optional integer data.  By convention, the first integer of the
; optional integer arrays is a code value to indicate the contents
; of the rest of the array.
;

nhtype = head(4)
nhead = head(40)
ndes = head(41)
if nhtype lt 20 then begin  ; override extra header blocks for old header 
  nhead = 1
  ndes = 0
  nia = 0
endif

if nhead gt 1 then begin  ; skip extra header blocks

 print,'Reading extra header blocks',nhead
 head2=intarr(256,/Nozero)

 if ndes gt 0 then begin  ; read extra string description blocks
  ldes=head(42)
  str=bytarr(ldes+1)
  k=0
  for i=1,ndes do begin
   readu,1,head2
   for i=0,255 do begin
    if k lt ldes then str(k)=head2(i) mod 256
    k=k+1
    if k lt ldes then str(k)=head2(i)/256
    k=k+1
   endfor
  endfor
  if N_params(0) ge 4 then begin  ; print header info
    print,"Extra header text: '",strtrim(string(str)),"'"
  endif
 endif

 if nhead-ndes-1 gt 0 then begin  ; read extra integer blocks
  for i=1,nhead-ndes-1 do begin
   readu,1,head2
  endfor
 endif
endif

; set up output info array containing pixel transformation info

info1=fltarr(36)

; keep image transformation information

info1(0)=head(0)  ; nsx
info1(1)=head(1)  ; nsy
info1(2)=head(16) ; iopt
iopt=head(16)
  
; get scale factors for converting projection info

info1(26)=head(126) ; ixdeg_off
info1(27)=head(127) ; iydeg_off
info1(28)=head(168) ; ideg_sc
info1(29)=head(39)  ; iscale_sc
info1(30)=head(189) ; ia0_off
info1(31)=head(240) ; ib0_off
info1(32)=head(255) ; i0_sc

nhtype=head(4)
if nhtype lt 20 then nhtype=1
info1(33)=nhtype
if nhtype lt 30 then begin ; set default header version 2.0 values
  if iopt eq -1 then begin  ; image only
    info1(26)=0
    info1(27)=0
    info1(28)=10
    info1(29)=1000
    info1(30)=0
    info1(31)=0
    info1(32)=100
  endif else if iopt eq 0 then begin ; rectalinear lat/long
    info1(26)=-100
    info1(27)=0
    info1(28)=100
    info1(29)=1000
    info1(30)=0
    info1(31)=0
    info1(32)=100
  endif else if (iopt eq 1) or (iopt eq 2) then begin ; lambert
    info1(26)=0
    info1(27)=0
    info1(28)=100
    info1(29)=1000
    info1(30)=0
    info1(31)=0
    info1(32)=1
  endif else if iopt eq 5 then begin ; polar stereographic
    info1(26)=-100
    info1(27)=0
    info1(28)=100
    info1(29)=1000
    info1(30)=0
    info1(31)=0
    info1(32)=1
  endif else if (iopt eq 11) or (iopt eq 12) or (iopt eq 13) then begin ; EASE
    info1(26)=0
    info1(27)=0
    info1(28)=10
    info1(29)=1000
    info1(30)=0
    info1(31)=0
    info1(32)=10
  endif else begin    ; unknown
    info1(26)=0
    info1(27)=0
    info1(28)=100
    info1(29)=1000
    info1(30)=0
    info1(31)=0
    info1(32)=100
 endelse
endif

; default projection values

info1(3)=head(2)/info1(28) - info1(26) ; xdeg
info1(4)=head(3)/info1(28) - info1(27) ; ydeg
info1(5)=head(5)/info1(29)             ; ascale
info1(6)=head(6)/info1(29)             ; bscale
info1(7)=head(7)/info1(32) - info1(30) ; a0
info1(8)=head(8)/info1(32) - info1(31) ; b0

; special cases

iopt = info1(2)

if iopt eq -1 then begin  ; image only
endif else if iopt eq 0 then begin ; rectalinear lat/lon
endif else if (iopt eq 1) or (iopt eq 2) then begin ; lambert
 info1(5)=info1(29)/head(5)
 info1(6)=info1(29)/head(6)
endif else if iopt eq 5 then begin ; polar stereographic
endif else if (iopt eq 11) or (iopt eq 12) or (iopt eq 13) then begin
 info1(5)=2.0d0*(head(5)/info1(29))*6371.228d0/25.067525d0
 info1(6)=2.0d0*(head(6)/info1(29))*25.067525d0
endif else print,'*** Unrecognized SIR option in loadsir'

; store additional useful data

info1(11)=head(11) ; year
info1(12)=head(12) ; start day
info1(13)=head(13) ; start min
info1(14)=head(14) ; end day
info1(15)=head(15) ; end min

idatatype=head(47)
if head(10) eq 0 then begin
  head(10)=1
  had(40)=0
endif

s=1./head(10)
soff=32767.0/head(10)
if idatatype eq 1 then soff=128.0/head(10)
ioff=head(9)

; store nodata value and min,max

info1(16)=head(48)*s+soff+ioff  ; nodata value  
info1(17)=head(49)*s+soff+ioff  ; vmin
info1(18)=head(50)*s+soff+ioff  ; vmax
info1(19)=idatatype             ; idatatype
info1(20)=head(9)               ; ioff
info1(21)=head(10)              ; iscale
info1(22)=head(17)              ; iregion
info1(23)=head(18)              ; itype
info1(24)=head(44)              ; ipol
info1(25)=head(45)*0.1          ; ifreqhm

; check for extra header records

if head(40) gt 1 then begin
  ndes=head(41) ; extra headers
  ldes=head(42) ; number of description bytes
  nia=head(43)  ; number of extra integers
  info1(34)=ldes
  info1(35)=nia

  if ldes gt 0 then begin
    des1=bytarr(ldes)
    readu,1,des1
    if ldes > 512*(ldes/512) then begin
      dummy=bytarr(ldes-512*(ldes/512))
      readu,1,dummy
    endif
  endif
  
  if nia gt 0 then begin
    iaopt1=intarr(nia)
    readu,1,iaopt1
    if nia > 512*(nia/512) then begin
      dummy=inttarr(nia-512*(nia/512))
      readu,1,dummy
    endif
  endif
endif else begin
  info1(34)=0
  info1(35)=0
endelse

if idatatype eq 4 then begin  ; floats (not fully portable)
 ; for floats, also use float values for nodata, vmin, vmax from header
   info1(16)=float(head,51*2)  ; nodata value
   info1(17)=float(head,53*2)  ; vmin
   info1(18)=float(head,55*2)  ; vmax
endif

if xll eq 0 and yll eq 0 and xur eq 0 and yur eq 0 then begin
    xll=long(1)
    yll=long(1)
    xur=long(head(0))
    yur=long(head(1))
endif

; read image data from file and convert to floating point array

if xll eq 1 and yll eq 1 and xur eq head(0) and yur eq head(1) then begin
  ; read entire image

 if idatatype eq 1 then begin             ; bytes
   body=bytarr(head(0),head(1),/Nozero)
   readu,1,body
   temp=fltarr(head(0),head(1),/Nozero)
   temp=s*(body)+soff+ioff
 endif else if idatatype eq 4 then begin  ; floats (not fully portable)
   temp=fltarr(head(0),head(1),/Nozero)
   readu,1,temp 
 endif else begin                         ; two byte integers (default)
   body=intarr(head(0),head(1),/Nozero)
   readu,1,body
   temp=fltarr(head(0),head(1),/Nozero)
   temp=s*(body)+soff+ioff
 endelse

endif else begin  ; read only part of image

 temp=fltarr(xur-xll+1,yur-yll+1,/Nozero)
 if idatatype eq 1 then begin             ; bytes
   body=bytarr(xur-xll+1,/Nozero)
   for j=yll,yur do begin
     offset=((j-1)*long(head(0))+(xll-1))+512*long(nhead)
     point_lun,1,offset
     readu,1,body
     for i=0,xur-xll do begin
       temp(i,j-yll)=body(i)
     endfor
   endfor
   temp=s*(temp)+soff+ioff
 endif else if idatatype eq 4 then begin  ; floats (not fully portable)
   body=fltarr(xur-xll+1,/Nozero)
   for j=yll,yur do begin
     offset=4*((j-1)*long(head(0))+(xll-1))+512*long(nhead)
     point_lun,1,offset
     readu,1,body
     for i=0,xur-xll do begin
       temp(i,j-yll)=body(i)
     endfor
   endfor
 endif else begin                         ; two byte integers (default)
   body=intarr(xur-xll+1,/Nozero)
   for j=yll,yur do begin
     offset=2*((j-1)*long(head(0))+(xll-1))+512*long(nhead)
     point_lun,1,offset
     readu,1,body
     for i=0,xur-xll do begin
       temp(i,j-yll)=body(i)
     endfor
   endfor
   temp=s*(temp)+soff+ioff
 endelse

 ; alter projection information so that SIR transformation code can be used
 
 if iopt eq -1 then begin  ; image only
   info1(7)=(xll-1)/info1(5)+info1(7) ; a0
   info1(8)=(yll-1)/info1(6)+info1(8) ; b0
 endif else if iopt eq 0 then begin ; rectalinear lat/lon
   info1(7)=(xll-1)/info1(5)+info1(7) ; a0
   info1(8)=(yll-1)/info1(6)+info1(8) ; b0
 endif else if (iopt eq 1) or (iopt eq 2) then begin ; lambert
   info1(7)=(xll-1)/info1(5)+info1(7) ; a0
   info1(8)=(yll-1)/info1(6)+info1(8) ; b0
 endif else if iopt eq 5 then begin ; polar stereographic
   info1(7)=(xll-1)*info1(5)+info1(7) ; a0
   info1(8)=(yll-1)*info1(6)+info1(8) ; b0
 endif else if (iopt eq 11) or (iopt eq 12) or (iopt eq 13) then begin
   info1(7)=(xll-1)+info1(7) ; a0
   info1(8)=(yll-1)+info1(8) ; b0
 endif
 info1(0)=xur-xll+1
 info1(1)=yur-yll+1

endelse

close,1

if nhtype lt 20 then begin  ; fill in information for old header
  nn=min(temp,max=mm)
  info1(17)=nn
  info1(18)=mm
  info1(16)=info1(17)
  if nn eq -32 then begin
    info1(23)=1 ; A
  endif else begin
    info1(23)=2 ; B
  endelse
  info1(24)=2 ; v pol
  info1(25)=53
endif


; return output image

outarray=temp

; set output info array

if N_params(0) ge 7 then info=info1

; set output strings
  
sensor1=bytarr(40)
for i=1,20 do begin
 j=2*i-2
 sensor1(j)=head(18+i) mod 256
 sensor1(j+1)=head(18+i)/256
endfor
if N_params(0) ge 9 then sensor=string(sensor1)

type1=bytarr(138)
for i=1,69 do begin
 j=2*i-2
 type1(j)=head(56+i) mod 256
 type1(j+1)=head(56+i)/256
endfor
if N_params(0) ge 10 then type=string(type1)

title1=bytarr(80)
for i=1,40 do begin
 j=2*i-2
 title1(j)=head(127+i) mod 256
 title1(j+1)=head(127+i)/256
endfor
if N_params(0) ge 11 then title=string(title1)

tag1=bytarr(40)
for i=1,20 do begin
 j=2*i-2
 tag1(j)=head(168+i) mod 256
 tag1(j+1)=head(168+i)/256
endfor
if N_params(0) ge 12 then tag=string(tag1)

cproc1=bytarr(100)
for i=1,50 do begin
 j=2*i-2
 cproc1(j)=head(189+i) mod 256
 cproc1(j+1)=head(189+i)/256
endfor
if N_params(0) ge 13 then cproc=string(cproc1)

ctime1=bytarr(28)
for i=1,14 do begin
 j=2*i-2
 ctime1(j)=head(240+i) mod 256
 ctime1(j+1)=head(240+i)/256
endfor
if N_params(0) ge 14 then ctime=string(ctime1)
 
if N_params(0) ge 15 then begin ; read extra description string
 if head(40) gt 1 then begin
   if ldes gt 0 then des=string(des1) else des="" ; bytarr(1)
   endif else des="" ; bytarr(1)
endif

if N_params(0) ge 16 then begin ; read extra integers
  if head(40) gt 1 then begin
    if nia gt 0 then iaopt=iopt1 else iaopt=intarr(1)
  endif else iaopt=intarr(1)
endif
  
if N_params(0) ge 8 then begin ; print header info 
 if printflag eq 1 then begin
  
 print,"SIR file '",filename,"'"
 print,' Size:    ',info1(0),' by ',info1(1)
 print,' Year: ',head(11),'  JD range: ',head(12),' to ',head(14)
 print," Sensor:  '",strtrim(string(sensor1)),"'"
 print," Title:   '",strtrim(string(title1)),"'"
 if head(4) gt 16 then begin
  print," Type:    '",strtrim(string(type1)),"'"
  print," Tag:     '",strtrim(string(tag1)),"'"
  print," Creator: '",strtrim(string(cproc1)),"'"
  print," Created: '",strtrim(string(ctime1)),"'"
  print,' NoData: ',info1(16),'  Vmin: ',info1(17),'  Vmax: ',info1(18)
  print,' Frequency: ',info1(25),'  Polarization: ',info1(24),' Head: ',info1(33)
 endif else begin
  print,' (old header style 1)'
 endelse

 if iopt eq -1 then begin
  print," Rectangular image-only form"
  print,"  x,y scale:  ",info1(5),info1(6)
  print,"  x,y offset: ",info1(7),info1(8)
  print,"  x,y span:   ",info1(3),info1(4)
 endif else if iopt eq 0 then begin
  print," Lat/Long Rectangular form"
  print,"  X,Y scale (pix/deg):     ",info1(5),info1(6)
  print,"  X (Lon), Y (Lat) offset: ",info1(7),info1(8)
  print,"  Lon,Lat spans:           ",info1(3),info1(4)
 endif else if (iopt eq 1) or (iopt eq 2) then begin
  if iopt eq 1 then print," Lambert equal-area project form (global radius)" else print," Lambert equal-area project form (Local radius)"
  print,"  X,Y scale (pix/km):     ",info1(5),info1(6)
  print,"  Lower-left corner (km): ",info1(7),info1(8)
  print,"  Center (Lon,Lat):       ",info1(3),info1(4)
 endif else if iopt eq 5 then begin
  print," Polar Stereographic form"
  print,"  X,Y scale (km/pix):  ",info1(5),info1(6)
  print,"  Lower-left corner:   ",info1(7),info1(8)
  print,"  Reference (Lon,Lat): ",info1(3),info1(4)
 endif else if (iopt eq 11) or (iopt eq 12) then begin
  print," EASE Polar Azimuth form"
  print,"  Col, Row Scales:  ",info1(5),info1(6)
  print,"  Origin (col,row): ",info1(7),info1(8)
  print,"  Center (col,row): ",info1(3),info1(4)
 endif else if iopt eq 13 then begin
  print," EASE Cylindrical form"
  print,"  Col, Row Scales:  ",info1(5),info1(6)
  print,"  Origin (col,row): ",info1(7),info1(8)
  print,"  Center (col,row): ",info1(3),info1(4)
 endif else print,'*** Unrecognized SIR option in loadsir'
endif
endif

STOP:
end


