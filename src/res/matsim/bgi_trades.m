%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (c) copyright 2014 David G. Long, Brigham Young Unversity
%
% Compute BGI tradeoffs
% written by D. Long at BYU 25 Jun 2014
% revised by D. Long at BYU 27 Jun 2014 + increased (some) font sizes
% revised by D. Long at BYU 19 Jul 2014 + added more cases, regen option
%
% This script should be run after the SIR tradeoff analysis in
% setup_make.m  The script computes simulated BGI products for various
% values of the BGI parameters to help choose the optimum values

% general program flow:
%  first, run the setup_make.m m script to create the input .setup files
%  second, create BGI images for various values of gamma
%  third, plots results
%  fourth, compare to SIR results
%
% BGI computational load is quite high, so script runs slowly
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% set a flag useful for debugging
RUN_BGI1=0;  % nominal run do not run external BGI program if set to zero
RUN_BGI1=1;  % nominal run external BGI program if set to one
RUN_BGI=0;  % trade run do not run external BGI programs if set to zero
RUN_BGI=1;  % trade run run external BGI programs if set to one
REGEN_BGI=0; % use existing BGI trade output file if possible
%REGEN_BGI=1; % force regeneration of BGI trade output files

% set default paramaters that control font size when printing to improve figure readability
set(0,'DefaultaxesFontName','Liberation Sans');
set(0,'DefaultaxesFontSize',18);
%set(0,'DefaulttextFontName','Liberation Sans');
%set(0,'DefaulttextFontSize',12);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step Zero:
% set simulation parameters
% setup_make.m should have already been run before using this scipt

% set default image parameters
workdir='./';

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
Npass_list=[2];
chan_list=[37 85 19];
Nscale_list=[3];

% loop over simulation run options
for Npass=Npass_list
  for chan=chan_list
    for Nscale=Nscale_list
      disp(sprintf('Working on Npass=%d Channel=%d Nscale=%d',Npass,chan,Nscale));
  
% set channel-specific parameters
switch chan
  case 19 % 19 GHz channel
  case 22 % 22 GHz channel
  case 37 % 37 GHz channel
  case 85 % 85 GHz channel (has denser sampling than other channels)
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

outfile=[workdir '/sir.setup'];


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step two:
%  run external sir program
%

gam=0.85;
noise_var=2.0^2;
gainThres=0.125;

if RUN_BGI1
  % noise-free
  cmd=sprintf('sim_BGI %s 0 %f %f %f %s', outfile,gam,noise_var,gainThres,workdir);
  disp(cmd)
  system(cmd);

  % noisy
  cmd=sprintf('sim_BGI %s 1 %f %f %f %s', outfile,gam,noise_var,gainThres,workdir);
  disp(cmd)
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
[fAb h_fAb]=loadsir([workdir, '/simA.bgi']);
[fMb h_fMb]=loadsir([workdir, '/simA_median.bgi']);

% noisy
[nAg h_nAg]=loadsir([workdir, '/simA2.grd']);
[nAn h_nAn]=loadsir([workdir, '/simA2.non']);
[nAa h_nAa]=loadsir([workdir, '/simA2.ave']);
[nAs h_nAs]=loadsir([workdir, '/simA2.sir']);
[nAb h_nAb]=loadsir([workdir, '/simA2.bgi']);
[nMb h_nMb]=loadsir([workdir, '/simA2_median.bgi']);

% compute error stats
[fAn_m,fAn_s,fAn_r]=compute_stats(tr-fAn);
[fAa_m,fAa_s,fAa_r]=compute_stats(tr-fAa);
[fAs_m,fAs_s,fAs_r]=compute_stats(tr-fAs);
[fAb_m,fAb_s,fAb_r]=compute_stats(tr-fAb);
[fMb_m,fMb_s,fMb_r]=compute_stats(tr-fMb);
[nAn_m,nAn_s,nAn_r]=compute_stats(tr-nAn);
[nAa_m,nAa_s,nAa_r]=compute_stats(tr-nAa);
[nAs_m,nAs_s,nAs_r]=compute_stats(tr-nAs);
[nAb_m,nAb_s,nAb_r]=compute_stats(tr-nAb);
[nMb_m,nMb_s,nMb_r]=compute_stats(tr-nMb);

% summarize results
disp(' ');
disp(sprintf('Channel: %d GHz  Passes: %d  Nscale: %d',chan,Npass,Nscale));
disp(' ');
disp(sprintf('Case        Mean    STD    RMS'));
disp(sprintf('N-F Non    %5.2f  %5.2f  %5.2f',fAn_m,fAn_s,fAn_r));
disp(sprintf('N-F Ave    %5.2f  %5.2f  %5.2f',fAa_m,fAa_s,fAa_r));
disp(sprintf('N-F SIR    %5.2f  %5.2f  %5.2f',fAs_m,fAs_s,fAs_r));
disp(sprintf('N-F BGI    %5.2f  %5.2f  %5.2f',fAb_m,fAb_s,fAb_r));
disp(sprintf('N-F BGM    %5.2f  %5.2f  %5.2f',fMb_m,fAb_s,fMb_r));
disp(sprintf('Noisy Non  %5.2f  %5.2f  %5.2f',nAn_m,nAn_s,nAn_r));
disp(sprintf('Noisy Ave  %5.2f  %5.2f  %5.2f',nAa_m,nAa_s,nAa_r));
disp(sprintf('Noisy SIR  %5.2f  %5.2f  %5.2f',nAs_m,nAs_s,nAs_r));
disp(sprintf('Noisy BGI  %5.2f  %5.2f  %5.2f',nAb_m,nAb_s,nAb_r));
disp(sprintf('Noisy BGM  %5.2f  %5.2f  %5.2f',nMb_m,nMb_s,nMb_r));

% write summary statistics to file
fid=fopen([workdir '/stats2.txt'],'w');
fprintf(fid,'Channel: %d GHz  Passes: %d  Nscale: %d\n',chan,Npass,Nscale);
fprintf(fid,'\n');
fprintf(fid,'Case        Mean    STD    RMS\n');
fprintf(fid,'N-F Non    %5.2f  %5.2f  %5.2f\n',fAn_m,fAn_s,fAn_r);
fprintf(fid,'N-F Ave    %5.2f  %5.2f  %5.2f\n',fAa_m,fAa_s,fAa_r);
fprintf(fid,'N-F SIR    %5.2f  %5.2f  %5.2f\n',fAs_m,fAs_s,fAs_r);
fprintf(fid,'N-F BGI    %5.2f  %5.2f  %5.2f\n',fAb_m,fAb_s,fAb_r);
fprintf(fid,'N-F BGM    %5.2f  %5.2f  %5.2f\n',fMb_m,fAb_s,fMb_r);
fprintf(fid,'Noisy Non  %5.2f  %5.2f  %5.2f\n',nAn_m,nAn_s,nAn_r);
fprintf(fid,'Noisy Ave  %5.2f  %5.2f  %5.2f\n',nAa_m,nAa_s,nAa_r);
fprintf(fid,'Noisy SIR  %5.2f  %5.2f  %5.2f\n',nAs_m,nAs_s,nAs_r);
fprintf(fid,'Noisy BGI  %5.2f  %5.2f  %5.2f\n',nAb_m,nAb_s,nAb_r);
fprintf(fid,'Noisy BGM  %5.2f  %5.2f  %5.2f\n',nMb_m,nMb_s,nMb_r);
fclose(fid);

% set color bar the same for all caes
sc=[190 260];

% plot various comparisons
myfigure(7) % noise-free
colormap('gray')
subplot(3,2,1)
imagesc(tr,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('true'); set(h,'FontSize',12);
subplot(3,2,2)
imagesc(fAn,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('non N-F %0.2f %0.2f %0.2f',fAn_m,fAn_s,fAn_r)); set(h,'FontSize',12);
subplot(3,2,3)
imagesc(fAa,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('ave N-F %0.2f %0.2f %0.2f',fAa_m,fAa_s,fAa_r)); set(h,'FontSize',12);
subplot(3,2,4)
imagesc(fAs,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('sir N-F %0.2f %0.2f %0.2f',fAs_m,fAs_s,fAs_r)); set(h,'FontSize',12);
subplot(3,2,5)
imagesc(fAb,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('BGI N-F %0.2f %0.2f %0.2f',fAb_m,fAb_s,fAb_r)); set(h,'FontSize',12);
subplot(3,2,6)
imagesc(fMb,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('BGM N-F %0.2f %0.2f %0.2f',fMb_m,fMb_s,fMb_r)); set(h,'FontSize',12);
print('-dpng',[workdir,'/NoiseFree2.png']);

myfigure(8) % noisy
colormap('gray')
subplot(3,2,1)
imagesc(tr,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('true');  set(h,'FontSize',12);
subplot(3,2,2)
imagesc(nAn,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('non noisy %0.2f %0.2f %0.2f',nAn_m,nAn_s,nAn_r));  set(h,'FontSize',12);
subplot(3,2,3)
imagesc(nAa,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('ave noisy %0.2f %0.2f %0.2f',nAa_m,nAa_s,nAa_r));  set(h,'FontSize',12);
subplot(3,2,4)
imagesc(nAs,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('sir noisy %0.2f %0.2f %0.2f',nAs_m,nAs_s,nAs_r));  set(h,'FontSize',12);
subplot(3,2,5)
imagesc(nAb,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('BGI noisy %0.2f %0.2f %0.2f',nAb_m,nAb_s,nAb_r));  set(h,'FontSize',12);
subplot(3,2,6)
imagesc(nMb,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title(sprintf('BGM noisy %0.2f %0.2f %0.2f',nMb_m,nMb_s,nMb_r));  set(h,'FontSize',12);
print('-dpng',[workdir,'/Noisy2.png']);

myfigure(9)
colormap('gray')
subplot(3,2,1)
imagesc(tr,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('true'); set(h,'FontSize',12);
subplot(3,2,2)
imagesc(nAg,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('grd noisy'); set(h,'FontSize',12);
subplot(3,2,3)
imagesc(nAa,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('ave noisy'); set(h,'FontSize',12);
subplot(3,2,4)
imagesc(nAs,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('sir noisy'); set(h,'FontSize',12);
subplot(3,2,5)
imagesc(nAb,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('BGI noisy'); set(h,'FontSize',12);
subplot(3,2,6)
imagesc(nMb,sc);h=colorbar; set(h,'FontSize',12);
axis image
axis off
h=title('BGM noisy'); set(h,'FontSize',12);
print('-dpng',[workdir,'/grd_comp2.png'])

if 1 % make large image plots
  myfigure(20)
  colormap('gray')
  imagesc(nAb,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off
  h=title(sprintf('BGI noisy %0.2f %0.2f %0.2f',nAb_m,nAb_s,nAb_r)); set(h,'FontSize',12);
  print('-dpng',[workdir,'/bgi_noisy.png']);
  imagesc(nMb,sc);h=colorbar; set(h,'FontSize',12); axis image; axis off
  h=title(sprintf('BGM noisy %0.2f %0.2f %0.2f',nMb_m,nMb_s,nMb_r)); set(h,'FontSize',12);
  print('-dpng',[workdir,'/bgm_noisy.png']);
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Step three:
%  generate BG algorithm outputs versus gamma 
%  read resulting files, and generate summary plots
%

% set list of gamma values to consider
%gamma_list=0.5*pi*[0 0.25 0.5 0.75 0.9 0.95 0.98 0.99 0.995 1.0]; % orig
gamma_list=0.5*pi*[0 0.1 0.15 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.65 0.75 0.85 0.9 0.95 0.98 0.99 0.995 1.0]; % extended
ngamma=length(gamma_list);

% initial stat arrays
fB_m=zeros([1 ngamma]);
fM_m=zeros([1 ngamma]);
fB_s=zeros([1 ngamma]);
fM_s=zeros([1 ngamma]);
fB_r=zeros([1 ngamma]);
fM_r=zeros([1 ngamma]);
nB_m=zeros([1 ngamma]);
nM_m=zeros([1 ngamma]);
nB_s=zeros([1 ngamma]);
nM_s=zeros([1 ngamma]);
nB_r=zeros([1 ngamma]);
nM_r=zeros([1 ngamma]);

myfigure(22);clf
colormap('gray');

for ig=1:ngamma
  bgi_gamma=gamma_list(ig);
  subworkdir=sprintf('%s/bgi_%f',workdir,bgi_gamma);
  cpwd=pwd();
  if exist(subworkdir,'dir') ~= 7
    mkdir(cpwd,subworkdir);
  end  
  
  % see if particular case has already been run
  if exist([subworkdir, '/simA.bgi'],'file') ~= 2 | REGEN_BGI == 1  
    if RUN_BGI % optionally run external BGI program
      % noise-free
      cmd=sprintf('sim_BGI %s 0 %f %f %f %s', outfile,bgi_gamma,noise_var,gainThres,subworkdir);
      disp(cmd)
      system(cmd);
      
      % noisy
      cmd=sprintf('sim_BGI %s 1 %f %f %f %s', outfile,bgi_gamma,noise_var,gainThres,subworkdir);
      disp(cmd)
      system(cmd);
    end
  else
    disp(sprintf('Reusing result for %f',bgi_gamma));
  end

  % read results and accumulate statistics
  % noise-free
  [fAb1 h_fAb1]=loadsir([subworkdir, '/simA.bgi']);
  [fMb1 h_fMb1]=loadsir([subworkdir, '/simA_median.bgi']);

  % noisy
  [nAb1 h_nAb1]=loadsir([subworkdir, '/simA2.bgi']);
  [nMb1 h_nMb1]=loadsir([subworkdir, '/simA2_median.bgi']);

  % compute error stats
  [fB_m(ig),fB_s(ig),fB_r(ig)]=compute_stats(tr-fAb1);
  [fM_m(ig),fM_s(ig),fM_r(ig)]=compute_stats(tr-fMb1);
  [nB_m(ig),nB_s(ig),nB_r(ig)]=compute_stats(tr-nAb1);
  [nM_m(ig),nM_s(ig),nM_r(ig)]=compute_stats(tr-nMb1);

  if mod(ig-1,2)==0
    ig2=floor((ig-1)/2)+1;    
    myfigure(22)
    subplot(5,2,ig2)
    imagesc(nAb1,sc); h=colorbar; set(h,'FontSize',12); axis image; axis off
    h=title(sprintf('BGI g=%f RMS=%0.2f',bgi_gamma/pi,nB_r(ig)));   set(h,'FontSize',12);
    drawnow;
  end

  % write summary statistics to file
  fid=fopen([subworkdir '/bgi_stats.txt'],'w');
  fprintf(fid,'Channel: %d GHz  Passes: %d  Nscale: %d\n',chan,Npass,Nscale);
  fprintf(fid,'BGI Gamma: %d \n',bgi_gamma);
  fprintf(fid,'\n');
  fprintf(fid,'Case        Mean    STD    RMS\n');
  fprintf(fid,'N-F BGI    %5.2f  %5.2f  %5.2f\n',fB_m(ig),fB_s(ig),fB_r(ig));
  fprintf(fid,'N-F BGM    %5.2f  %5.2f  %5.2f\n',fM_m(ig),fM_s(ig),fM_r(ig));
  fprintf(fid,'Noisy BGI  %5.2f  %5.2f  %5.2f\n',nB_m(ig),nB_s(ig),nB_r(ig));
  fprintf(fid,'Noisy BGM  %5.2f  %5.2f  %5.2f\n',nM_m(ig),nM_s(ig),nM_r(ig));
  fclose(fid);
  
end
print('-dpng',[workdir,'/bgi_images.png']);

% inferred noise statistics
sB_m=nB_m-fB_m;
sM_m=nM_m-fM_m;
sB_s=sqrt((nB_s.^2-fB_s.^2));
sM_s=sqrt((nM_s.^2-fM_s.^2));
sB_r=sqrt(abs(nB_r.^2-fB_r.^2));
sM_r=sqrt(abs(nM_r.^2-fM_r.^2));

[nB_minval nB_minindex]=min(nB_r);
[nM_minval nM_minindex]=min(nM_r);

% generate plots of error versus BGI gamma
myfigure(30);clf
subplot(1,2,1);
plot([0 1.0],[0 0],':k');
hold on;plot(2*gamma_list/pi,fB_m,'b'); hold off
hold on;plot(2*gamma_list/pi,fM_m,'c'); hold off
hold on; plot(2*gamma_list/pi,nB_m,'r'); hold off;
hold on; plot(2*gamma_list/pi,nM_m,'g'); hold off;
hold on; plot(2*gamma_list(nB_minindex)/pi,nB_m(nB_minindex),'r*'); hold off;
hold on; plot(2*gamma_list(nM_minindex)/pi,nM_m(nM_minindex),'g*'); hold off;
%%hold on; plot([Nom_iter Nom_iter],[-0.05 0.05],'--k'); hold off;
xlabel('BG gamma''');
ylabel('Mean error (K)');
h=title('b=NF, c=NF med, r=N, g=Nmed'); set(h,'FontSize',12);
subplot(1,2,2);
plot(2*gamma_list/pi,10*log10(fB_r),'b');
hold on; plot(2*gamma_list/pi,10*log10(fM_r),'c'); hold off;
hold on; plot(2*gamma_list/pi,10*log10(nB_r),'r'); hold off;
hold on; plot(2*gamma_list/pi,10*log10(nM_r),'g'); hold off;
hold on; plot(2*gamma_list(nB_minindex)/pi,10*log10(nB_r(nB_minindex)),'r*'); hold off;
hold on; plot(2*gamma_list(nM_minindex)/pi,10*log10(nM_r(nM_minindex)),'g*'); hold off;
hold on; plot(2*gamma_list/pi,10*log10(sB_r)+4,'k'); hold off;
hold on; plot(2*gamma_list/pi,10*log10(sM_r)+4,'m'); hold off;
%%hold on; plot([Nom_iter Nom_iter],[3 8],'--k'); hold off;
ylabel('RMS error (dB K)');
xlabel('BG gamma''');
h=title('b=NF, c=NFmed, r=N, g=Nmed, k=sig+4, m=sigmed+4'); set(h,'FontSize',12);
print('-dpng',[workdir,'/bgi_gamma.png']);

% generate plots of error
myfigure(31)
%plot(10*log10(fB_r),sB_r,'b')
%hold on;plot(10*log10(fM_r),sM_r,'c');hold off
%hold on;plot(10*log10(nB_r),nB_r,'r');hold off
plot(10*log10(nB_r),nB_r,'r')
hold on;plot(10*log10(nM_r),nM_r,'g');hold off
hold on;plot(10*log10(nB_r(nB_minindex)),nB_r(nB_minindex),'r*');hold off
hold on;plot(10*log10(nM_r(nM_minindex)),nM_r(nM_minindex),'g*');hold off
xlabel('RMS signal error (dB)')
ylabel('Noise RMS error');
%h=title(sprintf('Ch=%d Np=%d (b=NF BG, c=NF BG med, r=N BG, g=N BG med)',chan,Npass));  set(h,'FontSize',12);
h=title(sprintf('Ch=%d Np=%d (r=N BG, g=N BG med)',chan,Npass));  set(h,'FontSize',12);
print('-dpng',[workdir,'/bgi_sig.png']);


end
end
end