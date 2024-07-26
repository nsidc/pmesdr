function [x, y] = CETB_latlon2pix(alon,alat,iopt,isc)
%
%  function [x, y] = CETB_latlon2pix1(lon,lat,iopt,isc)
%
%	Convert a lat,lon coordinate (lon,lat) to an image pixel location
%	(x,y) (in floating point).
%
% INPUTS:
%   lon,lat - longitude, latitude in deg
%   iopt    - EASE2 projection type 8=EASE2 N, 9=EASE2 S, 10=EASE2 T/M
%   isc     - grid scale factor (0..5) pixel size is (25/2^isc) km
%
% OUTPUTS:
%   x,y - pixel location (1-based)
%

% written by dgl at BYU 24 Nov 2015

[thelon thelat]=ease2grid(iopt,alon,alat,isc,0);
x=thelon+0.5;  % convert from 0-based to 1-based location at pixel center
y=thelat+0.5;
