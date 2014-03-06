Documentation of the BYU-MERS "SIR" image format

The BYU-MERS "sir" image format was developed by the Brigham Young
University (BYU) Microwave Earth Remote Sensing (MERS) research group
to store images of the earth along with the information required to
earth-locate the image pixels.

A "sir" file consists of one or more 512 byte headers containing all the 
information required read the remainder of the file and the map projection
information required to map pixels to lat/lon on the Earth surface.  Pixel 
values are generally stored as 2 byte (high order byte first) integers
though can be stored as bytes or IEEE floating point.  The latter is not
portable to all machines and so is not recommended.  Scale factors
to convert the integer or byte pixel values to native floating point units 
are stored in the file header. The origin of the images are in the lower 
left corner. The earth locaion of a pixels is identified with its 
lower-left corner.

The standard sir format supports a variety of image projections including:

  0. Rectangular array (no projection)
  1. A rectangular lat/lon array
  2. Two different types of Lambert equal-area projections which can be 
     use in both non-polar and polar projections
  3. Polar stereographic projections
  4. EASE grid polar projection with various resolutions
  5. EASE global projection with various resolutions

In general, *.sir image data files have been generated using the
scatterometer image reconstruction (SIR) resolution 
enhancement algorithm or one of its variants for radiometer processing.

The multivariate SIR algorithm is a non-linear resolution enhancement
algorithm based on modified algebraic reconstruction and maximum
entropy techniques.  The algorithm is described in

Long, D.G., P.J. Hardin, and P.T. Whiting, "Resolution Enhancement of
Spaceborne Scatterometer Data," IEEE Trans. Geoscience Remote Sens.,
Vol. 31, No. 3, pp. 700-715, May 1993

The SIR w/filtering (SIRF) algorithm has been successfully applied to SASS 
and NSCAT measurements to study tropical vegetation and glacial ice.  Variants
of SIR has been successfully applied to ERS-1 scatterometer and
various radiometers (SSM/I and SMMR).

For scatterometers, the multivariate form of the SIR algorithm models
the dependence of sigma-0 on incidence angle as

   sigma-0 (in dB) = A + B * (Inc Ang - 40 deg)

over the incidence angle range of 15 to 60 deg.  The output of the SIR
algorithm is images of the A and B coefficients.

A represents the "incidence angle normalized sigma-0" (effectively the
sigma-0 value at 40 deg incidence angle).  The units of A are dB.
Typically,  +2 < A < -45 dB.  However, A is clipped to a minimum -32 dB.
Values of A < -32 are used to indicate no data as well

The B coefficient describes the incidence angle dependence of sigma-0 an
 has the units of dB/deg.  At Ku-band global average of B is approximately 
-0.13 dB/deg.  Typically, -0.2 < B < -0.1.  B is clipped to a minimum
value of -3 dB/deg.  This value is used to denote no data as well.  

Single variable SIR or SIRF algorithms are used for radiometers and
produce only an A (in this case, the brightness temperature) image.
Typically, this can range from 165 to 320.

Be sure to use binary ftp to transfer *.sir files!


Subdirectories:

 c/                   Code to read sir files using C.  Utilities to convert
                       SIR images to other file types (e.g., tiff, gif)

 f/                   Code to read sir files using fortran77.  Utilities to convert
                       SIR images to other file types (e.g., tiff, gif)

 idl/                 Code to load sir file images into IDL or PVWAVE,
                        save to file and do forward/inverse transforms in IDL.
			see idl/readme.txt for information.

 matlab/              Code to load sir fileimages into matlab, save file,
                        and to do forward/inverse transforms in matlab.
                        Also includes some display routines.
			see matlab/readme.txt for information.

Note: This code may be copied and modified so long as (1) original or
modified code is not redistributed for profit and (2) acknowledgement is
made that the original code was obtained courtesy of David G. Long at the 
Microwave Earth Remote Sensing Laboratory at Brigham Young University, 
Provo, UT.


If you have any questions, please contact me.

David Long

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
(c) 1999, 2014 BYU MERS



