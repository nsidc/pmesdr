
# coding: utf-8

# # Using NCO utilities ncks and ncrcat to subset CETB files by location and concatenate by time series
# 
# To begin, you need to know the map coordinates of the spatial subset you want.
# 
# You can use various utilities to do this, we have written the python package cetbtools.ease2conv to help with it.
# 
# For this example, I want the upper left quadrant of the EASE2_N25km grid, which is bounded by
# the upper left corner of the grid and the North Pole at the center of the grid.

# In[45]:

get_ipython().magic(u'pylab inline')
from netCDF4 import Dataset
import matplotlib.pyplot as plt
import numpy as np
from cetbtools.ease2conv import Ease2Transform


# In[46]:

subULrow, subULcol = -0.5, -0.5
subLRrow, subLRcol = 359.5, 359.5
N25 = Ease2Transform("EASE2_N25km")
subULx, subULy = N25.grid_to_map(subULrow, subULcol)
subLRx, subLRy = N25.grid_to_map(subLRrow, subLRrow)
print "Subset UL x,y = %.3f, %.3f" % (subULx, subULy)
print "Subset LR x,y = %.3f, %.3f" % (subLRx, subLRy)


# So these min/max values for cols and rows can be passed to ncks to subset the TB variable from a CETB file with:
# 
# ncks -d cols,-9000000.,0. -d rows,0.,9000000. -v TB EASE2_N25km.F13_SSMI.2003001.19H.E.GRD.CSU.v0.1.nc d001.19H.ul.nc
# 
# This subsets the upper left quadrant of the EASE2_N25km grid into the file d001.19H.ul.nc.

# In[47]:

get_ipython().magic(u'cd /projects/PMESDR/vagrant/brodzik/')


# In[48]:

filename = "d001.19H.ul.nc"
fid = Dataset( filename, "r", format="NETCDF4")
fid


# In[49]:

tb = fid.variables['TB'][:]


# In[50]:

np.shape(tb)


# In[51]:

tb = np.squeeze(tb)
np.shape(tb)


# In[52]:

plt.imshow(tb)


# Then for each subset file you must change "time" from a fixed dimension to a record (unlimited dimension), using
# 
# ncks -O --mk_rec_dmn time d002.19H.ul.nc d002.19H.ul.new.nc
# 
# (this will change the ncdump -h information from: 
# 
# <blockquote>
# <p>dimensions:
# 	<p>time = 1 ;
# </blockquote>
#     
# to   
# 
# <blockquote>
# <p>dimensions:
# 	<p>time = UNLIIMITED ; // (1 currently)
# </blockquote>
# Then for a full list of files you can concatenate them in the time dimension with:
# 
# ncrcat -O *.new.nc all_days.19H.ul.nc
# 
# Thanks to this NASA recipe %pylab inline
# from netCDF4 import Dataset
# import matplotlib.pyplot as plt
# import numpy as npfor tips here:
# 
# http://disc.sci.gsfc.nasa.gov/recipes/?q=recipes/How-to-Concatenate-the-Time-Dimension-of-netCDF-Files-with-NCO

# In[53]:

combfile = "d001-003.19H.ul.nc"
combfid = Dataset(combfile, 'r', format="NETCDF")
combfid


# In[54]:

np.shape(combfid.variables["TB"][:])


# In[55]:

slice0 = combfid.variables["TB"][:][0,:,:]


# In[56]:

np.shape(slice)


# In[57]:

plt.imshow(slice0)


# In[58]:

slice1 = combfid.variables["TB"][:][1,:,:]
plt.imshow(slice1)


# In[59]:

slice2 = combfid.variables["TB"][:][2,:,:]
plt.imshow(slice2)


# # Test pixel map coordinates for Melt Onset analysis
# 
# For Karakoram pixel at 31.1N, 75.8E:

# In[60]:

lat, lon = 31.1, 75.8
row, col = N25.geographic_to_grid(lat, lon)
x, y = N25.grid_to_map(row, col)
print row,col
print x, y


# For Himalaya pixel at 38.1N, 88.3E:

# In[61]:

lat, lon = 38.1, 88.3
row, col = N25.geographic_to_grid(lat, lon)
x, y = N25.grid_to_map(row, col)
print row,col
print x, y


# In[ ]:



