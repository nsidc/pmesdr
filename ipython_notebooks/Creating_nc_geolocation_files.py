
# coding: utf-8

# # This notebook converts the flat, binary double-precision EASE2 lat/lon files to .nc files
# 
# ## Currently the geolocation_version is set to "v0.0"
# 
# ## To-do for v0.1 geolocation files:
# 
# Need to check back with Donna about whether the EASE2 geolocation data will be a data set in its own right.
# If so, the file attribute "id" will need to be set to the data set id

# In[1]:

import datetime
import glob
import json
from netCDF4 import Dataset
import numpy as np
import os
import re


# In[2]:

get_ipython().magic(u'cd /home/vagrant/measures-byu/ipython_notebooks')


# # Set grid-specific variables

# In[3]:

resolution_at_factor = ["25",
                        "12.5",
                        "6.25",
                        "3.125"]


# In[4]:

def ease2_grid_str(projection="N", resolution="25"):
    return "EASE2_%s%skm" % (projection, resolution)


# In[5]:

def ease2_grid_scale(projection="N", factor=0):
    base_scale = {"N": 25000.,
                  "S": 25000.,
                  "T": 25025.26000}
    return base_scale[projection] / (2. ** factor)


# In[6]:

ease2_map_origin_x = {"N": -9000000.,
                       "S": -9000000.,
                       "T": -17367530.44}
ease2_map_origin_y = {"N": 9000000.,
                       "S": 9000000.,
                       "T": 6756820.20000}
print ease2_map_origin_x["N"]
print ease2_map_origin_y["T"]


# In[7]:

def ease2_geo_filename(grid_name, filetype="lats"):
    list = glob.glob("/share/data/Geolocation/" + grid_name + "." + filetype + ".*double")
    if len(list) != 1:
        print("Unexpected number of " + filetype + " files found.\n")
        raise LookupError
        
    # Parse the filename for rows and columns
    m = re.search(r'\.([0-9]+)x([0-9]+)x1\.double', list[0])
    if m:
        ncols = m.group(1)
        nrows = m.group(2)
    else:
        print("Error parsing filename " + list[0] + " for dimensions.\n")
        raise LookupError
        
    return (list[0], int(nrows), int(ncols))


# In[8]:

file, rows, cols = ease2_geo_filename("EASE2_N25km")
print file
print rows
print cols
file, rows, cols = ease2_geo_filename("EASE2_N12.5km", filetype="lons")
print file, rows, cols



# In[9]:

print file
print os.path.basename(file)


# ## Define a function that converts the lat/lon files for a specified projection and grid to the .nc geolocation file
# 

# In[10]:

def ease2_make_geolocation_file(projection="N", factor=0):
    
    geolocation_version = "v0.0"
    grid_str = ease2_grid_str(projection, resolution_at_factor[factor])
    lat_filename, lat_nrows, lat_ncols = ease2_geo_filename(grid_str, filetype="lats")
    lon_filename, lon_nrows, lon_ncols = ease2_geo_filename(grid_str, filetype="lons")
    if lat_nrows != lon_nrows or lat_ncols != lon_ncols:
        print("Mismatched lat/lon dimensions\n")
        raise LookupError
        
    out_filename = "/projects/PMESDR/vagrant/Geolocation/%s.geolocation.%s.nc" % (grid_str, geolocation_version)
    
    # Set global attributes
    fid = Dataset(out_filename, 'w', format='NETCDF4')
    fid.Conventions = "CF-1.6"
    fid.title = "EASE-Grid 2.0 Pixel Geolocations"
    fid.product_version = geolocation_version
    fid.software_version_id = open("/vagrant/VERSION", "r").read().rstrip()
    fid.software_repository = "git@bitbucket.org:nsidc/measures-byu.git"
    fid.source = json.dumps([os.path.basename(lat_filename), os.path.basename(lon_filename)])
    fid.history = "Creating_nc_geolocation_files.py"
    fid.comment = "Latitude and longitude values at centers of EASE-Grid 2.0 grid cells"
    fid.references = json.dumps(["EASE-Grid 2.0 documentation: http://nsidc.org/data/ease/ease_grid2.html",
                                 "Brodzik, Mary J.; Billingsley, Brendan; Haran, Terry; Raup, Bruce; Savoie, Matthew H. 2012.",
                                 "EASE-Grid 2.0: Incremental but Significant Improvements for Earth-Gridded Data Sets.",
                                 "ISPRS Int. J. Geo-Inf. 1, no. 1: 32-45.",
                                 "Brodzik, Mary J.; Billingsley, Brendan; Haran, Terry; Raup, Bruce; Savoie, Matthew H. 2014.",
                                 "Correction: Brodzik, M. J., et al. EASE-Grid 2.0: Incremental but Significant Improvements for Earth-Gridded Data Sets.",
                                 "ISPRS Int. J. Geo-Inf. 3, no. 3: 1154-1156."
                                ])
    fid.summary = "Geolocation latitude and longitude for EASE-Grid 2.0"
    fid.institution = ["National Snow and Ice Data Center\n",
                       "Cooperative Institute for Research in Environmental Sciences\n",
                       "University of Colorado at Boulder\n",
                       "Boulder, CO"]
    fid.publisher = ["National Snow and Ice Data Center\n",
                     "Cooperative Institute for Research in Environmental Sciences\n",
                     "University of Colorado at Boulder\n",
                     "Boulder, CO"]
    fid.publisher_url = "http://nsidc.org"
    fid.publisher_email = "nsidc@nsidc.org"
    fid.project = "NASA 2012 MEaSUREs (Making Earth System Data Records for Use in Research Environments)"
    fid.standard_name_vocabulary = "CF Standard Name Table (v27, 28 September 2013)"
    fid.cdm_data_type = "grid"
    fid.keywords = "EARTH SCIENCE SERVICES > DATA ANALYSIS AND VISUALIZATION > GEOGRAPHIC INFORMATION SYSTEMS"
    fid.keywords_vocabulary = "NASA Global Change Master Directory (GCMD) Earth Science Keywords, Version 8.1"
    fid.naming_authority = "org.doi.dx"
    fid.id = "TBD: 10.5067/MEASURES/CRYOSPHERE/nsidc-XXXX.000"
    fid.date_created = str(datetime.datetime.now())
    fid.acknowledgement = ["This data set was created with funding from NASA MEaSUREs Grant #NNX13AI23A.\n",
                           "Data archiving and distribution is supported by the NASA NSIDC Distributed Active Archive Center (DAAC)."]
    fid.license = "No constraints on data access or use"
    fid.processing_level = "Level 3"
    fid.creator_name = "Mary J. Brodzik"
    fid.creator_email = "brodzik@nsidc.org"
    fid.creator_url = "http://nsidc.org/pmesdr"
    fid.contributor_name = "Mary J. Brodzik"
    fid.contributor_role = "Principal Investigator"
    fid.citation = ["Brodzik, M. J..\n",
                    "EASE-Grid 2.0 Pixel Geolocations.\n",
                    "Version 0.01.\n",
                    "[Indicate subset used].\n",
                    "Boulder, Colorado USA: NASA DAAC at the National Snow and Ice Data Center." ]
    
    # Create dimension variables
    # The following calcuations assume that the map origin is exactly centered
    # in the grid, which allows me to just multiply the map origin by -1 to get the 
    # diagonal corner
    scale = ease2_grid_scale(projection, factor)
    nrows = lat_nrows
    row_min = -1 * ease2_map_origin_y[projection]
    row_max = ease2_map_origin_y[projection]
    col_min = ease2_map_origin_x[projection]
    col_max = -1 * ease2_map_origin_x[projection]
    ncols = lat_ncols
    rows = fid.createDimension("rows", nrows)
    cols = fid.createDimension("cols", ncols)
    rows_var = fid.createVariable("rows", "f8", ("rows",))
    cols_var = fid.createVariable("cols", "f8", ("cols",))
    rows_var.standard_name = "projection_y_coordinate"
    rows_var.units = "meters"
    rows_var.axis = "Y"
    rows_var.valid_range = [row_min, row_max]
    cols_var.standard_name = "projection_x_coordinate"
    cols_var.units = "meters"
    cols_var.axis = "X"
    cols_var.valid_range = [col_min, col_max]
    
    # Copy appropriate crs metadata from template file
    # For the string variables being copied, if we do not cast them with str() they will
    # be written to the output file in a way that ncdump sees them as "string crs:name"
    # and for some reason these are then ignored by gdal tools, which don't recognize the crs
    # information and can't make a geotiff with PROJCS information.
    # If they are copied with str(), then gdal tools see them as expected.
    # I have no idea why.
    template = Dataset("/home/vagrant/measures-byu/src/prod/cetb_file/templates/cetb_global_template.nc", 'r', "NETCDF4")
    src_crs = template.variables['crs_EASE2_%s' % projection]
    crs = fid.createVariable( 'crs', 'S1', () )
    crs.grid_mapping_name = str(src_crs.grid_mapping_name)
    if projection == "N" or projection == "S":
        crs.longitude_of_projection_origin = src_crs.longitude_of_projection_origin
        crs.latitude_of_projection_origin = src_crs.latitude_of_projection_origin
    else:
        crs.longitude_of_central_meridian = src_crs.longitude_of_central_meridian
        crs.standard_parallel = src_crs.standard_parallel
    crs.false_easting = src_crs.false_easting
    crs.false_northing = src_crs.false_northing
    crs.proj4text = str(src_crs.proj4text)
    crs.srid = str(src_crs.srid)
    crs.crs_wkt = str(src_crs.crs_wkt)
    crs.long_name = grid_str
    crs.scale_factor_at_projection_origin = scale
    
    # populate dimension variable values
    # Using linspace to ensure these values are exact distances
    rows_var[:] = np.linspace(row_min + 0.5 * scale, row_max - 0.5 * scale, endpoint=True, num=nrows)[::-1]
    cols_var[:] = np.linspace(col_min + 0.5 * scale, col_max - 0.5 * scale, endpoint=True, num=ncols)
    
    # Read lat/lon data arrays and store as variables
    lats = np.fromfile(lat_filename, dtype='f8').reshape((nrows, ncols))
    lons = np.fromfile(lon_filename, dtype='f8').reshape((nrows, ncols))
    
    latitude = fid.createVariable( 'latitude', 'f8', ("rows", "cols"), zlib=True, fill_value=-999.)
    latitude[:] = lats
    latitude.standard_name = "latitude"
    latitude.long_name = "Latitude"
    latitude.units = "degrees_north"
    if projection == "N":
        latitude.valid_range = [0., 90.]
    elif projection == "S":
        latitude.valid_range = [-90., 0.]
    else:
        latitude.valid_range = [-67.0575406, 67.0575406]
        latitude.axis = "Y" # only for cylindrical grids
    latitude.grid_mapping = "crs"
    latitude.coverage_content_type = "image"
    
    longitude = fid.createVariable( 'longitude', 'f8', ("rows", "cols"), zlib=True, fill_value=-999.)
    longitude[:] = lons
    longitude.standard_name = "longitude"
    longitude.long_name = "Longitude"
    longitude.units = "degrees_east"
    longitude.valid_range = [-180., 180.]
    if projection != "N" and projection != "S":
        longitude.axis = "X" # only for cylindrical grids
    longitude.grid_mapping = "crs"
    longitude.coverage_content_type = "image"

    print "Non-fill Range of latitude: %s - %s" % (np.amin(latitude[:][latitude[:] > -999.]), np.amax(latitude))
    print "Non-fill Range of longitude: %s - %s" % (np.amin(longitude[:][longitude[:] > -999.]), np.amax(longitude))
    
    template.close()
    fid.close()
    print "Geolocation file written to: %s" % out_filename


# In[11]:

# Just run one for testing
# ease2_make_geolocation_file("N",1)


# In[12]:

for projection in ["N", "S", "T"]:
    for factor in np.arange(4):
        print("Next: %s, %s" % (projection, factor))
        ease2_make_geolocation_file(projection, factor)


# In[ ]:



