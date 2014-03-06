PRO coltab,filename
;
; load a color table from a binary file
;
if n_elements(filename) eq 0 then begin
   print,'SYNTAX:    coltab,''filename'' '
   goto, STOP
endif

coltab=bytarr(256,3)

openr,1,filename
readu,1,coltab
close,1
r=coltab(*,0)
g=coltab(*,1)
b=coltab(*,2)

tvlct,r,g,b

STOP:
end
