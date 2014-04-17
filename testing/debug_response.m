%
% read and plot debug_response.out file produced by meas_meta_setup
%
clear;

fname='debug_response.out';

fid=fopen(fname,'r');
line=fgetl(fid);
response_threshold=sscanf(line,'%f');

% for each measurement
mcnt=0;
while feof(fid)==0
  
  line=fgetl(fid);
  if feof(fid), break; end;
    a=sscanf(line,'%d %d %d %d %f %f %f %f');
  iscan=a(1);
  iregion=a(2);
  ibeam=a(3);
  i=a(4);
  clat=a(5);
  clon=a(6);
  sc_last_lat=a(7);
  sc_last_lon=a(8);
  
  disp(sprintf('%d %d %d %d %f %f %f %f',iscan, iregion, ibeam, i, clat, clon, sc_last_lat, sc_last_lon));
  
  line=fgetl(fid);
  if feof(fid), break; end;
  a=sscanf(line,'%d %d %f %f %f %f %d %d %d');
  ix2=a(1);
  iy2=a(2);
  theta=a(3);
  azang=a(4);
  angerr=a(5);
  dscale=a(6);
  nsx=a(7);
  nsy=a(8);
  iadd=a(9);

  line=fgetl(fid);
  if feof(fid), break; end;
  a=sscanf(line,'%f %d %d %d %d %d %d');
  dscale1=a(1);
  BOX_SIZE=a(2);
  ixsize=a(3);
  iysize=a(4);
  ixsize1=a(5);
  ixsize2=a(6);
  iysize1=a(7);
  iysize2=a(8);
  
  z=NaN([length(iysize1:iysize2),length(ixsize1:ixsize2)]);
  ix1=z;
  iy1=z;
  x_rel=z;
  y_rel=z;
  resp=z;
  iadd1=z;
  alon1=z;
  alat1=z;
  plon=z;
  plat=z;
  
  for iy1=iysize1:iysize2
    iy=iy1-iysize1+1;
    for ix1=ixsize1:ixsize2
      ix=ix1-ixsize1+1;

      line=fgetl(fid);
      if feof(fid), break; end;
      a=sscanf(line,'%d %d %f %f %f %d %f %f %f %f');
      ix1(ix,iy)=a(1);
      iy1(ix,iy)=a(2);
      x_rel(ix,iy)=a(3);
      y_rel(ix,iy)=a(4);
      resp(ix,iy)=a(5);
      iadd1(ix,iy)=a(6);
      alon1(ix,iy)=a(7);
      alat1(ix,iy)=a(8);
      plon(ix,iy)=a(9);
      plat(ix,iy)=a(10);
	
    end
  end
  mcnt=mcnt+1;
  
  % max location error
  disp(sprintf('  max pixel location err: %f %f (lat,lon deg)',[max(max(abs(plat-alat1))) max(max(abs(plon-alon1)))]));
  
  myfigure(mcnt);
  subplot(2,2,1)
  imagesc(resp);colorbar; axis xy
  title(sprintf('%d %d %d %d %f %f',iscan, iregion, ibeam, i, clat, clon));

  subplot(2,2,2)
  if 1
    imagesc(atan2(y_rel,x_rel)*180/pi,[-180 180]); colorbar; axis xy
  else
    plot(plon,plat,'b.');
    %plot(alon1,alat1,'b.');
    hold on; plot(clon,clat,'k*'); hold off;
    xlabel('lon');ylabel('lat');
  end
  title(sprintf('%f %f %f %f',theta,azang,angerr,dscale));

  lim=[min([min(min(y_rel)) min(min(x_rel))]) ...
	max([max(max(y_rel)) max(max(x_rel))])];
  lim=round(lim/10)*10;
  
  subplot(2,2,3)
  imagesc(x_rel,lim); colorbar; axis xy
  hold on; plot(0,0,'k*'); hold off;
  title('x\_rel');
  
  subplot(2,2,4)
  imagesc(y_rel,lim); colorbar; axis xy
  hold on; plot(0,0,'k*'); hold off;
  title('y\_rel');
  
  drawnow;

  
  myfigure(100+mcnt)
  h=subplot(1,2,2);
  plot(plon,plat,'b.');
  %plot(alon1,alat1,'b.');
  hold on; plot(clon,clat,'k*'); hold off;
  xlabel('lon');ylabel('lat');
  title(sprintf('%f %f %f %f',theta,azang,angerr,dscale));

  conts=[0 -3 -6 -9 -12 -20 -30];
  g=subplot(1,2,1);
  respdB=10*log10(resp/max(max(resp))+1.e-10);
  contour(plon,plat,respdB,conts); colorbar('North');
  hold on; plot(clon,clat,'k*'); hold off;
  set(g,'XLim',get(h,'Xlim'));
  set(g,'YLim',get(h,'Ylim'));
  title('SRF dB')
  
end

fclose(fid);
  
disp(sprintf('%d measurements',mcnt));