Software readers and utilities for the BYU-MERS "SIR" image format
in various languages.

The BYU-MERS "sir" image format was developed by the Brigham Young
University (BYU) Microwave Earth Remote Sensing (MERS) research group
to store images of the earth along with the information required to
earth-locate the image pixels.

Note that in standard BYU sir files the pixel addresses run from 1 
to N rather than than from 0 to N-1.  Pixel (1,1) is in the lower 
left of the image.  For an input (1,1) the Lat,Lon values for 
correspond to the lower-left corner of the (1,1) pixel.

In a standard BYU sir file, the file header (first 512 bytes)
store meta information about the file, including the projection
and transformation information.  The readers in the subdirectories
are designed to implement pixel to lat/lon and the reverse as
well as read and write sir files.

If you have any questions, please contact me.


==============================================================================

Dr. David G. Long                                              long@ee.byu.edu
Professor                                                voice: (801) 422-4383
Electrical and Computer Engineering Department             fax: (801) 422-0201
459 Clyde Building
Brigham Young University
Provo, Utah    84602

BYU Electrical and Computer Engineering home page:           www.ee.byu.edu
BYU Microwave Earth Remote Sensing (MERS) Lab home page:   www.mers.byu.edu

==============================================================================

Last revised:  26 Feb 2014  DGL
(c) 2014 BYU MERS

