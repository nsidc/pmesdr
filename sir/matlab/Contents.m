% MATLAB utilities for the BYU-MERS "SIR" image format
%
% The BYU-MERS "sir" image format was developed by the Brigham Young
% University (BYU) Microwave Earth Remote Sensing (MERS) research group
% to store images of the earth along with the information required to
% earth-locate the image pixels.
%
% Files in this directory are useful for reading SIR files into matlab
% and locating pixels.  Use loadsir.m to load the file into memory.
%
% Main routines:  (version 3.0 SIR format)
%
% loadsir.m            loads image and header information into matlab
% loadpartsir.m        loads part of/all image and header into matlab
% printsirhead.m       prints out the header information from loadsir
% pix2latlon.m         given a pixel location, compute lat and lon
% latlon2pix.m         compute pixel location given lat and lon
% sirlex.m             compute image array index given pixel location
% isirlex.m            compute pixel location give image array index
% sirheadtext.m        modifies text fields of sir header before write
% writesir.m           writes sir format file
%
% Geometry support routines:
%
% easegrid.m           forward EASE grid transformation
% ieasegrid.m          inverse EASE grid transformation
% ilambert1.m          inverse Lambert grid transformation
% ipolster.m           inverse polar stereographic grid transformation
% lambert1.m           forward Lambert grid transformation
% mod.m                compute modulo function
% polster.m            forward polar stereographic grid transformation
%
% Fancy view utility
% viewsir.m            loads and displays image with interactive values
% viewsir1.m           (working routine for viewsir, users should call viewsir)
%
% Display support routines:
%
% scaleimage.m         utility to scale image for display
% showimage.m          simplified display of an image
% showsir.m            display a sir image
% sircolorbar.m        display a color bar
%
% Head manipulation routine:
%
% setsirhead.m         set particular values in sir header by name
% sirheadvalue.m       get particular values in sir header by name
% sirheadtext.m        set text values in sir header
% display_head.m       help routine for sirheadvalue
%

% copyright 2002, BYU MERS


