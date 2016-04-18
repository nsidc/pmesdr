/* standard C BYU SIR file format i/o routines

   read_sir_header3
   read_sir_header    (obsolete)
   read_sir_data
   read_sir_data_block
   read_sir_data_byte
   write_sir3
   write_sir   (obsolete)
   write_sir_header

   includes print_head routine, strnc, and swap

   version 2.0 header DGL 19 Dec. 1998
   version 3.0 header DGL 28 Oct. 2000
   modified DGL 9 Feb. 2002 + corrections to read_sir_data_block
   modified DGL 14 Feb. 2002 + corrections to read_sir_data, added
                               sir_update_header routine
   modified DGL 16 Feb. 2002 + fixed write_sir when writing floats w/swap
   modified RDL 5 Jan. 2012 + fixed print_head to display units in GHz
   modified DGL  7 Mar 2004 + added EASE2 grid options

   Written in Ansi C 

   (c) copyright 1998, 2000, 2002, 2014 BYU MERS
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "sir3.h"

/********* standard C sir format file i/o routines ****************/

/* note: read_sir_header and write_sir are for version 2.0 headers and 
   will be phased out.  Use read_sir_header3 and write_sir3 instead. */


#ifdef SWAP
void swap(short *, int);
#endif

/* standard sir file format read routines */
	    
int read_sir_header3(FILE *imf, int *nhead, int *ndes, int *nhtype,
		    int *idatatype, int *nsx, int *nsy, 
		    float *xdeg, float *ydeg, 
		    float *ascale, float *bscale, float *a0, float *b0, 
		    int *ixdeg_off, int *iydeg_off, int *ideg_sc,
		    int *iscale_sc, int *ia0_off, int *ib0_off, int *i0_sc,
		    int *ioff, int *iscale, int *iyear, 
		    int *isday, int *ismin, int *ieday, int *iemin, 
		    int *iregion, int *itype, int *iopt,
		    int *ipol, int *ifreqhm, int *ispare1,
		    float *anodata, float *v_min, float *v_max,
		    char *sensor, char *title, char *type, char *tag,
		    char *crproc, char *crtime, int maxdes, 
		    char *descrip, int *ldes, int maxi, short *iaopt,
		    int *nia)
{
   /* v3 read header of BYU SIR file --- written by D.Long Oct 2000 */

/*  Header consists of a variable number of 512 byte blocks.  The
    block contains scaling information and strings:
    note that character strings may or may not be null terminated in file

    The first block consists of 256 short integers.  Indexing them
    from 1..256 in an array temp, they contain:


	temp(1)	= nsx			! pixels in x direction
	temp(2) = nsy			! pixels in y direction
        temp(3) <= xdeg			! span of x
	temp(4) <= ydeg			! span of y
	temp(5) = nhtype                ! header type (old<15,20,30)
	temp(6) <= ascale               ! x scaling
	temp(7) <= bscale               ! y scaling
	temp(8) <= a0                   ! x (or lon) origin
         note: longitudes should be in the range -180 to 180
	temp(9) <= b0                   ! y (or lat) origin
         note: latitudes should be in the range -90 to 90

     scaling for projection parameters are generally:

        temp(3) = nint((xdeg + float(ixdeg_off)) * float(ideg_sc))
        temp(4) = nint((ydeg + float(iydeg_off)) * float(ideg_sc))
        temp(6) = nint(ascale * float(iscale_sc))
        temp(7) = nint(bscale * float(iscale_sc))
        temp(8) = nint((a0 + float(ia0_off)) * float(i0_sc))
        temp(9) = nint((b0 + float(ib0_off)) * float(i0_sc))

     with the following projection specific exceptions:

	if (iopt.eq.1.or.iopt.eq.2) then		! lambert
           temp(6) = nint(float(iscale_sc)/ascale)
           temp(7) = nint(float(iscale_sc)/bscale)
	if (iopt.eq.11.or.iopt.eq.12.or.iopt.eq.13) then ! EASE grid
           temp(6) = nint(float(iscale_sc)*anint(10.*ascale*
     *                         25.067525/6371.228)*0.05)
           temp(7) = nint(float(iscale_sc)*anint(10.*bscale/
     *                         25.067525)*0.05)

	temp(10) = ioff			! offset to be added to scale val
	temp(11) = iscale		! scale factor ival=(val-ioff)/iscale
	temp(12) = iyear		! year for data used
	temp(13) = isday		! starting JD
	temp(14) = ismin		! time of day for first data (in min)
	temp(15) = ieday		! ending JD
	temp(16) = iemin		! time of day for last data (in min)
	temp(17) = iopt			! projection type
					!  -1 = no projection, image only
					!   0 = rectalinear lat/lon
					!   1 = lambert equal area
					!   2 = lambert equal area (local rad)
					!   5 = polar stereographic
					!   8 = EASE2 north equal area grid
					!   9 = EASE2 south equal area grid
					!  10 = EASE2 cylindrical grid
					!  11 = EASE1 north equal area grid
					!  12 = EASE1 south equal area grid
					!  13 = EASE1 cylindrical grid
	temp(18) = iregion		! region id code
	temp(19) = itype		! image type code
                                       ! some standard values: (0=unknown)
                                       ! 1 = scatterometer A (dB)
                                       ! 2 = scatterometer B (dB/deg)
                                       ! 3 = radiometer Tb (K)
                                       ! 9 = topography (m)
	temp(20)-temp(39) 40 chars of sensor
        temp(40) = iscale_sc            ! ascale/bscale scale factor
        temp(41) = nhead                ! number of 512 byte header blocks
        temp(42) = ndes                 ! number of 512 byte blocks description
        temp(43) = ldes                 ! number of bytes of description
        temp(44) = nia                  ! number of optional integers
        temp(45) = ipol                 ! polarization (0=n/a,1=H,2=V)
        temp(46) = ifreqhm              ! frequency in 100's MHz (0 if n/a)
        temp(47) = ispare1              ! spare
        temp(48) = idatatype            ! data type code 0,2=i*2,1=i*1,4=float

       the value of idata type determines how data is stored and how
       anodata, vmin, and vmax are stored.

       if idatatype = 1 data is stored as bytes (minv=128)
       if idatatype = 2 data is stored as 2 byte integers (minv=32766)
       if idatatype = 4 data is stored as IEEE floating point

       if idatatype = 1,2 anodata,vmin,vmax are stored as 2 byte integers
         in temp(49)..temp(51)  minv, ioff and iscal used to convert
         integers or bytes into floating point values
         nodata, vmin, and vmax must be representable with ioff and iscale
            temp(*) = (value-ioff)*iscale-minv
            value = float(temp(*)+minv)/float(iscale)+ioff
       idatatype=2 is considered the SIR standard format

       if idatatype = f anodata,vmin,vmax are stored as floating points
         in temp(42)..temp(57) and minv, ioff and iscale are ignored here
         and when reading the file.
         floating point numbers are NOT standard across platforms and
         are therefore not recommended

        temp(49) <= anodata           ! value representing no data
        temp(50) <= vmin              ! minimum useful value from creator prg
        temp(51) <= vmax              ! maximum useful value from creator prg
        temp(52,53) = anodata         ! IEEE floating value of no data
        temp(54,55) = vmin            ! IEEE floating minimum useful value
        temp(56,57) = vmax            ! IEEE floating maximum useful value

        temp(58)-temp(126) 150 chars of type
        temp(127) = ixdeg_off         ! xdeg offset
        temp(128) = iydeg_off         ! ydeg offset
        temp(129)-temp(168) 80 chars of title
        temp(169) = ideg_sc           ! xdeg,ydeg scale factor
        temp(170)-temp(189) 40 chars of tag
        temp(190) = ia0_off           ! b0 offset 
        temp(191)-temp(240) 100 chars of crproc
        temp(241) = ib0_off           ! b0 offset 
        temp(242)-temp(255) 28 chars of crtime
        temp(256) = i0_sc             ! a0,b0 scale factor

     optional header blocks:

       ndes header blocks of 512 bytes: chars of description
       nhead-ndes-1 header blocks of 512 bytes: values of iaopt
        by convention, first value iaopt is a code telling how to interpret
        the rest of the array if nia>0.  Usage of additional blocks is
        user dependent and non-standard.

       remainder of file is image data in a multiple of 512 byte blocks

       one,two byte integer scaling (idatatype=1,2) is
          intval = (fvalue-ioff)*iscale-minv
          fvalue = float(intval+minv)/float(iscale)+ioff
       no scaling of float values for (idatatype=4)

*/

   short in, temp[256];
   int nch,i;
   float soff;

   union {
     short i2[2];
     float f2;
   } un;

   rewind(imf);  /* make sure to be start of file */

/* read first header block */

   if (fread(temp, sizeof(short), 256, imf) == 0) return(-1);
#ifdef SWAP
   swap(temp, 256);
#endif

/* decode nsx */

   *nsx = temp[0];
   *nsy = temp[1];
   *iopt  = temp[16];
   *nhtype = temp[4];

   if (*nhtype < 20) *nhtype=1;

   if (*nhtype < 30) {  /* set version 3.0 parameters to default version 2.0 values */

     switch (*iopt){
     case -1: /* image only */
       *ideg_sc=10;
       *iscale_sc=1000;
       *i0_sc=100;
       *ixdeg_off=0;
       *iydeg_off=0;
       *ia0_off=0;
       *ib0_off=0;
       break;
     case 0: /* rectalinear lat/lon */
       *ideg_sc=100;
       *iscale_sc=1000;
       *i0_sc=100;
       *ixdeg_off=-100;
       *iydeg_off=0;
       *ia0_off=0;
       *ib0_off=0;
       break;
     case 1: /* lambert */
     case 2:
       *ideg_sc=100;
       *iscale_sc=1000;
       *i0_sc=1;
       *ixdeg_off=0;
       *iydeg_off=0;
       *ia0_off=0;
       *ib0_off=0;
       break;
     case 5: /* polar stereographic */
       *ideg_sc=100;
       *iscale_sc=100;
       *i0_sc=1;
       *ixdeg_off=-100;
       *iydeg_off=0;
       *ia0_off=0;
       *ib0_off=0;
       break;
     case 8: /* EASE2 grid */
     case 9:
     case 10:
       *ideg_sc=10;
       *iscale_sc=100;
       *i0_sc=10;
       *ixdeg_off=0;
       *iydeg_off=0;
       *ia0_off=0;
       *ib0_off=0;
       break;
     case 11: /* EASE1 grid */
     case 12:
     case 13:
       *ideg_sc=10;
       *iscale_sc=1000;
       *i0_sc=10;
       *ixdeg_off=0;
       *iydeg_off=0;
       *ia0_off=0;
       *ib0_off=0;
       break;
     default: /* unknown */
       *ideg_sc=100;
       *iscale_sc=1000;
       *i0_sc=100;
       *ixdeg_off=0;
       *iydeg_off=0;
       *ia0_off=0;
       *ib0_off=0;
     }

   } else {  /* get projection offset and scale factors from file */
     
     *iscale_sc = temp[ 39];
     *ixdeg_off = temp[126];
     *iydeg_off = temp[127];
     *ideg_sc   = temp[168];
     *ia0_off   = temp[189];
     *ib0_off   = temp[240];
     *i0_sc     = temp[255];

   }

   /* decode default projection parameters */

   *xdeg = temp[2] / (float) *ideg_sc - *ixdeg_off;
   *ydeg = temp[3] / (float) *ideg_sc - *iydeg_off;
   *ascale = temp[5] / (float) *iscale_sc;
   *bscale = temp[6] / (float) *iscale_sc;
   *a0 = temp[7] / (float) *i0_sc - *ia0_off;
   *b0 = temp[8] / (float) *i0_sc - *ib0_off;

   /* handle special cases which depend on transformation option */

   switch (*iopt){
   case -1: /* image only */
     break;
   case 0:  /* rectalinear lat/lon */
     break;
   case 1:  /* lambert */
   case 2:
     *ascale = (float) *iscale_sc / (float) temp[5];
     *bscale = (float) *iscale_sc / (float) temp[6];
     break;
   case 5:  /* polar stereographic */
     break;
   case 8: /* EASE2 grid */
   case 9:
   case 10:
     break;     
   case 11: /* EASE1 grid */
   case 12:
   case 13:
     *ascale = 2.0L*((double) temp[5] / (double) *iscale_sc) * 6371.228L/25.067525L;
     *bscale = 2.0L*((double) temp[6] / (double) *iscale_sc) * 25.067525L;
     break;
   default:
     fprintf(stderr,"\n *** Unrecognized SIR option in read_sir_header3 %d ***\n",*iopt);
   }


   *ioff = temp[9];
   *iscale = temp[10];
   if (*iscale == 0) *iscale=1;

   *iyear = temp[11];
   *isday = temp[12];
   *ismin = temp[13];
   *ieday = temp[14];
   *iemin = temp[15];

   *iregion = temp[17];
   *itype = temp[18];

   *nhead = temp[40];
   if (*nhead == 0) *nhead=1;
   *ndes = temp[41];
   *ldes = temp[42];
   *nia = temp[43];
   *ipol = temp[44];
   *ifreqhm = temp[45];
   *ispare1 = temp[46];
   *idatatype = temp[47];
   if (*idatatype == 0) *idatatype = 2;

   if (*iscale == 0) *iscale=1;
   soff = 32767.0/(float) *iscale;
   if (*idatatype == 1) soff = 128.0/(float) *iscale;

   *anodata = (float) temp[48]/(float) *iscale + *ioff + soff;
   *v_min = (float) temp[49]/(float) *iscale + *ioff + soff;
   *v_max = (float) temp[50]/(float) *iscale + *ioff + soff;

   if (*idatatype == 4) {

#ifdef SWAP
     un.i2[1]=temp[51];
     un.i2[0]=temp[52];
     *anodata = un.f2;
     un.i2[1]=temp[53];
     un.i2[0]=temp[54];
     *v_min = un.f2;
     un.i2[1]=temp[55];
     un.i2[0]=temp[56];
     *v_max = un.f2;
#else
     un.i2[0]=temp[51];
     un.i2[1]=temp[52];
     *anodata = un.f2;
     un.i2[0]=temp[53];
     un.i2[1]=temp[54];
     *v_min = un.f2;
     un.i2[0]=temp[55];
     un.i2[1]=temp[56];
     *v_max = un.f2;
#endif

   }
   
   for (in = 0; in < 20; in++) {
      sensor[2*in] = temp[19+in] % 256;
      sensor[2*in+1] = temp[19+in]/256; 
   }
   sensor[40] = '\0';

   for (in = 0; in < 69; in++) {
      type[2*in] = temp[57+in] % 256;
      type[2*in+1] = temp[57+in]/256;
   }
   type[138] = '\0';

   for (in = 0; in < 40; in++) {
      title[2*in] = temp[128+in] % 256;
      title[2*in+1] = temp[128+in]/256;
   }
   title[80] = '\0';

   for (in = 0; in < 20; in++) {
      tag[2*in] = temp[169+in] % 256;
      tag[2*in+1] = temp[169+in]/256;
   }
   tag[40] = '\0';

   for (in = 0; in < 50; in++) {
      crproc[2*in] = temp[190+in] % 256;
      crproc[2*in+1] = temp[190+in]/256;
   }
   crproc[100] = '\0';

   for (in = 0; in < 14; in++) {
      crtime[2*in] = temp[241+in] % 256;
      crtime[2*in+1] = temp[241+in]/256;
   }
   crtime[28] = '\0';

   if (*nhtype == 1) {  /* a really old style header */
     *nhead=1;
     *ndes=0;
     *ldes=0;
     *nia=0;
     type[0]='\0';
     tag[0]='\0';
     crproc[0]='\0';
     crtime[0]='\0';
     *nhtype=20;   /* upgrade style on read */
   }

   if (*nhead > 1) {   /* read additional header blocks */
      if (*ndes > 0) {
	if (*ldes >= maxdes)
	  fprintf(stderr,"*** read_sir_head3 warning: file description too short (needed: %d avail: %d)\n",*ldes, maxdes);

	 fseek(imf, 512, REL_BEGIN);
	 nch=(maxdes < *ldes ? maxdes : *ldes);
	 nch=0;
	 for (i = 0; i < *ndes; i++) {
	   if (fread(temp, sizeof(short), 256, imf) == 0) return(-1);
#ifdef SWAP
	   swap(temp, 256);
#endif
	   for (in = 0; in < 256; in++) {
	     if (nch < maxdes) descrip[nch] = temp[in] % 256;
	     nch++;
	     if (nch < maxdes) descrip[nch] = temp[in]/256;
	     nch++;
	   }
	 }
      }
      if (maxdes > 0) descrip[maxdes-1] = '\0';

      if (*nhead-*ndes-1 > 0) {
	 fseek(imf, 512*(*ndes+1), REL_BEGIN);
	 if (*nia >= maxi)
	   fprintf(stderr,"*** read_sir_head3 warning: header extra ints too short (needed: %d avail: %d)\n",*nia, maxi);

	 nch=(maxi < *nia ? maxi : *nia);
	 if (fread(iaopt, sizeof(short), nch, imf) == 0) return(-1);
#ifdef SWAP
	 swap(iaopt, nch);
#endif
      }
   }

   return(0);
}



int read_sir_data(FILE *imf, int nhead, int idatatype, 
		  int nsx, int nsy, int ioff, int iscale, float *stval)
{
  /* routine to read in SIR file image data into float array
     in memory.  This preserves the full resolution of the data. 
       . file imf must already be open 
       . file imf is closed after read */

  float s, soff, amag;
  int i,j,k;
  char *b;
  int dumb;
  
  short *a;

#ifdef SWAP
  union 
  {
    short i2[2];
    float f2;
  } un;
#endif
  
  /* create temporary work array to read file in one row at a time*/
  a = (short *) malloc(sizeof(short)*nsx*2);
  if (a == NULL) {
     fprintf(stderr,"*** ERROR: memory allocation failure...\n");
     fclose(imf);
     return(-1);
  }
  b = (char *) a;

  s = 1.0 / ((float) iscale);
  soff = 32767.0/ ((float) iscale) + ((float) ioff);
  if (idatatype == 1) soff = 128.0/ ((float) iscale) + ((float) ioff);

  fseek(imf,SIR_HEADER_SIZE*nhead,REL_BEGIN);

  k = 0;
  for (i = 0; i < nsy; i++) {

    switch(idatatype) {
    case 1:     /* bytes */
      if (fread(b, sizeof(char), nsx, imf) != nsx) {
	fclose(imf);
	return(-1);
      }
      for (j = 0; j < nsx; j++){
	amag = ((float) b[j]) * s + soff; /* pixel value */
	*(stval+k+j) = amag;              /* floating point array */
      }
      break;
      
    case 4:     /* floats */
#ifdef SWAP
      dumb = fread((char *)a, sizeof(short), 2*nsx, imf) ;
      swap(a,nsx);
      for (j = 0; j < nsx; j++){
	un.i2[1]=a[j*2];
	un.i2[0]=a[j*2+1];
	amag = un.f2;
	*(stval+k+j) = amag;              /* floating point array */
      }
#else
      dumb = fread((char *) (stval+k), sizeof(float), nsx, imf);
#endif
      break;
      
    default:    /* 0 or 2: read two byte integers */
      dumb = fread((char *)a, sizeof(short), nsx, imf) ;
#ifdef SWAP	
      swap(a,nsx);
#endif
      for (j = 0; j < nsx; j++){
	amag = ((float) a[j]) * s + soff; /* pixel value */
	*(stval+k+j) = amag;              /* floating point array */
      }
    }
    k = k + nsx;
  }

  free(a);
  fclose(imf);

  return(0);
}


int read_sir_data_block(FILE *imf, int nhead, int idatatype, 
			int nsx, int nsy, int ioff, int iscale, 
			int x1, int y1, int x2, int y2,
			float *stval)
{
  /* routine to read a selected block of data from a SIR file image data 
     into float array
       . file imf must already be open 
       . file imf is NOT closed after read 
     block is defined by the lower-left corner (x1,y1) and
     the upper-right corner (x2,y2) where 0 < x <= nsx and
     0 < y <= nsy and x2>=x1 and y2>=y1

     The output image has (x2-x1+1) by (y2-y1+1) pixels indexed according to
     image_index = (y-1) * (x2-x1+1) + x where 0 < y <= (y2-y1+1) and
     0 < x <= (x2-x1+1) and 1 < image_index <= (x2-x1+1) * (y2-y1+1).
     Obviously, it must be true that x2>=x1 and y2>=y1

     returns -5 on an input error, -1 on a file read, seek or other error.
*/

  float s, soff, amag;
  int i,j,k;
  int nx = x2-x1+1, ny = y2 - y1 + 1;
  char *b;
  short *a;

#ifdef SWAP
  union 
  {
    short i2[2];
    float f2;
  } un;
#endif

  if (x1 < 1 || y1 < 1 || nx < 1 || ny < 1 || x2 > nsx || y2 > nsy)
    return(-5);
  if (idatatype == 0) idatatype = 2;
    
  /* create temporary work array to read file in one row at a time*/
  a = (short *) malloc(sizeof(short)*nx*2);
  if (a == NULL) {
     fprintf(stderr,"*** ERROR: memory allocation failure...\n");
     fclose(imf);
     return(-1);
  }
  b = (char *) a;
  
  s = 1.0 / ((float) iscale);
  soff = 32767.0/ ((float) iscale) + ((float) ioff);
  if (idatatype == 1) soff = 128.0/ ((float) iscale) + ((float) ioff);

  k = 0;
  for (i = y1; i <= y2; i++) {

    fseek(imf, SIR_HEADER_SIZE * nhead + 
	  idatatype * ( (i-1)*nsx + (x1-1)), REL_BEGIN);

    switch(idatatype) {
    case 1:     /* bytes */
      if (fread(b, sizeof(char), nx, imf) != nx) {
	fclose(imf);
	return(-1);
      }
      for (j = 0; j < nx; j++){
	amag = ((float) b[j]) * s + soff; /* pixel value */
	*(stval+k+j) = amag;              /* floating point array */
      }
      break;
      
    case 4:     /* floats */
#ifdef SWAP
      if (fread((char *)a, sizeof(short), 2*nx, imf) != 2*nx) {
	fclose(imf);
	return(-1);
      }
      swap(a,nx);
      for (j = 0; j < nx; j++){
	un.i2[1]=a[j*2];
	un.i2[0]=a[j*2+1];
	amag = un.f2;
	*(stval+k+j) = amag;              /* floating point array */
      }
#else
      if (fread((char *) (stval+k), sizeof(float), nx, imf) != nx) {
	fclose(imf);	
	return(-1);
      }
      
#endif
      break;
      
    default:    /* 0 or 2: read two byte integers */
      if (fread((char *)a, sizeof(short), nx, imf) != nx) {
	fclose(imf);
	return(-1);
      }
#ifdef SWAP	
      swap(a,nx);
#endif
      for (j = 0; j < nx; j++){
	amag = ((float) a[j]) * s + soff; /* pixel value */
	*(stval+k+j) = amag;              /* floating point array */
      }
    }
    k = k + nx;
  }

  free(a);

  return(0);
}


int read_sir_data_byte(FILE *imf, int nhead, int idatatype, 
		       int nsx, int nsy, int ioff, int iscale, 
		       float smin, float smax, int bmin, int bmax,
		       char *out)
{
  /* routine to read in SIR file image data into a char
     in memory where user specifies scaling of float value into 
     a byte value.  Note: byte representation reduces the data range 
     and introduces quantization errors.  Use with caution
       . file imf must already be open 
       . file imf is closed after read */

  float s, soff, amag, bscale;
  int i, j, k, imag;

  int dumb;
  
  short *a;

#ifdef SWAP
  union 
  {
    short i2[2];
    float f2;
  } un;
#endif
  
  /* create temporary work array to read file in one row at a time*/
  a = (short *) malloc(sizeof(short)*nsx*2);
  if (a == NULL) {
     fprintf(stderr,"*** ERROR: memory allocation failure...\n");
     fclose(imf);
     return(-1);
  }

  s = 1.0 / ((float) iscale);
  soff = 32767.0/ ((float) iscale) + ((float) ioff);
  if (idatatype == 1) soff = 128.0/ ((float) iscale) + ((float) ioff);

  if (smax - smin > 0.0)
    bscale = (bmax - bmin)/(smax-smin);
  else
    bscale = 1.0;
  if (bmin < 0) bmin=0;
  if (bmax+bmin > 255) bmax=255-bmin;
  
  fseek(imf,SIR_HEADER_SIZE*nhead,REL_BEGIN);

  k = 0;
  for (i = 0; i < nsy; i++) {

    switch(idatatype) {
    case 1:     /* bytes */
      if (fread((char *)a, sizeof(char), nsx, imf) != nsx) {
	fclose(imf);
	return(-1);
      }
      for (j = 0; j < nsx; j++){
	amag = ((float) a[j]) * s + soff; /* floating point value */
	if (amag > smax) amag = smax;
	if (amag < smin) amag = smin;
	amag = bscale * (amag - smin);
	imag = (int) amag;
	if (imag < bmin) imag = bmin;
	if (imag > bmax) imag = bmax;
	*(out+k+j) = imag;
      }
      break;
      
    case 4:     /* floats */
#ifdef SWAP
      dumb = fread((char *)a, sizeof(short), 2*nsx, imf) ;
      swap(a,nsx);
      for (j = 0; j < nsx; j++){
	un.i2[1]=a[j*2];
	un.i2[0]=a[j*2+1];
	amag = un.f2;              /* floating point value */
	if (amag > smax) amag = smax;
	if (amag < smin) amag = smin;
	amag = bscale * (amag - smin);
	imag = (int) amag;
	if (imag < bmin) imag = bmin;
	if (imag > bmax) imag = bmax;
	*(out+k+j) = imag;
      }
#else
      dumb = fread((char *)a, sizeof(float), nsx, imf);
      for (j = 0; j < nsx; j++){
	amag = *(((float *) a) + j);              /* floating point value */
	if (amag > smax) amag = smax;
	if (amag < smin) amag = smin;
	amag = bscale * (amag - smin);
	imag = (int) amag;
	if (imag < bmin) imag = bmin;
	if (imag > bmax) imag = bmax;
	*(out+k+j) = imag;
      }
#endif
      break;
      
    default:    /* 0 or 2: read two byte integers */
      dumb = fread((char *)a, sizeof(short), nsx, imf) ;
#ifdef SWAP	
      swap(a,nsx);
#endif
      for (j = 0; j < nsx; j++){
	amag = ((float) a[j]) * s + soff; /* floatint point value */
	if (amag > smax) amag = smax;
	if (amag < smin) amag = smin;
	amag = bscale * (amag - smin);
	imag = (int) amag;
	if (imag < bmin) imag = bmin;
	if (imag > bmax) imag = bmax;
	*(out+k+j) = imag;
      }
    }
    k = k + nsx;
  }

  free(a);
  fclose(imf);

  return(0);  
}


int read_sir_header(FILE *imf, int *nhead, int *ndes, int *nhtype,
		    int *idatatype, int *nsx, int *nsy, 
		    float *xdeg, float *ydeg, 
		    float *ascale, float *bscale, float *a0, float *b0, 
		    int *ioff, int *iscale, int *iyear, 
		    int *isday, int *ismin, int *ieday, int *iemin, 
		    int *iregion, int *itype, int *iopt,
		    int *ipol, int *ifreqhm, int *ispare1,
		    float *anodata, float *v_min, float *v_max,
		    char *sensor, char *title, char *type, char *tag,
		    char *crproc, char *crtime, int maxdes, 
		    char *descrip, int *ldes, int maxi, short *iaopt,
		    int *nia)
{
  int ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc;
  int err;
  
  err = read_sir_header3(imf, nhead, ndes, nhtype,
			  idatatype, nsx, nsy, xdeg, ydeg, 
			  ascale, bscale, a0, b0, 
			  &ixdeg_off, &iydeg_off, &ideg_sc,
			  &iscale_sc, &ia0_off, &ib0_off, &i0_sc,
			  ioff, iscale, iyear, 
			  isday, ismin, ieday, iemin, 
			  iregion, itype, iopt,
			  ipol, ifreqhm, ispare1,
			  anodata, v_min, v_max,
			  sensor, title, type, tag,
			  crproc, crtime, maxdes, 
			  descrip, ldes, maxi, iaopt, nia);
  if (*nhtype > 30)
    fprintf(stderr,"*** read_sir_head warning: header type %d Use read_sir_header3 to avoid loss of info\n",*nhtype);
  
  return(err);
}





#define rnd(a) ((a) >= 0 ? floor((a)+0.5L) : ceil((a)-0.5L))

/* standard sir file format write routines */

#define BUFFER 6000           /* space in buffer */ 

int write_sir_header3(FILE *imf, int *nhead, int nhtype,
		      int idatatype, int nsx, int nsy, 
		      float xdeg, float ydeg, 
		      float ascale, float bscale, float a0, float b0, 
		      int ixdeg_off, int iydeg_off, int ideg_sc,
		      int iscale_sc, int ia0_off, int ib0_off, int i0_sc,
		      int ioff, int iscale, int iyear, 
		      int isday, int ismin, int ieday, int iemin, 
		      int iregion, int itype, int iopt,
		      int ipol, int ifreqhm, int ispare1,
		      float anodata, float v_min, float v_max,
		      char *sensor, char *title, char *type, char *tag,
		      char *crproc, char *crtime, 
		      char *descrip, int ldes, short *iaopt, int nia);

int write_sir3(char *fname, float *stval, int *nhead, int nhtype,
	       int idatatype, int nsx, int nsy, 
	       float xdeg, float ydeg, 
	       float ascale, float bscale, float a0, float b0, 
	       int ixdeg_off, int iydeg_off, int ideg_sc,
	       int iscale_sc, int ia0_off, int ib0_off, int i0_sc,
	       int ioff, int iscale, int iyear, 
	       int isday, int ismin, int ieday, int iemin, 
	       int iregion, int itype, int iopt,
	       int ipol, int ifreqhm, int ispare1,
	       float anodata, float v_min, float v_max,
	       char *sensor, char *title, char *type, char *tag,
	       char *crproc, char *crtime, 
	       char *descrip, int ldes, short *iaopt, int nia)
{
  /* see write_sir_header3 for details of SIR file format and variable values */

   FILE *imf;

   short a[BUFFER];
   int i,j,k;
   long kk,minv,maxv;
   float s, soff, am;
   int iunder = 0, iover = 0;

#ifdef SWAP
   union 
   {
     short i2[2];
     float f2;
   } un;
#endif

   if (strlen(fname) < 1) return(0);  /* don't output empty file */

   imf = fopen(fname,"wb");
   if (imf == NULL) {
      fprintf(stderr,"*** Error opening file '%s'\n",fname);      
      return(-1);
   }
   
   if (iscale == 0) iscale=1;
   s = (float) iscale;
   soff = (float) ioff;
   minv = 32767;
   maxv = 32767;
   if (idatatype == 0) idatatype = 2;
   if (idatatype == 1) {
     minv = 128;
     maxv = 127;
   }

   am = anodata;

   if (nsx < 1 || nsx > 32667 || nsy < 1 || nsx > 32667) {
     fprintf(stderr,"*** write_sir3 invalid image size error %d %d\n",nsx,nsy);      
     return(-1);
   }
   
   /* write SIR image header information */

   if (write_sir_header3(imf, nhead, nhtype, idatatype, nsx, nsy, 
			 xdeg, ydeg, ascale, bscale, a0, b0, 
			 ixdeg_off, iydeg_off, ideg_sc,
			 iscale_sc, ia0_off, ib0_off, i0_sc,
			 ioff, iscale, iyear, isday, ismin, ieday, iemin, 
			 iregion, itype, iopt, ipol, ifreqhm, ispare1,
			 anodata, v_min, v_max, sensor, title, type, tag,
			 crproc, crtime, descrip, ldes, iaopt, nia)) {
     fprintf(stderr,"*** Error writing file header '%s'\n",fname); 
     fclose(imf);
     return(-1);
   }

   fseek(imf, SIR_HEADER_SIZE * *nhead, REL_BEGIN);

   /* write image portion of SIR file, converting input float values to appropriate
      one or two byte integers or floats */

   k = 0;
   for (i=0; i < nsy; i++) {
     switch(idatatype){             /* bytes */
     case 1:
       for (j=0; j < nsx; j++) {
	 am = *(stval+k);
	 if (am > 1e23) {
	   fprintf(stderr,"*** write_sir3: Overvalue pixel: %f at %d %d %d\n",am,i,j,k);
	   am = (maxv / s) + soff + 1;
	 }
	 if (am < -1e23) {
	   fprintf(stderr,"*** write_sir3: Undervalue pixel: %f at %d %d %d\n",am,i,j,k);
	   am = (minv / s) + soff - 1;
	 }
	 kk = rnd((am - soff) * s);
	 kk = kk - minv;
	 if (kk < -minv) {
	   kk = -minv;
	   iunder++;
	 }
	 if (kk > maxv) {
	   kk = maxv;
	   iover++;
	 }
	 ((char *) a)[j] = kk;
	 k++;
       }
       if (fwrite((char *) a, sizeof(char), nsx, imf) != nsx) {
	 fprintf(stderr,"*** Error writing data in write_sir3\n");
	 fclose(imf);
	 return(-2);
       }
       break;

     case 4:                      /* floats */
#ifdef SWAP
       for (j = 0; j < nsx; j++){
	 un.f2 = *(stval+k);
	 a[2*j]=un.i2[1];
	 a[2*j+1]=un.i2[0];
	 k++;
       }
       swap(a,2*nsx);
       if (fwrite((char *) a, sizeof(short), 2*nsx, imf) != 2*nsx) {
	 fprintf(stderr,"*** Error writing data in write_sir\n");
	 fclose(imf);
	 return(-2);
       }
#else
       if (fwrite((char *) (stval+k), sizeof(float), nsx, imf) != nsx) {
	 fprintf(stderr,"*** Error writing data in write_sir\n");
	 fclose(imf);
	 return(-2);
       }
#endif
       break;

     default:                    /* short ints - this is the major mode*/
       for (j=0; j < nsx; j++) {
	 am = *(stval+k);
	 if (am > 1e23) {
	   printf("*** write_sir3: Oversize pixel value %f %d %d %d\n",am,i,j,k);
	   am = (maxv / s) + soff + 1;
	 }
	 if (am < -1e23) {
	   printf("*** write_sir3: Undersize pixel value %f %d %d %d\n",am,i,j,k);
	   am = (minv / s) + soff - 1;
	 }
	 kk = rnd((am - soff) * s);
	 kk = kk - minv;
	 if (kk < -minv) {
	   kk = -minv;
	   iunder++;
	 }
	 if (kk > maxv) {
	   kk = maxv;
	   iover++;
	 }
	 a[j] = kk;
	 k++;
       }
#ifdef SWAP
       swap(a,nsx);
#endif
       if (fwrite((char *) a, sizeof(short), nsx, imf) != nsx) {
	 fprintf(stderr,"*** Error writing data in write_sir\n");
	 fclose(imf);
	 return(-2);
       }
       break;
     }
   }

/* add zero pad to ensure file is a multiple of 512 bytes long */

   switch(idatatype) {
   case 1:
     j=512;
     kk=1;
     break;
   case 4:
     j=128;
     kk=4;
     break;
   default:
     j=256;
     kk=2;
   }
   
   if (k % j != 0) {
      i = 512 - kk * (k % j);
      for (j = 0; j < i; j++)
	 a[j] = 0;
      if (fwrite((char *) a, sizeof(char), i, imf) != i) {
	 fprintf(stderr,"*** Error writing zero pad data\n");
	 fclose(imf);
	 return(-1);
      }
   }
	 
   fclose(imf);

   /*   fprintf(stdout,"File successfully written in write_sir\n"); */

   if  (iunder != 0 || iover != 0) 
     fprintf(stdout," write_sir Underflows: %d  Overflows: %d  Count: %d\n",iunder,iover,nsx*nsy);
   return(0);
}

	    
int write_sir_header3(FILE *imf, int *nhead, int nhtype, int idatatype, 
		      int nsx, int nsy, 
		      float xdeg, float ydeg, 
		      float ascale, float bscale, float a0, float b0,
		      int ixdeg_off, int iydeg_off, int ideg_sc,
		      int iscale_sc, int ia0_off, int ib0_off, int i0_sc,
		      int ioff, int iscale, int iyear, 
		      int isday, int ismin, int ieday, int iemin, 
		      int iregion, int itype, int iopt,
		      int ipol, int ifreqhm, int ispare1,
		      float anodata, float v_min, float v_max,
		      char *sensor, char *title, char *type, char *tag,
		      char *crproc, char *crtime, 
		      char *descrip, int ldes, short *iaopt, int nia)
{

   /* v3 read header of BYU SIR file --- written by D.Long Oct 2000 */

/*  Header consists of a variable number of 512 byte blocks.  The
    block contains scaling information and strings:
    note that character strings may or may not be null terminated in file

    The first block consists of 256 short integers.  Indexing them
    from 1..256 in an array temp, they contain:


	temp(1)	= nsx			! pixels in x direction
	temp(2) = nsy			! pixels in y direction
        temp(3) <= xdeg			! span of x
	temp(4) <= ydeg			! span of y
	temp(5) = nhtype                ! header type (old<15,20,30)
	temp(6) <= ascale               ! x scaling
	temp(7) <= bscale               ! y scaling
	temp(8) <= a0                   ! x (or lon) origin
         note: longitudes should be in the range -180 to 180
	temp(9) <= b0                   ! y (or lat) origin
         note: latitudes should be in the range -90 to 90

     scaling for projection parameters are generally:

        temp(3) = nint((xdeg + float(ixdeg_off)) * float(ideg_sc))
        temp(4) = nint((ydeg + float(iydeg_off)) * float(ideg_sc))
        temp(6) = nint(ascale * float(iscale_sc))
        temp(7) = nint(bscale * float(iscale_sc))
        temp(8) = nint((a0 + float(ia0_off)) * float(i0_sc))
        temp(9) = nint((b0 + float(ib0_off)) * float(i0_sc))

     with the following projection specific exceptions:

	if (iopt.eq.1.or.iopt.eq.2) then		! lambert
           temp(6) = nint(float(iscale_sc)/ascale)
           temp(7) = nint(float(iscale_sc)/bscale)
	if (iopt.eq.11.or.iopt.eq.12.or.iopt.eq.13) then ! EASE1 grid
           temp(6) = nint(float(iscale_sc)*anint(10.*ascale*
     *                         25.067525/6371.228)*0.05)
           temp(7) = nint(float(iscale_sc)*anint(10.*bscale/
     *                         25.067525)*0.05)

	temp(10) = ioff			! offset to be added to scale val
	temp(11) = iscale		! scale factor ival=(val-ioff)/iscale
	temp(12) = iyear		! year for data used
	temp(13) = isday		! starting JD
	temp(14) = ismin		! time of day for first data (in min)
	temp(15) = ieday		! ending JD
	temp(16) = iemin		! time of day for last data (in min)
	temp(17) = iopt			! projection type
					!  -1 = no projection, image only
					!   0 = rectalinear lat/lon
					!   1 = lambert equal area
					!   2 = lambert equal area (local rad)
					!   5 = polar stereographic
					!   8 = EASE2 north equal area grid
					!   9 = EASE2 south equal area grid
					!  10 = EASE2 cylindrical grid
					!  11 = EASE1 north equal area grid
					!  12 = EASE1 south equal area grid
					!  13 = EASE1 cylindrical grid
	temp(18) = iregion		! region id code
	temp(19) = itype		! image type code
                                       ! some standard values: (0=unknown)
                                       ! 1 = scatterometer A (dB)
                                       ! 2 = scatterometer B (dB/deg)
                                       ! 3 = radiometer Tb (K)
                                       ! 9 = topography (m)
	temp(20)-temp(39) 40 chars of sensor
        temp(40) = iscale_sc            ! ascale/bscale scale factor
        temp(41) = nhead                ! number of 512 byte header blocks
        temp(42) = ndes                 ! number of 512 byte blocks description
        temp(43) = ldes                 ! number of bytes of description
        temp(44) = nia                  ! number of optional integers
        temp(45) = ipol                 ! polarization (0=n/a,1=H,2=V)
        temp(46) = ifreqhm              ! frequency in 100's MHz (0 if n/a)
        temp(47) = ispare1              ! spare
        temp(48) = idatatype            ! data type code 0,2=i*2,1=i*1,4=float

       the value of idata type determines how data is stored and how
       anodata, vmin, and vmax are stored.

       if idatatype = 1 data is stored as bytes (minv=128)
       if idatatype = 2 data is stored as 2 byte integers (minv=32766)
       if idatatype = 4 data is stored as IEEE floating point

       if idatatype = 1,2 anodata,vmin,vmax are stored as 2 byte integers
         in temp(49)..temp(51)  minv, ioff and iscal used to convert
         integers or bytes into floating point values
         nodata, vmin, and vmax must be representable with ioff and iscale
            temp(*) = (value-ioff)*iscale-minv
            value = float(temp(*)+minv)/float(iscale)+ioff
       idatatype=2 is considered the SIR standard format

       if idatatype = f anodata,vmin,vmax are stored as floating points
         in temp(42)..temp(57) and minv, ioff and iscale are ignored here
         and when reading the file.
         floating point numbers are NOT standard across platforms and
         are therefore not recommended

        temp(49) <= anodata           ! value representing no data
        temp(50) <= vmin              ! minimum useful value from creator prg
        temp(51) <= vmax              ! maximum useful value from creator prg
        temp(52,53) = anodata         ! IEEE floating value of no data
        temp(54,55) = vmin            ! IEEE floating minimum useful value
        temp(56,57) = vmax            ! IEEE floating maximum useful value

        temp(58)-temp(126) 150 chars of type
        temp(127) = ixdeg_off         ! xdeg offset
        temp(128) = iydeg_off         ! ydeg offset
        temp(129)-temp(168) 80 chars of title
        temp(169) = ideg_sc           ! xdeg,ydeg scale factor
        temp(170)-temp(189) 40 chars of tag
        temp(190) = ia0_off           ! b0 offset 
        temp(191)-temp(240) 100 chars of crproc
        temp(241) = ib0_off           ! b0 offset 
        temp(242)-temp(255) 28 chars of crtime
        temp(256) = i0_sc             ! a0,b0 scale factor

     optional header blocks:

       ndes header blocks of 512 bytes: chars of description
       nhead-ndes-1 header blocks of 512 bytes: values of iaopt
        by convention, first value iaopt is a code telling how to interpret
        the rest of the array if nia>0.  Usage of additional blocks is
        user dependent and non-standard.

       remainder of file is image data in a multiple of 512 byte blocks

       one,two byte integer scaling (idatatype=1,2) is
          intval = (fvalue-ioff)*iscale-minv
          fvalue = float(intval+minv)/float(iscale)+ioff
       no scaling of float values for (idatatype=4)

*/

   short in, temp[256];
   int i,j;
   long kk,minv,maxv;
   float s,soff,am;
   int ndes;
   
   union 
     {
       short i2[2];
       float f2;
   } un;

   if (nhtype < 30 ||   /* set default projection and offset parameters */
       ideg_sc < 0.0 || iscale_sc < 0 || i0_sc < 0) {
     
     if (nhtype >= 30 &&   /* set default projection and offset parameters */
	 (ideg_sc < 0.0 || iscale_sc < 0 || i0_sc < 0))
       fprintf(stderr,"*** write_sir3 projection scale error %d %d %d %d %d %d %d\n",
	       ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc);

     /* set parameters to header version 2.0 defaults */


     switch (iopt){
     case -1: /* image only */
       ideg_sc=10;
       iscale_sc=1000;
       i0_sc=100;
       ixdeg_off=0;
       iydeg_off=0;
       ia0_off=0;
       ib0_off=0;
       break;
     case 0: /* rectalinear lat/lon */
       ideg_sc=100;
       iscale_sc=1000;
       i0_sc=100;
       ixdeg_off=-100;
       iydeg_off=0;
       ia0_off=0;
       ib0_off=0;
       break;
     case 1: /* lambert */
     case 2:
       ideg_sc=100;
       iscale_sc=1000;
       i0_sc=1;
       ixdeg_off=0;
       iydeg_off=0;
       ia0_off=0;
       ib0_off=0;
       break;
     case 5: /* polar stereographic */
       ideg_sc=100;
       iscale_sc=100;
       i0_sc=1;
       ixdeg_off=-100;
       iydeg_off=0;
       ia0_off=0;
       ib0_off=0;
       break;
     case 8: /* EASE2 grid */
     case 9:
     case 10:
       ideg_sc=10;
       iscale_sc=100;
       i0_sc=10;
       ixdeg_off=0;
       iydeg_off=0;
       ia0_off=0;
       ib0_off=0;
       break;
     case 11: /* EASE1 grid */
     case 12:
     case 13:
       ideg_sc=10;
       iscale_sc=1000;
       i0_sc=10;
       ixdeg_off=0;
       iydeg_off=0;
       ia0_off=0;
       ib0_off=0;
       break;
     default: /* unknown */
       ideg_sc=100;
       iscale_sc=1000;
       i0_sc=100;
       ixdeg_off=0;
       iydeg_off=0;
       ia0_off=0;
       ib0_off=0;
     }
     if (nhtype < 30) nhtype=30;
   }

   /* scale and store projection parameters */

   temp[0] = nsx;
   temp[1] = nsy;
   temp[4] = nhtype;

   temp[2] = rnd((xdeg + (float) ixdeg_off) * (float) ideg_sc);
   temp[3] = rnd((ydeg + (float) iydeg_off) * (float) ideg_sc);
   temp[5] = rnd(ascale * (float) iscale_sc);
   temp[6] = rnd(bscale * (float) iscale_sc);
   temp[7] = rnd((a0 + (float) ia0_off) * (float) i0_sc);
   temp[8] = rnd((b0 + (float) ib0_off) * (float) i0_sc);

   temp[39]  = iscale_sc;
   temp[126] = ixdeg_off;
   temp[127] = iydeg_off;
   temp[168] = ideg_sc;
   temp[189] = ia0_off;
   temp[240] = ia0_off;
   temp[255] = i0_sc;

   /* handle special cases which depend on transformation option */

   switch (iopt){
   case -1: /* image only */
     break;
   case 0:  /* rectalinear lat/lon */
     break;
   case 1:  /* lambert */
   case 2:
     temp[5] = rnd((float) iscale_sc / ascale);
     temp[6] = rnd((float) iscale_sc / bscale);
     break;
   case 5:  /* polar stereographic */
     break;
   case  8: /* EASE2 grid */
   case  9:
   case 10:
     break;     
   case 11: /* EASE1 grid */
   case 12:
   case 13:
     temp[5] = rnd((float) iscale_sc * rnd(10.*ascale*25.067525/6371.228)*0.05);
     temp[6] = rnd((float) iscale_sc * rnd(10.*bscale/25.067525)*0.05);
     break;
   default:
     fprintf(stderr,"\n *** Unrecognized SIR option in write_sir_header3 %d ***\n",iopt);
   }

   if (iscale <= 0) iscale =1;
   
   temp[9]  = ioff;
   temp[10] = iscale;
   temp[11] = iyear;
   temp[12] = isday;
   temp[13] = ismin;
   temp[14] = ieday;
   temp[15] = iemin;
   temp[16] = iopt;
   temp[17] = iregion;
   temp[18] = itype;

   *nhead=1;
   ndes=0;
   if (ldes > 0) {
      ndes=ldes/512;
      if (ldes % 512 > 0) ndes++;
      *nhead = *nhead+ndes;
   }
   if (nia > 0) {
      *nhead = *nhead + nia/256;
      if (nia % 256 > 0) *nhead=*nhead+1;
   }
   
   temp[40] = *nhead;
   temp[41] = ndes;
   temp[42] = ldes;
   temp[43] = nia;
   temp[44] = ipol;
   temp[45] = ifreqhm;
   temp[46] = ispare1;
   temp[47] = idatatype;

   s = (float) iscale;
   soff = (float) ioff;
   minv = 32767;
   maxv = 32767;
   if (idatatype == 1) {
     minv = 128;
     maxv = 127;
   }

   am = anodata;
   kk = rnd((am - soff) * s);
   kk = kk - minv;
   if (kk < -minv) {
      kk = -minv;
      if (idatatype != 4) fprintf(stdout,"*** write_sir_header3: underflow on 'anodata' %f %d %d %ld\n",am,ioff,iscale,kk);
   }
   if (kk > maxv) {
      kk = maxv;
      if (idatatype != 4) fprintf(stdout,"*** write_sir_header3: overflow on 'anodata' %f %d %d %ld\n",am,ioff,iscale,kk);
   }
   temp[48] = kk;

   am = v_min;
   kk = rnd((am - soff) * s);
   kk = kk - minv;
   if (kk < -minv) {
      kk = -minv;
      if (idatatype != 4) fprintf(stdout,"*** write_sir_header3:underflow on 'vmin' %f %d %d %ld\n",am,ioff,iscale,kk);
   }
   if (kk > maxv) {
      kk = maxv;
      if (idatatype != 4) fprintf(stdout,"*** write_sir_header3:overflow on 'vmin' %f %d %d %ld\n",am,ioff,iscale,kk);
   }
   temp[49] = kk;

   am = v_max;
   kk = rnd((am - soff) * s);
   kk = kk - minv;
   if (kk < -minv) {
      kk = -minv;
      if (idatatype != 4) fprintf(stdout,"*** underflow on 'vmax' %f %d %d %ld\n",am,ioff,iscale,kk);
   }
   if (kk > maxv) {
      kk = maxv;
      if (idatatype != 4) fprintf(stdout,"*** overflow on 'vmax' %f %d %d %ld\n",am,ioff,iscale,kk);
   }
   temp[50] = kk;

#ifdef SWAP
   un.f2 = anodata;
   temp[51]=un.i2[1];
   temp[52]=un.i2[0];
   un.f2 = v_min;
   temp[53]=un.i2[1];
   temp[54]=un.i2[0];
   un.f2 = v_max;
   temp[55]=un.i2[1];
   temp[56]=un.i2[0];
#else
   un.f2 = anodata;
   temp[51]=un.i2[0];
   temp[52]=un.i2[1];
   un.f2 = v_min;
   temp[53]=un.i2[0];
   temp[54]=un.i2[1];
   un.f2 = v_max;
   temp[55]=un.i2[0];
   temp[56]=un.i2[1];
#endif

   /* blank fill residual parts of strings */

   if (strlen(sensor) < 40) 
     for (j=strlen(sensor)+1; j < 40; j++)
       sensor[j]=' ';
   if (strlen(type) < 138) 
     for (j=strlen(type)+1; j < 138; j++)
       type[j]=' ';
   if (strlen(title) < 80) 
     for (j=strlen(title)+1; j < 80; j++)
       title[j]=' ';
   if (strlen(tag) < 40) 
     for (j=strlen(tag)+1; j < 40; j++)
       tag[j]=' ';
   if (strlen(crproc) < 100) 
     for (j=strlen(crproc)+1; j < 100; j++)
       crproc[j]=' ';
   if (strlen(crtime) < 28) 
     for (j=strlen(crtime)+1; j < 28; j++)
       crtime[j]=' ';

   /* save strings */

   for (in = 0; in < 20; in++) {
     j=2*in;
     temp[19+in] = sensor[j] + sensor[j+1]*256;
   }

   for (in = 0; in < 69; in++) {
     j=2*in;
     temp[57+in] = type[j] + type[j+1]*256;
   }

   for (in = 0; in < 40; in++) {
     j=2*in;
     temp[128+in] = title[j] + title[j+1]*256;
   }

   for (in = 0; in < 20; in++) {
     j=2*in;
     temp[169+in] = tag[j] + tag[j+1]*256;
   }

   for (in = 0; in < 50; in++) {
     j=2*in;
     temp[190+in] = crproc[j] + crproc[j+1]*256;
   }

   for (in = 0; in < 14; in++) {
     j=2*in;
     temp[241+in] = crtime[j] + crtime[j+1]*256;
   }

#ifdef SWAP
   swap(temp,256);
#endif

   rewind(imf);   /* insure header is written at the start of file */

   if (fwrite(temp, sizeof(short), 256, imf) != 256) return(-1);

/* write extra header blocks */

   if (ndes > 0) {
     j=0;
     for (i=0; i < ndes; i++) {
       for (in = 0; in < 256; in++) {
	 if (j < ldes) temp[in] = descrip[j];
	 j++;
	 if (j < ldes) temp[in] = temp[in] + descrip[j]*256;
	 j++;
       }
#ifdef SWAP
       swap(temp,256);
#endif
       if (fwrite(temp, sizeof(short), 256, imf) != 256) return(-1);
     }
   }
   
/* write extra integers */

   if (*nhead-ndes-1 > 0) {
     for (i=0; i < *nhead-ndes-1; i++) {
       for (j=0; j<256; j++) {
	 if (j+i*256 < nia)
	   temp[j] = iaopt[j+i*256];
	 else
	   temp[j]=0;
       }
#ifdef SWAP
       swap(temp,256);
#endif
       if (fwrite(temp, sizeof(short), 256, imf) != 256) return(-1);
     }
   }
   return(0);
}


/* write_sir is included for compatiblity with older code,
   use write_sir3 instead */

int write_sir(char *fname, float *stval, int *nhead, int ndes, int nhtype,
	      int idatatype, int nsx, int nsy, 
	      float xdeg, float ydeg, 
	      float ascale, float bscale, float a0, float b0, 
	      int ioff, int iscale, int iyear, 
	      int isday, int ismin, int ieday, int iemin, 
	      int iregion, int itype, int iopt,
	      int ipol, int ifreqhm, int ispare1,
	      float anodata, float v_min, float v_max,
	      char *sensor, char *title, char *type, char *tag,
	      char *crproc, char *crtime, 
	      char *descrip, int ldes, short *iaopt, int nia)
{
  int err;
  int ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc;

  /* set parameters to version 2.0 defaults */

  switch (iopt){
  case -1: /* image only */
    ideg_sc=10;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 0: /* rectalinear lat/lon */
    ideg_sc=100;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=-100;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 1: /* lambert */
  case 2:
    ideg_sc=100;
    iscale_sc=1000;
    i0_sc=1;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 5: /* polar stereographic */
    ideg_sc=100;
    iscale_sc=100;
    i0_sc=1;
    ixdeg_off=-100;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 8: /* EASE2 grid */
  case 9:
  case 10:
    ideg_sc=10;
    iscale_sc=100;
    i0_sc=10;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 11: /* EASE1 grid */
  case 12:
  case 13:
    ideg_sc=10;
    iscale_sc=1000;
    i0_sc=10;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  default: /* unknown */
    ideg_sc=100;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
  }
  if (nhtype < 30) nhtype=30;

  err=write_sir3(fname, stval, nhead, nhtype,
		 idatatype, nsx, nsy, xdeg, ydeg, 
		 ascale, bscale, a0, b0, 
		 ixdeg_off, iydeg_off, ideg_sc,
		 iscale_sc, ia0_off, ib0_off, i0_sc,
		 ioff, iscale, iyear, 
		 isday, ismin, ieday, iemin, 
		 iregion, itype, iopt,
		 ipol, ifreqhm, ispare1,
		 anodata, v_min, v_max,
		 sensor, title, type, tag,
		 crproc, crtime, 
		 descrip, ldes, iaopt, nia);

  return(err);
  
}



#ifdef SWAP

void swap(short *i, int n)
{
   char *s, t;
   int j;
   
   for (j = 0; j < n; j++) {
     s = (char *) (i+j);
     t = *s;
     *s = *(s+1);
     *(s+1) = t;
   }
   return;
}

#endif


/****** print out sir file header information to output file *****/

char *strnc(char*, char*, int);

void print_head(FILE *omf, int nhead, int ndes, int nhtype, int idatatype, int nsx,
		int nsy, float xdeg, float ydeg, float ascale,
		float bscale, float a0, float b0, 
		int ioff, int iscale, int iyear, int isday, int ismin,
		int ieday, int iemin, int iregion, int itype, int iopt,
		int ipol, int ifreqhm, int ispare1,
		float anodata, float v_min, float v_max,
		char *sensor, char *title, char *type, char *tag,
		char *crproc, char *crtime, 
		char *descrip, int ldes, short *iaopt, int nia)
{
  int i;
  char stmp[150];
  float x0,y0,fmap_scale, fr0, fs0;
  int bcols, brows;  

  if (nhtype < 20) fprintf(omf," (Old style header) %d\n",nhtype);
  fprintf(omf,"  Title:   '%s'\n",strnc(title,stmp,150));
  fprintf(omf,"  Sensor:  '%s'\n",strnc(sensor,stmp,150));
  if (nhtype > 16) {
     fprintf(omf,"  Type:    '%s'\n",strnc(type,stmp,150));
     fprintf(omf,"  Tag:     '%s'\n",strnc(tag,stmp,150));
     fprintf(omf,"  Creator: '%s'\n",strnc(crproc,stmp,150));
     fprintf(omf,"  Created: '%s'\n",strnc(crtime,stmp,150));
  }
  fprintf(omf,"  Size: %d x %d    Total:%d",nsx,nsy, nsx*nsy);
  fprintf(omf,"  Offset: %d  Scale: %d\n",ioff,iscale);
  fprintf(omf,"  Year: %d  JD range: %d-%d",iyear,isday,ieday);
  fprintf(omf,"  Region Number: %d  Type: %d  Form: %d\n",iregion,itype,iopt);
  if (nhtype > 16) {
     fprintf(omf,"  Polarization: %d  Frequency: %f GHz\n",ipol,ifreqhm*0.1);
     fprintf(omf,"  Datatype: %d  Headers: %d  Ver:%d\n",idatatype,nhead,nhtype);
     fprintf(omf,"  Nodata: %f   Vmin: %f  Vmax: %f\n",anodata,v_min,v_max);
     if (ldes > 0)
	fprintf(omf,"  Description: (%d) '%s'\n",ldes,strnc(descrip,stmp,150));
     if (nia > 0) {
	fprintf(omf,"  Extra Ints: %d\n",nia);
	for (i=0; i < nia; i++)
	   fprintf(omf,"     %d %d",i,iaopt[i]);
     }	
  } else {
    v_min=-32.0;
    v_max=0.;
  }

  switch(iopt) {
   case -1:
     fprintf(omf,"  Rectangular image-only projection: \n");
     fprintf(omf,"   Xspan,  Yspan:  %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Xscale, Yscale: %f , %f\n",ascale,bscale);
     fprintf(omf,"   Xorg,   Yorg:   %f , %f\n",a0,b0);
     break;

   case 0:
     fprintf(omf,"  Rectangular Lat/Long projection: \n");
     fprintf(omf,"   Size (deg):     %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Lon, Lat scale: %f , %f (pix/deg)\n",ascale,bscale);
     fprintf(omf,"   Offsets:        %f , %f\n",a0,b0);
     break;

   case 2:
     fprintf(omf,"  Lambert form: (local radius)\n");
   case 1:
     if (iopt==1) fprintf(omf,"  Lambert projection: (fixed radius)\n");
     fprintf(omf,"   Center point:      %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Lon, Lat scale:    %f , %f (km/pix)\n",1./ascale,1./bscale);
     fprintf(omf,"   Lower-Left Corner: %f , %f\n",a0,b0);
     break;


   case 5:
     fprintf(omf,"  Polar sterographic form: \n");
     fprintf(omf,"   Center Lon,Lat:    %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   X,Y scales:        %f , %f (km/pix)\n",ascale,bscale);
     fprintf(omf,"   Lower-Left Corner: %f , %f\n",a0,b0);
     break;

   case 8:
   case 9:
     ease2sf(iopt, ascale, bscale, 
	     &fmap_scale, &bcols, &brows, &fr0, &fs0);
     x0=-fmap_scale*fs0;     
     y0=-fmap_scale*fr0;     
     fprintf(omf,"  EASE2 polar azimuthal form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   head A,B scales:           %f , %f\n",ascale,bscale);
     /* fprintf(omf,"   head origin (col,row): %f , %f\n",a0,b0); */
     if (iopt==8)
       fprintf(omf,"   Reference Latitude:   %f (deg)\n",90.0);
     else
       fprintf(omf,"   Reference Latitude:   %f (deg)\n",-90.0);
     fprintf(omf,"   Map base, scale, index: %f %.1f , %.1f\n",fmap_scale,ascale,bscale);
     fprintf(stdout,"   Map origin (col,row): %f , %f\n",x0,y0);          
     break;

   case 10:
     ease2sf(iopt, ascale, bscale, 
	     &fmap_scale, &bcols, &brows, &fr0, &fs0);
     x0=-fmap_scale*fs0;     
     y0=-fmap_scale*fr0;     
     fprintf(omf,"  EASE2 cylindrical form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   head A,B scales:           %f , %f\n",ascale,bscale);
     /* fprintf(omf,"   head origin (col,row): %f , %f\n",a0,b0); */
     fprintf(omf,"   Reference Latitude:   %f (deg)\n",30.0);
     fprintf(omf,"   Map base, scale, index: %f %.1f , %.1f\n",fmap_scale,ascale,bscale);
     fprintf(stdout,"   Map origin (col,row): %f , %f\n",x0,y0);          
     break;

   case 11:
   case 12:
     fprintf(omf,"  EASE1 polar azimuthal form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   A,B scales:           %f , %f\n",ascale,bscale);
     fprintf(omf,"   Map origin (col,row): %f , %f\n",a0,b0);
     break;

   case 13:
     fprintf(omf,"  EASE1 cylindrical form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   A,B scales:           %f , %f\n",ascale,bscale);
     fprintf(omf,"   Map origin (col,row): %f , %f\n",a0,b0);
     break;

   default:
     fprintf(omf,"  Unrecognized SIR file option: %d\n",iopt);
     fprintf(omf,"   Xspan,  Yspan:  %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Xscale, Yscale: %f , %f\n",ascale,bscale);
     fprintf(omf,"   Xorg,   Yorg:   %f , %f\n",a0,b0);
     break;
  }

  fprintf(omf,"  Image Min, Max: %f , %f\n\n",v_min,v_max);
  fflush(omf);
  
  return;
}

void print_head3(FILE *omf, int nhead, int ndes, int nhtype, int idatatype, int nsx,
		int nsy, float xdeg, float ydeg, float ascale,
		float bscale, float a0, float b0, 
		int ixdeg_off, int iydeg_off, int ideg_sc,
		int iscale_sc, int ia0_off, int ib0_off, int i0_sc,
		int ioff, int iscale, int iyear, int isday, int ismin,
		int ieday, int iemin, int iregion, int itype, int iopt,
		int ipol, int ifreqhm, int ispare1,
		float anodata, float v_min, float v_max,
		char *sensor, char *title, char *type, char *tag,
		char *crproc, char *crtime, 
		char *descrip, int ldes, short *iaopt, int nia)
{
  int i;
  char stmp[150];


  if (nhtype < 20) fprintf(omf," (Old style header) %d\n",nhtype);
  fprintf(omf,"  Title:   '%s'\n",strnc(title,stmp,150));
  fprintf(omf,"  Sensor:  '%s'\n",strnc(sensor,stmp,150));
  if (nhtype > 16) {
     fprintf(omf,"  Type:    '%s'\n",strnc(type,stmp,150));
     fprintf(omf,"  Tag:     '%s'\n",strnc(tag,stmp,150));
     fprintf(omf,"  Creator: '%s'\n",strnc(crproc,stmp,150));
     fprintf(omf,"  Created: '%s'\n",strnc(crtime,stmp,150));
  }
  fprintf(omf,"  Size: %d x %d    Total:%d",nsx,nsy, nsx*nsy);
  fprintf(omf,"  Offset: %d  Scale: %d\n",ioff,iscale);
  fprintf(omf,"  Year: %d  JD range: %d-%d",iyear,isday,ieday);
  fprintf(omf,"  Region Number: %d  Type: %d  Form: %d\n",iregion,itype,iopt);
  if (nhtype > 16) {
     fprintf(omf,"  Polarization: %d  Frequency: %f GHz\n",ipol,ifreqhm*0.1);
     fprintf(omf,"  Datatype: %d  Headers: %d  Ver:%d\n",idatatype,nhead,nhtype);
     fprintf(omf,"  Nodata: %f   Vmin: %f  Vmax: %f\n",anodata,v_min,v_max);
     if (ldes > 0)
	fprintf(omf,"  Description: (%d) '%s'\n",ldes,strnc(descrip,stmp,150));
     if (nia > 0) {
	fprintf(omf,"  Extra Ints: %d\n",nia);
	for (i=0; i < nia; i++)
	   fprintf(omf,"     %d %d",i,iaopt[i]);
     }	
  } else {
    v_min=-32.0;
    v_max=0.;
  }

  switch(iopt) {
   case -1:
     fprintf(omf,"  Rectangular image-only projection: \n");
     fprintf(omf,"   Xspan,  Yspan:  %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Xscale, Yscale: %f , %f\n",ascale,bscale);
     fprintf(omf,"   Xorg,   Yorg:   %f , %f\n",a0,b0);
     break;

   case 0:
     fprintf(omf,"  Rectangular Lat/Long projection: \n");
     fprintf(omf,"   Size (deg):     %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Lon, Lat scale: %f , %f (pix/deg)\n",ascale,bscale);
     fprintf(omf,"   Offsets:        %f , %f\n",a0,b0);
     break;

   case 2:
     fprintf(omf,"  Lambert form: (local radius)\n");
   case 1:
     if (iopt==1) fprintf(omf,"  Lambert projection: (fixed radius)\n");
     fprintf(omf,"   Center point:      %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Lon, Lat scale:    %f , %f (km/pix)\n",1./ascale,1./bscale);
     fprintf(omf,"   Lower-Left Corner: %f , %f\n",a0,b0);
     break;


   case 5:
     fprintf(omf,"  Polar sterographic form: \n");
     fprintf(omf,"   Center Lon,Lat:    %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   X,Y scales:        %f , %f (km/pix)\n",ascale,bscale);
     fprintf(omf,"   Lower-Left Corner: %f , %f\n",a0,b0);
     break;

   case 8:
   case 9:
     fprintf(omf,"  EASE2 polar azimuthal form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   A,B scales:           %f , %f\n",ascale,bscale);
     fprintf(omf,"   Map origin (col,row): %f , %f\n",a0,b0);
     break;

   case 10:
     fprintf(omf,"  EASE2 cylindrical form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   A,B scales:           %f , %f\n",ascale,bscale);
     fprintf(omf,"   Map origin (col,row): %f , %f\n",a0,b0);
     break;

   case 11:
   case 12:
     fprintf(omf,"  EASE2 polar azimuthal form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   A,B scales:           %f , %f\n",ascale,bscale);
     fprintf(omf,"   Map origin (col,row): %f , %f\n",a0,b0);
     break;

   case 13:
     fprintf(omf,"  EASE2 cylindrical form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   A,B scales:           %f , %f\n",ascale,bscale);
     fprintf(omf,"   Map origin (col,row): %f , %f\n",a0,b0);
     break;

   default:
     fprintf(omf,"  Unrecognized SIR file option: %d\n",iopt);
     fprintf(omf,"   Xspan,  Yspan:  %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Xscale, Yscale: %f , %f\n",ascale,bscale);
     fprintf(omf,"   Xorg,   Yorg:   %f , %f\n",a0,b0);
     break;
  }
  fprintf(omf,"  Transform scale factors: \n");
  fprintf(omf,"   Xdeg_off, Ydeg_off, scale: %d %d %d\n",ixdeg_off, iydeg_off, ideg_sc);
  fprintf(omf,"   Xscale, Yscale:            %d\n",iscale_sc);
  fprintf(omf,"   Xorg, Yorg, scale:         %d %d %d\n",ia0_off,ib0_off, i0_sc);

  fprintf(omf,"  Image Visible Min, Max: %f , %f\n\n",v_min,v_max);
  fflush(omf);
  
  return;
}

/* routine to truncate off trailing spaces for printing */

char *strnc(char *in, char *out, int cnt)
{
  int l;
  
  strncpy(out, in, cnt);
  for (l=strlen(out); l > 0; l--)
    if (*(out+l) != ' ' && *(out+l) != '\0') {
      *(out+l+1)='\0';
      return(out);
    }
  return(out);
}


/* routine to update old-style header info to include additional
   information in the newer style header

   note: the algorithm used by this routine to select the nodata value
   and the viewing range is only approximate and is not optimal.
 */

void update_sir_header(float *stval, int nhtype, int nsx, int nsy, 
		       float *anodata, float *v_min, float *v_max)
{
  int i;
  float smin=1.e25, smax=-1.e25;
  
  if (nhtype == 1) {   /* only required for old stype header */
     for (i = 0; i < nsx * nsy; i++) {
       if (*(stval+i) < smin) smin = *(stval+i);
       if (*(stval+i) > smax) smax = *(stval+i);
     }
    *anodata = smin;
    *v_min = smin;
    *v_max = smax;
    if (abs(smin+32.0) < 2.0) {
      *anodata = -32.0;
      *v_min = -32.0;
    }
    if (abs(smin+3.2) < 2.0) {
      *anodata = -3.2;
      *v_min = -3.0;
    }
    if (abs(smax) < 2.0) *v_max = 0.0;
   }
}

