function RSS=read_ssmiRSS(fname);
%
% function RSS=read_ssmiRSS(fname)
%
% returns a struct RSS containing the contents of the RSS binary V7 SSMI
% file named fname
%

% initialize output
RSS=[];

% open file
fid=fopen(fname,'r','ieee-le');
if fid <= 0
  return;
end
RSS.KSAT=double(fread(fid,1,'int'));
RSS.IORBIT=double(fread(fid,1,'int'));
RSS.NUMSCAN=double(fread(fid,1,'int'));
RSS.ASTART_TIME=double(fread(fid,24,'char'));
RSS.SCAN_TIME=fread(fid,3600,'double');
RSS.ORBIT=fread(fid,3600,'double');
RSS.SC_LAT=double(fread(fid,[3600],'float'));
RSS.SC_LON=double(fread(fid,[3600],'float'));
RSS.SC_ALT=double(fread(fid,[3600],'float'));
RSS.IQUAL_FLAG=double(fread(fid,3600,'int'));

RSS.CEL_LAT=double(fread(fid,[128,3600],'short'))*0.01;
RSS.CEL_LON=double(fread(fid,[128,3600],'short'))*0.01+180;
RSS.CEL_EIA=double(fread(fid,[128,3600],'short'))*0.002+45;
RSS.CEL_AZM=double(fread(fid,[128,3600],'short'))*0.01+180;
RSS.CEL_SUN=double(fread(fid,[128,3600],'short'))*0.01;
RSS.CEL_LND=double(fread(fid,[128,3600],'short'))*0.4;
RSS.CEL_ICE=double(fread(fid,[128,3600],'short'));

RSS.CEL_85V=double(fread(fid,[128,3600],'short'));
RSS.CEL_85H=double(fread(fid,[128,3600],'short'));
RSS.CEL_19V=double(fread(fid,[64,1800],'short'));
RSS.CEL_19H=double(fread(fid,[64,1800],'short'));
RSS.CEL_22V=double(fread(fid,[64,1800],'short'));
RSS.CEL_37V=double(fread(fid,[64,1800],'short'));
RSS.CEL_37H=double(fread(fid,[64,1800],'short'));

RSS.CEL_85V(RSS.CEL_85V~=0)=RSS.CEL_85V(RSS.CEL_85V~=0)*0.01+100;
RSS.CEL_85H(RSS.CEL_85H~=0)=RSS.CEL_85H(RSS.CEL_85H~=0)*0.01+100;
RSS.CEL_19V(RSS.CEL_19V~=0)=RSS.CEL_19V(RSS.CEL_19V~=0)*0.01+100;
RSS.CEL_19H(RSS.CEL_19H~=0)=RSS.CEL_19H(RSS.CEL_19H~=0)*0.01+100;
RSS.CEL_22V(RSS.CEL_22V~=0)=RSS.CEL_22V(RSS.CEL_22V~=0)*0.01+100;
RSS.CEL_37V(RSS.CEL_37V~=0)=RSS.CEL_37V(RSS.CEL_37V~=0)*0.01+100;
RSS.CEL_37H(RSS.CEL_37H~=0)=RSS.CEL_37H(RSS.CEL_37H~=0)*0.01+100;

fclose(fid);

return

