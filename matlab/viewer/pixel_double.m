function out = pixel_double(in)
%
% function out = pixel_double(in)
%
% doubles the size of the input image by duplicating pixels
% to create output image.  resulting images align at origin.
% note: the input image can be recovered from the output 
% image by taking every other pixel.
%
% INPUTS:
%  in   - 2d array w/size (r,c)
%
% OUTPUTS: 
%  out  - 2d array w/size (2r,2c)

% written by DGL at BYU 1996

[m n]=size(in);
m2=2*m;
n2=2*n;
out=zeros([m2 n2]);
[Y X]=meshgrid(1:m,1:n);
ind=sub2ind([m n],Y,X);

sub=[-1 0];
for x=1:2
  for y=1:2
    ind2=sub2ind([m2 n2],2*Y+sub(y),2*X+sub(x));
    out(ind2)=in(ind);
  end
end
