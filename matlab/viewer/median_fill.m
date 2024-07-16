function out=median_fill(in,nsize,thres)
%
%  function out=median_fill(in,nsize,thres)    
%
%  input pixel values that are less than thres are filled 
%  in with the median of nearby pixels within a window
%  +/- nsize about pixel. when computing median, only pixel
%  values greater than thres are considered. input pixels
%  greater than thres are unchanged and copied to outupt.
%
% INPUTS:
%  in    - 2d array
%  nsize - integer window size, must be odd defining
%          nearby window (dim=2*nsize+1) centered on pixel
%  thres - threshold
%
% OUTPUTS: 
%  out  - 2d array with filled pixel values
%  iopt - EASE2 projection type 8=EASE2 N, 9=EASE2 S, 10=EASE2 T/M
%   isc - grid scale factor (0..5) pixel size is (25/2^isc) km
%

% written by DGL at BYU 1993

nsize2=floor(nsize/2);

% initial [default] copy
out=in;

[m n]=size(in);

% find pixels below threshold
[r c]=find(in<thres);

% for each pixel
for k=1:length(r)
  % define window centered on pixel
  % (have to deal with edges)
  r1=max([r(k)-nsize2 1]);
  r2=min([r(k)+nsize2 m]);
  c1=max([c(k)-nsize2 1]);
  c2=min([c(k)+nsize2 n]);
  [x y]=meshgrid(r1:r2,c1:c2);
  
  % extract values within window
  vals=in(sub2ind(size(in),x,y));
  
  % compute median of values exceeding threshold
  out(r(k),c(k))=median(vals(find(vals>thres)));
end
