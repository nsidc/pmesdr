PRO writesir,filename,outarray,info,sensor_s,type_s,title_s,tag_s,cproc_s,ctime_s,autoset
;
; write an image using the BYU SIR file format v3.0
;
; written by dgl 26 Mar 1997 at BYU
; revised by dgl  9 Sep 2003 at BYU + version 3.0 header , added comments,
;                                     modified autoscaling
;
; note this code is written for big_endian machines (e.g., HP, SUN)
; use of /swap_if_little_endian in openr statement ensures proper
; byte decoding on little endian (intel) machines
;
; This routine assumes image origin in lower left corner (SIR
; convention and IDL default) 
;
if n_elements(filename) eq 0 then begin
   print,"SYNTAX: writesir,'filename',outarray,info,sensor_s,type_s,title_s,tag_s,cproc_s,ctime_s,autoset"
   print,"variables beyond info optional, autoset=1 automaticly computes the scaling"
   goto, STOP
endif
;
; if autoset is present and non-zero, ioff and iscale are set from
; the image array rather than using the values in the info array
; it is recommended that other values be copied from another SIR file
; header as it there is no error checking on the header values
;

; info is an array containing header information
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
; info(17) vmin      minimum useful value (can be overridden by autoset)
; info(18) vmax      maximum useful value (can be overridden by autoset)
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
;	8-13 = EASE grids)
;  head(17) is the region id number
;  head(18) is the type code
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

; open output file

openw,1,filename,/swap_if_little_endian

;print,"writesir: writing to file: '",filename,"'"

; generate first header block

head=intarr(256,/Nozero)

head(0)=info(0)  ; nsx
head(1)=info(1)  ; nsy
head(4)=30       ; force header type code

iopt=info(2)
xdeg=info(3)
ydeg=info(4)
ascale=info(5)
bscale=info(6)
a0=info(7)
b0=info(8)

nhtype=info(33)
if nhtype lt 20 then begin  ; fill in information for old header
  nhtype=30
  nn=min(outarray,max=mm)
  info(17)=nn
  info(18)=mm
  info(16)=info(17)
  if nn eq -32 then begin
    info(23)=1 ; default A
  endif else begin
    info(23)=2 ; default B
  endelse
  info(24)=2 ; v pol
  info(25)=53
; set default header version 2.0 values
  if iopt eq -1 then begin  ; image only
    info(26)=0
    info(27)=0
    info(28)=10
    info(29)=1000
    info(30)=0
    info(31)=0
    info(32)=100
  endif else if iopt eq 0 then begin ; rectalinear lat/long
    info(26)=-100
    info(27)=0
    info(28)=100
    info(29)=1000
    info(30)=0
    info(31)=0
    info(32)=100
  endif else if (iopt eq 1) or (iopt eq 2) then begin ; lambert
    info(26)=0
    info(27)=0
    info(28)=100
    info(29)=1000
    info(30)=0
    info(31)=0
    info(32)=1
  endif else if iopt eq 5 then begin ; polar stereographic
    info(26)=-100
    info(27)=0
    info(28)=100
    info(29)=1000
    info(30)=0
    info(31)=0
    info(32)=1
  endif else if (iopt eq 8) or (iopt eq 9) or (iopt eq 10) then begin ; EASE2
    info(26)=0
    info(27)=0
    info(28)=10
    info(29)=100
    info(30)=0
    info(31)=0
    info(32)=10
  endif else if (iopt eq 11) or (iopt eq 12) or (iopt eq 13) then begin ; EASE1
    info(26)=0
    info(27)=0
    info(28)=10
    info(29)=1000
    info(30)=0
    info(31)=0
    info(32)=10
  endif else begin    ; unknown
    info(26)=0
    info(27)=0
    info(28)=100
    info(29)=1000
    info(30)=0
    info(31)=0
    info(32)=100
 endelse
endif

head(4)=nhtype       ; header type code

; get scale factors for converting projection info
ixdeg_off=info(26)
iydeg_off=info(27)
ideg_sc=info(28)
iscale_sc=info(29)
ia0_off=info(30)
ib0_off=info(31)
i0_sc=info(32)

; store default projection scale factors
head(2) = nint((xdeg + ixdeg_off) * ideg_sc)
head(3) = nint((ydeg + iydeg_off) * ideg_sc)
head(5) = nint(ascale * iscale_sc)
head(6) = nint(bscale * iscale_sc)
head(7) = nint((a0 + ia0_off) * i0_sc)
head(8) = nint((b0 + ib0_off) * i0_sc)

; handle special cases which depend on transformation option
if iopt eq -1 then begin  ; image only
endif else if iopt eq 0 then begin ; rectalinear lat/lon
; stored as lon +/- 180.0 deg
endif else if (iopt eq 1) or (iopt eq 2) then begin ; lambert
; stored as lon +/- 180.0 deg
    head(5) = nint(iscale_sc / ascale)
    head(6) = nint(iscale_sc / bscale)
endif else if iopt eq 5 then begin ; polar stereographic
; stored as lon +/- 180.0 deg
endif else if (iopt eq 11) or (iopt eq 12) or (iopt eq 13) then begin ; EASE1
    head(5) = nint(iscale_sc * nint(10.0 * ascale * 25.067525/6371.228)*0.05)
    head(6) = nint(iscale_sc * nint(10.0 * bscale/25.067525)*0.05)
endif else begin
    print,'*** Unrecognized SIR option in WRITESIR ***'
endelse

; store projection scale factors
head(39)=iscale_sc
head(126)=ixdeg_off
head(127)=iydeg_off
head(168)=ideg_sc
head(189)=ia0_off
head(240)=ib0_off
head(255)=i0_sc

idatatype=info(19) ; should be 1=byte, 2=i*2, 4=float

vmin=info(17)
vmax=info(18)
ioff=info(20)
iscale=info(21)

auto=1 ; by default, autoscale
if N_params(0) ge 10 then auto=autoset ; user input for autoscale

if auto eq 1 then begin  ; auto scaling, set ioff and iscale
  vmin=min(outarray)
  vmax=max(outarray)
  ioff=fix(vmin)
  denom=vmax-ioff
  if denom eq 0 then denom=1
  num=65000
  if info(19) eq 1 then num=256
  iscale=nint(num/denom)
;  print,'writesir autoset scaling:'
;  print,'  Image min,max',vmin,vmax
;  print,'  Scale factors',ioff,iscale
endif

head(9)=ioff
if iscale le 0.0 then iscale = 1
head(10)=iscale
head(11)=info(11) ; year
head(12)=info(12) ; start day
head(13)=info(13) ; start min
head(14)=info(14) ; end day
head(15)=info(15) ; end min
head(16)=info(2)  ; iopt
head(17)=info(22) ; iregion
head(18)=info(23) ; itype
head(44)=info(24) ; ipol
head(45)=info(25)*10 ; ifreqhm

s=float(head(10))
if s eq 0 then s=1
soff=32767.0/s
if idatatype eq 1 then soff=128.0/head(10)

head(48)=fix((info(16)-ioff-soff)*s)  ; nodata value  
head(49)=fix((vmin-ioff-soff)*s)  ; vmin
head(50)=fix((vmax-ioff-soff)*s)  ; vmax

; for floats, also use float values for nodata, vmin, vmax
if idatatype eq 4 then begin
  head(51)=fix(info(16),0)  ; nodata value
  head(52)=fix(info(16),2)
  head(53)=fix(info(17),0)  ; vmin
  head(54)=fix(info(17),2)
  head(55)=fix(info(18),0)  ; vmax
  head(56)=fix(info(18),2)
endif

nhead=1
ndes=0
ldes=0
nia=0
head(40)=nhead ; number of 512 header plots
head(41)=ndes
head(42)=ldes
head(43)=nia   ; number of extra I*2's
ispare1=0
head(44)=info(24)   ; polarization code
head(45)=anint(info(25)*10.0)   ; frequency
head(46)=ispare1   ; spare
head(47)=info(19)  ; idatatype

; prepare strings for headers

sensor=bytarr(40)
if N_params(0) ge 4 then begin
  sensor1=byte(sensor_s)
  l=39<strlen(sensor_s)
  sensor(indgen(l))=sensor1(indgen(l))
endif

type=bytarr(138)
if N_params(0) ge 5 then begin
  type1=byte(type_s)
  l=137<strlen(type_s)
  type(indgen(l))=type1(indgen(l))
endif

title=bytarr(80)
if N_params(0) ge 6 then begin
  title1=byte(title_s)
  l=79<strlen(title_s)
  title(indgen(l))=title1(indgen(l))
endif

tag=bytarr(40)
if N_params(0) ge 7 then begin
  tag1=byte(tag_s)
  l=39<strlen(tag_s)
  tag(indgen(l))=tag1(indgen(l))
endif

cproc=bytarr(100)
if N_params(0) ge 8 then begin
  cproc1=byte(cproc_s)
  l=99<strlen(cproc_s)
  cproc(indgen(l))=cproc1(indgen(l))
endif

ctime=bytarr(28)
if N_params(0) ge 9 then begin
  ctime1=byte(ctime_s)
  l=27<strlen(ctime_s)
  ctime(indgen(l))=ctime1(indgen(l))
endif

; write strings to headers, changing 0's to spaces

for i=1,20 do begin
  j=2*i-2
  if sensor(j) eq 0 then sensor(j) = 32
  if sensor(j+1) eq 0 then sensor(j+1) = 32
  head(18+i)=sensor(j)+256*sensor(j+1)
endfor

for i=1,69 do begin
  j=2*i-2
  if type(j) eq 0 then type(j) = 32
  if type(j+1) eq 0 then type(j+1) = 32
  head(56+i)=type(j)+256*type(j+1)
endfor

for i=1,40 do begin
  j=2*i-2
  if title(j) eq 0 then title(j) = 32
  if title(j+1) eq 0 then title(j+1) = 32
  head(127+i)=title(j)+256*title(j+1)
endfor

for i=1,20 do begin
  j=2*i-2
  if tag(j) eq 0 then tag(j) = 32
  if tag(j+1) eq 0 then tag(j+1) = 32
  head(168+i)=tag(j)+256*tag(j+1)
endfor

for i=1,50 do begin
  j=2*i-2
  if cproc(j) eq 0 then cproc(j) = 32
  if cproc(j+1) eq 0 then cproc(j+1) = 32
  head(189+i)=cproc(j)+256*cproc(j+1)
endfor

for i=1,14 do begin
  j=2*i-2
  if ctime(j) eq 0 then ctime(j) = 32
  if ctime(j+1) eq 0 then ctime(j+1) = 32
  head(240+i)=ctime(j)+256*ctime(j+1)
endfor

;print," Sensor:  '",strtrim(string(sensor)),"'"
;print," Title:   '",strtrim(string(title)),"'"
;print," Type:    '",strtrim(string(type)),"'"
;print," Tag:     '",strtrim(string(tag)),"'"
;print," Creator: '",strtrim(string(cproc)),"'"
;print," Created: '",strtrim(string(ctime)),"'"

; write header block to file

writeu,1,head

; write image data to file

tval=size(outarray)
nbytes=tval(1)*tval(2)
if idatatype eq 1 then begin             ; bytes
  temp=byte((outarray-ioff-soff)*s)
  writeu,1,temp
endif else if idatatype eq 4 then begin  ; floats (not fully portable)
  writeu,1,outarray
  nbytes=nbytes*4
endif else begin                         ; two byte integers (default)
  temp=fix((outarray-ioff-soff)*s)
  writeu,1,temp
  nbytes=nbytes*2
endelse

; add pad bytes to ensure that file is a multiple of 512 bytes

nrecs=nbytes/512
if nrecs*512 ne nbytes then begin
  temp=bytarr((nrecs+1)*512-nbytes)
  writeu,1,temp
endif

close,1

STOP:
end

function NINT,a
;
; computes the nearest integer, return integer
;
i=fix(a)
if a-i gt 0.5 then i=i+1
if i-a gt 0.5 then i=i-1
return,i
end

function anint,a
;
; computes the nearest integer, returns float
;
i=long(a)
if a-i gt 0.5 then i=i+1
if i-a gt 0.5 then i=i-1
return,float(i)
end
