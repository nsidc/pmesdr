%
% given a standard .sir file, generate a synthetic true images
% with some features of interest
%
% written by DG Long at BYU 28 Mar 2014
%

in_name='work/true1.sir';
out_name='work/true2.sir';

% read input sir image
[v head]=loadsir(in_name);

% modify image
v(v>100)=220;  % background value

% create some bands
for k=1:10
  band=0:4*k;
  v(:,150+band+k*50)=240; % scene feature value
end

% temporarily eliminate no-data values
v1=v;
v1(v<110)=220;

% lowpass filter image
LPF=ones(size(v1));
LPF(size(LPF,1)/4+1:3*size(LPF,1)/4,size(LPF,2)/4+1:3*size(LPF,2)/4)=0;
Fv=real(ifft2(fft2(v1).*LPF));

% restore no-data values
Fv(v<110)=100;

% show the image
myfigure(1)
showsir(Fv,head)

% write output
writesir(out_name,fv,head);
