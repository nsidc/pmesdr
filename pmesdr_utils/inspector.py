import matplotlib.pyplot as plt
from netCDF4 import Dataset
import numpy as np
from osgeo import gdal, osr
import re
import sys


def make_png(res, filename):

    print "Making png image for: " + res + ", " + filename

    # Read and reshape the array
    # If the array were stored correctly, we shouldn't have to
    # reshape it here.
    try:
        f = Dataset(filename, 'r', 'NETCDF4')
    except RuntimeError:
        sys.stderr.write("Error opening file " + filename + "\n")
        exit(-1)

    keys = f.variables.keys()
    var_name = 'none'
    for key in keys:
        if ( 'a_image' == key and res != '25' ):
            var_name = 'a_image'
            break
        if ( 'grd_a_image' == key and res == '25' ):
            var_name = 'grd_a_image'
            break
        if ( 'bgi_image' == key ):
            var_name = 'bgi_image'
            break

    if ( 'none' == var_name ):
        sys.stderr.write( filename + ": " + "contains none of a_image, grd_a_image nor bgi_image.\n" )
        exit(-1)

    # Eventually this will need to get the bgi array if it's a bgi dump file
    # Better yet, standardize the array names with what they really should be
    tb = f.variables[ var_name ][ : ]
    rows, cols = np.shape(tb)
    tb = np.flipud(tb.reshape(cols, rows))

    print np.amin(tb), np.amax(tb)
    tb[ tb > 590. ] = 0.

    if ( var_name == 'a_image' ):
        label = 'SIR_TB'
    elif ( var_name == 'bgi_image' ):
        label = 'BGI_TB'
    else:
        label = 'GRD_TB'

    # Make the figure
    fig, ax = plt.subplots(1, 1)
    ax.set_title( filename )
    plt.imshow(tb, cmap=plt.cm.gray, vmin=100, vmax=320)
    plt.axis('off')
    plt.colorbar(shrink=0.35, label=label)
    outfile = filename + '.' + label + '.png'
    fig.savefig(outfile, dpi=300, bbox_inches='tight')
    print "png image saved to: " + outfile


def make_geotiff(grid, filename):

    try:
        f = Dataset(filename, 'r', 'NETCDF4')
    except RuntimeError:
        sys.stderr.write("Error opening file " + filename + "\n")
        exit(-1)

    # Parse the grid for pieces we need
    try:
        m = re.match(r'e2([nst])_(\d+)', grid)
        projection, resolution = m.groups()
    except AttributeError:
        sys.stderr.write("Error parsing grid for projection/resolution.\n")
        exit(-1)

    keys = f.variables.keys()
    var_name = 'none'
    for key in keys:
        if ( 'a_image' == key and resolution != '25' ):
            var_name = 'a_image'
            break
        if ( 'grd_a_image' == key and resolution == '25' ):
            var_name = 'grd_a_image'
            break
        if ( 'bgi_image' == key ):
            var_name = 'bgi_image'
            break

    if ( 'none' == var_name ):
        sys.stderr.write( filename + ": " + "contains none of a_image, grd_a_image nor bgi_image.\n" )
        exit(-1)

    # This reshape command should not be necessary if we are writing the .nc files
    # correctly
    tb = f.variables[var_name][:]
    sys.stderr.write( "Reading image from: " + var_name + "\n")
    cols, rows = np.shape(tb)
    print cols, rows
    tb = np.flipud(tb.reshape(rows, cols))
    print np.amin(tb), np.amax(tb)

    if ( var_name == 'a_image' ):
        label = 'SIR_TB'
    elif ( var_name == 'bgi_image' ):
        label = 'BGI_TB'
    else:
        label = 'GRD_TB'

    outfilename = filename + '.' + label + '.tif'
    driver = gdal.GetDriverByName("GTiff")
    dest_ds = driver.Create(outfilename, cols, rows, 1, gdal.GDT_UInt16)

    # Initialize the projection information
    # When we can connect to epsg v8.6 or later, we should replace proj.4 strings
    # with epsg codes.  For now, we'll just do proj.4 strings
    proj = osr.SpatialReference()
    if projection == 'n':
        dest_srs = "+proj=laea +lat_0=90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m"
    elif projection == 's':
        dest_srs = "+proj=laea +lat_0=-90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m"
    else:
        dest_srs = "+proj=cea +lat_0=0 +lon_0=0 +lat_ts=30 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m"

    proj.SetFromUserInput(dest_srs)
    dest_ds.SetProjection(proj.ExportToWkt())

    # Initialize the grid information (extent and scale)
    # Thanks to web page at:
    # http://geoexamples.blogspot.com/2012/01/creating-files-in-ogr-and-gdal-with.html
    # The geotransform defines the relation between the raster coordinates x, y and the
    # geographic coordinates, using the following definition:
    # Xgeo = geotransform[0] + Xpixel*geotransform[1] + Yline*geotransform[2]
    # Ygeo = geotransform[3] + Xpixel*geotransform[4] + Yline*geotransform[5]
    # The first and fourth parameters define the origin of the upper left pixel
    # The second and sixth parameters define the pixels size.
    # The third and fifth parameters define the rotation of the raster.
    # Values are meters
    if re.match(r'[ns]', projection):
        map_UL_x = -9000000.
        map_UL_y = 9000000.
        if resolution == '25':
            scale_x = 25000.00000
            scale_y = -25000.00000
        elif resolution == '3':
            scale_x = 3125.00000
            scale_y = -3125.00000
        else:
            sys.stderr.write("Unrecognized resolution " + resolution + "\n")
    else:
        map_UL_x = -17367530.44
        map_UL_y = 6756820.20000
        if resolution == '25':
            scale_x = 25025.26000
            scale_y = -25025.26000
        elif resolution == '3':
            scale_x = 3128.15750
            scale_y = -3128.15750
        else:
            sys.stderr.write("Unrecognized resolution " + resolution + "\n")

    geotransform = (map_UL_x, scale_x, 0., map_UL_y, 0., scale_y)
    dest_ds.SetGeoTransform(geotransform)

    dest_ds.GetRasterBand(1).WriteArray((tb + 0.5).astype(np.uint16))
    dest_ds = None

    sys.stderr.write("Wrote geotiff to " + outfilename + "\n")
