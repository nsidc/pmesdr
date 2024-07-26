function [alon, alat]=CETB_pix2latlon(x,y,iopt,isc)
%
% function [lon, lat]=CETB_pix2latlon(x,y,iopt,isc)
%
%	Given an image pixel location (x,y) (1..nsx,1..nsy)
%	computes the lat,lon coordinates (lon,lat). The lat,lon returned 
%	corresponds to the center of the pixel
%
%	Note:  while routine will attempt to convert any input (x,y)
%	values, only (x,y) values with 1 <= x <= nsx+1 and 1 <= y <= nsy+1
%	are contained within image.
%
% INPUTS:
%   x,y - input pixel location (1-based)
%  iopt - EASE2 projection type 8=EASE2 N, 9=EASE2 S, 10=EASE2 T/M
%   isc - grid scale factor (0..5) pixel size is (25/2^isc) km
%
% OUTPUTS:
%   lon,lat - longitude, latitude in deg
%

% written by dgl at BYU 24 Nov 2015

thelon=x-0.5; % convert from 1-based to 0-based address at pixel center
thelat=y-0.5;
[alon,alat]=iease2grid(iopt,thelon,thelat,isc,0);

