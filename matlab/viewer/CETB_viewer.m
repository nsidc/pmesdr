function CETB_viewer(filein)
%
% CETB_viewer(input_filename)
%
% view CETB image with interactive location, color tables, etc.
%
if nargin>0,
  CETB_viewer_engine('initialize',filein)
else
  warning('No file name specified.  Default image being used.');
  CETB_viewer_engine('initialize')
end
