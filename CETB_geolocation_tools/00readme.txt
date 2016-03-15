Readme file for CETB project geolocation utilities

Geolocation files
-----------------

For each of the EASE-Grid 2.0 CETB projections and grid
resolutions, we have created a netcdf file in the directory that
contains this readme file.  Each .nc geolocation file contains
variable arrays with the latitude and longitude values at the
center of each grid cell.

Geolocation filenames are:

EASE2_<P><res>.geolocation.v0.0.nc, where:

EASE2_ : indicates the projections are EASE-Grid 2.0
<P>    : one of N (Northern), S (Southern), T (Cylindrical
         Temperate/Tropical latitudes)
<res>  : one of 25km, 12.5km, 6.25km, 3.125km, resolution
       	 of grid
.geolocation : data file contains latitude/longitude
	       geolocation variables
.v0.0        : initial version of data (subject to revision
	       based on user feedback)
.nc          : netCDF4 format

The latitude/longitude variables contained in the files contain
location values in decimal degrees at the center of the
respective grid cell location.

Please refer to the following publications for projection details
on the EASE-Grid 2.0 projections:

Brodzik, M. J., B. Billingsley, T. Haran, B. Raup,
M. H. Savoie. 2012. EASE-Grid 2.0: Incremental but Significant
Improvements for Earth-Gridded Data Sets. ISPRS International
Journal of Geo-Information, 1(1):32-45, doi:10.3390/ijgi1010032.
http://www.mdpi.com/2220-9964/1/1/32/

and

Brodzik, M. J., B. Billingsley, T. Haran, B. Raup,
M. H. Savoie. 2014. Correction: Brodzik, M. J. et al. EASE-Grid
2.0: Incremental but Significant Improvements for Earth-Gridded
Data Sets. ISPRS International Journal of Geo-Information 2012,
1, 32-45. ISPRS International Journal of Geo-Information,
3(3):1154-1156, doi:10.3390/ijgi3031154.
http://www.mdpi.com/2220-9964/3/3/1154/.

***************************************************************
***************************************************************

For map transformation software, and/or to perform
transformations at locations other than the centers of grid
cells, you have several options, as follows.

***************************************************************
For python users:

We have created a python package for linux and osx platforms
available from anaconda.org:

http://anaconda.org/nsidc/cetbtools

Use the conda package manager to install the cetbtools.  This ipython
notebook includes examples on how to open and display data in a
CETB file, and examples on how to understand the projection and
grid metadata, the time variable and run the map transformations
from (row,col) <--> (lat, lon):

http://nbviewer.jupyter.org/gist/mjbrodzik/a250ff522cb49dc155eb

Note: Even if you are not a python progammer, reviewing this
notebook will likely help you understand how the data in the CETB
files are organized.  You only need a browser to view the
notebook.

***************************************************************
For C users:

The NSIDC mapx software package is a C-callable library that
performs (lat,lon)<->(col,row) transformations for many
projections, including the EASE-Grid 2.0 projection/grids.

NSIDC user services maintains this page with information on how
to download/install/use mapx:

https://support.nsidc.org/entries/42415620-Mapx-Map-Transformations-Library

The CETB .gpd file definitions can be obtained from bitbucket.org, here:

https://bitbucket.org/nsidc/maps

The .gpd files used for the CETB project are:

EASE2_<P>25km.gpd
EASE2_<P>12.5km.gpd
EASE2_<P>6.25km.gpd
EASE2_<P>3.125km.gpd

for <P>= N, S, and T, respectively.

In any given CETB file, the "long_name" attribute to the "crs"
variable contains the name of the mapx gpd file that corresponds
to the data in the file.

Also refer to the ipython notebook mentioned above in the python
section, for a link to view an ipython notebook in your browser
to better understand the file contents and organization.

***************************************************************
For matlab users:

We have created a tarball with matlab forward and reverse
transformations in the directory containing this readme file.
The matlab utilities are in:

CETB_matlab_utiles.tar.gz

Once you have downloaded and untarred the file, follow
instructions for using the utilities at the unpacked file

viewer/readme.txt

Also refer to the ipython notebook mentioned above in the python
section, for a link to view an ipython notebook in your browser
to better understand the file contents and organization.

***************************************************************
For IDL users

Updates to the NSIDC IDL easeconv routines for the full set of
CETB grids are coming soon.

***************************************************************
Other utilities:

A great benefit of EASE-Grid 2.0 data definitions is that other
standard map projection software now understands the geolocation
information.

The list of software that understands EASE-Grid 2.0 includes
GDAL, PROJ.4, ENVI, ArcGIS and others.  Please refer to Table 2
on this page for updates and the latest information from NSIDC on
compatible software:

https://nsidc.org/data/ease/versions.html


***************************************************************
Questions/feedback:

As usual, questions and feedback are welcome:

Mary Jo Brodzik
brodzik@nsidc.org

This information was last updated:
10 March 2016
