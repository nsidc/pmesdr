IDL utilities for the BYU-MERS "SIR" image format

The BYU-MERS "sir" image format was developed by the Brigham Young
University (BYU) Microwave Earth Remote Sensing (MERS) research group
to store images of the earth along with the information required to
earth-locate the image pixels.

Files in this directory are useful for reading SIR files into IDL
and locating pixels.  The routine loadsir.pro reads the SIR files.
The central routines for pixel location are pixtolatlon and latlon2pix.
The other files are support routines.  A nice viewer routine (xsir) is
also provided.  The viewer includes full and zoomed views.  With
a mouse click a user can select a pixel and view its value and position.

Note that these routines use the SIR-standard pixel address from 1 to N
rather than than from 0 to N-1 as is typically done in IDL.  Note that
(1,1) is in the lower left of the image of a SIR File.  For an input location
of pixel (1,1) the Lat,Lon values returned by pixtolatlon correspond to 
the location of the lower-left corner of the (1,1)th pixel.

**********************************************************************

Fancy viewer routines:  (includes zoom, pixel info, etc)

xsir.pro                generic fancy viewer
xsir_idl.pro            fancy viewer tuned for IDL (has extra featuers)
xsir_pvwave.pro         fancy viewer tuned for PV-WAVE


Geometry routines:

 easegrid.pro		forward EASE grid transformation
 ieasegrid.pro		inverse EASE grid transformation
 ilambert1.pro		inverse Lambert equal area transformation
 ipolster.pro		inverse polar stereographic transformion
 lambert1.pro		forward Lambert equal area transformation
 latlon2pix.pro		convert (lon,lat) to (x,y) pixels
 pixtolatlon.pro	convert (x,y) pixels to (lon,lat)
 polster.pro		foward polar stereographic transformion

SIR file read/write routines:

 writesir.pro           write image to sir file format
 loadsir.pro            load sir file image into IDL or PVWAVE

Using the basic read/write routines:

Use 'loadsir' to read sir format files.  The command format is:

loadsir,filename,outarray,info,printflag,sensor,type,title,tag,cproc,ctime,des,iaopt

where
  filename  = name to read file from (in)
  outarray  = image array (out)
  info      = 36 element array containing summary header information (out)
  printflag = 1 to print header information to console, 0 to not print (in)
  sensor    = string containing name of sensor
  type      = string containing data type
  title     = string containing image title
  tag       = string containing image tag line
  crproc    = string containing description of creation program
  crtime    = string containing description of creation time
  des       = optional extra header description string 
	      (set to blank if not present in SIR file header)
  iaopt     = optional extra header integer array
	      (a single scalar if not present in SIR file header)

Arguments after info are optional.

Header information required for the transformations are stored in
the info array.  To determine the lon,lat position for pixel
i,j use 'pixtolatlon,lon,lat,i,j,info'.  Note that i,j can be real
numbers and be located for non-existent pixels such as (-5,-100)
though this can lead to errors in the inverse transformations.  To
get a pixel location from lon,lat use 'latlon2pix,lon,lat,x,y,info'.
To get integer pixel locations, quantize x and y to 1..Nx and 1..Ny
respectively.  No check of the point being within the image is made.

A quick way to view the retrieved image is to execute the following from the IDL prompt:

 window,xsize=info(0),ysize=info(1)
 tv,bytscl(out,min=info(17),max=info(18))

A more sophisticated tool is xsir_idl.pro or xsir_pvwave.pro

To write a sir format file use:

writesir,filename,imarray,info,sensor,type,title,tag,cproc,ctime,autoset,des,iaopt

where (all inputs)
  filename  = name to read file from
  imarray   = image array (float)
  info      = 36 element array containing header information
  sensor    = string containing name of sensor
  type      = string containing data type
  title     = string containing image title
  tag       = string containing image tag line
  crproc    = string containing description of creation program
  crtime    = string containing description of creation time
  autoset   = 0 to use scale parameters in info, 1 to set scales automatically
  des       = optional extra header description string
  iaopt     = optional extra header integer array

It is recommended that info be initialized from a previously read in sir
file and modified as necessary.


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






