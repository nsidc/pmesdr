%
% matlab demo script to display a particular CETB file and plot
% a box overlay.  Also illustrates lat/lon to pix mapping and a 
% simple map overlay routine.
%
% written by DGL at BYU 27 Feb 2016
%
% location of CETB file
location='/auto/temp/long/measures_files/BYUpmesdr/F13_RSS_N_2003005/';
%
% CETB file name
fname='EASE2_N3.125km.F13_SSMI.2003005.85V.E.SIR.RSS.v0.1.nc';
%
% create full path to file
name=[location fname];

%
if 0 % optionally display CETB file header contents
  A=ncinfo(name);
  Natt=size(A.Attributes,2);
  for k=1:Natt
    fprintf('%s = "%s"\n',A.Attributes(k).Name,A.Attributes(k).Value);
  end
end

%
% define Wagner Island box example area
%
% specify box using 25 km pixel locations (in matlab display format), 
% then compute corresponding pixels in higher resolution images
% this computation ensures that the same area is included in all
% resolutions
%
% 25 km grid pixels
yb0=250;
ye0=309;
xb0=330;
xe0=389;

% 6.25 km grid pixels
xb2=(xb0-1)*4+1;
xe2=(xe0-1)*4+4;
yb2=(yb0-1)*4+1;
ye2=(ye0-1)*4+4;

% 3.125 km grid pixels
xb1=(xb0-1)*8+1;
xe1=(xe0-1)*8+7;
yb1=(yb0-1)*8+1;
ye1=(ye0-1)*8+7;

% store high res corners in arrays
yy=[yb1 yb1 ye1 ye1 yb1];
xx=[xb1 xe1 xe1 xb1 xb1];

% open and display CETB file using matlab utility function
% the show routine optionally returns the image EASE2 parameters
[img,iopt,isc]=CETB_show(name,1);
title('TB (K)');
colormap('gray')
drawnow

% vertical size of data array
nsy=size(img,2);

if 1 % optionally add coastline overlay
  % use utility to plot gshhs map overlay
  plot_gshhs_ease2('i',iopt,isc,nsy,'k');
  drawnow
end

if 0 & iopt==8 % optionally plot a simple N polar grid
  hold on;
  for alat=0:10:90;
    lon=0:10:360;
    lat=lon*0+alat;
    [x,y]=CETB_latlon2pix(lon,lat,iopt,isc);
     plot(x,nsy-y+1,'b');
  end

  [x,y]=CETB_latlon2pix(0,90,iopt,isc); % N pole
  plot(x,nsy-y+1,'b.');
  
  for alon=0:30:360;
    lat=0:5:90;
    lon=lat*0+alon;
    [x,y]=CETB_latlon2pix(lon,lat,iopt,isc);
    plot(x,nsy-y+1,'b');
  end
  hold off;
end
drawnow

% add plot box on matlab image
hold on;
plot(xx,yy,'r');
hold off;
axis off

% save figure to file
print -dpng TB.png

% illustrate map projection computations

% convert box corners in pixels to lat/lon
% note: matlab display is vertically flipped
yy1=nsy-yy+1;
[lon,lat]=CETB_pix2latlon(xx,yy1,iopt,isc);

% specify box in lat/lon and plot on image
% (plot overwrites previously plotted box)
[x,y]=CETB_latlon2pix(lon,lat,iopt,isc);
y1=nsy-y+1;
hold on;
plot(x,y1,'g');
hold off;

% write corner locations for user
for k=1:4
  fprintf('x,y=%d,%d: lat,lon=%f,%f\n',xx(k),yy1(k),lat(k),lon(k));
end

if 1 % optionally display other arrays in CETB file
     % note transpose of arrays used when calling imagesc
  [img,iopt,isc,TBn,IncAng,TBstd,Time]=CETB_load(name);

  % note: to take advantage of matlab autoscaling, set out-range values to NaN
  IncAng(IncAng<30)=NaN;
  TBstd(TBstd>300)=NaN;
  Time(Time<-5000)=NaN;
  
  figure(2)
  imagesc(TBn'); colorbar;
  title('TB number of measurements');
  axis off;
  print -dpng TBn.png

  figure(3)
  imagesc(IncAng'); colorbar;
  title('TB Incidence Angle (deg)');
  axis off;
  print -dpng TBinc.png
  
  figure(4)
  imagesc(TBstd'); colorbar;
  title('TB std dev (K)');
  axis off;
  print -dpng TBstd.png
  
  figure(5)
  imagesc(Time'); colorbar;
  title('TB Time (min)');
  axis off;
  print -dpng TBtime.png
end