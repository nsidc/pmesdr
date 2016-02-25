
# coding: utf-8

# In[1]:

import datetime
from netCDF4 import Dataset
import numpy as np
import json


# In[2]:

get_ipython().magic(u'cd /home/vagrant/measures-byu/ipython_notebooks')


# # Set grid-specific variables

# In[3]:

resolution_at_factor = ["25",
                        "12.5",
                        "6.25",
                        "3.125"]
#for factor in np.arange(4):
#    print resolution_at_factor[factor]


# In[4]:

def ease2_grid_str(projection="N", resolution="25"):
    return "EASE2_%s%skm" % (projection, resolution)
#for factor in np.arange(4):
#    print ease2_grid_str("N", resolution=resolution_at_factor[factor])


# In[5]:

def ease2_grid_scale(projection="N", factor=0):
    base_scale = {"N": 25000.,
                  "S": 25000.,
                  "T": 25025.26000}
    return base_scale[projection] / (2. ** factor)
# for factor in np.arange(4):
#     print ease2_grid_scale("N",factor)
#     print ease2_grid_scale("S",factor)
#     print ease2_grid_scale("T",factor)
    
    


# ## Define a function that converts the lat/lon files for a specified projection and grid to the .nc geolocation file
# 

# In[8]:

def ease2_make_geolocation_file(projection="N", factor=0):
    # lat_filename =
    out_filename = "/projects/PMESDR/vagrant/Geolocation/%s.geolocation.nc" % ease2_grid_str(projection,
                                                                                             resolution_at_factor[factor])
    
    # Set global attributes
    fid = Dataset(out_filename, 'w', format='NETCDF4')
    fid.Conventions = "CF-1.6"
    fid.title = "EASE-Grid 2.0 Pixel Geolocations"
    fid.product_version = "v0.01"
    fid.software_version_id = open("/vagrant/VERSION", "r").read().rstrip()
    fid.software_repository = "git@bitbucket.org:nsidc/measures-byu.git"
    fid.source = "TBD(list of geolocation lat/lon files used)"
    fid.history = "Jupyter notebook: Creating_nc_geolocation_files"
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
    fid.keywords = "TBD"
    fid.keywords_vocabulary = "TBD"
    fid.naming_authority = "org.doi.dx"
    fid.id = "TBD: 10.5067/MEASURES/CRYOSPHERE/nsidc-XXXX.001"
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
 
    
    fid.close()
    print "Geolocation file written to: %s" % out_filename


# In[10]:

ease2_make_geolocation_file("N",1)


# In[ ]:

fid = Dataset('/projects/PMESDR/vagrant/Geolocation/EASE2_N25km.geolocation.nc', 'w', format='NETCDF4')


# In[ ]:

fid.Conventions = "CF-1.6"
fid.title = "EASE-Grid 2.0 Pixel Geolocations"
fid.product_version = "v0.01"
fid.software_version_id = open("/vagrant/VERSION", "r").read().rstrip()
fid.software_repository = "git@bitbucket.org:nsidc/measures-byu.git"
fid.source = "TBD(list of geolocation lat/lon files used)"
fid.history = "Jupyter notebook: Creating_nc_geolocation_files"
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
fid.keywords = "TBD"
fid.keywords_vocabulary = "TBD"
fid.naming_authority = "org.doi.dx"
fid.id = "TBD: 10.5067/MEASURES/CRYOSPHERE/nsidc-XXXX.001"
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


# # Create dimension variables
# 

# In[ ]:

scale = 25000.
nrows = 720
row_min = -9000000.
row_max = 9000000.
col_min = -9000000.
col_max = 9000000.
ncols = 720
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


# # Copy appropriate crs metadata from template file

# In[ ]:

template = Dataset("/home/vagrant/measures-byu/src/prod/cetb_file/templates/cetb_global_template.nc", 'r', "NETCDF4")
src_crs = template.variables['crs_EASE2_N']


# In[ ]:

crs = fid.createVariable( 'crs', 'S1', () )
crs.grid_mapping_name = src_crs.grid_mapping_name
crs.longitude_of_projection_origin = src_crs.longitude_of_projection_origin
crs.latitude_of_projection_origin = src_crs.latitude_of_projection_origin
crs.false_easting = src_crs.false_easting
crs.false_northing = src_crs.false_northing
crs.proj4text = src_crs.proj4text
crs.srid = src_crs.srid
crs.crs_wkt = src_crs.crs_wkt
crs.long_name = "EASE2_N25km"
crs.scale_factor_at_projection_origin = scale


# In[ ]:

template.close()


# # Populate dimension variable values
# 
# Use linspace to ensure these values are exact distances

# In[ ]:

rows_var[:] = np.linspace(row_min + 0.5 * scale, row_max - 0.5 * scale, endpoint=True, num=nrows)[::-1]
cols_var[:] = np.linspace(col_min + 0.5 * scale, col_max - 0.5 * scale, endpoint=True, num=ncols)


# # Read lat/long data and store as variables

# In[ ]:

lats = np.fromfile('/projects/PMESDR/tmp/geolocation/EASE2_N25km.lats.720x720x1.double', dtype='f8').reshape((nrows, ncols))
lons = np.fromfile('/projects/PMESDR/tmp/geolocation/EASE2_N25km.lons.720x720x1.double', dtype='f8').reshape((nrows, ncols))


# In[ ]:

latitude = fid.createVariable( 'latitude', 'f8', ("rows", "cols"), zlib=True, fill_value=-999.)
latitude.standard_name = "latitude"
latitude.long_name = "Latitude"
latitude.units = "degrees_north"
latitude.valid_range = [-90., 90.]
# latitude.axis = "Y" # only for T grids
latitude[:] = lats
longitude = fid.createVariable( 'longitude', 'f8', ("rows", "cols"), zlib=True, fill_value=-999.)
longitude.standard_name = "latitude"
longitude.long_name = "Latitude"
longitude.units = "degrees_north"
longitude.valid_range = [-90., 90.]
# longitude.axis = "X" # only for T grids
longitude[:] = lons


# In[ ]:

fid.close()


# In[ ]:

np.amin(lats)


# In[ ]:

np.amin(lons)


# In[ ]:



