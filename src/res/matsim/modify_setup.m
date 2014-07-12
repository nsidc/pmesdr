function des=modify_setup(file_in, file_out, icase)
%
% function modify_setup(file_in, file_out, icase)
%
% reads simplified setup file from file_in, modifies MRF based on
% value of icase, and writes modified setup file to file_out
%

% (c) copyright 2014 David G. Long, Brigham Young Unversity
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% open files
iid=fopen(file_in,'r');
fid=fopen(file_out,'w');
%disp(sprintf('Writing %s',file_out));

% copy header
M=fread(iid,1,'int32');
N=fread(iid,1,'int32');
sampspacing=fread(iid,1,'float32');
grd_size=fread(iid,1,'float32');
true1=fread(iid,[M N],'float32');

fwrite(fid,[M, N],'int32');         % write image size
fwrite(fid,sampspacing,'float32');  % write pixel resolution
fwrite(fid,grd_size,'float32');     % write GRD pixel size
fwrite(fid,true1,'float32');        % write "truth" image

cnt=0;
for i=1:1e10
  % read measurement and associated response function from input .setup file
  iadd=fread(iid,1,'int32');
  if length(iadd)<1, break; end;
  n=fread(iid,1,'int32');
  z=fread(iid,1,'float32');
  z1=fread(iid,1,'float32');
  pointer=fread(iid,n,'int32');
  aresp1=fread(iid,n,'float32');
  
  %disp(sprintf('%d %d %d %f %f',i,n,icase,max(aresp1),sum(aresp1)));
    
  % modify MRF
  % note that measurements are not modified
  switch icase
    case 1 % do nothing
      des='True';
    case 2 % 3dB binary
      mmax=max(aresp1);
      ind=find(aresp1>0.5*mmax);
      aresp1(:)=0;
      aresp1(ind)=1;
      des='-3dB binary';
    case 3 % 6dB binary
      mmax=max(aresp1);
      ind=find(aresp1>0.25*mmax);
      aresp1(:)=0;
      aresp1(ind)=1;
      des='-6dB binary';
    case 4 % very large binary
      aresp1(:)=1/length(aresp1);
      des='-30dB binary';
    case 5 % truncated 3 dB
      mmax=max(aresp1);
      aresp1(aresp1<0.5*mmax)=0;
      des='Truncated 3dB';
    case 6 % truncated 6 dB
      mmax=max(aresp1);
      aresp1(aresp1<0.25*mmax)=0;
      des='Truncated 3dB';
    case 7 % square
      aresp1=aresp1.^2;
      aresp1=aresp1/sum(aresp1);
      des='Squared';
    case 8 % square-root
      aresp1=sqrt(aresp1);
      des='Square root';
    otherwise % do nothing
      des='invalid case';
  end
  
  % normalize sum to 1
  aresp1=aresp1/sum(aresp1);
  
  % drop zero gain terms
  pointer=pointer(aresp1>0);
  aresp1=aresp1(aresp1>0);

  %disp(sprintf('%f',aresp1))
  
  % save measurement and associated response function to .setup file
  fwrite(fid,[iadd length(pointer)],'int32');
  fwrite(fid,[z z1],'float32');
  fwrite(fid,pointer,'int32');
  fwrite(fid,aresp1,'float32');
  cnt=cnt+1;
end

% close files
fclose(fid);
fclose(iid);