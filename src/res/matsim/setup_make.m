%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (c) copyright 2014 David G. Long, Brigham Young Unversity
%
% simple 2D simulation driver code for simplified SIR algorithm implemenation
% written by D. Long at BYU 21 Jun 2014
% revised by D. Long at BYU 23 Jun 2014 + compute all cases
% revised by D. Long at BYU 27 Jun 2014 + increased (some) font sizes
%
% This is a simple simulation of how combining multiple measurements
% with SIR can significantly improve the effective resolution of the
% estimated TB image.  The simulated geometry is similar to an SSM/I.

% general program flow:
%  first, create sample locations based on simple instrument simulation
%  sedond, create simulated response function for measurements
%  third, define the final image size/resolution
%  fourth, create a synthetic "truth" image
%  fifth, generate simulated measurements from truth image and 
%           write to .setup file
%  sixth, use sim_SIR.c to process the .setup file into GRD, SIR and 
%           AVE images for noisy and noise-free cases
%  seventh, read resulting .SIR-formatted files, plot, and compute error
%  eighth, generate .SIR file images versus iteration and create plots
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% set flags useful for debugging
%RUN_SETUP=0; % do not regenerate setup files
RUN_SETUP=1; % generate setup files
%RUN_SIR=0; % skip running external SIR programs when set to zero
RUN_SIR=1; % run external SIR programs if set to one

% set default paramaters that control font size when printing to improve figure readability
set(0,'DefaultaxesFontName','Liberation Sans');
set(0,'DefaultaxesFontSize',18);
%set(0,'DefaulttextFontName','Liberation Sans');
%set(0,'DefaulttextFontSize',12);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step Zero:
% set simulation parameters
%
workdir='./';

% define the possible run cases

% number of passes over target area (uncomment one line)
%Npass=1;  % single pass
Npass=2;   % two passes
%Npass=3;
%Npass=4;

% choose one frequency channel to uncomment
%chan=19; % 19 GHz channel
%chan=22; % 22 GHz channel
chan=37; % 37 GHz channel
%chan=85; % 85 GHz channel (also alters sampling scheme)

% Output image scaline (uncomment one line)
%Nscale=1;           % output image scaling factor
%Nscale=2;           % output image scaling factor
Nscale=3;           % output image scaling factor
%Nscale=4;           % output image scaling factor

% set simulation options for looping

% list of number of passes to actually process
Npass_list=1:2;
% list of channels to process
chan_list=[19,37,85]; % 22 not included since similar to 19
% list of scaling parameters to consider
Nscale_list=2:4;

% over-ride for testing
%Npass_list=1;
%chan_list=[19,37];
%Nscale_list=3;

% loop over simulation run options
for Npass=Npass_list
  for chan=chan_list
    for Nscale=Nscale_list
      disp(sprintf('Working on Npass=%d Channel=%d Nscale=%d',Npass,chan,Nscale));
%
% set swath parameters
%
swathwidth=1400;    % swath width in km
scvel=7;            % s/c ground track velocity in km/sec
rotrate=31.6/60;    % rotation vel in rot/sec
srate=0.00844;      % sample rate in measurements/sec
DeltaT=2.0;         % thermal noise STD (in K) for signal simulation
AntAzAngRange=180+[-51,51]; % angular range of swath
rotrad=swathwidth/sin((AntAzAngRange(2)-180)*pi/180)/2;
thres=0.001;        % set response threshold for output to .setup file

% set default number of iterations
if Nscale < 3
  Nom_iter=20;      % nominal number of SIR iterations to run 
  maxiter=100;      % number of SIR iterations for this simulation
else
  Nom_iter=20;      % nominal number of SIR iterations to run 
  maxiter=50;       % number of SIR iterations for this simulation
end
  
% set channel-specific parameters
switch chan
  case 19 % 19 GHz channel
    footprint=[43,69];   % effective 3dB footprint size 
  case 22 % 22 GHz channel
    footprint=[40,60];    % effective 3dB footprint size 
  case 37 % 37 GHz channel
    footprint=[28,37];    % effective 3dB footprint size 
  case 85 % 85 GHz channel (has denser sampling than other channels)
    footprint=[13,15];    % effective 3dB footprint size 
    % the SSM/I 85 GHz channel has (effectively) a denser sampling, which
    % can be simulated by doubling the spin rate and adjusting srate
    rotrate=2*31.6/60;    % (effective) rotation vel in rot/sec
    srate=0.00422;        % sample rate in measurements/sec
  otherwise
    disp('*** Invalid channel ***');
    return;
end

% generate working directory name
workdir=sprintf('Ch%d_P%d_Ns%d',chan,Npass,Nscale);
cpwd=pwd();
if exist(workdir,'dir') ~=7
  mkdir(cpwd,workdir);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step One:
% compute approximate sample locations for a simulated SSM/I-like single
% channel radiometer

% generate simulated sampling locations
swathlen=swathwidth;  % swath length for simulation
sec=(swathlen*2+swathwidth)/scvel; % seconds of data
time=[0:srate:sec];     % time axis in sec
ang=time*rotrate*360.0; % antenna angle in deg

% select only sample points within swath antenna azimuth range
ind=find(mod(ang,360.0)>min(AntAzAngRange) & mod(ang,360.)<=max(AntAzAngRange));
time=time(ind);
ang=ang(ind);

% compute measurement locations relative to nadir track
x0=0; y0=-swathwidth/2; % swath orgin
along1=y0+cos(ang*pi/180)*rotrad+time*scvel;
cross1=x0+sin(ang*pi/180)*rotrad;

along_loc=along1;
cross_loc=cross1;
ant_ang=ang;

% now simulate multiple passes
% note: the multiple pass geometry varies with latitude. for this
% simulation a single particular location is treated.  for simplicity
% the relative scan angles previously computed are used
along0=swathlen/2;
cross0=0;
for pass=2:Npass
  rot=(pass-1)*20;
  rot0=rot*pi/180;
  cshift=(pass-1)*300;
  
  a=along0+cos(rot0)*(along1-along0)+sin(rot0)*(cross1-cross0);
  c=cshift-sin(rot0)*(along1-along0)+cos(rot0)*(cross1-cross0);

  along_loc=[along_loc a];
  cross_loc=[cross_loc c];  
  ant_ang=[ant_ang ang+rot];
end
along1=along_loc;
cross1=cross_loc;
ang=ant_ang;

% save space
clear a c along_loc cross_loc ant_ang time
  
% For convenence, select only part of swath width to analyze
ind=find(along1 >= -25 & along1 <= swathlen+25 & ...
         cross1 >= -25 & cross1 <= swathwidth/2+25);

ind=find(along1 >= -25 & along1 <= swathlen+25 & ...
         cross1 >= -swathwidth/2-25 & cross1 <= swathwidth/2+25);
 
along=along1(ind);
cross=cross1(ind);
ang=ang(ind);

if 1   % show measurement locations
  myfigure(3)
  plot(cross,along,'.')  % antenna boresite positions
  title('Boresite locations along-scan')
  xlabel('Cross-track distance (km)')
  ylabel('Along-track distance (km)')
  axis([0 swathwidth/2 0 swathlen])
  print('-dpng',[workdir,'/boresite.png']);
end

% show measurements locations and swath density
myfigure(1)
h=subplot(2,1,2);
plot(cross,along,'.')  % center positions
%title('Measurement locations')
xlabel('Cross-track dis (km)')
ylabel('Along-track dis (km)')
axis([0 swathwidth/2 0 swathlen])
axis([0 500 0 100])
grid on;
set(h,'Ytick',[0 25 50 75 100]);
subplot(2,1,1)
xhist=0:25:500;
nh=histc(cross,xhist);
plot(xhist,25*nh/swathlen)
title('Measurement density in 25 km X 25 km area')
xlabel('Cross-track distance (km)')
ylabel('Count')
axis([0 500 0 10])
print('-dpng',[workdir,'/MeaslocDensity.png'])

%disp('Measurement locations computed.  Hit return to continue...'); pause

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step Two:
% create measurement responses

% first define output resolution in km/pix
sampspacing=25.0/2^Nscale;  % sir, ave pixel resolution in km/pix
grd_size=25.0;              % nominal grd pixel size in km

% set footprint measurement response size
MajorAxis=max(footprint);
MinorAxis=min(footprint);

% generate a centered local grid
Ngrid=ceil(2*MajorAxis/sampspacing);
if mod(Ngrid,2)==0
  Ngrid=Ngrid+1;
end
x=((1:Ngrid)-floor(Ngrid/2)-1)*sampspacing;
y=((1:Ngrid)-floor(Ngrid/2)-1)*sampspacing;
[X, Y]=meshgrid(x,y);

if 1 % compute an example at a particular location and orientation
  x0=X(Ngrid/2+0.5,Ngrid/2+0.5);
  y0=Y(Ngrid/2+0.5,Ngrid/2+0.5);
  ang0=30*pi/180;

  xx= (X-x0)*cos(ang0)+(Y-y0)*sin(ang0);
  yy=-(X-x0)*sin(ang0)+(Y-y0)*cos(ang0);
  xx=xx/MajorAxis;
  yy=yy/MinorAxis;

  kang=atan2(yy,xx);
  x1=zeros(size(xx)); ind=find(cos(kang)~=0); x1(ind)=xx(ind)./cos(kang(ind));
  y1=zeros(size(yy)); ind=find(sin(kang)~=0); y1(ind)=yy(ind)./sin(kang(ind));
  V=exp(-x1.*x1).*exp(-y1.*y1);

  % truncate response
  V(V<thres)=0;

  myfigure(2);
  % show a sample measurement response
  imagesc(x,y,V); h=colorbar; set(h,'FontSize',12);
  xlabel('km');ylabel('km')
  h=title(sprintf('Chan: %d  Resolution: %0.4f km/pix  size: %dx%d km',chan,sampspacing,footprint)); set(h,'FontSize',12); 
  print('-dpng',[workdir,'/MRF.png']);
  drawnow
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step 3: define output image grid

% define processing grid for the output
M=floor(swathlen/sampspacing);
if (floor(M/2)*2 ~= M)
  M=M+1;
end
N=floor((swathwidth/2)/sampspacing);
if (floor(N/2)*2 ~= N)
  N=N+1;
end

% define the image bandlimit
BL=0.3;
BL1=floor(M*BL);
BL2=floor(N*BL);
%[M N BL1 BL2 BL]

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step 4:
% create a synthetic image to generate simulated measurements
% (quite arbitrary)

% generate an image with background
true=zeros([M N])+200; % ocean
true(M/2:M,1:N)=230;   % add some land
true(M/2:M,1:N/2)=220; % add some land

% add some thin line features
true(floor(M/3),:)=240;
true(:,floor(2*N/3)-10)=240;

% add gradients
for i=1:N
  true(floor(5*M/6):M,i)=200+i*(250-200)/N;
end
for i=floor(M/6):M
  true(i,1:floor(N/6))=200+i*(250-200)/(5*M/6);
end

% add some warm spots
const=250;
true(M/2+20:M/2+30,N/2-40:N/2-30)=250;
true(floor(2*M/3):floor(2*M/3)+10,floor(2*N/3)+20:floor(2*N/3)+30)=const;
true(floor(2*M/3):floor(2*M/3)+ 8,floor(  N/3)- 8:floor(  N/3))=const;
true(floor(2*M/3):floor(2*M/3)+ 6,floor(  N/2)- 6:floor(  N/2))=const;
true(floor(M/6):floor(M/6)+8,floor(2*N/3)+10:floor(2*N/3)+18)=const;
true(floor(M/6):floor(M/6)+6,floor(  N/2)+10:floor(  N/2)+16)=const;
true(floor(M/6):floor(M/6)+4,floor(  N/3)+10:floor(  N/3)+14)=const;

% bandlimit true image to ensure Nyquist criterion is met for
% sampling and reconstruction.  Multiple passes ensures out-of-band
% signal is small and prevent negative TB values
true=bandlimit2d2(true,BL1,BL2,0);
true=abs(true);
true=bandlimit2d2(true,BL1,BL2,0);
true=abs(true);
true=bandlimit2d2(true,BL1,BL2,0);
true=abs(true);

% display true image
myfigure(4)
colormap('gray')
imagesc(flipud(true')); h=colorbar; set(h,'FontSize',12);
title('True image')
axis off
axis image
%print('-dpng',[workdir,'/true.png']);

%disp('True image created.  Hit return to continue...'); pause

% convert 2d image into a linear array for storage and processing
true1=reshape(true,1,M*N);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step 5:
% for each measurement, write measurement and response to the .setup file

if RUN_SETUP
% first, write the setup file header and true image to .setup file
outfile=[workdir '/sir.setup'];
disp(sprintf('Creating output file: %s',outfile));
fid=fopen(outfile,'w');

fwrite(fid,[M, N],'int32');         % write image size
fwrite(fid,sampspacing,'float32');  % write pixel resolution
fwrite(fid,grd_size,'float32');     % write GRD pixel size
fwrite(fid,true1,'float32');        % write "truth" image

% for each measurement, compute the response function
nn=length(along);
disp(['Size: ',num2str(M),' x ',num2str(N),'  Meas: ',num2str(nn)])
cnt=0;
% for each masuremement
for i=1:nn
  ia=fix(along(i)/sampspacing);
  ic=fix(cross(i)/sampspacing);
  if ia > 0 & ia < M+1 & ic > 0 & ic < N+1  
  
    % compute nominal measurement response function
    Ang=ang(i)*pi/180;
    xx= X*cos(Ang)+Y*sin(Ang);
    yy=-X*sin(Ang)+Y*cos(Ang);
    xx=xx/MajorAxis;
    yy=yy/MinorAxis;

    kang=atan2(yy,xx);
    x1=zeros(size(xx));
    ind=find(cos(kang)~=0); 
    x1(ind)=xx(ind)./cos(kang(ind));
    y1=zeros(size(yy));
    ind=find(sin(kang)~=0); 
    y1(ind)=yy(ind)./sin(kang(ind));
  
    % response function
    aresp=exp(-x1.*x1).*exp(-y1.*y1);

    % plot response function fpr testing
    %myfigure(5)
    %imagesc(x,y,aresp);h=colorbar; set(h,'FontSize',12);
    %title(sprintf('Angle: %f',ang(i)));
    %disp('pausing...');pause;

    % center response function at measurement location
    Xloc=X+cross(i);
    Yloc=Y+along(i);
    Xloc1=fix(Xloc/sampspacing);
    Yloc1=fix(Yloc/sampspacing);

    %disp(sprintf('%d of %d: %f,%f %d,%d',i,nn,cross(i),along(i),fix(cross(i)/sampspacing),fix(along(i)/sampspacing)));
  
    % find valid pixels of measurement within image area
    ind=find(Xloc1>0 & Xloc1 <=N & Yloc1>0 & Yloc1 <=M & ...
	reshape(aresp,size(Xloc))>thres);

    if ~isempty(ind) 
      
      pointer=sub2ind([M,N],fix(Yloc1(ind)),fix(Xloc1(ind)));
      iadd=sub2ind([M,N],fix(along(i)/sampspacing),fix(cross(i)/sampspacing))-1;
    
      % response function with area
      aresp1=aresp(ind).'/sum(aresp(ind));
      
      % generate synthetic measurement
      z=sum(true1(pointer).*aresp1);  % noise-free
      z1=z+randn(size(z))*DeltaT;   % add noise
      
      % save measurement and associated response function to .setup file
      fwrite(fid,[iadd length(pointer)],'int32');
      fwrite(fid,[z z1],'float32');
      fwrite(fid,pointer,'int32');
      fwrite(fid,aresp1,'float32');
      cnt=cnt+1;
    
      if 0
	disp(sprintf('keep %d of %d: %f %f %d %d %d %d,%d',i,nn,z,z1,iadd,length(pointer),prod(size(X)),ia,ic));
	myfigure(6)
	subplot(1,2,1)
	imagesc(x,y,aresp);h=colorbar; set(h,'FontSize',12);
	h=title(sprintf('Angle: %f',mod(ang(i),360.0))); set(h,'FontSize',12);
	axis image
	subplot(1,2,2)
	bp=zeros(size(true));
	bp(pointer)=aresp1/max(aresp1);
	imagesc(bp);h=colorbar; set(h,'FontSize',12); set(h,'FontSize',12);
	h=title(sprintf('max: %f',max(aresp1)));
	disp('pausing...');pause;
      end

      if mod(i,1000)==1
	disp(sprintf('progress %d %f %f %d %d %d %d,%d',i,z,z1,iadd,length(pointer),nn,ia,ic));
      end
    end
  end
end

disp(sprintf('wrote %d measurements', cnt));
% close output file
fclose(fid);

end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step six:
%  run external sir program
%

if RUN_SIR
  % noise-free
  cmd=sprintf('sim_SIR %s 0 0 %d %s', outfile,Nom_iter,workdir);
  system(cmd);

  % noisy
  cmd=sprintf('sim_SIR %s 0 1 %d %s', outfile,Nom_iter,workdir);
  system(cmd);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step seven:
%  load and display image results
%
[tr h_t]=loadsir([workdir, '/true.sir']);

% noise-free
[fAg h_fAg]=loadsir([workdir, '/simA.grd']);
[fAn h_fAn]=loadsir([workdir, '/simA.non']);
[fAa h_fAa]=loadsir([workdir, '/simA.ave']);
[fAs h_fAs]=loadsir([workdir, '/simA.sir']);

% noisy
[nAg h_nAg]=loadsir([workdir, '/simA2.grd']);
[nAn h_nAn]=loadsir([workdir, '/simA2.non']);
[nAa h_nAa]=loadsir([workdir, '/simA2.ave']);
[nAs h_nAs]=loadsir([workdir, '/simA2.sir']);

% compute error stats
[fAn_m,fAn_s,fAn_r]=compute_stats(tr-fAn);
[fAa_m,fAa_s,fAa_r]=compute_stats(tr-fAa);
[fAs_m,fAs_s,fAs_r]=compute_stats(tr-fAs);
[nAn_m,nAn_s,nAn_r]=compute_stats(tr-nAn);
[nAa_m,nAa_s,nAa_r]=compute_stats(tr-nAa);
[nAs_m,nAs_s,nAs_r]=compute_stats(tr-nAs);

% summarize results
disp(' ');
disp(sprintf('Channel: %d GHz  Footprint size: %f x %f   Passes: %d',chan,footprint,Npass));
disp(sprintf('SIR threshold: %f dB  Noise STD: %f K',10*log10(thres),DeltaT));  
disp(sprintf('Resolution: %f km/pix  Bandlimit: %f km',sampspacing,sampspacing/BL));
disp(sprintf('Sample image size: %d x %d',M,N));
disp(' ');
disp(sprintf('Case        Mean    STD    RMS'));
disp(sprintf('N-F Non    %5.2f  %5.2f  %5.2f',fAn_m,fAn_s,fAn_r));
disp(sprintf('N-F Ave    %5.2f  %5.2f  %5.2f',fAa_m,fAa_s,fAa_r));
disp(sprintf('N-F SIR    %5.2f  %5.2f  %5.2f',fAs_m,fAs_s,fAs_r));
disp(sprintf('Noisy Non  %5.2f  %5.2f  %5.2f',nAn_m,nAn_s,nAn_r));
disp(sprintf('Noisy Ave  %5.2f  %5.2f  %5.2f',nAa_m,nAa_s,nAa_r));
disp(sprintf('Noisy SIR  %5.2f  %5.2f  %5.2f',nAs_m,nAs_s,nAs_r));

% set color bar the same for all caes
sc=[190 260];

% plot various comparisons
myfigure(7) % noise-free
colormap('gray')
subplot(2,2,1)
imagesc(tr,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('true'); set(h,'FontSize',12);
subplot(2,2,2)
imagesc(fAn,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('non N-F %0.2f %0.2f %0.2f',fAn_m,fAn_s,fAn_r)); set(h,'FontSize',12);
subplot(2,2,3)
imagesc(fAa,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('ave N-F %0.2f %0.2f %0.2f',fAa_m,fAa_s,fAa_r)); set(h,'FontSize',12);
subplot(2,2,4)
imagesc(fAs,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('sir N-F %0.2f %0.2f %0.2f',fAs_m,fAs_s,fAs_r)); set(h,'FontSize',12);
print('-dpng',[workdir,'/NoiseFree.png']);

myfigure(8) % noisy
colormap('gray')
subplot(2,2,1)
imagesc(tr,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('true'); set(h,'FontSize',12);
subplot(2,2,2)
imagesc(nAn,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('non noisy %0.2f %0.2f %0.2f',nAn_m,nAn_s,nAn_r)); set(h,'FontSize',12);
subplot(2,2,3)
imagesc(nAa,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('ave noisy %0.2f %0.2f %0.2f',nAa_m,nAa_s,nAa_r)); set(h,'FontSize',12);
subplot(2,2,4)
imagesc(nAs,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('sir noisy %0.2f %0.2f %0.2f',nAs_m,nAs_s,nAs_r)); set(h,'FontSize',12);
print('-dpng',[workdir,'/Noisy.png']);

myfigure(9)
colormap('gray')
subplot(2,2,1)
imagesc(tr,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('true'); set(h,'FontSize',12);
subplot(2,2,2)
imagesc(nAg,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('grd noisy'); set(h,'FontSize',12);
subplot(2,2,3)
imagesc(nAa,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('ave noisy'); set(h,'FontSize',12);
subplot(2,2,4)
imagesc(nAs,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('sir noisy'); set(h,'FontSize',12);
print('-dpng',[workdir,'/grd_comp.png'])

if 1 % make large image plots
  myfigure(20)
  colormap('gray')
  imagesc(tr,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off; 
  h=title('true'); set(h,'FontSize',12); print('-dpng',[workdir,'/true.png']);
  imagesc(fAn,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off;
  h=title(sprintf('non N-F %0.2f %0.2f %0.2f',fAn_m,fAn_s,fAn_r)); set(h,'FontSize',12); print('-dpng',[workdir,'/non_nf.png']);
  imagesc(fAa,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off;
  h=title(sprintf('ave N-F %0.2f %0.2f %0.2f',fAa_m,fAa_s,fAa_r)); set(h,'FontSize',12); print('-dpng',[workdir,'/ave_nf.png']);
  imagesc(fAs,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off
  h=title(sprintf('sir N-F %0.2f %0.2f %0.2f',fAs_m,fAs_s,fAs_r)); set(h,'FontSize',12); print('-dpng',[workdir,'/sir_nf.png']);
  imagesc(nAn,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off
  h=title(sprintf('non noisy %0.2f %0.2f %0.2f',nAn_m,nAn_s,nAn_r)); set(h,'FontSize',12); print('-dpng',[workdir,'/non_noisy.png']);
  imagesc(nAa,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off
  h=title(sprintf('ave noisy %0.2f %0.2f %0.2f',nAa_m,nAa_s,nAa_r)); set(h,'FontSize',12); print('-dpng',[workdir,'/ave_noisy.png']);
  imagesc(nAs,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off
  h=title(sprintf('sir noisy %0.2f %0.2f %0.2f',nAs_m,nAs_s,nAs_r)); set(h,'FontSize',12); print('-dpng',[workdir,'/sir_noisy.png']);
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step eight:
%  generate SIR algorithm outputs versus iteration number
%  read resulting files, and generate summary plots
%

if RUN_SIR
  % run noise-free SIR
  cmd=sprintf('sim_SIR %s 0 0 -%d %s', outfile,maxiter,workdir);
  system(cmd);

  % run noisy SIR
  cmd=sprintf('sim_SIR %s 0 1 -%d %s', outfile,maxiter,workdir);
  system(cmd);
end

% initial stat arrays
f_m=zeros([1 maxiter]);
n_m=zeros([1 maxiter]);
d_m=zeros([1 maxiter]);
f_s=zeros([1 maxiter]);
n_s=zeros([1 maxiter]);
d_s=zeros([1 maxiter]);
f_r=zeros([1 maxiter]);
n_r=zeros([1 maxiter]);
d_r=zeros([1 maxiter]);

% for each SIR iteration number, read in file and compute error statistics
% plot selected images
myfigure(12);clf
colormap('gray')
ncnt=1;
for iter=1:maxiter
  fname=sprintf([workdir, '/simA_%d.sir'],iter);
  nname=sprintf([workdir,'/simA2_%d.sir'],iter);
  [fimg head]=loadsir(fname);
  [nimg head]=loadsir(nname);
  [f_m(iter),f_s(iter),f_r(iter)]=compute_stats(tr-fimg);
  [n_m(iter),n_s(iter),n_r(iter)]=compute_stats(tr-nimg);
  [d_m(iter),d_s(iter),d_r(iter)]=compute_stats(fimg-nimg);
  if iter==1 | iter==10 | iter==20 | iter==30
    myfigure(12);
    subplot(4,2,2*(ncnt-1)+1)
    imagesc(nimg,sc);h=colorbar; set(h,'FontSize',12);
    axis image
    axis off
    h=title(sprintf('Noisy iter=%d',iter)); set(h,'FontSize',12);
    subplot(4,2,2*ncnt)
    imagesc(fimg,sc);h=colorbar; set(h,'FontSize',12);
    axis image
    axis off
    h=title(sprintf('N-F iter=%d',iter)); set(h,'FontSize',12);
    ncnt=ncnt+1;
  end
end
print('-dpng',[workdir,'/iterimage.png']);


% inferred noise error for SIR vs iteration
s_m=n_m-f_m;
s_s=sqrt((n_s.^2-f_s.^2));
s_r=sqrt(abs(n_r.^2-f_r.^2));
% for AVE
a_s=sqrt((nAa_s.^2-fAa_s.^2));
a_r=sqrt((nAa_r.^2-fAa_r.^2));
% for grd (non)
g_s=sqrt((nAn_s.^2-fAn_s.^2));
g_r=sqrt((nAn_r.^2-fAn_r.^2));

% generate plots of error versus iteration
myfigure(10)
subplot(1,2,1)
plot([0 60],[0 0],':k');
hold on;plot(f_m,'b'); hold off
hold on; plot(nAa_m,'k*'); hold off;
hold on; plot(nAn_m,'c*'); hold off;
hold on; plot(n_m,'r'); hold off;
hold on; plot([Nom_iter Nom_iter],[-0.05 0.05],'--k'); hold off;
xlabel('Iteration');
ylabel('Mean error (K)');
h=title('r=noisy b=noise-free k=AVE c=non'); set(h,'FontSize',12);
subplot(1,2,2)
plot(10*log10(f_r),'b');
hold on; plot(10*log10(n_r),'r'); hold off;
hold on; plot(10*log10(nAa_r),'k*'); hold off;
hold on; plot(10*log10(nAn_r),'c*'); hold off;
hold on; plot(10*log10(s_r)+4,'g'); hold off;
hold on; plot([Nom_iter Nom_iter],[3 8],'--k'); hold off;
ylabel('RMS error (dB K)');
%plot((f_r),'b');
%hold on; plot((n_r),'r'); hold off;
%ylabel('RMS error (K)');
xlabel('Iteration');
h=title('r=noisy b=noise-free g=noise+4 K=AVE c=non)'); set(h,'FontSize',12);
print('-dpng',[workdir,'/iterate.png']);

% generate plots of error convergence
myfigure(11)
plot(10*log10(f_r),s_r,'.')
hold on;plot(10*log10(f_r(Nom_iter)),s_r(Nom_iter),'r*');hold off
hold on;plot(10*log10(fAa_r),a_r,'g*');hold off
hold on;plot(10*log10(fAn_r),g_r,'k*');hold off
xlabel('RMS signal error (dB)')
ylabel('RMS noise error');
h=title(sprintf('%d fp=%dx%d DeltaT=%0.1f Np=%d (r=SIR, g=Ave, k=grd)',chan,footprint,DeltaT,Npass)); set(h,'FontSize',12);
print('-dpng',[workdir,'/sig.png']);


% summarize results (again) for human operator
disp(' ');
disp(sprintf('Channel: %d GHz  Footprint size: %f x %f   Passes: %d',chan,footprint,Npass));
disp(sprintf('SIR threshold: %f dB  Noise STD: %f K',10*log10(thres),DeltaT));  
disp(sprintf('Resolution: %f km/pix  Bandlimit: %f km',sampspacing,sampspacing/BL));
disp(sprintf('Sample image size: %d x %d',M,N));
disp(sprintf(' '));
disp(sprintf('Case        Mean    STD    RMS'));
disp(sprintf('N-F Non    %5.2f  %5.2f  %5.2f',fAn_m,fAn_s,fAn_r));
disp(sprintf('N-F Ave    %5.2f  %5.2f  %5.2f',fAa_m,fAa_s,fAa_r));
disp(sprintf('N-F SIR    %5.2f  %5.2f  %5.2f',fAs_m,fAs_s,fAs_r));
disp(sprintf('Noisy Non  %5.2f  %5.2f  %5.2f',nAn_m,nAn_s,nAn_r));
disp(sprintf('Noisy Ave  %5.2f  %5.2f  %5.2f',nAa_m,nAa_s,nAa_r));
disp(sprintf('Noisy SIR  %5.2f  %5.2f  %5.2f',nAs_m,nAs_s,nAs_r));


% and write summary statistics to file
fid=fopen([workdir '/stats.txt'],'w');
fprintf(fid,'Channel: %d GHz  Footprint size: %f x %f   Passes: %d\n',chan,footprint,Npass);
fprintf(fid,'Footprint size: %f x %f   Passes: %d\n',footprint,Npass);
fprintf(fid,'SIR threshold: %f dB  Noise STD: %f K\n',10*log10(thres),DeltaT);  
fprintf(fid,'Resolution: %f km/pix  Bandlimit: %f km\n',sampspacing,sampspacing/BL);
fprintf(fid,'Sample image size: %d x %d\n',M,N);
fprintf(fid,' \n');
fprintf(fid,'Case        Mean    STD    RMS\n');
fprintf(fid,'N-F Non    %5.2f  %5.2f  %5.2f\n',fAn_m,fAn_s,fAn_r);
fprintf(fid,'N-F Ave    %5.2f  %5.2f  %5.2f\n',fAa_m,fAa_s,fAa_r);
fprintf(fid,'N-F SIR    %5.2f  %5.2f  %5.2f\n',fAs_m,fAs_s,fAs_r);
fprintf(fid,'Noisy Non  %5.2f  %5.2f  %5.2f\n',nAn_m,nAn_s,nAn_r);
fprintf(fid,'Noisy Ave  %5.2f  %5.2f  %5.2f\n',nAa_m,nAa_s,nAa_r);
fprintf(fid,'Noisy SIR  %5.2f  %5.2f  %5.2f\n',nAs_m,nAs_s,nAs_r);
fclose(fid);


end
end
end