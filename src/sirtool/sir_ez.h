/* easy BYU SIR file format C include file

This include file defines an easy programing C interface for the BYU SIR
file format.  A data type which stores the key SIR header information
is defined to permit easy transfer of the SIR information into the
various I/O and geometry routines.  Function prototypes are also defined.

written  4 Nov 2000 by DGL at BYU
revised 30 Apr 2001 by DGL at BYU + added isir_lex, fixed sir_lex
updated 14 Feb 2002 by DGL at BYU + added sir_update_head, comments

(c) 2000,2001,2002 by BYU MERS 

*/


/* define the SIR head data type */

#define MAXDES 512  /* see note below */
#define MAXI 128    /* see note below */

typedef struct
{
  /* the following variables are defined in the standard SIR file
     header (version 3.0) and are fixed length */

  int nhead;          /* number of 512 byte header records */
  int nhtype;         /* version number of SIR file header */

  int nsx, nsy;       /* image size */

  int iopt;           /* projection option */
  float xdeg, ydeg, ascale, bscale, a0, b0;     /* projection parameters */

  int ixdeg_off, iydeg_off, ideg_sc, iscale_sc; /* projection storage scale */
  int ia0_off, ib0_off, i0_sc;                  /* factors */ 

  int idatatype;      /* data storage type */
  int ioff, iscale;   /* data storage scale factor and offset value */
  float anodata, v_min, v_max;               /* no data value, visible range */

  int iyear, isday, ismin, ieday, iemin;        /* file time codes */
  int iregion, itype, ipol, ifreqhm;            /* region id number, etc */
  int ispare1;        /* spare int */  

  char title[101],    /* standard null-terminated info strings */
    sensor[41],          /* sensor description */
    type[139],           /* image/data type description */
    tag[101],            /* file tag */
    crproc[101],         /* file creation process */
    crtime[29];          /* creation time */

  /*
     After the first 512 byte header of variables defined above,
     the SIR file format permits optional additional 512 byte headers
     containing an optional user-defined string 'descrip' and an 
     array of short ints 'iaopt'.  These may be arbitrarily long, 
     though if present are typically short.  To permit either option,
     a short storage area for each is included in this typedef with
     flags and pointers to permit users to externally allocate additional
     storage for longer strings and arrays.  To use the external storage
     the flags should be set, pointers defined, and maxmium length
     parameters set PRIOR to calls to sir_ez routines.
  */

  int ndes;           /* number of characters of extra description string */
  int ldes;           /* number of 512 records devoted to descript */
  char *descrip;      /* pointer to extra character description */
  int nia;            /* number of extra integers */
  short *iaopt;       /* pointer to extra integer array */

  int maxdes;         /* maximum characters in extra description string */ 
  int maxi;           /* maximum number of extra integers */

  /* the following variables are used to indicate if the internal default
     storage locations are used for descrip and iaopt on file read or
     if externally defined storage locations are to be used.  External
     storage used if flag is set to 1 in which case:
       . the pointers *descrip and *iaopt should be set to point to their
         respective storage areas
       . maxdes and maxi should be set to the maximum storage space
     When default internal storage is used, pointers and maxdes,maxi are
     set to point within this structure */

  int descrip_flag;  /* extra description string flag */
  int iaopt_flag;    /* extrat integer string flag */
  
  char descrip_string[MAXDES+1];  /* default descrip storage location */
  short iaopt_array[MAXI];        /* default iaopt storage location */

} sir_head;


/* sir_ez function prototypes */

void sir_init_head(sir_head *head);
/* sets optional parameter flags to their default values.
   call before using sir_head */

int get_sir_head_file(FILE *imf, sir_head *head);
/* read sir file header given file pointer. Returns negative value
   on read error. */

int get_sir_head_name(char *fname, sir_head *head, FILE **imf);
/* read sir file header given file name.  Returns negative value
   on read/open error. */

float *sir_data_alloc(sir_head *head);
/* returns a pointer for a malloc'd storage array sized for storage
   of the sir image data */

int get_sir_data(FILE *imf, float *stval, sir_head *head);
/* read sir file image data into a float array given file pointer.   
   Returns negative value on read error. Closes file on successful read. */

int get_sir_data_block(FILE *imf, float *stval, sir_head *head,
		       int x1, int y1, int x2, int y2);
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


int get_sir_data_byte(FILE *imf, char *stval, sir_head *head, float smin,
		      float smax, int bmin, int bmax);
/* read sir file image data into a byte array given a file pointer.
   The input image values are first converted to floats and then
   scaled to byte using 

   byte_value=(bmax-bmin)*(float_value-smin)/(smax-smin)+bmin

   Note: this technique generally results in loss of information */

int get_sir(char *fname, sir_head *head, float **stval);
/* opens sir file, reads header, malloc's image data storage,
   reads data and closes output file.  Returns negative value on
   open, read, or malloc error */

void print_sir_head(FILE *omf, sir_head *head);
/* summary print sir file header information to file pointer */

int put_sir(char *fname, sir_head *head, float *stval);
/* write sir file header and data to file.  Returns negative value
   on write error. */

int sir_latlon2pix(float alon, float alat, float *x, float *y, sir_head *head);
/* compute pixel location given lat,lon location.  Return pixel number 
   as non-negative value.  A negative value indicates pixel is outside of 
   the image. */

void sir_pix2latlon(float x, float y, float *alon, float *alat, sir_head *head);
/* compute lat, lon from pixel location.  Will work for values outside of 
   image for most projections */

int sir_pix(float x, float y, int *ix, int *iy, sir_head *head);
/* given a floating point x,y value, compute the integer pixel locations
   ix,iy and return the pixel index as a non-negative value in the
   range 0...nsx*nxy-1 consistent with C array indexing.
   A negative value indicates pixel is outside of the image. */

int sir_lex(int ix, int iy, sir_head *head);
/* given a pixel x,y value returns the pixel index as non-negative value.
   A negative value indicates pixel is outside of image. 
   If ix and iy are in range (1 <= x <= head.nsx) (1 <= y <= head.nsy)) 
   the lexicographic index of pixel (x,y) is 
   index = (y-1)*head.nsx + x-1 where 1 <= index <= head.nsx*head.nsy 
   The range index=0...nsx*nxy-1 is consistent with C array indexing. */

int isir_lex(int *ix, int *iy, int i, sir_head *head);
/* given a pixel index i, in the range i=0...nsx*nxy-1 consistent with 
   C array indexing, returns the pixel coordinates in SIR convention
   (1 <= ix <= head.nsx) (1 <= iy <= head.nsy)) 
   A negative return value indicates index i is outside of image, 
   Zero is returned if i is in the range (0..nsx*nxy-1)
   the lexicographic index of pixel (x,y) is 
   index = (y-1)*head.nsx + x-1 where 1 <= index <= head.nsx*head.nsy */

void sir_update_head(sir_head *head, float *stval);
/* routine to update old-style header info to include additional
   information in the newer style header.  Sets default anodata
   and v_min v_max values based on image contents.  Call after reading
   image content information. */
