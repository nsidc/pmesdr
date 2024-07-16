function varargout=CETB_load(fname)
%
% img=CETB_load(fname)
% [TBimg,iopt,isc]=CETB_load(fname)
% [TBimg,iopt,isc,TBn]=CETB_load(fname)
% [TBimg,iopt,isc,TBn,IncAng]=CETB_load(fname)
% [TBimg,iopt,isc,TBn,IncAng,TBstd]=CETB_load(fname)
% [TBimg,iopt,isc,TBn,IncAng,TBstd,Time]=CETB_load(fname)
%
% load CETB netcdf file TB values into memory and optionally return 
% CETB EASE2 projection info.  Note: memory is stored in original
% file order, for conventional display in matlab use the transpose
% of the output array, e.g., imagesc(img')  Optional projection
% information is decoded from the CETB file name.
%
% INPUTS:
%  fname - CETB file name including directory
%
% OUTPUTS:
% TBimg - TB array (K)
%  iopt - EASE2 projection type 8=EASE2 N, 9=EASE2 S, 10=EASE2 T/M
%   isc - grid scale factor (0..5) pixel size is (25/2^isc) km
%   TBn - TB_numb_samples array 
% IncAng- Incidence_angle array (deg)
% TBstd - TB_std_dev array (K)
%  Time - TB_time array (time in min from start of image period)
%

% written by DGL at BYU 27 Feb 2016

% see if can access file
if ~exist(fname,'file')
  error(sprintf('*** Could not find file input file: %s\n',fname));
end

if nargout==0
  error('*** At least one output required in CETB_load');
end

% get image data from file using netcdf routine
varargout{1}=ncread(fname,'TB');

if nargout>1
  % decode projection information from fname
  ind=findstr(fname,'EASE2_');
  proj=fname(ind+6);
  ind2=findstr(fname(ind:end),'km');
  pscale=sscanf(fname(ind+7:ind+ind2-2),'%f');
  switch proj
    case 'N'
      iopt=8;
    case 'S'
      iopt=9;
    case 'T'
      iopt=10;
    otherwise % invalid
      iopt=-1;
  end
  isc=round(log(25/pscale)/log(2));
  if isc<0 | isc>5
    error('*** decoded EASE2 scale factor error');
  end
  
  varargout{2}=iopt;
  if nargout>2
    varargout{3}=isc;
      if nargout>3
	varargout{4}=ncread(fname,'TB_num_samples');
	if nargout>4
	  varargout{5}=ncread(fname,'Incidence_angle');
	  if nargout>5
	    varargout{6}=ncread(fname,'TB_std_dev');
	    if nargout>6
	      varargout{7}=ncread(fname,'TB_time');
	    end
	  end
	end
      end
    end
  end

