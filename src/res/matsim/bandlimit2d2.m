function out=bandlimit2d2(in,m,n,square)
%
% function out=bandlimit2d(in,m,n,square)
%
% bandlimit the M x N 2d signal 'in' to 'm' x 'n' points of its 2d fft
% if square==1, rectangular ideal lowpass filtering used (default)
%    otherwise, elliptical ideal lowpass filteringused
%
nn=nargin;
if (nn<3)
  error('*** insufficient arguments to bandlimit2d2, 3 required');
elseif (nn >= 4)
  sq=square;
else
  sq=1;
end

s=fft2(in);
[M N]=size(s);
ind=1:M*N;
[i,j]=ind2sub([M,N],ind);
i=i-1;
in=find(i>M/2);
i(in)=i(in)-M;
j=j-1;
in=find(j>N/2);
j(in)=j(in)-N;

if (sq==1)        % rectangular ideal lowpass filter
  ind=find(abs(i)>m | abs(j)>n);
else              % eliptical ideal lowpass filter
  in=i.^2+j.^2;
  ang=atan2(j,i);
  r2=(cos(ang)*m).^2+(sin(ang)*n).^2;
  ind=find(in>r2);
end

s(ind)=0;
out=real(ifft2(s));

