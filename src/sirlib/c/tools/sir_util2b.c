/*
   Utility program for working with BYU .SIR format files

   Print file header and map transformation information.
   Optionally, extracts a sub region, writes out data to text, csv,
   bmp, gif, tiff, or geotiff file.  Compute projection 
   transformations and write out image corner locations.

   Written by DGL 11 Apr 2009
   Revised by DGL 16 Apr 2009 + include option printing of proj4
                                  and gdal_translate parameters
   Revised by DGL 17 Feb 2011 + added tif and geotiff capability, color tables, no data scaling
   Revised by DGL 03 Oct 2012 + fixed bug in polar stereographic case
   Revised by DGL 22 Mar 2014 + added EASE2
 
   should be linked with 
    sir_ez.c sir_geom.c sir_io.c
   uses
    sir_ez.h sir3.h

   note: to support so many different platforms the code ended up not very elegant

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define _CRT_SECURE_NO_WARNINGS  /* for windows */

#include "sir_ez.h"  /* get easy sir routine interface */

/* menu function definitions */
void print_menu(FILE *out);
void single_pixel_value(float *stval, sir_head *head);
void geographic_to_pixel(sir_head *head);
void pixel_to_geographic(sir_head *head);
void dump_values(float *stval, sir_head *head);
void dump_locs(sir_head *head);
void convert_image(float *stval, sir_head *head, int ColorMap_flag, unsigned char *rtab, unsigned char *gtab, unsigned char *btab);
void write_locs(sir_head *head);
void subregion(float *stval, sir_head *head);
void corners(sir_head *head);
void projection_info(sir_head *head);
void read_colormap(int *ColorMap_flag, unsigned char *rtab, unsigned char *gtab, unsigned char *btab);
void edit_colortable(int *ColorMap_flag, unsigned char *rtab, unsigned char *gtab, unsigned char *btab);

const short isle_var=1;         /* for endian test */
#define ISLE *(char*)&isle_var  /* for endian test */

/* main code */
int main(int argc, char **argv)
{
  char  *in, infname[250];
  int endflag=1, option, ierr;
  float *stval;        /* pointer to input image storage */
  sir_head head;       /* sir_ez defined structure to store SIR header info */
  FILE *fid;  
  int i,j;
  int ColorMap_flag=0; /* no color table */
  unsigned char rtab[256], gtab[256], btab[256];

  /* default greyscale color table for bmp file*/
  for (i=0; i<256; i++) {
    rtab[i]=i;
    gtab[i]=rtab[i];
    btab[i]=rtab[i];
  }
  
  fprintf(stdout,"BYU SIR utility program\n\n") ;
  if (argc < 2) {
    fprintf(stdout,"\nusage: %s input_file \n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file    =  input SIR file\n");

    fprintf(stdout,"\nEnter input file name: ");
    fscanf(stdin,"%s",infname);
    in=infname;
    
  } else
    in=argv[1];

  /* check endianness and proper definition of SWAP in sir3.h */
  if (ISLE) 
#ifdef SWAP
    {
      fprintf(stdout,"*** error: This is a little-endian machine and SWAP is defined\n");
      fprintf(stdout,"*** error: sir3.h will need to be changed for code to run properly.\n");
      return(-1);
    }
#else
    ;
#endif
  else 
#ifdef SWAP
    ;
#else
    {
      fprintf(stdout,"*** error: This is a big-endian machine and SWAP is not defined\n");
      fprintf(stdout,"*** error: sir3.h will need to be changed for code to run properly.\n");
      return(-1);
    }
#endif

  fprintf(stdout,"Reading file %s\n",in);

  /* check for existence of file */
  fid=fopen(in,"rb");
  if (fid==NULL) {
     fprintf(stdout,"*** File not found '%s'\n",in);
     exit(-1);
  }
  fclose(fid);
  
  /* initialize sir_ez structure */
  sir_init_head(&head);
  
  /* read in input SIR file */
  ierr = get_sir(in, &head, &stval);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR header from file '%s'\n",in);
     exit(-1);
  }
  /* for compatibility with old versions, update header */
  sir_update_head(&head, stval);

  /* print header information */
  fprintf(stdout,"\nSIR file: %s\n",in);
  fprintf(stdout,"size: %d X %d\n",head.nsx, head.nsy);

  /* print program operation menu */
  while (endflag) {
    (void) print_menu(stdout);
    fprintf(stdout,"Select menu option: ");
    fscanf(stdin,"%d",&option);
    
    switch(option) {
    case 1:
      fprintf(stdout,"\n");      
      (void) print_sir_head(stdout, &head);
      break;
    case 2:
      (void) single_pixel_value(stval, &head);
      break;
    case 3:
      (void) geographic_to_pixel(&head);
      break;
    case 4:
      (void) pixel_to_geographic(&head);
      break;
    case 5:
      (void) dump_values(stval, &head);
      break;
    case 6:
      (void) convert_image(stval, &head, ColorMap_flag, rtab, gtab, btab);
      break;
    case 7:
      (void) dump_locs(&head);
      break;
    case 8:
      (void) write_locs(&head);
      break; 
    case 9:
      (void) subregion(stval, &head);
      break;
    case 10:
      (void) corners(&head);
      break;
    case 11:
      (void) projection_info(&head);
      break;
    case 12:
      (void) read_colormap(&ColorMap_flag,rtab,gtab,btab);
      break;
    case 13:
      (void) edit_colortable(&ColorMap_flag,rtab,gtab,btab);
      break;
      
    default:
      endflag=0;
      break;
    }
  }
  return(0);
}

void print_menu(FILE *out) {
  fprintf(out,"\nBYU sir_util program options\n");
  fprintf(out," 0 : Quit\n");
  fprintf(out," 1 : Print SIR file header info\n");
  fprintf(out," 2 : Print out single pixel value\n");
  fprintf(out," 3 : Compute x,y pixel locations from lat,lon\n");
  fprintf(out," 4 : Compute lat,lon from x,y pixel locations\n");
  fprintf(out," 5 : Dump data values to text file\n");
  fprintf(out," 6 : Convert to image file (BMP, GIF, TIFF or geoTIFF)\n");
  fprintf(out," 7 : Write pixel lat/lon positions to text files\n");
  fprintf(out," 8 : Write pixel lat/lon positions to SIR-format files\n");
  fprintf(out," 9 : Create subregion\n");
  fprintf(out,"10 : Print corner locations\n");
  fprintf(out,"11 : Binary file and map projection info\n");
  fprintf(out,"12 : Read a color table file (to use with convert to image file)\n");
  fprintf(out,"13 : Edit color table\n");
  return; 
}

void f2ipix1(float x, float y, int *ix, int *iy, int nsx, int nsy)
{
   /* quantizes the floating point pixel location to the actual pixel value
      returns a zero if location is outside of image limits
      a small amount (0.002 pixels) of rounding is permitted*/

  /* printf("f2ipix1: %f %f %d %d\n",x,y,nsx,nsy); */
  if (x+0.0002 >= 1.0 && x+0.0002 <= (float) (nsx+1))
    *ix = floor(x+0.0002);
  else
    *ix = 0;

  if (y+0.0002 >= 1.0 && y+0.0002 <= (float) (nsy+1))
    *iy = floor(y+0.0002);
  else
    *iy = 0;

  return;
}

void single_pixel_value(float *stval, sir_head *head){
  float x=1.0, y=1.0;
  int ix, iy, i;
  
  fprintf(stdout,"\nSIR file origin (1,1) is at the lower left corner of the lower left pixel\n");
  fprintf(stdout,"Enter horizontal (x) pixel location: [1..%d] ",head->nsx);
  fscanf(stdin,"%f",&x);
  fprintf(stdout,"Enter vertical (y) pixel location: [1..%d] ",head->nsy);
  fscanf(stdin,"%f",&y);

  f2ipix1(x, y, &ix, &iy, head->nsx, head->nsy);
  fprintf(stdout,"\nInput x,y: %f,%f  Quantized to: ix,iy: %d,%d\n",x,y,ix,iy);
  if (ix==0 || iy==0)
    fprintf(stdout," * location is outside of image boundaries\n");
  else {
    i = sir_lex(ix, iy, head);
    fprintf(stdout,"Pixel (%d) value: %f\n",i, stval[i]);
  }
  return;
}

void geographic_to_pixel(sir_head *head){
  float x,y,alon,alat;
  int ix,iy;
    
  fprintf(stdout,"\nGeographics location to pixel location\n");
  fprintf(stdout,"Enter longitude (deg): ");
  fscanf(stdin,"%f",&alon);
  fprintf(stdout,"Enter latitude (deg):  ");
  fscanf(stdin,"%f",&alat);

  (void) sir_latlon2pix(alon, alat, &x, &y, head); /* (lat,lon) -> (x,y) */
  f2ipix1(x, y, &ix, &iy, head->nsx, head->nsy);
  
  fprintf(stdout,"\nInput lon=%f, lat=%f  x=%f (%d), y=%f (%d)\n",alon,alat,x,ix,y,iy);
  fprintf(stdout," (0) indicates point is outside of image area)\n");
  return;
}

void pixel_to_geographic(sir_head *head){
  float x,y,alon,alat;
    
  fprintf(stdout,"\nSIR file origin (1,1) is at the lower left corner of the lower left pixel\n");
  fprintf(stdout," (fractional pixels and values outside of the image may be specified.)\n");
  fprintf(stdout,"Enter horizontal (x) pixel location: [1..%d] ",head->nsx);
  fscanf(stdin,"%f",&x);
  fprintf(stdout,"Enter vertical (y) pixel location: [1..%d] ",head->nsy);
  fscanf(stdin,"%f",&y);
  
  sir_pix2latlon(x, y, &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
  fprintf(stdout,"Input pixel coords: x=%f, y=%f  long %f,  lat %f\n",x,y,alon,alat);
  return;
}

void corners(sir_head *head){
  int x,y;  
  float alon,alat;
  double Erad=6378.135, F=298.26, era=(1.-1./F), dtr=0.01745329252, E2=0.006693883;  
  
  print_sir_head(stdout,head);
  
  /* add Earth radius used for special cases*/
  switch(head->iopt) {

  case 0:
    break;

  case 2:
     Erad=Erad*era/sqrt(pow(era*cos((double) head->ydeg * dtr),2.0)+pow(sin(dtr * (double) head->ydeg),2.0));
  case 1:
     if (head->iopt==1) Erad=6378.0;     
     fprintf(stdout,"  Spherical Earth radius used=%f (km)\n",Erad);
     break;

  case 5:
     Erad=6378.273;     
     fprintf(stdout,"  Elliptical Earth radius=%f (km) e2=%11.9f\n",Erad,E2);
     break;

  case 8:  /* EASE 1 and 2 grids */
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
    break;

  default:
    fprintf(stdout,"\nImage size: (%d X %d)\n",head->nsx,head->nsy);
    break;
  }

  fprintf(stdout,"\nImage corner locations (location at lower-left of pixel)\n");

  x=1; y=1;  
  sir_pix2latlon((float) x, (float) y, &alon, &alat, head);    /* (x,y) -> (lat,lon) */
  fprintf(stdout," LL: (x=%d, y=%d)  lon=%f lat=%f\n",x,y,alon,alat);

  x=1; y=head->nsy;  
  sir_pix2latlon((float) x, (float) y, &alon, &alat, head);    /* (x,y) -> (lat,lon) */
  fprintf(stdout," UL: (x=%d, y=%d)  lon=%f lat=%f\n",x,y,alon,alat);

  x=head->nsx; y=head->nsy;
  sir_pix2latlon((float) x, (float) y, &alon, &alat, head);    /* (x,y) -> (lat,lon) */
  fprintf(stdout," UR: (x=%d, y=%d)  lon=%f lat=%f\n",x,y,alon,alat);

  x=head->nsx; y=1;
  sir_pix2latlon((float) x, (float) y, &alon, &alat, head);    /* (x,y) -> (lat,lon) */
  fprintf(stdout," LR: (x=%d, y=%d)  lon=%f lat=%f\n",x,y,alon,alat);

  fprintf(stdout,"\nImage corner locations (location at center of pixel)\n");

  x=1; y=1;  
  sir_pix2latlon(0.5+(float) x, 0.5+(float) y, &alon, &alat, head);    /* (x,y) -> (lat,lon) */
  fprintf(stdout," LL: (x=%d, y=%d)  lon=%f lat=%f\n",x,y,alon,alat);

  x=1; y=head->nsy;  
  sir_pix2latlon(0.5+(float) x, 0.5+(float) y, &alon, &alat, head);    /* (x,y) -> (lat,lon) */
  fprintf(stdout," UL: (x=%d, y=%d)  lon=%f lat=%f\n",x,y,alon,alat);

  x=head->nsx; y=head->nsy;
  sir_pix2latlon(0.5+(float) x, 0.5+(float) y, &alon, &alat, head);    /* (x,y) -> (lat,lon) */
  fprintf(stdout," UR: (x=%d, y=%d)  lon=%f lat=%f\n",x,y,alon,alat);

  x=head->nsx; y=1;
  sir_pix2latlon(0.5+(float) x, 0.5+(float) y, &alon, &alat, head);    /* (x,y) -> (lat,lon) */
  fprintf(stdout," LR: (x=%d, y=%d)  lon=%f lat=%f\n",x,y,alon,alat);

  return;
}


void projection_info(sir_head *head){
  int x,y;  
  float alon,alat;
  double Erad=6378.135, F=298.26, era=(1.-1./F), dtr=0.01745329252, E2=0.006693883;
  float x0,y0,x1,y1,lat0=90.0;  
  float fmap_scale, fr0, fs0;
  int bcols, brows;  
  
  fprintf(stdout,"\nBinary SIR file has a %d byte header\n",head->nhead*512);
  if (head->nhead>1)
    fprintf(stdout," which includes the optional information header blocks (%d=char values, %d=int values)\n",head->ldes*512,head->maxi);

  switch(head->idatatype) {
  case 1:
    fprintf(stdout," Binary values are stored as bytes (signed char)\n");
    fprintf(stdout," Conversion to floating point: FP = (float) (byte value) / %d.0 + %d.0)\n",head->iscale,head->ioff);
    break;
    
  case 2:
    fprintf(stdout," Binary values are stored as 2-byte integers (signed short)\n");
    fprintf(stdout," Conversion to floating point: FP = (float) (short value) / %d.0 + %d.0)\n",head->iscale,head->ioff);
    break;

  case 4:
    fprintf(stdout," Binary values are 4-byte floating point numbers (float) (no conversion necessary)\n");
  }
  fprintf(stdout," Pixel values are stored in left-to-right, bottom-to-top order\n");
  fprintf(stdout," For pixels numbered x=1..%d, y=1..%d the\n  pixel index (1..%d) is computed as index=(x-1)*%d+y\n",head->nsx,head->nsy,head->nsx*head->nsy,head->nsx);
  fprintf(stdout," For pixels numbered x=0..%d, y=0..%d the\n  pixel index (0..%d) is computed as index=x*%d+y\n",head->nsx-1,head->nsy-1,head->nsx*head->nsy-1,head->nsx);

  switch(head->iopt) {
  case -1:
     fprintf(stdout,"\nImage-only file -- no map projection information\n");
     fprintf(stdout,"  Image size: (%d X %d)\n",head->nsx,head->nsy);
     return;
     
  case 0:
     fprintf(stdout,"\nRectangular Lat/Lon projection:\n");
     fprintf(stdout,"  Image size: (%d X %d)\n",head->nsx,head->nsy);
     fprintf(stdout,"  Span:                x=%f ,   y=%f (deg)\n",head->xdeg,head->ydeg);
     fprintf(stdout,"  Scale:               x=%f ,   y=%f (pix/deg)\n",head->ascale,head->bscale);
     fprintf(stdout,"  Lower-left Corner: lon=%f , lat=%f (deg)\n",head->a0,head->b0);
     return;

  case 2:
     Erad=Erad*era/sqrt(pow(era*cos((double) head->ydeg * dtr),2.0)+pow(sin(dtr * (double) head->ydeg),2.0));
  case 1:
     if (head->iopt==1) Erad=6378.0;     

     x0=head->a0;
     x1=head->a0+(head->nsx-1)/head->ascale;
     y0=head->b0;
     y1=head->b0+(head->nsy-1)/head->bscale;

     fprintf(stdout,"\nLambert Equal Area Azimuth projection:\n");
     fprintf(stdout,"  Image size: (%d X %d)\n",head->nsx,head->nsy);
     fprintf(stdout,"  Spherical Earth radius used=%f (km)\n",Erad);
     fprintf(stdout,"  Center point:      lon=%f , lat=%f (deg)\n",head->xdeg,head->ydeg);
     fprintf(stdout,"  Scale factor:      lon=%f , lat=%f (km/pix)\n",1./head->ascale,1./head->bscale);
     fprintf(stdout,"  Lower-Left Corner:   x=%f ,   y=%f (km)\n",head->a0,head->b0);

     fprintf(stdout,"\nRecommended proj4 src string:\n '+a=%.3f +rf=50000000 +proj=laea +lat_0=%f +lon_0=%f'\n",Erad*1000,head->ydeg,head->xdeg);

     /* technically, the following is the proper line for the proj/invproj string but +f=0 is
	not supported by gdal_translate and so is not recommended */
     /* fprintf(stdout,"or proj/invproj src string: '+a=%.3f +f=0 +proj=laea +lat_0=%f +lon_0=%f'\n",Erad*1000,head->ydeg,head->xdeg); */
     fprintf(stdout,"\nGDAL conversion method: make a .bmp or .gif image, use gdal_translate\n");
     fprintf(stdout,"gdal_translate -a_srs ""+a=%.3f +rf=50000000 +proj=laea +lat_0=%f +lon_0=%f"" -a_ullr %f %f %f %f -of Gtiff (input gif or bmp file name) (output geotiff file name)\n",Erad*1000,head->ydeg,head->xdeg,x0*1000,y1*1000,x1*1000,y0*1000);
     return;

   case 5:
     Erad=6378.273;

     x0=head->a0;
     x1=head->a0+(head->nsx-1)*head->ascale;
     y0=head->b0;
     y1=head->b0+(head->nsy-1)*head->bscale;

     fprintf(stdout,"\nPolar Sterographic projection:\n");
     fprintf(stdout,"  Image size: (%d X %d)\n",head->nsx,head->nsy);
     fprintf(stdout,"  Elliptical Earth radius=%f (km) e2=%11.9f\n",Erad,E2);
     fprintf(stdout,"  Reference point: lon=%f , lat=%f (deg)\n",head->xdeg,head->ydeg);
     fprintf(stdout,"  X,Y scales:      lon=%f , lat=%f (km/pix)\n",head->ascale,head->bscale);
     fprintf(stdout,"  Lower-Left Corner: x=%f ,   y=%f (km/pix)\n",head->a0,head->b0);

     fprintf(stdout,"\nRecommended proj4 src string:\n '+a=wgs84 +proj=stere +lat_0=%f +lon_0=%f'\n",head->ydeg,head->xdeg);
     /* technically, should use +wgs72 but this is not supported by gdal_translate 
        and so is not recommended.  Error is relatively small for typical .SIR file pixel sizes. */
     fprintf(stdout,"\nGDAL conversion method: make a .bmp or .gif image, use gdal_translate\n");
     /*fprintf(stdout,"gdal_translate -a_srs \"+datum=wgs84 +proj=stere +lat_0=%f +lon_0=%f\" -a_ullr %f %f %f %f -of Gtiff (input gif or bmp file name) (output geotiff file name)\n",head->ydeg,head->xdeg,x0*1000,y1*1000,x1*1000,y0*1000);*/
     lat0 = 90.0;
     if (head->ydeg < 0.0)
       lat0 = -90.0;
     fprintf(stdout,"gdal_translate -a_srs \"+datum=wgs84 +proj=stere +lat_0=%f +lat_ts=%f +lon_0=%f +k=1 +x_0=0 +y_0=0 +a=6378273.000 +b=6356889.449 +units=m +no_defs\" -a_ullr %f %f %f %f -of Gtiff (input gif or bmp file name) (output geotiff file name)\n",lat0,head->ydeg,head->xdeg,x0*1000,y1*1000,x1*1000,y0*1000);
     return;

   case 8:
   case 9:
     ease2sf(head->iopt, head->ascale, head->bscale, 
	     &fmap_scale, &bcols, &brows, &fr0, &fs0);
     x0=-fmap_scale*fs0;     
     x1=x0+bcols*fmap_scale;
     y0=-fmap_scale*fr0;     
     y1=y0+brows*fmap_scale;

     fprintf(stdout,"\nEASE2 Polar Azimuthal projection:\n");
     fprintf(stdout,"  Image size: (%d X %d)\n",head->nsx,head->nsy);
     if (head->iopt==8)
       fprintf(stdout,"  Reference Latitude:   %f (deg)\n",90.0);
     else
       fprintf(stdout,"  Reference Latitude:   %f (deg)\n",-90.0);
     fprintf(stdout,"  Map center (col,row): %f , %f\n",head->nsy*0.5,head->nsx*0.5);
     fprintf(stdout,"  Map base scale, index: %.1f , %.1f, %f\n",head->ascale,head->bscale, fmap_scale);
     fprintf(stdout,"  Map origin (col,row): %f , %f\n",x0,y0);     

     if (head->iopt == 8) {	 
       fprintf(stdout,"\nRecommended proj4 src string:\n '+proj=laea +lat_0=90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m'\n");
       fprintf(stdout,"\nRecommended Geotiff conversion method: make a .bmp or .gif image, use gdal_translate\n");
       fprintf(stdout,"gdal_translate -a_srs \"+datum=wgs84 +proj=laea +lat_0=90 +lon_0=0\" -a_ullr %f %f %f %f -of Gtiff (input gif or bmp file name) (output geotiff file name)\n",x0*1000,y1*1000,x1*1000,y0*1000);
     } else {
       fprintf(stdout,"\nRecommended proj4 src string:\n '+proj=laea +lat_0=-90 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m'\n");
       fprintf(stdout,"\nRecommended Geotiff conversion method: make a .bmp or .gif image, use gdal_translate\n");
       fprintf(stdout,"gdal_translate -a_srs \"+datum=wgs84 +proj=laea +lat_0=-90 +lon_0=0\" -a_ullr %f %f %f %f -of Gtiff (input gif or bmp file name) (output geotiff file name)\n",x0*1000,y1*1000,x1*1000,y0*1000);
     }
     return;

   case 10:
     ease2sf(head->iopt, head->ascale, head->bscale, 
	     &fmap_scale, &bcols, &brows, &fr0, &fs0);
     x0=-fmap_scale*fs0;     
     x1=x0+bcols*fmap_scale;
     y0=-fmap_scale*fr0;     
     y1=y0+brows*fmap_scale;

     fprintf(stdout,"\nEASE2 Cylindrical projection:\n");
     fprintf(stdout,"  Image size: (%d X %d)\n",head->nsx,head->nsy);
     fprintf(stdout,"  Reference Latitude:   %f (deg)\n",30.0);
     fprintf(stdout,"  Map center (col,row): %f , %f\n",head->nsy*0.5,head->nsx*0.5);
     fprintf(stdout,"  Map base scale, index: %.1f , %.1f, %f\n",head->ascale,head->bscale, fmap_scale);
     fprintf(stdout,"  Map origin (col,row): %f , %f\n",x0,y0);     

     fprintf(stdout,"\nRecommended proj4 src string:\n '+proj=cea +lat_0=0 +lon_0=0 +lat_1=30 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m'\n");
     fprintf(stdout,"\nRecommended Geotiff conversion method: make a .bmp or .gif image, use gdal_translate\n");
     fprintf(stdout,"gdal_translate -a_srs \"+datum=wgs84 +proj=cea +lat_0=0 +lon_0=0 +lat_1=30\" -a_ullr %f %f %f %f -of Gtiff (input gif or bmp file name) (output geotiff file name)\n",x0*1000,y1*1000,x1*1000,y0*1000);
     return;

   case 11:
   case 12:
     fprintf(stdout,"\nEASE1 Polar Azimuthal projection:\n");
     fprintf(stdout,"  Image size: (%d X %d)\n",head->nsx,head->nsy);
     if (head->iopt==11)
       fprintf(stdout,"  Reference Latitude:   %f (deg)\n",45.0);
     else
       fprintf(stdout,"  Reference Latitude:   %f (deg)\n",-45.0);
     fprintf(stdout,"  Map center (col,row): %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"  A,B scales:           %f , %f\n",head->ascale,head->bscale);
     fprintf(stdout,"  Map origin (col,row): %f , %f\n",head->a0,head->b0);
     fprintf(stdout,"\nproj and gdal_translate do not support this projection\n");
     
     return;

   case 13:
     fprintf(stdout,"\nEASE1 Cylindrical projection:\n");
     fprintf(stdout,"  Image size: (%d X %d)\n",head->nsx,head->nsy);
     fprintf(stdout,"  Reference Latitude:   %f (deg)\n",30.0);
     fprintf(stdout,"  Map center (col,row): %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"  A,B scales:           %f , %f\n",head->ascale,head->bscale);
     fprintf(stdout,"  Map origin (col,row): %f , %f\n",head->a0,head->b0);
     fprintf(stdout,"\nproj and gdal_translate do not support this projection\n");
     return;

   default:
     fprintf(stdout,"\nUnknown projection:\n");
     fprintf(stdout,"\nImage size: (%d X %d)\n",head->nsx,head->nsy);
     fprintf(stdout,"\nproj and gdal_translate do not support this projection\n");
     return;
  }

  return;
}


void dump_values(float *data, sir_head *head){
  char outfname[250];
  int option=0, order=0, i, j, nsx=head->nsx, nsy=head->nsy;
  char ch=' ';
  FILE *fid;

  fprintf(stdout,"\nOutput image data array size [%d,%d]\n",head->nsx,head->nsy);
  fprintf(stdout,"Select spacing format: [0=space, 1=comma, 2=tab] ");
  fscanf(stdin,"%d",&option);

  switch (option) {
  case 1:
    ch=',';
    break;
  case 2:
    ch='\t';
    break;
  case 0:
  default:
    ch=' ';
    break;
  }
  
  fprintf(stdout,"  0=left-to-right, top-to-bottom\n");
  fprintf(stdout,"  1=left-to-right, bottom-to-top (SIR standard)\n");
  fprintf(stdout,"  2=top-to-bottom, left-to-right\n");
  fprintf(stdout,"  3=bottom-to-top, left-to-right\n");
  fprintf(stdout,"Value order options: ");
  fscanf(stdin,"%d",&order);
  
  fprintf(stdout,"Enter output file name: ");
  fscanf(stdin,"%s",outfname);

  fid=fopen(outfname,"wb");
  if (fid==NULL) {
    fprintf(stdout,"*** error opening output file\n");
    return;
  }

  switch (order) {
  case 0:/* left-to-right, top-to-bottom */
    for (i = nsy-1; i >=0; i--) {
      for (j = 0; j < nsx-1; j++)
	fprintf(fid,"%f%c",*(data+i*nsx+j),ch);
      fprintf(fid,"%f\n",*(data+i*nsx+(nsx-1)));
    }
    break;
  case 2:/* top-to-bottom, left-to-right */
    for (j = nsx-1; j >= 0; j--) {
      for (i = nsy-1; i >0; i--)
	fprintf(fid,"%f%c",*(data+i*nsx+j),ch);
      fprintf(fid,"%f\n",*(data+0*nsx+j));
    }
    break;
  case 3:/* bottom-to-top, left-to-right */
    for (j = 0; j < nsx; j++) {
      for (i = 0; i <nsy-1; i++)
	fprintf(fid,"%f%c",*(data+i*nsx+j),ch);
      fprintf(fid,"%f\n",*(data+(nsy-1)*nsx+j));
    }
    break;
  
  case 1:/* left-to-right, bottom-to-top (normal SIR order)*/
  default:    
    for (i = 0; i <= nsy; i++) {
      for (j = 0; j < nsx-1; j++)
	fprintf(fid,"%f%c",*(data+i*nsx+j),ch);
      fprintf(fid,"%f\n",*(data+i*nsx+(nsx-1)));
    }
    break;
  }
  fclose(fid);
  fprintf(stdout,"SIR input file successfully processed\n");
  return;
}


void dump_locs(sir_head *head){
  char outfname_lat[250], outfname_lon[250];
  int option=0, order=0, i, j, nsx=head->nsx, nsy=head->nsy;
  char ch=' ';
  FILE *fid1, *fid2;
  float alon, alat, x=0.0, y=0.0;

  fprintf(stdout,"\nOutput pixel location array size [%d,%d]\n",head->nsx,head->nsy);
  fprintf(stdout,"Select spacing format: [0=space, 1=comma, 2=tab] ");
  fscanf(stdin,"%d",&option);

  switch (option) {
  case 1:
    ch=',';
    break;
  case 2:
    ch='\t';
    break;
  case 0:
  default:
    ch=' ';
    break;
  }

  fprintf(stdout,"  0=lower-left of pixel (SIR standard)\n");
  fprintf(stdout,"  1=upper-left of pixel\n");
  fprintf(stdout,"  2=center of pixel\n");
  fprintf(stdout,"Geolocation position option: ");
  fscanf(stdin,"%d",&option);
  
  switch (option) {
  case 1:
    y=1.0;
    fprintf(stdout,"Reported location: Upper-left of pixel\n\n");
    break;
  case 2:
    y=0.5;
    x=0.5;
    fprintf(stdout,"Reported location: Center of pixel\n\n");
    break;
  case 0:
  default:
    fprintf(stdout,"Reported location: Lower-left of pixel (SIR standard)\n\n");
    break;
  }
  
  fprintf(stdout,"  0=left-to-right, top-to-bottom\n");
  fprintf(stdout,"  1=left-to-right, bottom-to-top (SIR standard)\n");
  fprintf(stdout,"  2=top-to-bottom, left-to-right\n");
  fprintf(stdout,"  3=bottom-to-top, left-to-right\n");
  fprintf(stdout,"Value order options: ");
  fscanf(stdin,"%d",&order);
  
  fprintf(stdout,"Enter output latitude file name: ");
  fscanf(stdin,"%s",outfname_lat);
  fprintf(stdout,"Enter output longitude file name: ");
  fscanf(stdin,"%s",outfname_lon);

  fid1=fopen(outfname_lat,"wb");
  if (fid1==NULL) {
    fprintf(stdout,"*** error opening output file %s\n",outfname_lat);
    return;
  }
  fid2=fopen(outfname_lon,"wb");
  if (fid2==NULL) {
    fprintf(stdout,"*** error opening output file %s\n",outfname_lon);
    return;
  }

  switch (order) {
  case 0:/* left-to-right, top to bottom */
    for (i = nsy-1; i >=0; i--) {
      for (j = 0; j < nsx-1; j++) {
	sir_pix2latlon(x+(float) (j+1), y+(float) (i+1), &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
	fprintf(fid1,"%f%c",alat,ch);
	fprintf(fid2,"%f%c",alon,ch);
      }
      sir_pix2latlon(x+(float) (nsx), y+(float) (i+1), &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
      fprintf(fid1,"%f\n",alat);
      fprintf(fid2,"%f\n",alon);
    }
    break;
  case 2:/* top-to-bottom, left-to-right */
    for (j = nsx-1; j >= 0; j--) {
      for (i = nsy-1; i >0; i--){
	sir_pix2latlon(x+(float) (j+1), y+(float) (i+1), &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
	fprintf(fid1,"%f%c",alat,ch);
	fprintf(fid2,"%f%c",alon,ch);
      }
      sir_pix2latlon(x+(float) (j+1), y+(float) 1, &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
      fprintf(fid1,"%f\n",alat);
      fprintf(fid2,"%f\n",alon);
    }
    break;
  case 3:/* bottom-to-top, left-to-right */
    for (j = 0; j < nsx; j++) {
      for (i = 0; i < nsy-1; i++) {
	sir_pix2latlon(x+(float) (j+1), y+(float) (i+1), &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
	fprintf(fid1,"%f%c",alat,ch);
	fprintf(fid2,"%f%c",alon,ch);
      }
      sir_pix2latlon(x+(float) (j+1), y+(float) nsy, &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
      fprintf(fid1,"%f\n",alat);
      fprintf(fid2,"%f\n",alon);
    }
    break;
  
  case 1:/* left-to-right, bottom to top (normal SIR order)*/
  default:    
    for (i = 0; i <= nsy; i++) {
      for (j = 0; j < nsx-1; j++) {
	sir_pix2latlon(x+(float) (j+1), y+(float) (i+1), &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
	fprintf(fid1,"%f%c",alat,ch);
	fprintf(fid2,"%f%c",alon,ch);
      }
      sir_pix2latlon(x+(float) nsx, y+(float) (i+1), &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
      fprintf(fid1,"%f\n",alat);
      fprintf(fid2,"%f\n",alon);
    }
    break;
  }
  fclose(fid1);
  fclose(fid2);
  fprintf(stdout,"location files successfully processed\n");
  return;
}


int read_colortable(char *name, unsigned char *rtab, unsigned char *gtab, unsigned char *btab, int flag)
{
  FILE *imf;
  int i,r,g,b;
  
  imf = fopen(name,"rb"); 
  if (imf == NULL) {
    fprintf(stderr,"*** ERROR: cannot open color table file: %s\n",name);
    fflush(stderr);
    return(-1);
  } else {
    if (flag == 0) { /* binary color table file (one byte/entry 256 red, then green, etc. */
      if (fread(rtab, sizeof(char), 256, imf) == 0) {
	fprintf(stderr," *** Error reading color table file (r)\n");
	fclose(imf);
	return(-1);
      }
      if (fread(gtab, sizeof(char), 256, imf) == 0) {	
	fprintf(stderr," *** Error reading color table file (g)\n");
	fclose(imf);
	return(-1);
      }
      if (fread(btab, sizeof(char), 256, imf) == 0) {
	fprintf(stderr," *** Error reading color table file (b)\n");
	fclose(imf);
	return(-1);
      }
    } else { /* ascii color table file 256 lines with one r,g,b value per line */
      for (i=0; i< 256; i++)
	if (fscanf(imf,"%d %d %d",&r,&g,&b) == EOF) {
	  fprintf(stderr," *** Error reading color table file line %d\n",i);
	  fclose(imf);
	  return(-1);
	} else {	  
	  *(rtab+i) = r;
	  *(gtab+i) = g;
	  *(btab+i) = b;
	}
    }
    fclose(imf);
  }
  return(0);
}


void read_colormap(int *ColorMap_flag, unsigned char *rtab, unsigned char *gtab, unsigned char *btab)
{
  char infname[254];
  int flag=0, ierr=0;

  fprintf(stdout,"\nBinary format: one byte [0..255] per color per pixel\n (256 bytes red, 256 bytes green 256 bytes blue)\n");
  fprintf(stdout,"ASCII format: 3 integers [0..255] of red, green, blue\n (256 lines of red, green, blue)\n");
  fprintf(stdout,"\nEnter color map file format (0=binary  1=ascii): ");
  fscanf(stdin,"%d",&flag);
  if (flag)
    fprintf(stdout,"  Binary color table format file selected\n");
  else
    fprintf(stdout,"  Ascii color table format file selected\n");  
  fprintf(stdout,"\nEnter input color table file name: ");
  fscanf(stdin,"%s",infname);
  ierr=read_colortable(infname, rtab, gtab, btab, flag);
  if (ierr == 0)
    *ColorMap_flag=1;
  else
    *ColorMap_flag=0;  
  return;
}


void edit_colortable(int *ColorMap_flag, unsigned char *rtab, unsigned char *gtab, unsigned char *btab)
{
  int endflag=1, option, i, v;  

  fprintf(stdout,"\nEdit color table (256 entries of red, green, blue with values 0..255)\n");
  if (! *ColorMap_flag)
    fprintf(stdout," Color table is set to default greyscale\n");  

  while (endflag) {
    fprintf(stdout,"Options:\n 0 : Return\n");
    fprintf(stdout," 1 : Display color table values\n");  
    fprintf(stdout," 2 : Change color table entry\n");
    fprintf(stdout," 3 : Set to gray scale and turn off color table\n");
    fprintf(stdout,"Select menu option: ");
    fscanf(stdin,"%d",&option);

    switch (option) {
    case 0:
      endflag=0;
      break;

    case 1:
      fprintf(stdout,"\nIdx Red Grn Blu [0..255]\n");      
      for (i=0; i < 256; i++)
	fprintf(stdout,"%3d %3d %3d %3d\n",i,rtab[i],gtab[i],btab[i]);
      break;

    case 2:
      i=0;
      while (i > -1 && i < 256) {	
	fprintf(stdout,"Table index (0..255, -1 to end): ");
	fscanf(stdin,"%d",&i);
	if (i>255 || i < -1)
	  fprintf(stdout," Invalid index, must be 0..255\n");
        else if (i>-1) {
	  fprintf(stdout,"%3d red=%3d blue=%3d green=%3d\n",i,rtab[i],gtab[i],btab[i]);
	  fprintf(stdout," New red value [0..255]: ");
	  fscanf(stdin,"%d",&v);
	  if (v<0) v=0;
	  if (v>255) v=255;
	  rtab[i]=v;
	  fprintf(stdout," New green value [0..255]: ");
	  fscanf(stdin,"%d",&v);
	  if (v<0) v=0;
	  if (v>255) v=255;
	  gtab[i]=v;
	  fprintf(stdout," New blue value [0..255]: ");
	  fscanf(stdin,"%d",&v);
	  if (v<0) v=0;
	  if (v>255) v=255;
	  btab[i]=v;
	  fprintf(stdout,"%3d red=%3d blue=%3d green=%3d\n",i,rtab[i],gtab[i],btab[i]);
	}
      }
      
      *ColorMap_flag=1;
      break;

    case 3:
      for (i=0; i < 256; i++) {
	rtab[i]=i;
	gtab[i]=rtab[i];
	btab[i]=btab[i];
      }      
      *ColorMap_flag=0;      
      break;
      
    default:
      break;
    }
  }
  return;  
}


void copy_sir_head(sir_head *head, sir_head *head2) {
  int k;
  /* copy information from one sir_head structure to another */
  /* does NOT duplicate storage of items attached by pointers */

  head2->nhead=head->nhead;
  head2->nhtype=head->nhtype;
  head2->nsx=head->nsx;
  head2->nsy=head->nsy;
  head2->iopt=head->iopt;
  head2->xdeg=head->xdeg;
  head2->ydeg=head->ydeg;
  head2->ascale=head->ascale;
  head2->bscale=head->bscale;
  head2->a0=head->a0;
  head2->b0=head->b0;
  head2->ixdeg_off=head->ixdeg_off;
  head2->iydeg_off=head->iydeg_off;
  head2->ideg_sc=head->ideg_sc;
  head2->iscale_sc=head->iscale_sc;
  head2->ia0_off=head->ia0_off;
  head2->ib0_off=head->ib0_off;
  head2->i0_sc=head->i0_sc;
  head2->idatatype=head->idatatype;
  head2->ioff=head->ioff;
  head2->iscale=head->iscale;
  head2->anodata=head->anodata;
  head2->v_max=head->v_max;
  head2->v_min=head->v_min;
  head2->iyear=head->iyear;
  head2->isday=head->isday;
  head2->ismin=head->ismin;
  head2->ieday=head->ieday;
  head2->iemin=head->iemin;
  head2->iregion=head->iregion;
  head2->itype=head->itype;
  head2->ipol=head->ipol;
  head2->ifreqhm=head->ifreqhm;
  head2->ispare1=head->ispare1;

  /* note: only pointers are copied */
  strncpy(head2->title,head->title,101);
  strncpy(head2->sensor,head->sensor,41);
  strncpy(head2->type,head->type,139);
  strncpy(head2->tag,head->tag,101);
  strncpy(head2->crproc,head->crproc,101);
  strncpy(head2->crtime,head->crtime,29);
  
  head2->ndes=head->ndes;
  head2->ldes=head->ldes;
  head2->descrip=head->descrip;
  head2->nia=head->nia;
  head2->iaopt=head->iaopt;
  head2->maxdes=head->maxdes;
  head2->maxi=head->maxi;
  head2->descrip_flag=head->descrip_flag;
  head2->iaopt_flag=head->iaopt_flag;
  strncpy(head2->descrip_string,head->descrip_string,MAXDES+1);
  for (k=0; k<MAXI; k++)
    head2->iaopt_array[k]=head->iaopt_array[k];

  return;
}

void write_locs(sir_head *head) {
  char outfname_lat[250], outfname_lon[250];
  int option=0, i, j, nsx=head->nsx, nsy=head->nsy, ierr;
  float alon, alat, x=0.0, y=0.0, *data;
  sir_head head2;

  sir_init_head(&head2);     /* initialize sir_ez structure */
  copy_sir_head(head, &head2);

  
  fprintf(stdout,"\nOutput pixel location array size [%d,%d]\n",head->nsx,head->nsy);

  fprintf(stdout,"  0=lower-left of pixel (SIR standard)\n");
  fprintf(stdout,"  1=upper-left of pixel\n");
  fprintf(stdout,"  2=center of pixel\n");
  fprintf(stdout,"Geolocation position option: ");
  fscanf(stdin,"%d",&option);
  
  switch (option) {
  case 1:
    y=1.0;
    fprintf(stdout,"Reported location: Upper-left of pixel\n\n");
    break;
  case 2:
    y=0.5;
    x=0.5;
    fprintf(stdout,"Reported location: Center of pixel\n\n");
    break;
  case 0:
  default:
    fprintf(stdout,"Reported location: Lower-left of pixel (SIR standard)\n\n");
    break;
  }
  
  fprintf(stdout,"Enter output latitude file name: ");
  fscanf(stdin,"%s",outfname_lat);
  fprintf(stdout,"Enter output longitude file name: ");
  fscanf(stdin,"%s",outfname_lon);
  
  data = (float *) malloc(sizeof(float) * nsx * nsy);
  if (data==NULL) {
    fprintf(stdout,"*** error allocating memory\n");
    return;
  }

  /* to save ram (at the expense of CPU), save only one of lat,lon at a time */
  
  /* SIR standard order left-to-right, bottom to top (normal SIR order)*/
  for (i = 0; i <= nsy; i++) {
    for (j = 0; j < nsx-1; j++) {
      sir_pix2latlon(x+(float) (j+1), y+(float) (i+1), &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
      *(data+i*nsx+j) = alat;
    }
  }
  
/* modify header components to store the dynamic range of latitude */

  head2.ioff = -91;
  head2.iscale = 360;
  head2.anodata = -91.0;
  head2.v_min = -90.0;
  head2.v_max = 90.0;
  head2.itype = 31;     /* set type to latitude */
  head2.idatatype = 2;  /* default two byte integer storage */

  sprintf(head2.crproc,"BYU sir_util");
  sprintf(head2.type,"Latitude");

  /* write out latitude array as a SIR-format file */
  fprintf(stdout," Writing latitude file to '%s'\n", outfname_lat);
  ierr = put_sir(outfname_lat, &head2, data);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing latitude output file ***\n");
     fflush(stderr);
  }

  /* now do longitude */
  
  for (i = 0; i <= nsy; i++) {
    for (j = 0; j < nsx-1; j++) {
      sir_pix2latlon(x+(float) (j+1), y+(float) (i+1), &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
      *(data+i*nsx+j) = alon;
    }
  }

  /* modify header to storage dynamic range of longitude */

  head2.ioff = -181;
  head2.iscale = 180;
  head2.anodata = -181.0;
  head2.v_min = -180.0;
  head2.v_max = 180.0;
  head2.itype = 30;     /* set type to longitude */

  sprintf(head2.crproc,"BYU sir_util");
  sprintf(head2.type,"Longitude");

  /* write out longitude array as a SIR-format file */
  fprintf(stdout," Writing latitude file to '%s'\n", outfname_lat);
  ierr = put_sir(outfname_lon, &head2, data);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing latitude output file ***\n");
     fflush(stderr);
  }
  
  fprintf(stdout,"location files successfully processed\n");

  /* free temporary storage */
  free(data);
  return;
}

void subregion(float *stval, sir_head *head) {
  int   ierr, i, j, nsx2, nsy2, x1, y1, x2, y2, ix, iy;
  float alon, alat, a02, b02, xdeg2, ydeg2;
  float *stval2;       /* pointer to output image storage */
  char temp[180];

  /* get LL and UR corners.  These can be directly input or
     a bounding box can be specified */
  
  fprintf(stdout,"\nSpecify subregion bounds.  Note: SIR file origin (1,1) is at\n");
  fprintf(stdout,"the lower left corner of the lower left pixel\n");
  fprintf(stdout,"Enter left horizontal (x) pixel location: [1..%d] ",head->nsx);
  fscanf(stdin,"%d",&x1);
  fprintf(stdout,"Enter right horizontal (x) pixel location: [%d..%d] ",x1,head->nsx);
  fscanf(stdin,"%d",&x2);
  fprintf(stdout,"Enter lower vertical (y) pixel location: [1..%d] ",head->nsy);
  fscanf(stdin,"%d",&y1);
  fprintf(stdout,"Enter upper vertical (y) pixel location: [%d..%d] ",y1,head->nsy);
  fscanf(stdin,"%d",&y2);

  /* ensure selected locations are completely contained within image and
     are in correct LL, UR arrangement */

  if (x1 < 1) x1 = 1;
  if (y1 < 1) y1 = 1;
  if (x2 < 1) x2 = 1;
  if (y2 < 1) y2 = 1;
  
  if (x1 > head->nsx) x1 = head->nsx;
  if (y1 > head->nsy) y1 = head->nsy;
  if (x2 > head->nsx) x2 = head->nsx;
  if (y2 > head->nsy) y2 = head->nsy;

  if (x2 < x1) {
    i = x1;
    x1 = x2;
    x2 = 1;
  }
  if (y2 < y1) {
    i = y1;
    y1 = y2;
    y2 = 1;
  }

  /* compute subregion size and echo final corners to user */
  
  nsx2 = x2 - x1 + 1;
  nsy2 = y2 - y1 + 1;
  
  fprintf(stdout,"Array sizes: (in) %d x %d  (out) %d x %d\n",head->nsx,head->nsy,nsx2,nsy2);
  sir_pix2latlon((float) x1, (float) y1, &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
  fprintf(stdout," LL pixel location: %d, %d  Lon, Lat: %f, %f\n",x1,y1,alon,alat);
  sir_pix2latlon((float) x2, (float) y2, &alon, &alat, head);  /* (ix,iy) -> (lat,lon) */
  fprintf(stdout," UR pixel location: %d, %d  Lon, Lat: %f, %f\n",x2,y2,alon,alat);
  
  if (nsx2 < 1 || nsy2 < 1) {
    printf("*** zero size subregion, operation aborted.\n");
    return;
  }
  
  /* compute new image projection information */

  xdeg2 = head->xdeg;
  ydeg2 = head->ydeg;
  
  switch (head->iopt) {

  case 1:   /* Lambert */
  case 2:
    a02 = (x1 - 1) / head->ascale + head->a0;
    b02 = (y1 - 1) / head->bscale + head->b0;
    break;

  case 8:   /* EASE2 grids */
  case 9:
  case 10:
    a02 = head->a0 + (x1 - 1);
    b02 = head->b0 + (y1 - 1);
    break;

  case 11:  /* EASE1 grids */
  case 12:
  case 13:
    a02 = head->a0 + (x1 - 1);
    b02 = head->b0 + (y1 - 1);
    break;
    
  case 5:   /* polar stereographic */
    a02 = (x1 - 1) * head->ascale + head->a0;
    b02 = (y1 - 1) * head->bscale + head->b0;
    break;

  case -1:  /* image only */
  case 0:   /* lat/lon */
  default:  /* unknown */
    a02 = (x1 - 1) * head->xdeg / (float) head->nsx + head->a0;
    b02 = (y1 - 1) * head->ydeg / (float) head->nsy + head->b0;
    xdeg2 = (float) nsx2 * head->xdeg / (float) head->nsx;
    ydeg2 = (float) nsy2 * head->ydeg / (float) head->nsy;
    break;
  }

  /* allocate new subregion image array */

  stval2 = (float *) malloc(sizeof(float) * nsx2 * nsy2);
  if (stval2 == NULL) {
     fprintf(stderr,"*** error subregion image memory allocation failure...\n");
     return;
  }

  /* copy data from subregion into new array */

  for (ix = x1; ix <= x2; ix++)
     for (iy = y1; iy <= y2; iy++) {
       i = (iy-1) * head->nsx + ix - 1;/* word number (1..nsx*nsy) of pixel (ix,iy) */
       j = (iy-y1) * nsx2 + ix - x1;  /* word number (1..nsx2*nsy2) of pixel (ix,iy) */
       *(stval2 + j) = *(stval + i);
     }

  /* copy data back to original array */
  for  (ix=0; ix < nsx2*nsy2; ix++)
    *(stval+ix)=*(stval2+ix);

  /* free temporary storage */
  free(stval2);
					 
  /* generate new header values, reusing original header info */

  head->nsx = nsx2;
  head->nsy = nsy2;
  head->a0 = a02; 
  head->b0 = b02;
  head->xdeg = xdeg2; 
  head->ydeg = ydeg2;

  sprintf(temp,"SubReg: %s",head->title);
  strncpy(head->title,temp,80);

  sprintf(temp,"subregion - %s",head->crproc);
  strncpy(head->crproc,temp,100);

  fprintf(stdout,"Subregion created.  Save to .sir file? [y/n] ");
  fscanf(stdin,"%s",&temp);

  if (temp[0]=='y' || temp[0] =='Y') {
    fprintf(stdout,"Output file name: ");
    fscanf(stdin,"%s",temp);
    fprintf(stdout," Writing file to '%s'\n", temp);
    ierr = put_sir(temp, head, stval);
    if (ierr < 0) {
      fprintf(stderr,"*** ERROR writing output file ***\n");
      fflush(stderr);
    } else
      fprintf(stdout,"Write complete\n");
  }
  return;
}


float fround(float r)  /* ensure compatibility across compilers */
{	
  int i = (int) r;
  float ret_val = (float) i;
  if (ret_val - r > 0.5) ret_val -= 1.0;
  if (r - ret_val > 0.5) ret_val += 1.0;
  return(ret_val);
}


int WriteBMP(char *fname, char *pic, int w, int h, char *rmap, char *gmap, char *bmap);
int writegif(char *name_in, int *len_in, char *pic, int *pw, int *ph,
	     char *rmap, char *gmap, char *bmap, 
	     int *pnumcols, int *pcolorstyle);
int write_geotiff(char *filename, unsigned char *byte, int nx, int ny,
		  int proj, double pixscale, double uy, double lx,
		  double proj_org, double proj_lat, double proj_lon,
		  int colormap_flag, int *red, int *green, int *blue, int Geotiff);


void convert_image(float *stval, sir_head *head, int ColorMap_flag, 
		   unsigned char *rtab, unsigned char *gtab, unsigned char *btab)
  /* creates a BMP, GIF, TIFF, or GeoTIFF image */
{
  char outfname[250];
  int option=0, i, j, k, ierr, nsx=head->nsx, nsy=head->nsy;
  float smin, smax, am, scale, scaleoffset;
  unsigned char *data;
  int red[256], green[256], blue[256];
  int Show_nodata=0;
  int GeoTiff_flag=0;
  int gproj=0;
  double pixscale=1.0, uy=0.0, lx=0.0, proj_org=0.0, proj_lat=0.0, proj_lon=0.0; 
  double latitude_of_projection_origin;
  double latitude_of_true_scale;   
  double longitude_of_projection_origin;
  double semimajor_radius, f;
  double semiminor_radius;

  fprintf(stdout,"\nWrite out as 8-bit BMP, GIF, TIFF, or GeoTIFF (selected projections)\n");
  fprintf(stdout,"Output format [0=BMP, 1=GIF, 2=TIFF, 3=GeoTIFF]: ");
  fscanf(stdin,"%d",&option);

  fprintf(stdout," Enter min saturation value (vmin=%f): ",head->v_min);
  fscanf(stdin,"%f",&smin);
  fprintf(stdout," Enter max saturation value (vmax=%f): ",head->v_max);
  fscanf(stdin,"%f",&smax);
  fprintf(stdout," Separately indicate no-data value at bottom of color table (1=yes, 0=no): ");
  fscanf(stdin,"%d",&Show_nodata);
  
  fprintf(stdout,"Enter output file name: ");
  fscanf(stdin,"%s",outfname);

  fprintf(stdout,"\nSaturation Min, Max: %f , %f\n\n",smin,smax);
  switch (option) {
  case 0:
    fprintf(stdout," BMP output file %s\n",outfname);
    break;
  case 1:
    fprintf(stdout," GIF output file %s\n",outfname);
    break;
  case 2:
    fprintf(stdout," TIFF output file %s\n",outfname);
    GeoTiff_flag=0;
    break;
  case 3:
    fprintf(stdout," GeoTIFF output file %s\n",outfname);
    GeoTiff_flag=1;
    break;
  default:
    fprintf(stdout,"*unrecognized image option, returning...\n");
    return;
  }

  /* allocate temporary working array */
  data = (unsigned char *) malloc(sizeof(char)*nsx*nsy);
  if (data == NULL) {
    fprintf(stdout,"*** error allocating memory\n");
    return;
  }

  /* create 8-bit scaled image */
  scale = (smax-smin);
  if (scale > 0.) 
    scale = 255./ scale;
  else
    scale = 1; 
  scaleoffset = smin;

  for (i = 0; i < nsy; i++)
    for (j = 0; j < nsx; j++){          /* scale floating point to byte values */
      am = scale * (*(stval+i*nsx+j) - scaleoffset);
      if (am > 255.) am = 255.;		/* check overflow */
      if (am <   0.) am = 0.;		/* check underflow */
      if (Show_nodata) { /* use bottom of color table to indicate no data */
	am += 1.0;
	if (am > 255.0) am = 255.0;
	if (am < 1.0) am = 1.0;
	if (*(stval+i*nsx+j) < head->anodata+0.0001) am=0.;  /* no data value */
      } else {	
	if (am > 255.0) am = 255;
	if (am < 0.0) am = 0;
      }
      *(data+(nsy-i-1)*nsx+j) = ((int)(am));    /* convert to 8 bit value */
    }


  /* write output image to file*/
  switch (option) {
  case 0:   /* bmp */    
    ierr=WriteBMP(outfname, (char *) data, nsx, nsy, (char *)rtab, (char *)gtab, (char *)btab);
    break;

  case 1: /* gif */
  default:    
    k=strlen(outfname); i=0; j=256;
    ierr=writegif(outfname, &k, (char *) data, &nsx, &nsy, (char *)rtab, (char *)gtab, (char *)btab, &j, &i);
    break;

  case 2: /* tiff only */
    /* scale 8 bit color table to 16 bits for tiff file */
    for (j=0;j<256;j++) {
      red[j]  =(int) rtab[j] * 256;
      green[j]=(int) gtab[j] * 256;
      blue[j] =(int) btab[j] * 256;
    }
    GeoTiff_flag=0;    
    ierr=write_geotiff(outfname, data, nsx, nsy, gproj, pixscale,
		       uy, lx, proj_org, proj_lat, proj_lon,
		       ColorMap_flag, red, green, blue, GeoTiff_flag);
    break;
    
  case 3: /* geotiff */

    /* get projection transformation information from BYU SIR file */
    switch (head->iopt) {
    case 5:  /* Polar stereographic */
      /* precise projection parameters used in generating SIR images */
      semimajor_radius = 6378273.0;
      f = 2.0/0.006693883;
      semiminor_radius = semimajor_radius * sqrt(1.0 - 0.006693883);
      longitude_of_projection_origin = fround(head->xdeg);
      latitude_of_true_scale = fround(head->ydeg);

      latitude_of_projection_origin = 90.0;
      if (fround(head->ydeg) < 0.0)
	latitude_of_projection_origin = -90.0;
   
      /* for precision, use exact values rather than the quantized 
	 values from SIR file header for common specific cases, 
	 when possible.  This is not really required. */
      if (head->iregion == 100) { /* Antarctic */
	latitude_of_projection_origin = -90.0;
	latitude_of_true_scale = -70.0;
	longitude_of_projection_origin = 0.0;
      } else if (head->iregion == 110) { /* Arctic */
	latitude_of_projection_origin = 90.0;
	latitude_of_true_scale = 70.0;
	longitude_of_projection_origin = -45.0;
      } else if (head->iregion == 112) { /* NHe */
	latitude_of_projection_origin = 90.0;
	latitude_of_true_scale = 70.0;
	longitude_of_projection_origin = -45.0;
      }
 
      gproj=2; /* Polar stereographic */
      pixscale=(double) fround(head->ascale*1000.0); /* the same for both x and y */
      proj_org=latitude_of_projection_origin;    
      proj_lat=latitude_of_true_scale;
      proj_lon=longitude_of_projection_origin;
      uy=fround(head->a0*1000.0); /* image pixel coordinates at LL of SIR pixel*/
      lx=fround(head->b0*1000.0)+head->nsy*fround(head->bscale*1000.0);
      GeoTiff_flag=1;
      break;

    case -1:  /* no projection, can not do GeoTiff */
      fprintf(stdout," * No projection information, only a conventional tiff image will be made\n");
      GeoTiff_flag=0;
      break;

    case 0:   /* Lat/lon grid "projection" */
      fprintf(stdout," * Lat/Lon SIR projection. Only a conventional tiff image will be made\n");
      GeoTiff_flag=0;
      break;
    
    case 1:   /* Lambert equal area fixed radius projection */
      /* precise projection parameters used in generating SIR images */
      semimajor_radius = 6378000.0;     
      f=0.0;   
      semiminor_radius = semimajor_radius;
      /* fall through next case intended */
    case 2:   /* Lambert equal area local radius projection */
      fprintf(stdout,"Lambert Equal Area Projection\n");    
      if (head->iopt == 2) {
	/* precise projection parameters used in generating SIR images */
	semimajor_radius = 6378135.0;     
	f=298.260;   
	semiminor_radius = semimajor_radius * (1.0 - 1.0/f);
      }

      latitude_of_projection_origin = head->ydeg;
      latitude_of_true_scale = head->ydeg;
      longitude_of_projection_origin = head->xdeg;

      /* set write_geotiff parameters */
      gproj=1;    /* Lambert */
      pixscale=(double) fround(1000.0/head->ascale); /* the same for both x and y */
      proj_org=latitude_of_true_scale;
      proj_lat=latitude_of_projection_origin;      
      proj_lon=longitude_of_projection_origin;
      uy=fround(1000.0*head->a0);  /* image pixel coordinates at LL of SIR pixel*/
      lx=fround(1000.0*head->b0);     
      break;

    case  8: /* EASE2 grid north */
    case  9: /* EASE2 grid south */
    case 10: /* EASE2 grid cylindrical */
      fprintf(stdout," * Geotiff EASE2 grid projection not currently supported. Only a conventional tiff image will be made\n");
      GeoTiff_flag=0;
      break;

    case 11: /* EASE1 grid north */
    case 12: /* EASE1 grid south */
    case 13: /* EASE1 grid cylindrical */
      fprintf(stdout," * Geotiff EASE1 grid projection not supported. Only a conventional tiff image will be made\n");
      GeoTiff_flag=0;
      break;

    default:
      fprintf(stdout," * Unknown projection. Only a conventional tiff image will be made\n");
      GeoTiff_flag=0;
      break;
    }

    /* scale 8 bit color table to 16 bits for tiff file */
    for (j=0;j<256;j++) {
      red[j]  =(int) rtab[j] * 256;
      green[j]=(int) gtab[j] * 256;
      blue[j] =(int) btab[j] * 256;
    }
    ierr=write_geotiff(outfname, data, nsx, nsy, gproj, pixscale,
		       uy, lx, proj_org, proj_lat, proj_lon,
		       ColorMap_flag, red, green, blue, GeoTiff_flag);
    break;
  }
  
 if (ierr<0)
   fprintf(stdout,"*** error writing output file\n");
 else
   fprintf(stdout," Output file: %s\n",outfname);

 /* free temporary storage */
 free(data);
 
 return;
}



/*****************************************************************/
/*
 * xvgifwr.c  -  handles writing of GIF files.  based on flgife.c and
 *               flgifc.c from the FBM Library, by Michael Maudlin
 *
 * Note: slightly brain-damaged, in that it'll only write non-interlaced 
 *       GIF files (in the interests of speed, or something)
 *
 */

/*****************************************************************
 * Portions of this code Copyright (C) 1989 by Michael Mauldin.
 * Permission is granted to use this file in whole or in part provided
 * that you do not sell it for profit and that this copyright notice
 * and the names of all authors are retained unchanged.
 *
 * Authors:  Michael Mauldin (mlm@cs.cmu.edu)
 *           David Rowley (mgardi@watdcsu.waterloo.edu)
 *
 * Based on: compress.c - File compression ala IEEE Computer, June 1984.
 *
 *	Spencer W. Thomas       (decvax!harpo!utah-cs!utah-gr!thomas)
 *	Jim McKie               (decvax!mcvax!jim)
 *	Steve Davies            (decvax!vax135!petsd!peora!srd)
 *	Ken Turkowski           (decvax!decwrl!turtlevax!ken)
 *	James A. Woods          (decvax!ihnp4!ames!jaw)
 *	Joe Orost               (decvax!vax135!petsd!joe)
 *****************************************************************/
 
/*
 * Copyright 1989, 1990 by the University of Pennsylvania
 *
 * Permission to use, copy, and distribute for non-commercial purposes,
 * is hereby granted without fee, providing that the above copyright
 * notice appear in all copies and that both the copyright notice and this
 * permission notice appear in supporting documentation.
 *
 * The software may be modified for your own purposes, but modified versions
 * may not be distributed.
 *
 * This software is provided "as is" without any express or implied warranty.
 */

typedef unsigned char byte;

/* MONO returns total intensity of r,g,b components */
#define MONO(rd,gn,bl) (((rd)*11 + (gn)*16 + (bl)*5) >> 5)  /*.33R+ .5G+ .17B*/

typedef long int  count_int;

static int  Width, Height;
static int  curx, cury;
static long CountDown;
static int  Interlace;
static byte bw[2] = {0, 0xff};

#ifdef __STDC__
static void putword(int, FILE *);
static void compress(int, FILE *, byte *, int);
static void output(int);
static void cl_block(void);
static void cl_hash(count_int);
static void char_init(void);
static void char_out(int);
static void flush_char(void);
#else
static void putword(), compress(), output(), cl_block(), cl_hash();
static void char_init(), char_out(), flush_char();
#endif

/*************************************************************/

int writegif(char *name_in, int *len_in, char *pic, int *pw, int *ph, 
	     char *rmap, char *gmap, char *bmap, int *pnumcols, 
	     int *pcolorstyle)
{
  FILE *fp;
  int RWidth, RHeight;
  int LeftOfs, TopOfs;
  int Resolution, ColorMapSize, InitCodeSize, Background, BitsPerPixel;
  int i,j,w,h,numcols,colorstyle;

  char fname[250];

  w = *pw;
  h = *ph;
  numcols = *pnumcols;
  colorstyle = *pcolorstyle;
  
  for (i=0; i < *len_in; i++)
    fname[i] = *(name_in+i);	
  fname[*len_in] = 0;
  
  fp = fopen(&fname[0],"wb");

  /* if writing B/W stipple... */
  if (colorstyle==2) {
    rmap = gmap = bmap = (char *) bw;
    numcols = 2;
  }

  Interlace = 0;
  Background = 0;

  /* figure out 'BitsPerPixel' */
  for (i=1; i<8; i++)
    if ( (1<<i) >= numcols) break;
  
  BitsPerPixel = 8;

  ColorMapSize = 1 << BitsPerPixel;
	
  RWidth  = Width  = w;
  RHeight = Height = h;
  LeftOfs = TopOfs = 0;
	
  Resolution = BitsPerPixel;

  CountDown = w * h;    /* # of pixels we'll be doing */

  if (BitsPerPixel <= 1) InitCodeSize = 2;
                    else InitCodeSize = BitsPerPixel;

  curx = cury = 0;

  if (!fp) {
    fprintf(stderr,  "WriteGIF: file not open for writing\n" );
    return (1);
  }

  /*  if (DEBUG)  
    fprintf(stderr,"WrGIF: pic=%lx, w,h=%dx%d, numcols=%d, Bits%d,Cmap=%d\n",
	    pic, w,h,numcols,BitsPerPixel,ColorMapSize);
      */
  fwrite("GIF87a", 1, 6, fp);    /* the GIF magic number */

  putword(RWidth, fp);           /* screen descriptor */
  putword(RHeight, fp);

  i = 0x80;	                 /* Yes, there is a color map */
  i |= (8-1)<<4;                 /* OR in the color resolution (hardwired 8) */
  i |= (BitsPerPixel - 1);       /* OR in the # of bits per pixel */
  fputc(i,fp);          

  fputc(Background, fp);         /* background color */

  fputc(0, fp);                  /* future expansion byte */


  if (colorstyle == 1) {         /* greyscale */
    for (i=0; i<ColorMapSize; i++) {
      j = MONO(rmap[i], gmap[i], bmap[i]);
      fputc(j, fp);
      fputc(j, fp);
      fputc(j, fp);
    }
  }
  else {
    for (i=0; i<ColorMapSize; i++) {       /* write out Global colormap */
      fputc(rmap[i], fp);
      fputc(gmap[i], fp);
      fputc(bmap[i], fp);
    }
  }

  fputc( ',', fp );              /* image separator */

  /* Write the Image header */
  putword(LeftOfs, fp);
  putword(TopOfs,  fp);
  putword(Width,   fp);
  putword(Height,  fp);
  if (Interlace) fputc(0x40, fp);   /* Use Global Colormap, maybe Interlace */
            else fputc(0x00, fp);

  fputc(InitCodeSize, fp);
  compress(InitCodeSize+1, fp, (byte *) pic, w*h);

  fputc(0,fp);                      /* Write out a Zero-length packet (EOF) */
  fputc(';',fp);                    /* Write GIF file terminator */

  fclose(fp);  
  return (0);
}




/******************************/
static void putword(w, fp)
int w;
FILE *fp;
{
  /* writes a 16-bit integer in GIF order (LSB first) */
  fputc(w & 0xff, fp);
  fputc((w>>8)&0xff, fp);
}

/***********************************************************************/


static unsigned long cur_accum = 0;
static int           cur_bits = 0;

#define BITS	12
#define MSDOS	1

#define HSIZE  5003            /* 80% occupancy */

typedef unsigned char   char_type;


static int n_bits;                   /* number of bits/code */
static int maxbits = BITS;           /* user settable max # bits/code */
static int maxcode;                  /* maximum code, given n_bits */
static int maxmaxcode = 1 << BITS;   /* NEVER generate this */

#define MAXCODE(n_bits)     ( (1 << (n_bits)) - 1)

static  count_int      htab [HSIZE];
static  unsigned short codetab [HSIZE];
#define HashTabOf(i)   htab[i]
#define CodeTabOf(i)   codetab[i]

static int hsize = HSIZE;            /* for dynamic table sizing */

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i) CodeTabOf(i)
#define tab_suffixof(i)        ((char_type *)(htab))[i]
#define de_stack               ((char_type *)&tab_suffixof(1<<BITS))

static int free_ent = 0;                  /* first unused entry */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
static int clear_flg = 0;

static long int in_count = 1;            /* length of input */
static long int out_count = 0;           /* # of codes output (for debugging) */

/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the 
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

static int g_init_bits;
static FILE *g_outfile;

static int ClearCode;
static int EOFCode;


/********************************************************/
static void compress(init_bits, outfile, data, len)
int   init_bits;
FILE *outfile;
byte *data;
int   len;
{
  register long fcode;
  register int i = 0;
  register int c;
  register int ent;
  register int disp;
  register int hsize_reg;
  register int hshift;

  /*
   * Set up the globals:  g_init_bits - initial number of bits
   *                      g_outfile   - pointer to output file
   */
  g_init_bits = init_bits;
  g_outfile   = outfile;

  /* initialize 'compress' globals */
  maxbits = BITS;
  maxmaxcode = 1<<BITS;
  memset((char *) htab, 0, sizeof(htab));
  memset((char *) codetab, 0, sizeof(codetab));
  hsize = HSIZE;
  free_ent = 0;
  clear_flg = 0;
  in_count = 1;
  out_count = 0;
  cur_accum = 0;
  cur_bits = 0;


  /*
   * Set up the necessary values
   */
  out_count = 0;
  clear_flg = 0;
  in_count = 1;
  maxcode = MAXCODE(n_bits = g_init_bits);

  ClearCode = (1 << (init_bits - 1));
  EOFCode = ClearCode + 1;
  free_ent = ClearCode + 2;

  char_init();
  ent = *data++;  len--;

  hshift = 0;
  for ( fcode = (long) hsize;  fcode < 65536L; fcode *= 2L )
    hshift++;
  hshift = 8 - hshift;                /* set hash code range bound */

  hsize_reg = hsize;
  cl_hash( (count_int) hsize_reg);            /* clear hash table */

  output(ClearCode);
    
  while (len) {
    c = *data++;  len--;
    in_count++;

    fcode = (long) ( ( (long) c << maxbits) + ent);
    i = (((int) c << hshift) ^ ent);    /* xor hashing */

    if ( HashTabOf (i) == fcode ) {
      ent = CodeTabOf (i);
      continue;
    }

    else if ( (long)HashTabOf (i) < 0 )      /* empty slot */
      goto nomatch;

    disp = hsize_reg - i;           /* secondary hash (after G. Knott) */
    if ( i == 0 )
      disp = 1;

probe:
    if ( (i -= disp) < 0 )
      i += hsize_reg;

    if ( HashTabOf (i) == fcode ) {
      ent = CodeTabOf (i);
      continue;
    }

    if ( (long)HashTabOf (i) > 0 ) 
      goto probe;

nomatch:
    output(ent);
    out_count++;
    ent = c;

    if ( free_ent < maxmaxcode ) {
      CodeTabOf (i) = free_ent++; /* code -> hashtable */
      HashTabOf (i) = fcode;
    }
    else
      cl_block();
  }

  /* Put out the final code */
  output(ent);
  out_count++;
  output(EOFCode);
}


/*****************************************************************
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 *      code:   A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *              that n_bits =< (long)wordsize - 1.
 * Outputs:
 *      Outputs code to the file.
 * Assumptions:
 *      Chars are 8 bits long.
 * Algorithm:
 *      Maintain a BITS character long buffer (so that 8 codes will
 * fit in it exactly).  Use the VAX insv instruction to insert each
 * code in turn.  When the buffer fills up empty it and start over.
 */

static
unsigned long masks[] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
                                  0x001F, 0x003F, 0x007F, 0x00FF,
                                  0x01FF, 0x03FF, 0x07FF, 0x0FFF,
                                  0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

static void output(code)
int code;
{
  cur_accum &= masks[cur_bits];

  if (cur_bits > 0)
    cur_accum |= ((long)code << cur_bits);
  else
    cur_accum = code;
	
  cur_bits += n_bits;

  while( cur_bits >= 8 ) {
    char_out( (unsigned int) (cur_accum & 0xff) );
    cur_accum >>= 8;
    cur_bits -= 8;
  }

  /*
   * If the next entry is going to be too big for the code size,
   * then increase it, if possible.
   */

  if (free_ent > maxcode || clear_flg) {

    if( clear_flg ) {
      maxcode = MAXCODE (n_bits = g_init_bits);
      clear_flg = 0;
    }
    else {
      n_bits++;
      if ( n_bits == maxbits )
	maxcode = maxmaxcode;
      else
	maxcode = MAXCODE(n_bits);
    }
  }
	
  if( code == EOFCode ) {
    /* At EOF, write the rest of the buffer */
    while( cur_bits > 0 ) {
      char_out( (unsigned int)(cur_accum & 0xff) );
      cur_accum >>= 8;
      cur_bits -= 8;
    }

    flush_char();
	
    fflush( g_outfile );

    if( ferror( g_outfile ) )
      fprintf(stderr,"unable to write GIF file");
  }
}


/********************************/
static void cl_block ()             /* table clear for block compress */
{
  /* Clear out the hash table */

  cl_hash ( (count_int) hsize );
  free_ent = ClearCode + 2;
  clear_flg = 1;

  output(ClearCode);
}


/********************************/
static void cl_hash(hsize)          /* reset code table */
register count_int hsize;
{
  register count_int *htab_p = htab+hsize;
  register long i;
  register long m1 = -1;

  i = hsize - 16;
  do {                            /* might use Sys V memset(3) here */
    *(htab_p-16) = m1;
    *(htab_p-15) = m1;
    *(htab_p-14) = m1;
    *(htab_p-13) = m1;
    *(htab_p-12) = m1;
    *(htab_p-11) = m1;
    *(htab_p-10) = m1;
    *(htab_p-9) = m1;
    *(htab_p-8) = m1;
    *(htab_p-7) = m1;
    *(htab_p-6) = m1;
    *(htab_p-5) = m1;
    *(htab_p-4) = m1;
    *(htab_p-3) = m1;
    *(htab_p-2) = m1;
    *(htab_p-1) = m1;
    htab_p -= 16;
  } while ((i -= 16) >= 0);

  for ( i += 16; i > 0; i-- )
    *--htab_p = m1;
}


/******************************************************************************
 *
 * GIF Specific routines
 *
 ******************************************************************************/

/*
 * Number of characters so far in this 'packet'
 */
static int a_count;

/*
 * Set up the 'byte output' routine
 */
static void char_init()
{
	a_count = 0;
}

/*
 * Define the storage for the packet accumulator
 */
static char accum[ 256 ];

/*
 * Add a character to the end of the current packet, and if it is 254
 * characters, flush the packet to disk.
 */
static void char_out(c)
int c;
{
  accum[ a_count++ ] = c;
  if( a_count >= 254 ) 
    flush_char();
}

/*
 * Flush the packet to disk, and reset the accumulator
 */
static void flush_char()
{
  if( a_count > 0 ) {
    fputc( a_count, g_outfile );
    fwrite( accum, 1, a_count, g_outfile );
    a_count = 0;
  }
}	



/**************************************************************************/
/* WriteBMP routines 
   this version only write 8 bit BMP image files w/o compression */

static void putshort(FILE *fp, int i)
{
  int c, c1;
  c = ((unsigned int ) i) & 0xff;  c1 = (((unsigned int) i)>>8) & 0xff;
  putc(c, fp);   putc(c1,fp);
  return;  
}


/*******************************************/
static void putint(FILE *fp,int i)
{
  int c, c1, c2, c3;
  c  = ((unsigned int ) i)      & 0xff;  
  c1 = (((unsigned int) i)>>8)  & 0xff;
  c2 = (((unsigned int) i)>>16) & 0xff;
  c3 = (((unsigned int) i)>>24) & 0xff;
  putc(c, fp);   putc(c1,fp);  putc(c2,fp);  putc(c3,fp);
  return;  
}


/*******************************************/
void writeBMP8(FILE *fp, char *pic8, int w, int h)
{
  int   i,j,padw;
  char *pp;

  padw = ((w + 3)/4) * 4; /* 'w' padded to a multiple of 4pix (32 bits) */

  for (i=h-1; i>=0; i--) {
    pp = pic8 + (i * w);
    for (j=0; j<w; j++) putc(*pp++, fp);
    for ( ; j<padw; j++) putc(0, fp);
  }
  return;  
}  


/*******************************************/
int WriteBMP(char *fname, char *pic, int w, int h, char *rmap, char *gmap, char *bmap)
{
  int i, nc, nbits=8, bperlin;
  FILE *fp;
  
  fp=fopen(fname,"wb");
  if (fp==NULL) {
    fprintf(stdout,"*** error opening output file %s\n",fname);
    return(-1);
  }

  nc = 1<<nbits;                      /* # of entries in cmap */
  bperlin = ((w * nbits + 31) / 32) * 4;   /* # bytes written per line */

  putc('B', fp);  putc('M', fp);           /* BMP file magic number */

  /* compute filesize and write it */
  i = 14 +                /* size of bitmap file header */
      40 +                /* size of bitmap info header */
      (nc * 4) +          /* size of colormap */
      bperlin * h;        /* size of image data */

  putint(fp, i);
  putshort(fp, 0);        /* reserved1 */
  putshort(fp, 0);        /* reserved2 */
  putint(fp, 14 + 40 + (nc * 4));  /* offset from BOfile to BObitmap */

  putint(fp, 40);         /* biSize: size of bitmap info header */
  putint(fp, w);          /* biWidth */
  putint(fp, h);          /* biHeight */
  putshort(fp, 1);        /* biPlanes:  must be '1' */
  putshort(fp, nbits);    /* biBitCount: 1,4,8, or 24 */
  putint(fp, 0);          /* biCompression:  (none) */
  putint(fp, bperlin*h);  /* biSizeImage:  size of raw image data */
  putint(fp, 75 * 39);    /* biXPelsPerMeter: (75dpi * 39" per meter) */
  putint(fp, 75 * 39);    /* biYPelsPerMeter: (75dpi * 39" per meter) */
  putint(fp, nc);         /* biClrUsed: # of colors used in cmap */
  putint(fp, nc);         /* biClrImportant: same as above */

  /* write out the colormap */
  for (i=0; i<nc; i++) {
    putc(bmap[i],fp);
    putc(gmap[i],fp);
    putc(rmap[i],fp);
    putc(0,fp);
  }

  /* write out the image */
  writeBMP8 (fp, pic, w, h);

  if (ferror(fp) || feof(fp)) return -1;
  
  return 0;
}


/*****************************************************************/
/*
 * write_geotiff routine
 *
 * a simple custom, self-contained geotiff writer to support writing
 * *selected* SIR files (Lambert and stereographic only)
 *
 * Note: *very* limited geotiff writing capability
 *
 * written by D.G. Long 12 Feb 2011 at BYU 
 *
 */
/*****************************************************************/


/* write 16 bit integer LSB first */
void fputshort(int w, FILE *fid)
{
  fputc(w & 0xff, fid);
  fputc((w>>8) &0xff,fid);
}

/* write 32 bit integer little endian order */
void fputlong(int w, FILE *fid)
{
  fputc(w & 0xff, fid);
  fputc((w>>8) &0xff,fid);
  fputc((w>>16) &0xff,fid);
  fputc((w>>24) &0xff,fid);
}

/* write double little endian order */
void fputdouble(double w, FILE *fid)
{
  unsigned char *x = (unsigned char *) &w;
  int i;

  if (ISLE)
    for (i=0; i<8; i++)
      fputc(x[i], fid);
  else
    for (i=8; i>0; --i)
      fputc(x[i-1], fid);
}


int write_geotiff(char *filename, unsigned char *byte, int nx, int ny,
		  int proj, double pixscale, double uy, double lx,
		  double proj_org, double proj_lat, double proj_lon,
		  int ColorMap_flag, int *red, int *green, int *blue, int Geotiff)
/* arguments:
  filename = output file name
  byte = 8 bit array to write to tif image
  nx,ny = size of image array
  proj = image projection
      proj==0 for tiff only 
      proj==1 for Lambert projection in geotiff
      proj==2 for polar stereographic projection in geotiff
  uy,lx = coordinates of upper left corner of projection (ignored if proj==0)
  proj_org = projection origin parameter (not used)
  proj_lat, proj_lon = projection parameters (meaning is projection dependent)
  ColorMap_flag = 0, grayscale, =1, use input red,green,blue values (256 values required).
    Black is represented by [0 0 0], and white is [65535 65535 65535]
  Geotiff =1, create geotiff, =0, create conventional tiff w/o geotiff headers
 */
{
  int ImageWidth = nx;
  int ImageLength = ny;

  int bit_depth=8;   /* unsigned 1 byte int, only value supported by this code */  
  int ColorMap=0;    /* set to 0 when not using colormap, set to 256 when using table */  
  int Orientation=1; /* imate orientation 1: Row from Top, Col from Left */

  double ModelPixelScaleTag[3]={0.0, 0.0, 0.0};
  double ModelTiepointTag[6]={0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  int GeoKeyDirectoryTag[19*4];
  char GeoAsciiParamsTag[256]="", str[100]="";  
  int GeoAsciiOffset = 0;
  double GeoDoubleParamsTag[7]={0.0, 0.0 ,0.0 ,0.0 ,0.0, 0.0, 0.0};  
  int GeoDoubleOffset = 0;
  int NumberOfKeys = 0;
  int PhotometricInterpretation = 1;	/* BlackIsZero */
  int Lambert=0; /* set to zero for Lambert, 1 for polar stereographic */
  int num_entry = 15; 
  int ifd_end = 0;
  int totalcnts = 8;
  int SampleFormat = 1; 

  int n, numpix, v; 
  FILE *fid;

  /* printf("write_geotiff %d %d '%s' %d %lf %lf %lf %lf %lf %lf %d\n",nx,ny,filename,proj,pixscale,uy,lx,proj_org,proj_lat,proj_lon,Geotiff); */


  /* generate TIFF header arrays 
     Note: code is specialized for just Lambert and polar stereographic
     of the type used in BYU .SIR files */

  ModelPixelScaleTag[0]=pixscale;
  ModelPixelScaleTag[1]=pixscale;
  ModelTiepointTag[3]=uy;
  ModelTiepointTag[4]=lx;

  if (proj>0)
    Geotiff=1; /* create a geotiff, default is image-only (no projection) */

  if (proj==1)
    Lambert=1;  /* Lambert */
  else
    Lambert=0;  /* polar stereographic */

  if (ColorMap_flag==1) { 
    ColorMap=256; /* only support 8 bit depth */    
    PhotometricInterpretation = 3;
    num_entry++;
  }

  if (Geotiff) {
    num_entry++;    

    /* start list of Geo keys */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 1;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 1;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = NumberOfKeys;

    /* Set GTModelTypeGeoKey to 1, 1024 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 1024;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 1;

    /* Set GTRasterTypeGeoKey to 1, 1025 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 1025;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1; /* RasterPixelIsArea */
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 1;

    /* Set GTCitationGeoKey to 'unnamed', 1026 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 1026;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34737;
    strcpy(str,"unnamed|");  
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = strlen(str);
    strncpy(&GeoAsciiParamsTag[GeoAsciiOffset],str,strlen(str));
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoAsciiOffset;
    GeoAsciiOffset += strlen(str);

    /* Set GeographicTypeGeoKey to WGS84, 2048 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 2048;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 4326; /* GCS_WGS_84 */

    /* Set GeogCitationGeoKey to WGS 84, 2049 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 2049;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34737;
    strcpy(str,"WGS 84|");  
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = strlen(str);
    strncpy(&GeoAsciiParamsTag[GeoAsciiOffset],str,strlen(str));
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoAsciiOffset;
    GeoAsciiOffset += strlen(str);
  
    /* Set GeogAngularUnitsGeoKey to deg, 2054 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 2054;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 9102; /* deg */

    if (Lambert) {
      /* Set GeogSemiMajorAxisGeoKey to value used by SIR, 2057 */
      NumberOfKeys++;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 2057;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
      GeoDoubleParamsTag[GeoDoubleOffset]=6378135.0; /* SIR-standard for Lambert */
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
      GeoDoubleOffset++;
    
      /* Set GeogInvFlatteningGeoKey to value used bySIR, 2059 */
      NumberOfKeys++;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 2059;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
      GeoDoubleParamsTag[GeoDoubleOffset]=298.260; /* SIR-standard for Lambert */
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
      GeoDoubleOffset++;
    
      /* Set GeogPrimeMeridianLongGeoKey to 0, 2061 */
      NumberOfKeys++;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 2061;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
      GeoDoubleParamsTag[GeoDoubleOffset]=0.0;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
      GeoDoubleOffset++;
    }

    /* Set ProjectedCSTypeGeoKey to user, 3072 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3072;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 32767; /* user defined */

    /* Set ProjectionGeoKey to user, 3074 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3074;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 32767; /* user defined */

    /* Set ProjCoordTransGeoKey to appropriate value, 3075 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3075;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    if (Lambert)
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 10; /* Lambert Equal Area */
    else
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 15; /* Polar Stereographic */

    /* Set ProjLinearUnitsGeoKey to m, 3076 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3076;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = 9001;

    if (!Lambert) { /* polar stereographic only */
      /* Set ProjNatOriginLatGeoKey to appropriate value, 3081 */
      NumberOfKeys++;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3081;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
      GeoDoubleParamsTag[GeoDoubleOffset]=proj_lat;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
      GeoDoubleOffset++;
    }
  
    /* Set ProjFalseEastingGeoKey to 0, 3082 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3082;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    GeoDoubleParamsTag[GeoDoubleOffset]=0.0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
    GeoDoubleOffset++;

    /* Set ProjFalseNorthingGeoKey to 0, 3083 */
    NumberOfKeys++;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3083;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
    GeoDoubleParamsTag[GeoDoubleOffset]=0.0;
    GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
    GeoDoubleOffset++;

    if (Lambert) { /* Lambert projection */
      /* Set ProjCenterLongGeoKey to appropriate value, 3088 */
      NumberOfKeys++;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3088;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
      GeoDoubleParamsTag[GeoDoubleOffset]=proj_lon;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
      GeoDoubleOffset++;
    
      /*  Set ProjCenterLatGeoKey to appropriate value, 3089 */
      NumberOfKeys++;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3089;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
      GeoDoubleParamsTag[GeoDoubleOffset]=proj_lat;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
      GeoDoubleOffset++;
    
    } else { /* polar stereographic projection */
      
      /* Set ProjScaleAtNatOriginGeoKey to 1, 3092 */
      NumberOfKeys++;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3092;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
      GeoDoubleParamsTag[GeoDoubleOffset]=1.0;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
      GeoDoubleOffset++;
    
      /* Set ProjStraightVertPoleLongGeoKey to appropriate value, 3095 */
      NumberOfKeys++;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+0] = 3095;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+1] = 34736;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+2] = 1;
      GeoDoubleParamsTag[GeoDoubleOffset]=proj_lon;
      GeoKeyDirectoryTag[(NumberOfKeys-1)*4+3] = GeoDoubleOffset;
      GeoDoubleOffset++;
    }
  
    /* since always using ModelPixelScaleTag and ModelTiepointTag */
    num_entry++; num_entry++;
    if (GeoAsciiOffset > 0)
      num_entry++;
    if (GeoDoubleOffset > 0)
      num_entry++;
  }

   ifd_end = 8 + ((bit_depth/8)*ImageLength*ImageWidth) + 2 + num_entry*12 + 4;

  /* Now ready to write the TIFF file*/

   /* open and start writing file */
   fid = fopen(filename, "wb");   
   if (fid == NULL) {
     fprintf(stderr,"*** Cannot create file '%s'\n", filename);
     return(-1);
   }

   /* write header */
   fwrite("II",1,2,fid); /* little-endian flag */
   fputshort(42,fid);    /* TIFF signature */
   totalcnts += (bit_depth/8)*ImageLength*ImageWidth;   
   fputlong(totalcnts,fid); /* IFD offset */

   /* Write image data (input array already in correct pixel order) */
   numpix=ImageWidth*ImageLength;   
   switch (bit_depth) {       
   case 8: /* only case supported */
     for (n=0; n<numpix; n++) {
       v=byte[n];
       fputc(v & 0xff,fid);
     }
     break;
     /* these cases not supported
       case 16:
       for (n=0; n<numpix; n++) {
       v=byte[n];
       fputshort(v,fid);
       }
       break;       
       case 32:
       for (n=0;n<numpix; n++) {
	 v=byte[n];
	 fputlong(v,fid);
       }
       break;
       */
   default:
     /* fprintf(stderr,"*** this bit_depth %d not supported\n",bit_depth); */
     return(-1);
     break;
   }

   /* TIFF header entries */
   fputshort(num_entry,fid);

   /* Entry 1, size */
   fputshort(256, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(ImageWidth, fid);
   fputshort(0,fid);   

   /* Entry 2, size */
   fputshort(257, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(ImageLength, fid);
   fputshort(0,fid);   

   /* Entry 3, bits per sample */
   fputshort(258, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(bit_depth, fid); /* bits per sample */
   fputshort(0,fid);   

   /* Entry 4, compression  */
   fputshort(259, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(1, fid); /* (uncompressed) */
   fputshort(0,fid);   

   /* Entry 5, PhotometricInterpretation */
   fputshort(262, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(PhotometricInterpretation, fid); /* 3=palette, 1=black is zero */
   fputshort(0,fid);   

   /* Entry 6 StripOffsets*/
   fputshort(273, fid);
   fputshort(4, fid);
   fputlong(ImageLength, fid);
   fputlong(ifd_end, fid);
   ifd_end = ifd_end + ImageLength*4;

   /* Entry 7 Orientation */
   fputshort(274, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(Orientation, fid);
   fputshort(0,fid);

   /* Entry 8 samples per pixels (1)*/
   fputshort(277, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(1, fid);
   fputshort(0,fid);

   /* Entry 9 rows per strip (1)*/
   fputshort(278, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(1, fid);
   fputshort(0,fid);

   /* Entry 10 */
   fputshort(279, fid);
   fputshort(4, fid);
   fputlong(ImageLength, fid);
   fputlong(ifd_end, fid);
   ifd_end = ifd_end + ImageLength*4;

   /* Entry 11 XResolution */
   fputshort(282, fid);
   fputshort(5, fid);
   fputlong(1, fid);
   fputlong(ifd_end, fid);
   ifd_end = ifd_end + 2*4;

   /* Entry 12 YResolution */
   fputshort(283, fid);
   fputshort(5, fid);
   fputlong(1, fid);
   fputlong(ifd_end, fid);
   ifd_end = ifd_end + 2*4;

   /* Entry 13 PlanarConfiguration */
   fputshort(284, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(1, fid);
   fputshort(0,fid);   

   /* Entry 14 ResolutionUnit */
   fputshort(296, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(2, fid);
   fputshort(0,fid);   

   if (ColorMap > 0) {
     fputshort(320, fid);
     fputshort(3, fid);
     fputlong(ColorMap*3, fid);
     fputlong(ifd_end, fid);
     ifd_end = ifd_end + ColorMap*2*3;
   }

   switch (bit_depth) {
   case 8: /* only case supported */
     SampleFormat = 1;
     break;     
   case 16:
     SampleFormat = 2;
     if (ColorMap) SampleFormat=1;     
     break;     
   case 32:
     SampleFormat = 3;
     break;
   }

   /* Entry 15 sampleformat */
   fputshort(339, fid);
   fputshort(3, fid);
   fputlong(1, fid);
   fputshort(SampleFormat, fid);
   fputshort(0,fid);

   if (Geotiff) {   /* geotiff stuff */
       
     /* ModelPixelScaleTag */
     fputshort(33550, fid);
     fputshort(12, fid);
     fputlong(3, fid);
     fputlong(ifd_end, fid);
     ifd_end += 3*8;

     /* ModelTiepointTag */
     fputshort(33922, fid);
     fputshort(12, fid);
     fputlong(6, fid);
     fputlong(ifd_end, fid);
     ifd_end += 6*8;

     /* GeoKeyDirectoryTag */
     fputshort(34735, fid);
     fputshort(3, fid);
     fputlong(4*NumberOfKeys, fid);
     fputlong(ifd_end, fid);
     ifd_end += 2*4*NumberOfKeys;

     if (GeoDoubleOffset > 0) {
       fputshort(34736, fid);  /* GeoDoubleParamsTag */
       fputshort(12, fid);
       fputlong(GeoDoubleOffset, fid);
       fputlong(ifd_end, fid);
       ifd_end += 8*GeoDoubleOffset;     
     }

     if (GeoAsciiOffset > 0) {
       GeoAsciiParamsTag[GeoAsciiOffset++]='\0'; /* NULL string ending */
       fputshort(34737, fid);  /* GeoAsciiParamsTag */
       fputshort(2, fid);
       fputlong(GeoAsciiOffset, fid);
       fputlong(ifd_end, fid);
       ifd_end += GeoAsciiOffset;     
     }
   }
   
   fputlong(0,fid);    /* terminate IFD list */
   fputshort(bit_depth,fid); /* 258 */
   fputshort(0,fid);   /* apparently extra, but needed here */

   for (n=1; n<ImageLength; n++)
     fputlong(n*ImageWidth*(bit_depth/8)+8, fid); /* StripOffsets 273 */
   for (n=0; n<ImageLength; n++)
     fputlong(ImageWidth*(bit_depth/8), fid);     /* StripByteCounts 279 */
   fputlong(96, fid); /* write XResolution (96 dpi) 282 */
   fputlong(1, fid);   
   fputlong(96, fid); /* write YResolution (96 dpi) 283 */
   fputlong(1, fid);   

   if (ColorMap > 0) {      /* color table values 320 */
     for (n=0; n<ColorMap; n++)
       fputshort(red[n],fid);
     for (n=0; n<ColorMap; n++)
       fputshort(green[n],fid);
     for (n=0; n<ColorMap; n++)
       fputshort(blue[n],fid);
   }
   
   if (SampleFormat > 2)
     fputshort(SampleFormat,fid); /* 339 */

   if (Geotiff) {   /* add geotiff arrays */
     for (n=0; n<3; n++)
       fputdouble(ModelPixelScaleTag[n],fid);   /* 33550 */
     for (n=0; n<6; n++)
       fputdouble(ModelTiepointTag[n],fid);     /* 33922 */

     GeoKeyDirectoryTag[3] = NumberOfKeys-1;
     for (n=0; n<NumberOfKeys*4; n++)
       fputshort(GeoKeyDirectoryTag[n],fid);    /* 34735 */   
     for (n=0; n<GeoDoubleOffset; n++)
       fputdouble(GeoDoubleParamsTag[n], fid);  /* 34736 */
     for (n=0; n<GeoAsciiOffset; n++)
       fputc(GeoAsciiParamsTag[n], fid);        /* 34737 */
   }   

   /* close file */
   fclose(fid);
   return(0);
}



