%function csu=read_ssmiCSU(inpfile);
%
% function csu=read_ssmiCSU(inpfile)
%
% matlab script for reading CSU SSM/I FCDR netcdf files
%
% returns contents of file as structure in csu
%

% written by D.G. Long,  2 May 2014

% set test file name
%inpfile='../../../sample_data/CSU_SSMI_FCDR_V01R00_F13_D19970301_S1557_E1739_R09999.nc';

% create output structure
csu=[];

% check for existence of file
if ~exist(inpfile,'file')
  error(['input file "',inpfile,'" does not exist']);
  return;
else

  NC_GLOBAL=netcdf.getConstant('NC_GLOBAL');
  
  % open netcdf file for reading
  ncid=netcdf.open ( inpfile, 'NOWRITE');
  if ncid < 1
    error(['netcdf open error ',num2str(ncid),' for file "',inpfile,'"']);
    return;
  end
  
  % get variable dimensions
  recid=netcdf.inqUnlimDims(ncid);
  nlscan_dimid=netcdf.inqDimID(ncid,'nscan_lores');
  nhscan_dimid=netcdf.inqDimID(ncid,'nscan_hires');
  [nlscan_name csu.nlscan]=netcdf.inqDim(ncid, nlscan_dimid);
  [nhscan_name csu.nhscan]=netcdf.inqDim(ncid, nhscan_dimid);

  % get global attributes
  csu.title=netcdf.getAtt(ncid,NC_GLOBAL,'title');
  csu.author=netcdf.getAtt(ncid,NC_GLOBAL,'creator_name');
  csu.email=netcdf.getAtt(ncid,NC_GLOBAL,'creator_email');
  csu.url=netcdf.getAtt(ncid,NC_GLOBAL,'creator_url');
  csu.institution=netcdf.getAtt(ncid,NC_GLOBAL,'institution');
  csu.version=netcdf.getAtt(ncid,NC_GLOBAL,'product_version');
  csu.revision_date=netcdf.getAtt(ncid,NC_GLOBAL,'revision_date');
  csu.platform=netcdf.getAtt(ncid,NC_GLOBAL,'platform');
  csu.sensor=netcdf.getAtt(ncid,NC_GLOBAL,'sensor');
  csu.orbit_number=netcdf.getAtt(ncid,NC_GLOBAL,'orbit_number');
  csu.source=netcdf.getAtt(ncid,NC_GLOBAL,'source');
  csu.filename=netcdf.getAtt(ncid,NC_GLOBAL,'id');
  csu.startdate=netcdf.getAtt(ncid,NC_GLOBAL,'time_coverage_start');
  csu.enddate=netcdf.getAtt(ncid,NC_GLOBAL,'time_coverage_end');
  csu.created=netcdf.getAtt(ncid,NC_GLOBAL,'date_created');

  % get variables
  csu.orbit_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'orbit_lores'));
  csu.scan_time=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'scan_time_lores'));
  csu.orbit_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'orbit_lores'));
  csu.scan_time_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'scan_time_lores'));
  csu.spacecraft_lat_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'spacecraft_lat_lores'));
  csu.spacecraft_lon_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'spacecraft_lon_lores'));
  csu.spacecraft_alt_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'spacecraft_alt_lores'));
  csu.lat_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'lat_lores'));
  csu.lon_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'lon_lores'));
  csu.eia_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'eia_lores'));
  csu.sun_glint_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'sun_glint_lores'));
  csu.quality_lores=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'quality_lores'));
  csu.tb19v=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'fcdr_tb19v'));
  csu.tb19h=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'fcdr_tb19h'));
  csu.tb22v=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'fcdr_tb22v'));
  csu.tb37v=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'fcdr_tb37v'));
  csu.tb37h=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'fcdr_tb37h'));
  csu.orbit_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'orbit_hires'));
  csu.scan_time_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'scan_time_hires'));
  csu.spacecraft_lat_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'spacecraft_lat_hires'));
  csu.spacecraft_lon_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'spacecraft_lon_hires'));
  csu.spacecraft_alt_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'spacecraft_alt_hires'));
  csu.lat_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'lat_hires'));
  csu.lon_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'lon_hires'));
  csu.eia_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'eia_hires'));
  csu.sun_glint_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'sun_glint_hires'));
  csu.quality_hires=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'quality_hires'));
  csu.tb85v=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'fcdr_tb85v'));
  csu.tb85h=netcdf.getVar(ncid,netcdf.inqVarID(ncid,'fcdr_tb85h'));

  % close netcdf file
  netcdf.close(ncid);

end

