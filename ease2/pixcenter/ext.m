f1='EASE2_N25km.lats.720x720x1.double';
f2='EASE2_N25km.lons.720x720x1.double';

fid=fopen(f1,'r');
lats=fread(fid,[720 720],'double');
fclose(fid);

myfigure(1)
imagesc(lats); colorbar;

fid=fopen(f2,'r');
lons=fread(fid,[720 720],'double');
fclose(fid);

myfigure(2)
imagesc(lons); colorbar;
