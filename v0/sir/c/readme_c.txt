C routines for the BYU-MERS "SIR" image format


The BYU-MERS "sir" image format was developed by the Brigham Young
University (BYU) Microwave Earth Remote Sensing (MERS) research group
to store images of the earth along with the information required to
earth-locate the image pixels.

Note that these routines use the C-standard pixel address from 0 to N-1
rather than the SIR-standard pixel address from 1 to N.  Note, however, that
(1,1) is in the lower left of the image of a SIR File.  For an input location
of pixel (1,1) the Lat,Lon values returned by pixtolatlon correspond to 
the location of the lower-left corner of the (1,1)th pixel.

This directory contains several programs which illustrate reading and
writing BYU SIR files.  The programs include utilities to convert the
SIR files into other file formats.

Library routines are in lib/ and include sir_io.c which has the read/
write routines, sir_geom.c which contains the geometry routines, and
sir_ez.c which is an easy interface to the low-level routines in
the other two files.  The library interfaces are documented in the
include files include/sir3.h (for sir_io.c and sir_geom.c) and
include/sir_ez.h for the sir_ez.c routines.

Programs illustrating of how to call the various routines and to convert
SIR files to various other image types are in this directory.  Given
an input SIR file, the program csir_locmap creates SIR images of the
latitude and longitudue values of the SIR file.  sir_dump and sir_dump_small
create an ASCII file of the SIR image values and locations.

The tools/ directory contains additional example programs and utilities to 
land mask SIR files (sirmask), compute the difference of two SIR files 
(sirdiff) extract subimages to another SIR file (sir_extractregion), modify 
the heading of a SIR file (sir_modhead), and combine and remap SIR images into 
different SIR projections (sir_remapper and sir_remapper2).  Generally, it 
is recommended that header info be initialized from a previously read in \
sir file and modified as necessary.  

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

