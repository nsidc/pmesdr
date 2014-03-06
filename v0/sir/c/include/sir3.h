/*
   Standard C include file for SIR I/O and projection transformation routinnes

   version 2.0 header DGL 19 Dec. 1998
   version 3.0 header DGL 28 Oct. 2000
           updated by DGL 17 Dec. 2001 + added forgotten print_head3 prototype
           updated by DGL 14 Feb. 2002 + added update_sir_header and comments

   Written in ANSI C.  Includes function prototypes

   SWAP should be defined for little endian (least-significant byte first)
      machines such as VAX and intel
   SWAP should be undefined for big endian (most-significant byte first
      machines typical for unix (HP, sun, convex, etc.) 

(c) copyright 1998, 2000, 2001, 2002 by BYU MERS
*/

#define SWAP 1                /* swap bytes (little endian: dec, pc) */
/*#undef SWAP*/                   /* no byte swapping (big endian: HP,SUN) */

/* sir routines need the following libraries included:
   <stdlib.h>
   <stdio.h>
   <string.h>
   <math.h>
   */

#define REL_BEGIN 0           /* fseek relative to beginning of file */
#define SIR_HEADER_SIZE 512   /* SIR file header size in bytes */


/* standard SIR routine function prototypes */


void pixtolatlon(float x, float y, float *alon, float *alat, 
		 int iopt, float xdeg, float ydeg,
		 float ascale, float bscale, float a0, float b0);
/* given pixel coordinates x,y and projection parameters iopt..b0
   computes the corresponding lon,lat */

void latlon2pix(float alon, float alat, float *x, float *y, 
		 int iopt, float xdeg, float ydeg,
		 float ascale, float bscale, float a0, float b0);
/* given a lon and lat and projection parameters iopt..b0
   computes the corresponding x and y values (which may be outside
   of conventional SIR image */

void f2ipix(float x, float y, int *ix, int *iy, int nsx, int nsy);
/* given pixel coordinates x,y as floats and the image size
   compute the corresponding  integer pixel coordinates.  Note that
   pixel coordinates are (1 <= x <= nsx) (1 <= y <= nsy) the fractional
   part of the float is truncated off, thus, (1,1) corresponds to the
   lower-left corner of the lower-left pixel.  A returned
   ix=0 or iy=0 indicates that pixel is outside of image (out of
   range).  If ix and iy are in range (non-zero) lexicographic index
   of pixel (x,y) is index = (y-1)*nsx + x where 1 <= index <= nsx*nsy */


/* note: read_sir_header and write_sir are for version 2.0 headers and 
   will be phased out as they result in possible loss of information.  
   They are included for compatibility with old code.
   Use read_sir_header3 and write_sir3 instead. */

int read_sir_header(FILE *imf, int *nhead, int *ndes, int *nhtype,
		    int *idatatype, int *nsx, int *nsy, 
		    float *xdeg, float *ydeg, 
		    float *ascale, float *bscale, float *a0, float *b0, 
		    int *ioff, int *iscale, int *iyear, 
		    int *isday, int *ismin, int *ieday, int *iemin, 
		    int *iregion, int *itype, int *iopt,
		    int *ispare1, int *ispare2, int *ispare3,
		    float *anodata, float *v_min, float *v_max,
		    char *sensor, char *title, char *type, char *tag,
		    char *crproc, char *crtime, int maxdes, 
		    char *descrip, int *ldes, int maxi, short *iaopt,
		    int *nia);
/* old version 2.0 header read routine.  Given pointer to file, read
   SIR file header information.  Note that maxdes and maxi must be passed
   in and represent the max size of the descrip string and array of
   iaopt shorts which will be returned from the optional headers if
   they exist in the file.  This code merely calls read_sir_header3. */

int read_sir_header3(FILE *imf, int *nhead, int *ndes, int *nhtype,
		    int *idatatype, int *nsx, int *nsy, 
		    float *xdeg, float *ydeg, 
		    float *ascale, float *bscale, float *a0, float *b0, 
		    int *ixdeg_off, int *iydeg_off, int *ideg_sc,
		    int *iscale_sc, int *ia0_off, int *ib0_off, int *i0_sc,
		    int *ioff, int *iscale, int *iyear, 
		    int *isday, int *ismin, int *ieday, int *iemin, 
		    int *iregion, int *itype, int *iopt,
		    int *ispare1, int *ispare2, int *ispare3,
		    float *anodata, float *v_min, float *v_max,
		    char *sensor, char *title, char *type, char *tag,
		    char *crproc, char *crtime, int maxdes, 
		    char *descrip, int *ldes, int maxi, short *iaopt,
		    int *nia);
/* SIR header read routine.  Given pointer to file, read
   SIR file header information.  Note that maxdes and maxi must be passed
   in and represent the max size of the descrip string and array of
   iaopt shorts which will be returned from the optional headers if
   they exist in the file.  Returns a negative value on file read error. */


int read_sir_data(FILE *imf, int nhead, int idatatype, int nsx, int nsy, 
		  int ioff, int iscale, float *stval);
/* read SIR image data into float array stval using passed in file pointer and
   other data.  stval should be allocated (float array of size nsx*nsy) prior
   to file call.  Returns negative value on file error.  Closes file on
   successfull completion. */


int read_sir_data_block(FILE *imf, int nhead, int idatatype, 
			int nsx, int nsy, int ioff, int iscale, 
			int x1, int y1, int x2, int y2,
			float *stval);
/* read a block of the SIR image data from file into float array
       . file imf must already be open 
       . file imf is NOT closed after read 
     block is defined by the lower-left corner (x1,y1) and
     the upper-right corner (x2,y2) where 0 < x <= nsx and  0 < y <= nsy

     The output image has (x2-x1+1) by (y2-y1+1) pixels indexed according to
     image_index = (y-1) * (x2-x1+1) + x where 0 < y <= (y2-y1+1) and
     0 < x <= (x2-x1+1) and 1 < image_index <= (x2-x1+1) * (y2-y1+1).
     Obviously, it must be true that x2>=x1 and y2>=y1

     returns -5 on an input error, -1 on a file read, seek or other error.
     returns 0 on success
*/

int read_sir_data_byte(FILE *imf, int nhead, int idatatype, 
		       int nsx, int nsy, int ioff, int iscale, 
		       float smin, float smax, int bmin, int bmax,
		       char *out);

/* read sir file image data into a byte array given a file pointer.
   The input image values are first converted to floats and then
   scaled to byte using 

   byte_value=(bmax-bmin)*(float_value-smin)/(smax-smin)+bmin

   Note: this technique generally results in loss of information */

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
		 char *descrip, int ldes, short *iaopt, int nia);
/* summary print sir file header information to file pointer (preferred version) */

void print_head(FILE *omf, int nhead, int ndes, int nhtype, int idatatype,
                int nsx, int nsy, float xdeg, float ydeg, float ascale,
                float bscale, float a0, float b0, 
                int ioff, int iscale, int iyear, int isday, int ismin,
                int ieday, int iemin, int iregion, int itype, int iopt,
                int ipol, int ifreqhm, int ispare1,
                float anodata, float v_min, float v_max,
                char *sensor, char *title, char *type, char *tag,
                char *crproc, char *crtime, 
                char *descrip, int ldes, short *iaopt, int nia);
/* summary print sir file header information to file pointer (old verion) */

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
	      char *descrip, int ldes, short *iaopt, int nia);
/* old version 2.0 header write routine.  Use write_sir3 instead. */

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
	      char *descrip, int ldes, short *iaopt, int nia);
/* SIR file write routine. Given file name, header values, and float array
   stval, writes data to SIR format file.  All header values should
   be set to appropriate values.  Optional header record(s) containing
   descrip string will not be written if ldes==0.  Optional header
   record(s) containing iaopt data will not be written if nia==0 */

void update_sir_header(float *stval, int nhtype, int nsx, int nsy, 
		       float *anodata, float *v_min, float *v_max);
/* routine to update old-style header info to include additional
   information in the newer style header.  Sets default anodata
   and v_min v_max values based on image contents. */

