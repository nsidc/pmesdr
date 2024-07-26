function plot_gshhs_ease2(resch,iopt,isc,NSY,col)
%
%  function plot_gshhs_eas2(resch,iopt,isc,nsy)
%
%  Plot a coastline overlay from the gshhs data set using CETB EASE2
%  plotting conventions
%
% INPUTS:
%   resch - gshss resolution ('f','h','l' [def],'i')
%   iopt  - EASE2 projection type 8=EASE2 N, 9=EASE2 S, 10=EASE2 T/M
%   isc   - grid scale factor (0..5) pixel size is (25/2^isc) km
%   nsy   - vertical dimension of map in pixels (for matlab plot flipping)
%   col   - color/line type argument (same as used in plot)
%

% written by dgl at BYU 24 Nov 2015

% path to gshh files (installation specific)
gshhs_loc='./gshhs/';

% select gshhs resolution 
% these ascii files were extracted from the binary gshhs files
switch resch
  case 'f'
    %gshhs_file='gshhs_f.dat'; % due to large size, not provided
    gshhs_file='gshhs_h.dat';
  case 'h'
    gshhs_file='gshhs_h.dat';
  case 'l'
    gshhs_file='gshhs_l.dat';
  case 'i'
    gshhs_file='gshhs_i.dat';
  otherwise
    gshhs_file='gshhs_l.dat';
end

% load mapfile
map=load([gshhs_loc gshhs_file]);

lons=map(:,2);
lats=map(:,1);
pen=map(:,3);
clear map;

if 0 % reduce plotting length if only showing part of Earth
if iopt==8 % N
  ind=find(lats > 0);
  lats=lats(ind);
  lons=lons(ind);
  pen=pen(ind);  
elseif iopt==9 % S
  ind=find(lats < 0);
  lats=lats(ind);
  lons=lons(ind);
  pen=pen(ind);
else % T 
  % do nothing
end
end

% add end point
lats(length(pen)+1)=0;
lons(length(pen)+1)=0;
pen(length(pen)+1)=3;

% fine line_up codes
ind=find(pen==3);
ind(length(ind)+1)=length(pen)+1;
n=length(ind);

% plot individual line segments
hold on;
for k=1:n-1
  lon=lons(ind(k):ind(k+1)-1);
  lat=lats(ind(k):ind(k+1)-1);
  [x,y]=CETB_latlon2pix(lon,lat,iopt,isc);
  y=NSY-y+1;
  plot(x,y+1,col);
end
hold off;