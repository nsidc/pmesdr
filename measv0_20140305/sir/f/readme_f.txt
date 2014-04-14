Fortran 77 routines for the BYU-MERS "SIR" image format


The BYU-MERS "sir" image format was developed by the Brigham Young
University (BYU) Microwave Earth Remote Sensing (MERS) research group
to store images of the earth along with the information required to
earth-locate the image pixels.

Note that these routines use the SIR-standard pixel address from 1 to N.  
(1,1) is in the lower left of the image of a SIR File.  For an input location
of pixel (1,1) the Lat,Lon values returned by pixtolatlon correspond to 
the location of the lower-left corner of the (1,1)th pixel.

This directory contains several programs which illustrate reading and
writing BYU SIR files.  The programs include utilities to convert the
SIR files into other file formats and to compute SIR "images" of the
lat/lon of each pixel in an other SIR image.  

The program sir_extractregion creates SIR files consisting of subregions of another SIR file.  The program fsir_dump converts a SIR file to an ASCII
file with pixel values and locations listed.  Given a SIR file, the program
fsir_locmap creates two SIR files which contain pixel values which are the 
latitude and longitudes of the pixels.

Library routines are in lib/ and include a platform-dependent file
SIR.inc which should be edited according to platform-specific byte
order and file reading.

As supplied, the sir_extractregion program uses command line arguments
via iargc and getarg.  Not all compilers and operating systems support
these routines.  These calls can be commented out if desired.

The SIREZ.f/SIREZ.inc files contain interface routines which use the
fortran77 extension 'record' and 'structure' to provide a simple interface
to the other routines.  This code is strictly optional.  Fortran files
with _EZ in their name use this code.  Some compilers do not accept
the fortran extensions used in this code.

Illustrations of how to call the various routines are in this directory
and use the library routines.

Generally, it is recommended that header info be initialized from a 
previously read in sir file and modified as necessary.

If you have any questions, please contact me.

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



