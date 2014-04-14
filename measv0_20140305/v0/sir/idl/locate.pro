PRO locate,outarray,info
;
;	GIVEN INFO DATA, ASKS FOR GRAPHICS INPUT, PRINTS IMAGE DATA
;
; INPUTS:
;   outarray - image array
;   info - image transformation info array from loadsir
;
; OUTPUTS:
;   print information
;

nsx=info(0)
nsy=info(1)

print,"Press button for lon, lat, x, y, value"
cursor,x,y,3,/DEVICE
pixtolatlon,lon,lat,x,y,info
i=(x-1)*nsx+y
print,lon,lat,x,y,outarray(i)

return
end






