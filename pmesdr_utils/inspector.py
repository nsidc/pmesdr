import matplotlib.pyplot as plt
from netCDF4 import Dataset
import numpy as np
import os
from osgeo import gdal, osr
import re
import sys


def make_png(filename, var_name='TB'):

    # Read the requested variable and eliminate the time dimension
    try:
        f = Dataset(filename, 'r', 'NETCDF4')
    except RuntimeError:
        sys.stderr.write("Error opening file " + filename + "\n")
        exit(-1)

    data = f.variables[ var_name ][ : ]
    times, rows, cols = np.shape(data)
    data = data.reshape(cols, rows)

    # Make the legend label and output filename
    # from the variable's long_name
    label = f.variables[var_name].long_name
    label_with_underscores = re.sub(r' ', r'_', label)

    # Figure out ranges and fill_values for scaling
    valid_range = f.variables[var_name].valid_range
    fill_value = f.variables[var_name]._FillValue
    if "packing_convention" in dir(f.variables[var_name]):
        print "data are packed..."
        add_offset = f.variables[var_name].add_offset
        scale_factor = f.variables[var_name].scale_factor
        valid_range = valid_range * scale_factor + add_offset
        fill_value = fill_value * scale_factor + add_offset

    # Some of the valid ranges are a little generous
    # so adjust them to make the images more meaningful
    # We might consider adding command-line parameters to
    # fine-tune these at will, and to control what range is
    # actually displayed
    if "TB" == var_name:
        valid_range[0] = 100.0
    elif "TB_num_samples" == var_name:
        valid_range[1] = np.amax(data)
    elif "Incidence_angle" == var_name:
        valid_range = [52., 54.]
    elif "TB_time" == var_name:
        mins_per_day = 60 * 24
        valid_range = [ -0.5 * mins_per_day, 1.5 * mins_per_day ]

    print " data range: " + str(np.amin(data[data != fill_value])) + \
        " - " + str(np.amax(data[data != fill_value]))
    print "valid range: " + str(valid_range[0]) + " - " + str(valid_range[1])

    # Make the figure
    fig, ax = plt.subplots( 1, 1 )
    ax.set_title( os.path.basename( filename ) )
    plt.imshow( data, cmap=plt.cm.gray,
                vmin=valid_range[0], vmax=valid_range[1] )
    plt.axis('off')
    plt.colorbar(shrink=0.35, label=label)
    outfile = filename + '.' + label_with_underscores + '.png'
    fig.savefig(outfile, dpi=300, bbox_inches='tight')

    f.close()
    print "png image saved to: " + outfile


def make_geotiff(filename, var_name="TB"):

    if "TB" != var_name:
        sys.stderr.write("geotiff only implemented for TB at this time.\n")
        sys.stderr.write("(Needs work on types of other variables.)\n")
        exit(-1)

    # Read the requested variable and eliminate the time dimension
    try:
        f = Dataset(filename, 'r', 'NETCDF4')
    except RuntimeError:
        sys.stderr.write("Error opening file " + filename + "\n")
        exit(-1)

    data = f.variables[ var_name ][ : ]
    times, rows, cols = np.shape(data)
    data = data.reshape(cols, rows)

    # Make the legend label and output filename
    # from the variable's long_name
    label = f.variables[var_name].long_name
    label_with_underscores = re.sub(r' ', r'_', label)

    outfilename = filename + '.' + label_with_underscores + '.tif'
    driver = gdal.GetDriverByName("GTiff")

    # This type specifier will need to be variable-specific
    # to allow this routine to work with other variables
    dest_ds = driver.Create(outfilename, cols, rows, 1, gdal.GDT_UInt16)

    # Initialize the projection information
    # When we can connect to epsg v8.6 or later,
    # we should replace proj.4 strings
    # with epsg codes.  For now, we'll just use proj.4 strings
    proj = osr.SpatialReference()
    dest_srs = str(f.variables["crs"].proj4text)
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
    # This UL information should be in the file crs variable information
    grid_name = str(f.variables["crs"].long_name)
    if re.match(r'EASE2_[NS]', grid_name):
        map_UL_x = -9000000.
        map_UL_y = 9000000.
    else:
        map_UL_x = -17367530.44
        map_UL_y = 6756820.20000

    scale = f.variables["crs"].scale_factor_at_projection_origin
    scale_x = scale
    scale_y = -1 * scale

    geotransform = (map_UL_x, scale_x, 0., map_UL_y, 0., scale_y)
    dest_ds.SetGeoTransform(geotransform)

    dest_ds.GetRasterBand(1).WriteArray((data + 0.5).astype(np.uint16))
    dest_ds = None

    f.close()

    sys.stderr.write("Wrote geotiff to " + outfilename + "\n")
