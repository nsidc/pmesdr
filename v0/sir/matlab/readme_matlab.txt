MATLAB utilities for the BYU-MERS "SIR" image format

The BYU-MERS "sir" image format was developed by the Brigham Young
University (BYU) Microwave Earth Remote Sensing (MERS) research group
as part of the NASA Scatterometer Climate Record Pathfinder (SCP) project
to store remotely sensed images  of the earth used for climate studies,
among other purposes.  The file format stores an image array of floating
point values along with the information required to earth-locate the 
image pixels.  Additional information on the applications, file format,
and available data can be obtained from the NASA-sponsored Scatterometer
Climate Record Pathfinder web site http://www.scp.byu.edu/ and/or the
NASA/JPL PO.DAAC http://podaac.jpl.nasa.gov/

Files in this directory are useful for reading SIR files into matlab
and locating pixels.  Use loadsir.m to load the file into memory.  The
routine pix2latlon.m computes the lat/lon position given a list of
x,y pixel locations.  latlong2pix does the reverse.  showsir and viewsir
are utilities to display SIR images.

Note that these routines use the SIR-standard pixel address from 1 to N.
Note that (1,1) is in the lower left of the image of a SIR File.  For an 
input location of pixel (1,1) the Lat,Lon values returned by pix2latlon
correspond2 to the location of the lower-left corner of the (1,1)th pixel.
For convenience, matlab-reported values are "flipped" vertically to
the matlab convention.

The matlab routine viewsir.m provides a fancy window utility to read
a SIR file and display to screen.  The utility includes zoom and point/click
to get value and location.

Main routines:  (version 3.0 SIR format)

 loadsir.m            loads image and header information into matlab
 loadpartsir.m        loads part of image and header information into matlab
 printsirhead.m       prints out the header information from loadsir
 pix2latlon.m         given a pixel location, compute lat and lon
 latlon2pix.m         compute pixel location given lat and lon
 sirheadtext.m        modifies text fields of sir header before write
 writesir.m           writes sir format file

Geometry and Support routines:

 easegrid.m           forward EASE grid transformation
 ieasegrid.m          inverse EASE grid transformation
 ilambert1.m          inverse Lambert grid transformation
 ipolster.m           inverse polar stereographic grid transformation
 lambert1.m           forward Lambert grid transformation
 mod.m                compute modulo function
 polster.m            forward polar stereographic grid transformation

Other files:

 Contents.m           Matlab help file
 readme_matlab.txt    this file
 scaleimage.m         utility to scale image for display
 showsir.m            simplified display routine for sir image
 viewsir.m            function interface for viewsir1.m to display SIR image
 viewsir1.m           called by viewsir
 sircolorbar.m        callled by viewsir1
 sir_locate.m         interactively select a point on image and print location
 setsirhead.m         set particular values in sir header by name
 sirheadvalue.m       get particular values in sir header by name
 sirheadtext.m        set text values in sir header
 display_head.m       help routine for sirheadvalue

Note: when using writesir it is recommended that the header from an existing
SIR file be used at a template and only the changed fields modified. 

If you have any questions, please contact the SCP.

==============================================================================

Dr. David G. Long                                     long@ee.byu.edu
Director, BYU Center for Remote Sensing              www.cers.byu.edu   
Professor, Electrical and Computer Eng. Dept.          www.ee.byu.edu
Brigham Young University                                  www.byu.edu
459 Clyde Building                                voice: 801-422-4383
Provo, Utah    84602                                fax: 801-422-0201

Scatterometer Climate Record Pathfinder:              www.scp.byu.edu

==============================================================================

Code is SIR header version 3.0 compliant.

Last revised:  26 Feb 2014  DGL
(c) 1999, 2000, 2002, 2014 BYU MERS





