MATLAB utilities for the BYU-MERS "SIR" image format

The BYU-MERS "sir" image format was developed by the Brigham Young
University (BYU) Microwave Earth Remote Sensing (MERS) research group
to store images of the earth along with the information required to
earth-locate the image pixels.

Files in this directory are useful for reading SIR files into matlab
and locating pixels.  Use loadsir.m to load the file into memory.

Main routines:  (version 2.0 SIR format)

 loadsir.m            loads image and header information into matlab
 printsirhead.m       prints out the header information from loadsir
 pix2latlon.m         given a pixel location, compute lat and lon
 latlon2pix.m         compute pixel location given lat and lon
 sirheadtext.m        modifies text fields of sir header before write
 writesir.m           writes sir format file

Support routines:

 easegrid.m           forward EASE grid transformation
 ieasegrid.m          inverse EASE grid transformation
 ilambert1.m          inverse Lambert grid transformation
 ipolster.m           inverse polar stereographic grid transformation
 lambert1.m           forward Lambert grid transformation
 mod.m                compute modulo function
 polster.m            forward polar stereographic grid transformation

Other files:

 Contents.m           Matlab help file
 readme.txt           this file
 scaleimage.m         utility to scale image for display
 showsir.m            simplified display routine for sir image

If you have any questions, please contact me.

David

==============================================================================

Dr. David G. Long                                              long@ee.byu.edu
Associate Professor                                      voice: (801) 378-4383
Electrical and Computer Engineering Department             fax: (801) 378-6586
459 Clyde Building
Brigham Young University
Provo, Utah    84602

BYU Electrical and Computer Engineering home page:   http://www.ee.byu.edu/

BYU Microwave Earth Remote Sensing (MERS) Laboratory home page:
    http://www.ee.byu.edu/ee/mers/mers-home.html

==============================================================================

Last revised:  9 April 1997 DGL
(c) 1997 BYU MERS

