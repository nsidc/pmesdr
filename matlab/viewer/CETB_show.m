function varargout=CETB_show(fname,varargin)
%
% CETB_show(fname <,fignum <,tbmin,tbmax>>)
% img=CETB_show(fname <,fignum <,tbmin,tbmax>>)
% [img,iopt,isc]=CETB_show(fname <,fignum <,tbmin,tbmax>>)
%
% read CETB netcdf file contents and display in optionally 
% specified matlab figure window with optional max/min.  
% Optionally returns the file contents and projection information.
%
% Note: returned image is stored in original the netcdf file order, 
% For conventional display in matlab use the transpose of the 
% output array, e.g., imagesc(img') 
%
% INPUTS:
%  fname  - CETB file name including directory
%  fignum - optional figure number for created window
%  tbmin,tbmax - optional specificiation for min,max TB
%
% OUTPUTS: (optional)
%  img  - TB array 
%  iopt - EASE2 projection type 8=EASE2 N, 9=EASE2 S, 10=EASE2 T/M
%   isc - grid scale factor (0..5) pixel size is (25/2^isc) km
%

% written by DGL at BYU 27 Feb 2016

if nargin==0
  error('*** At least one input required in CETB_show');
end

% get image data from file using netcdf routine
[img,iopt,isc]=CETB_load(fname);

tcb=[];
if nargin>1
  figure(varargin{1})
  if nargin>2
    tbmin=varargin{2};
    tbmax=varargin{3};
    tcb=[tbmin tbmax];
  end
end

if length(tcb)>0
  imagesc(img',tcb);
else
  imagesc(img');
end
axis off;
colorbar;

% optional outputs
if nargout>0
  varargout{1}=img;
  if nargout>1
    varargout{2}=iopt;
    if nargout>2
      varargout{3}=isc;
    end
  end
end
