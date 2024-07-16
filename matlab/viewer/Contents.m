% MATLAB utilities to support CETB files
%
% various routines for loading, displaying, and computing
% EASE2 map transformations 
%
% CETB_disp_example.m   example of how to load and display CETB file
% CETB_latlon2pix.m     compute map projection for lat/lon to pixel location
% CETB_load.m           load arrays and projection information for CETB file
% CETB_pix2latlon.m     compute map projection for pixel location to lat/lon
% CETB_show.m           load and display TB images from CETB file
% CETB_viewer_engine.m  support routine for CETB_viewer.m
% CETB_viewer.m         simple interactive viewer for CETB file
% median_fill.m         median fill holes in an image
% pixel_double.m        double size of image by pixel replication
% plot_gshhs_ease2.m    plot coastlines and rivers on CETB EASE2 image
%
% Geometry support routines:
% ease2grid.m           EASE2grid map projection engine lat/lon to x,y
% ease2_map_info.m      part of EASE2grid map projection engine
% iease2grid.m          EASE2 grid map projection engine x,y to lat/lon
%