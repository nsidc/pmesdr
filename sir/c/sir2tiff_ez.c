/*
   program to read a BYU .SIR image file and write out as a .tiff file

   Written: 12 May 2001

   uses WriteTIFF routine from old XV library distribution. 
   must be linked to libcsir and libtiff

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sir_ez.h"  /* easy sir routine interface */

/* TIFF routine definitions */

typedef unsigned char byte;

int WriteTIFF(FILE *fp, byte *pic, int ptype, int w, int h, int comp,
	      byte *rmap, byte *gmap, byte *bmap, int numcols, int colorstyle,
	      char *fname, char *comment);

#define F_FULLCOLOR 0
#define F_GREYSCALE 1
#define F_BWDITHER  2
#define F_REDUCED   3

#define	    COMPRESSION_NONE		1	/* dump mode */
#define	    COMPRESSION_CCITTRLE	2	/* CCITT modified Huffman RLE */
#define	    COMPRESSION_CCITTFAX3	3	/* CCITT Group 3 fax encoding */
#define	    COMPRESSION_CCITTFAX4	4	/* CCITT Group 4 fax encoding */
#define	    COMPRESSION_LZW		5	/* Lempel-Ziv  & Welch */
#define	    COMPRESSION_JPEG		6	/* !JPEG compression */
#define	    COMPRESSION_NEXT		32766	/* NeXT 2-bit RLE */


int main(int argc, char **argv)
{
  FILE *imf;
  int i, j, k, x, y, nsx, nsy;
  char outfname[255], *infname;

  char rtab[256], gtab[256], btab[256];
	
  float smin, smax;
  float am, scale, scaleoffset;

  float *stval;
  char *data;

  int ierr;

  FILE *fp;
  int ptype, comp, colorstyle;
  char comment[100];

  sir_head head;


  fprintf(stdout,"BYU SIR to TIFF conversion program\n") ;
  if(argc < 3) {
    fprintf(stdout,"\nusage: %s sir_in tif_out <dmin> <dmax> <ctab>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   sir_in  =  input SIR file\n");
    fprintf(stdout,"   tif_out =  output tiff file (optional, def=sir_in.tif)\n");
    fprintf(stdout,"   dmin    =  min saturation value (optional,def=vmin)\n");
    fprintf(stdout,"   dmax    =  max saturation value (optional,def=vmax)\n");
    fprintf(stdout,"   ctab    =  color table file (optional,def=greyscale)\n"); 
    return(0);
  }

  /* read SIR image */

  infname=argv[1];
  ierr = get_sir(infname, &head, &stval);
  if (ierr < 0) {
     fprintf(stdout,"ERROR: cannot read .sir file: %s\n",infname); 
     exit(-1);
  }

  nsx = head.nsx;
  nsy = head.nsy;
  smin = head.v_min;
  smax = head.v_max;

  if (argc > 2)
    strncpy(outfname,argv[2],255);
  else
    sprintf(outfname,"%s.tif",infname);
  
  if (argc > 3) sscanf(argv[3],"%f",&smin); 
  if (argc > 4) sscanf(argv[4],"%f",&smax); 
  if (smax - smin == 0.0) smax = smin + 1.0;

  /* allocate tiff image area (8 bits/pixel) */
  data = (char *) malloc(sizeof(char)*nsx*nsy);

  scale = (smax-smin);
  if (scale > 0.) scale = 255./ scale;
  scaleoffset = smin;

  /* convert SIR images values to bytes */

  for (y=0; y< nsy; y++)
    for (x=0; x< nsx; x++) {
      k = y * nsx + x;
      /* scale floating point to byte values */
      am = scale * (stval[k] - scaleoffset);
      if (am > 255.) am = 255.;		/* check overflow */
      if (am < 0.) am = 0.;		/* check underflow */
      data[(nsy-y-1)*nsx+x] = (char)((int)(am));    /* byte array for output*/
    }  

  fprintf(stdout,"\nTIFF Min, Max: %f , %f\n",smin,smax);

  /* default greyscale color table */

  j=255;
  for (i=0; i<256; i++) {
    if (i > 127)
      rtab[i]=i-256;
    else
      rtab[i]=i;
    gtab[i]=rtab[i];
    btab[i]=rtab[i];
  }

  colorstyle = F_GREYSCALE;
  
  if (argc > 5) { /* read user input color table file */
    fprintf(stdout,"Reading input color table file '%s'\n",argv[5]);
    imf = fopen(argv[5],"r"); 
    if (imf == NULL) {
      fprintf(stdout,"*** ERROR: cannot open color table file: %s\n",argv[5]);
      fprintf(stdout,"Using greyscale color table\n");
    } else {
      if (fread(rtab, sizeof(char), 256, imf) == 0) 
	fprintf(stdout," *** Error reading color table file\n");
      if (fread(gtab, sizeof(char), 256, imf) == 0) 
	fprintf(stdout," *** Error reading color table file\n");
      if (fread(btab, sizeof(char), 256, imf) == 0) 
	fprintf(stdout," *** Error reading color table file\n");
      fclose(imf);
      colorstyle = F_FULLCOLOR;
    }
  } else
    fprintf(stdout,"Using greyscale color table\n");

  fprintf(stdout,"Writing output tiff file '%s'\n", argv[2]);

  ptype=0;

  comp = COMPRESSION_NONE;   /* no tiff file compression */
  comp = COMPRESSION_LZW;    /* compress tiff image file */

  sprintf(comment,"BYU sir2geotiff conversion min, max: %f, %f",smin,smax);
  printf("%d,%d\n",nsx,nsy);
  
  ierr = WriteTIFF(fp, (byte *) data, ptype, nsx, nsy, comp,
		   (byte *) rtab, (byte *) gtab, (byte *) btab, 
		   256, colorstyle, outfname, comment);

  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output tiff file ***\n");
     fflush(stderr);
  }
  else
    fprintf(stdout,"tiff output file successfully written\n");

  return(0);
}

/****************************************************************/

/*
 * xvtiffwr.c - write routine for TIFF pictures, adapted from XV distribution
 *
 */

#if defined(BSDTYPES) || defined(VMS)
  typedef unsigned char  u_char;
  typedef unsigned short u_short;
  typedef unsigned int   u_int;
  typedef unsigned long  u_long;
#endif


/* include files */
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#ifdef __STDC__
#  include <stddef.h>
#  include <stdlib.h>
#endif


/* MONO returns total intensity of r,g,b triple (i = .33R + .5G + .17B) */
#define MONO(rd,gn,bl) ( ((int)(rd)*11 + (int)(gn)*16 + (int)(bl)*5) >> 5)



/*
 * TIFF I/O Library Definitions.
 */


/*
 * TIFF is defined as an incomplete type to hide the
 * library's internal data structures from clients.
 */
typedef	struct tiff TIFF;

#define	TIFF_VERSION	42

#define	TIFF_BIGENDIAN		0x4d4d
#define	TIFF_LITTLEENDIAN	0x4949

/*
 * Intrinsic data types required by the file format:
 *
 * 8-bit quantities	char/unsigned char
 * 16-bit quantities	int16/uint16
 * 32-bit quantities	int32/uint32
 * strings		unsigned char*
 */
typedef	short int16;
typedef	unsigned short uint16;	/* sizeof (uint16) must == 2 */
typedef	long int32;
typedef	unsigned long uint32;	/* sizeof (uint32) must == 4 */

typedef	struct {
	uint16	tiff_magic;	/* magic number (defines byte order) */
	uint16	tiff_version;	/* TIFF version number */
	uint32	tiff_diroff;	/* byte offset to first directory */
} TIFFHeader;


/*
 * TIFF Image File Directories are comprised of
 * a table of field descriptors of the form shown
 * below.  The table is sorted in ascending order
 * by tag.  The values associated with each entry
 * are disjoint and may appear anywhere in the file
 * (so long as they are placed on a word boundary).
 *
 * If the value is 4 bytes or less, then it is placed
 * in the offset field to save space.  If the value
 * is less than 4 bytes, it is left-justified in the
 * offset field.
 */
typedef	struct {
	uint16	tdir_tag;	/* see below */
	uint16	tdir_type;	/* data type; see below */
	uint32  tdir_count;	/* number of items; length in spec */
	uint32  tdir_offset;	/* byte offset to field data */
} TIFFDirEntry;

/*
 * NB: In the comments below,
 *  - items marked with a + are obsoleted by revision 5.0,
 *  - items marked with a ! are introduced in revision 6.0.
 *  - items marked with a $ are obsoleted by revision 6.0.
 */

/*
 * Tag data type information.
 *
 * Note: RATIONALs are the ratio of two 32-bit integer values.
 */
typedef	enum {
	TIFF_NOTYPE	= 0,	/* placeholder */
	TIFF_BYTE	= 1,	/* 8-bit unsigned integer */
	TIFF_ASCII	= 2,	/* 8-bit bytes w/ last byte null */
	TIFF_SHORT	= 3,	/* 16-bit unsigned integer */
	TIFF_LONG	= 4,	/* 32-bit unsigned integer */
	TIFF_RATIONAL	= 5,	/* 64-bit unsigned fraction */
	TIFF_SBYTE	= 6,	/* !8-bit signed integer */
	TIFF_UNDEFINED	= 7,	/* !8-bit untyped data */
	TIFF_SSHORT	= 8,	/* !16-bit signed integer */
	TIFF_SLONG	= 9,	/* !32-bit signed integer */
	TIFF_SRATIONAL	= 10,	/* !64-bit signed fraction */
	TIFF_FLOAT	= 11,	/* !32-bit IEEE floating point */
	TIFF_DOUBLE	= 12	/* !64-bit IEEE floating point */
} TIFFDataType;

/*
 * TIFF Tag Definitions.
 */
#define	TIFFTAG_SUBFILETYPE		254	/* subfile data descriptor */
#define	    FILETYPE_REDUCEDIMAGE	0x1	/* reduced resolution version */
#define	    FILETYPE_PAGE		0x2	/* one page of many */
#define	    FILETYPE_MASK		0x4	/* transparency mask */
#define	TIFFTAG_OSUBFILETYPE		255	/* +kind of data in subfile */
#define	    OFILETYPE_IMAGE		1	/* full resolution image data */
#define	    OFILETYPE_REDUCEDIMAGE	2	/* reduced size image data */
#define	    OFILETYPE_PAGE		3	/* one page of many */
#define	TIFFTAG_IMAGEWIDTH		256	/* image width in pixels */
#define	TIFFTAG_IMAGELENGTH		257	/* image height in pixels */
#define	TIFFTAG_BITSPERSAMPLE		258	/* bits per channel (sample) */
#define	TIFFTAG_COMPRESSION		259	/* data compression technique */
#define	    COMPRESSION_NONE		1	/* dump mode */
#define	    COMPRESSION_CCITTRLE	2	/* CCITT modified Huffman RLE */
#define	    COMPRESSION_CCITTFAX3	3	/* CCITT Group 3 fax encoding */
#define	    COMPRESSION_CCITTFAX4	4	/* CCITT Group 4 fax encoding */
#define	    COMPRESSION_LZW		5	/* Lempel-Ziv  & Welch */
#define	    COMPRESSION_JPEG		6	/* !JPEG compression */
#define	    COMPRESSION_NEXT		32766	/* NeXT 2-bit RLE */
#define	    COMPRESSION_CCITTRLEW	32771	/* #1 w/ word alignment */
#define	    COMPRESSION_PACKBITS	32773	/* Macintosh RLE */
#define	    COMPRESSION_THUNDERSCAN	32809	/* ThunderScan RLE */
/* compression codes 32908-32911 are reserved for Pixar */
#define     COMPRESSION_PIXARFILM       32908   /* Pixar companded 10bit LZW */
#define	TIFFTAG_PHOTOMETRIC		262	/* photometric interpretation */
#define	    PHOTOMETRIC_MINISWHITE	0	/* min value is white */
#define	    PHOTOMETRIC_MINISBLACK	1	/* min value is black */
#define	    PHOTOMETRIC_RGB		2	/* RGB color model */
#define	    PHOTOMETRIC_PALETTE		3	/* color map indexed */
#define	    PHOTOMETRIC_MASK		4	/* $holdout mask */
#define	    PHOTOMETRIC_SEPARATED	5	/* !color separations */
#define	    PHOTOMETRIC_YCBCR		6	/* !CCIR 601 */
#define	    PHOTOMETRIC_CIELAB		8	/* !1976 CIE L*a*b* */
#define	TIFFTAG_THRESHHOLDING		263	/* +thresholding used on data */
#define	    THRESHHOLD_BILEVEL		1	/* b&w art scan */
#define	    THRESHHOLD_HALFTONE		2	/* or dithered scan */
#define	    THRESHHOLD_ERRORDIFFUSE	3	/* usually floyd-steinberg */
#define	TIFFTAG_CELLWIDTH		264	/* +dithering matrix width */
#define	TIFFTAG_CELLLENGTH		265	/* +dithering matrix height */
#define	TIFFTAG_FILLORDER		266	/* data order within a byte */
#define	    FILLORDER_MSB2LSB		1	/* most significant -> least */
#define	    FILLORDER_LSB2MSB		2	/* least significant -> most */
#define	TIFFTAG_DOCUMENTNAME		269	/* name of doc. image is from */
#define	TIFFTAG_IMAGEDESCRIPTION	270	/* info about image */
#define	TIFFTAG_MAKE			271	/* scanner manufacturer name */
#define	TIFFTAG_MODEL			272	/* scanner model name/number */
#define	TIFFTAG_STRIPOFFSETS		273	/* offsets to data strips */
#define	TIFFTAG_ORIENTATION		274	/* +image orientation */
#define	    ORIENTATION_TOPLEFT		1	/* row 0 top, col 0 lhs */
#define	    ORIENTATION_TOPRIGHT	2	/* row 0 top, col 0 rhs */
#define	    ORIENTATION_BOTRIGHT	3	/* row 0 bottom, col 0 rhs */
#define	    ORIENTATION_BOTLEFT		4	/* row 0 bottom, col 0 lhs */
#define	    ORIENTATION_LEFTTOP		5	/* row 0 lhs, col 0 top */
#define	    ORIENTATION_RIGHTTOP	6	/* row 0 rhs, col 0 top */
#define	    ORIENTATION_RIGHTBOT	7	/* row 0 rhs, col 0 bottom */
#define	    ORIENTATION_LEFTBOT		8	/* row 0 lhs, col 0 bottom */
#define	TIFFTAG_SAMPLESPERPIXEL		277	/* samples per pixel */
#define	TIFFTAG_ROWSPERSTRIP		278	/* rows per strip of data */
#define	TIFFTAG_STRIPBYTECOUNTS		279	/* bytes counts for strips */
#define	TIFFTAG_MINSAMPLEVALUE		280	/* +minimum sample value */
#define	TIFFTAG_MAXSAMPLEVALUE		281	/* +maximum sample value */
#define	TIFFTAG_XRESOLUTION		282	/* pixels/resolution in x */
#define	TIFFTAG_YRESOLUTION		283	/* pixels/resolution in y */
#define	TIFFTAG_PLANARCONFIG		284	/* storage organization */
#define	    PLANARCONFIG_CONTIG		1	/* single image plane */
#define	    PLANARCONFIG_SEPARATE	2	/* separate planes of data */
#define	TIFFTAG_PAGENAME		285	/* page name image is from */
#define	TIFFTAG_XPOSITION		286	/* x page offset of image lhs */
#define	TIFFTAG_YPOSITION		287	/* y page offset of image lhs */
#define	TIFFTAG_FREEOFFSETS		288	/* +byte offset to free block */
#define	TIFFTAG_FREEBYTECOUNTS		289	/* +sizes of free blocks */
#define	TIFFTAG_GRAYRESPONSEUNIT	290	/* $gray scale curve accuracy */
#define	    GRAYRESPONSEUNIT_10S	1	/* tenths of a unit */
#define	    GRAYRESPONSEUNIT_100S	2	/* hundredths of a unit */
#define	    GRAYRESPONSEUNIT_1000S	3	/* thousandths of a unit */
#define	    GRAYRESPONSEUNIT_10000S	4	/* ten-thousandths of a unit */
#define	    GRAYRESPONSEUNIT_100000S	5	/* hundred-thousandths */
#define	TIFFTAG_GRAYRESPONSECURVE	291	/* $gray scale response curve */
#define	TIFFTAG_GROUP3OPTIONS		292	/* 32 flag bits */
#define	    GROUP3OPT_2DENCODING	0x1	/* 2-dimensional coding */
#define	    GROUP3OPT_UNCOMPRESSED	0x2	/* data not compressed */
#define	    GROUP3OPT_FILLBITS		0x4	/* fill to byte boundary */
#define	TIFFTAG_GROUP4OPTIONS		293	/* 32 flag bits */
#define	    GROUP4OPT_UNCOMPRESSED	0x2	/* data not compressed */
#define	TIFFTAG_RESOLUTIONUNIT		296	/* units of resolutions */
#define	    RESUNIT_NONE		1	/* no meaningful units */
#define	    RESUNIT_INCH		2	/* english */
#define	    RESUNIT_CENTIMETER		3	/* metric */
#define	TIFFTAG_PAGENUMBER		297	/* page numbers of multi-page */
#define	TIFFTAG_COLORRESPONSEUNIT	300	/* $color curve accuracy */
#define	    COLORRESPONSEUNIT_10S	1	/* tenths of a unit */
#define	    COLORRESPONSEUNIT_100S	2	/* hundredths of a unit */
#define	    COLORRESPONSEUNIT_1000S	3	/* thousandths of a unit */
#define	    COLORRESPONSEUNIT_10000S	4	/* ten-thousandths of a unit */
#define	    COLORRESPONSEUNIT_100000S	5	/* hundred-thousandths */
#define	TIFFTAG_TRANSFERFUNCTION	301	/* !colorimetry info */
#define	TIFFTAG_SOFTWARE		305	/* name & release */
#define	TIFFTAG_DATETIME		306	/* creation date and time */
#define	TIFFTAG_ARTIST			315	/* creator of image */
#define	TIFFTAG_HOSTCOMPUTER		316	/* machine where created */
#define	TIFFTAG_PREDICTOR		317	/* prediction scheme w/ LZW */
#define	TIFFTAG_WHITEPOINT		318	/* image white point */
#define	TIFFTAG_PRIMARYCHROMATICITIES	319	/* !primary chromaticities */
#define	TIFFTAG_COLORMAP		320	/* RGB map for pallette image */
#define	TIFFTAG_HALFTONEHINTS		321	/* !highlight+shadow info */
#define	TIFFTAG_TILEWIDTH		322	/* !rows/data tile */
#define	TIFFTAG_TILELENGTH		323	/* !cols/data tile */
#define TIFFTAG_TILEOFFSETS		324	/* !offsets to data tiles */
#define TIFFTAG_TILEBYTECOUNTS		325	/* !byte counts for tiles */
#define	TIFFTAG_BADFAXLINES		326	/* lines w/ wrong pixel count */
#define	TIFFTAG_CLEANFAXDATA		327	/* regenerated line info */
#define	    CLEANFAXDATA_CLEAN		0	/* no errors detected */
#define	    CLEANFAXDATA_REGENERATED	1	/* receiver regenerated lines */
#define	    CLEANFAXDATA_UNCLEAN	2	/* uncorrected errors exist */
#define	TIFFTAG_CONSECUTIVEBADFAXLINES	328	/* max consecutive bad lines */
#define	TIFFTAG_SUBIFD			330	/* subimage descriptors */
#define	TIFFTAG_INKSET			332	/* !inks in separated image */
#define	    INKSET_CMYK			1	/* !cyan-magenta-yellow-black */
#define	TIFFTAG_INKNAMES		333	/* !ascii names of inks */
#define	TIFFTAG_DOTRANGE		336	/* !0% and 100% dot codes */
#define	TIFFTAG_TARGETPRINTER		337	/* !separation target */
#define	TIFFTAG_EXTRASAMPLES		338	/* !info about extra samples */
#define	    EXTRASAMPLE_UNSPECIFIED	0	/* !unspecified data */
#define	    EXTRASAMPLE_ASSOCALPHA	1	/* !associated alpha data */
#define	    EXTRASAMPLE_UNASSALPHA	2	/* !unassociated alpha data */
#define	TIFFTAG_SAMPLEFORMAT		339	/* !data sample format */
#define	    SAMPLEFORMAT_UINT		1	/* !unsigned integer data */
#define	    SAMPLEFORMAT_INT		2	/* !signed integer data */
#define	    SAMPLEFORMAT_IEEEFP		3	/* !IEEE floating point data */
#define	    SAMPLEFORMAT_VOID		4	/* !untyped data */
#define	TIFFTAG_SMINSAMPLEVALUE		340	/* !variable MinSampleValue */
#define	TIFFTAG_SMAXSAMPLEVALUE		341	/* !variable MaxSampleValue */
#define	TIFFTAG_JPEGPROC		512	/* !JPEG processing algorithm */
#define	    JPEGPROC_BASELINE		1	/* !baseline sequential */
#define	    JPEGPROC_LOSSLESS		14	/* !Huffman coded lossless */
#define	TIFFTAG_JPEGIFOFFSET		513	/* !pointer to SOI marker */
#define	TIFFTAG_JPEGIFBYTECOUNT		514	/* !JFIF stream length */
#define	TIFFTAG_JPEGRESTARTINTERVAL	515	/* !restart interval length */
#define	TIFFTAG_JPEGLOSSLESSPREDICTORS	517	/* !lossless proc predictor */
#define	TIFFTAG_JPEGPOINTTRANSFORM	518	/* !lossless point transform */
#define	TIFFTAG_JPEGQTABLES		519	/* !Q matrice offsets */
#define	TIFFTAG_JPEGDCTABLES		520	/* !DCT table offsets */
#define	TIFFTAG_JPEGACTABLES		521	/* !AC coefficient offsets */
#define	TIFFTAG_YCBCRCOEFFICIENTS	529	/* !RGB -> YCbCr transform */
#define	TIFFTAG_YCBCRSUBSAMPLING	530	/* !YCbCr subsampling factors */
#define	TIFFTAG_YCBCRPOSITIONING	531	/* !subsample positioning */
#define	    YCBCRPOSITION_CENTERED	1	/* !as in PostScript Level 2 */
#define	    YCBCRPOSITION_COSITED	2	/* !as in CCIR 601-1 */
#define	TIFFTAG_REFERENCEBLACKWHITE	532	/* !colorimetry info */
/* tags 32952-32956 are private tags registered to Island Graphics */
#define TIFFTAG_REFPTS			32953	/* image reference points */
#define TIFFTAG_REGIONTACKPOINT		32954	/* region-xform tack point */
#define TIFFTAG_REGIONWARPCORNERS	32955	/* warp quadrilateral */
#define TIFFTAG_REGIONAFFINE		32956	/* affine transformation mat */
/* tags 32995-32999 are private tags registered to SGI */
#define	TIFFTAG_MATTEING		32995	/* $use ExtraSamples */
#define	TIFFTAG_DATATYPE		32996	/* $use SampleFormat */
#define	TIFFTAG_IMAGEDEPTH		32997	/* z depth of image */
#define	TIFFTAG_TILEDEPTH		32998	/* z depth/data tile */
/* tags 33300-33309 are private tags registered to Pixar */
/*
 * TIFFTAG_PIXAR_IMAGEFULLWIDTH and TIFFTAG_PIXAR_IMAGEFULLLENGTH
 * are set when an image has been cropped out of a larger image.  
 * They reflect the size of the original uncropped image.
 * The TIFFTAG_XPOSITION and TIFFTAG_YPOSITION can be used
 * to determine the position of the smaller image in the larger one.
 */
#define TIFFTAG_PIXAR_IMAGEFULLWIDTH    33300   /* full image size in x */
#define TIFFTAG_PIXAR_IMAGEFULLLENGTH   33301   /* full image size in y */


/*
 * The following typedefs define the intrinsic size of
 * data types used in the *exported* interfaces.  These
 * definitions depend on the proper definition of types
 * in tiff.h.  Note also that the varargs interface used
 * pass tag types and values uses the types defined in
 * tiff.h directly.
 *
 * NB: ttag_t is unsigned int and not unsigned short because
 *     ANSI C requires that the type before the ellipsis be a
 *     promoted type (i.e. one of int, unsigned int, pointer,
 *     or double).
 * NB: tsize_t is int32 and not uint32 because some functions
 *     return -1.
 * NB: toff_t is not off_t for many reasons; TIFFs max out at
 *     32-bit file offsets being the most important
 */
typedef	unsigned int ttag_t;	/* directory tag */
typedef	uint16 tdir_t;		/* directory index */
typedef	uint16 tsample_t;	/* sample number */
typedef	uint32 tstrip_t;	/* strip number */
typedef uint32 ttile_t;		/* tile number */
typedef	int32 tsize_t;		/* i/o size in bytes */
typedef	void* tdata_t;		/* image data ref */
typedef	void* thandle_t;	/* client data handle */
typedef	int32 toff_t;		/* file offset */

#ifndef NULL
#define	NULL	0
#endif

/*
 * Flags to pass to TIFFPrintDirectory to control
 * printing of data structures that are potentially
 * very large.   Bit-or these flags to enable printing
 * multiple items.
 */
#define	TIFFPRINT_NONE		0x0		/* no extra info */
#define	TIFFPRINT_STRIPS	0x1		/* strips/tiles info */
#define	TIFFPRINT_CURVES	0x2		/* color/gray response curves */
#define	TIFFPRINT_COLORMAP	0x4		/* colormap */
#define	TIFFPRINT_JPEGQTABLES	0x100		/* JPEG Q matrices */
#define	TIFFPRINT_JPEGACTABLES	0x200		/* JPEG AC tables */
#define	TIFFPRINT_JPEGDCTABLES	0x200		/* JPEG DC tables */

/*
 * Macros for extracting components from the
 * packed ABGR form returned by TIFFReadRGBAImage.
 */
#define	TIFFGetR(abgr)	((abgr) & 0xff)
#define	TIFFGetG(abgr)	(((abgr) >> 8) & 0xff)
#define	TIFFGetB(abgr)	(((abgr) >> 16) & 0xff)
#define	TIFFGetA(abgr)	(((abgr) >> 24) & 0xff)

#include <stdio.h>
#include <stdarg.h>

typedef	void (*TIFFErrorHandler)(char* module, char* fmt, va_list);
typedef	tsize_t (*TIFFReadWriteProc)(thandle_t, tdata_t, tsize_t);
typedef	toff_t (*TIFFSeekProc)(thandle_t, toff_t, int);
typedef	int (*TIFFCloseProc)(thandle_t);
typedef	toff_t (*TIFFSizeProc)(thandle_t);
typedef	int (*TIFFMapFileProc)(thandle_t, tdata_t*, toff_t*);
typedef	void (*TIFFUnmapFileProc)(thandle_t, tdata_t, toff_t);

extern	char* TIFFGetVersion(void);

extern	void TIFFClose(TIFF*);
extern	int TIFFFlush(TIFF*);
extern	int TIFFFlushData(TIFF*);
extern	int TIFFGetField(TIFF*, ttag_t, ...);
extern	int TIFFVGetField(TIFF*, ttag_t, va_list);
extern	int TIFFGetFieldDefaulted(TIFF*, ttag_t, ...);
extern	int TIFFVGetFieldDefaulted(TIFF*, ttag_t, va_list);
extern	int TIFFReadDirectory(TIFF*);
extern	tsize_t TIFFScanlineSize(TIFF*);
extern	tsize_t TIFFStripSize(TIFF*);
extern	tsize_t TIFFVStripSize(TIFF*, uint32);
extern	tsize_t TIFFTileRowSize(TIFF*);
extern	tsize_t TIFFTileSize(TIFF*);
extern	tsize_t TIFFVTileSize(TIFF*, uint32);
extern	int TIFFFileno(TIFF*);
extern	int TIFFGetMode(TIFF*);
extern	int TIFFIsTiled(TIFF*);
extern	int TIFFIsByteSwapped(TIFF*);
extern	uint32 TIFFCurrentRow(TIFF*);
extern	tdir_t TIFFCurrentDirectory(TIFF*);
extern	tstrip_t TIFFCurrentStrip(TIFF*);
extern	ttile_t TIFFCurrentTile(TIFF*);
extern	int TIFFReadBufferSetup(TIFF*, tdata_t, tsize_t);
extern	int TIFFLastDirectory(TIFF*);
extern	int TIFFSetDirectory(TIFF*, tdir_t);
extern	int TIFFSetSubDirectory(TIFF*, uint32);
extern	int TIFFUnlinkDirectory(TIFF*, tdir_t);
extern	int TIFFSetField(TIFF*, ttag_t, ...);
extern	int TIFFVSetField(TIFF*, ttag_t, va_list);
extern	int TIFFWriteDirectory(TIFF *);
extern	void TIFFPrintDirectory(TIFF*, FILE*, long);
extern	int TIFFReadScanline(TIFF*, tdata_t, uint32, tsample_t);
extern	int TIFFWriteScanline(TIFF*, tdata_t, uint32, tsample_t);
extern	int TIFFReadRGBAImage(TIFF*, uint32, uint32, uint32*, int stop);
extern	TIFF* TIFFOpen(char*,  char*);
extern	TIFF* TIFFFdOpen(int,  char*,  char*);
extern	TIFF* TIFFClientOpen( char* name,  char* mode,
	    thandle_t clientdata,
	    TIFFReadWriteProc readproc, TIFFReadWriteProc writeproc,
	    TIFFSeekProc seekproc, TIFFCloseProc closeproc,
	    TIFFSizeProc sizeproc,
	    TIFFMapFileProc mapproc, TIFFUnmapFileProc unmapproc);
extern	 char* TIFFFileName(TIFF*);
extern	void TIFFError( char*,  char*, ...);
extern	void TIFFWarning( char*,  char*, ...);
extern	TIFFErrorHandler TIFFSetErrorHandler(TIFFErrorHandler handler);
extern	TIFFErrorHandler TIFFSetWarningHandler(TIFFErrorHandler handler);
extern	ttile_t TIFFComputeTile(TIFF*, uint32, uint32, uint32, tsample_t);
extern	int TIFFCheckTile(TIFF*, uint32, uint32, uint32, tsample_t);
extern	ttile_t TIFFNumberOfTiles(TIFF*);
extern	tsize_t TIFFReadTile(TIFF*,
	    tdata_t, uint32, uint32, uint32, tsample_t);
extern	tsize_t TIFFWriteTile(TIFF*,
	    tdata_t, uint32, uint32, uint32, tsample_t);
extern	tstrip_t TIFFComputeStrip(TIFF*, uint32, tsample_t);
extern	tstrip_t TIFFNumberOfStrips(TIFF*);
extern	tsize_t TIFFReadEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFReadRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteEncodedStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteRawStrip(TIFF*, tstrip_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteEncodedTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	tsize_t TIFFWriteRawTile(TIFF*, ttile_t, tdata_t, tsize_t);
extern	void TIFFSetWriteOffset(TIFF*, toff_t);
extern	void TIFFSwabShort(uint16 *);
extern	void TIFFSwabLong(uint32 *);
extern	void TIFFSwabArrayOfShort(uint16 *, unsigned long);
extern	void TIFFSwabArrayOfLong(uint32 *, unsigned long);
extern	void TIFFReverseBits(unsigned char *, unsigned long);
extern	 unsigned char* TIFFGetBitRevTable(int);
extern	void TIFFModeCCITTFax3(TIFF* tif, int isClassF);	/* XXX */


#define ALLOW_JPEG 0  /* set to '1' to allow 'JPEG' choice in dialog box */
#define PIC24 1


/*********************************************/
void setupColormap(TIFF *tif, byte *rmap, byte *gmap, byte *bmap)
{
  short red[256], green[256], blue[256];
  int i;
  
  /* convert 8-bit colormap to 16-bit */
  for (i=0; i<256; i++) {
#define	SCALE(x)	((((int)x)*((1L<<16)-1))/255)
    red[i] = SCALE(rmap[i]);
    green[i] = SCALE(gmap[i]);
    blue[i] = SCALE(bmap[i]);
  }
  TIFFSetField(tif, TIFFTAG_COLORMAP, red, green, blue);
}



/*******************************************/
int WriteTIFF(FILE *fp, byte *pic, int ptype, int w, int h, int comp,
  byte *rmap, byte *gmap, byte *bmap, int numcols, int colorstyle,
  char *fname, char *comment)
{
  TIFF *tif;
  byte *pix;
  int   i,j;

  tif = TIFFOpen(fname, "w");

  if (!tif) return 0;

  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
  TIFFSetField(tif, TIFFTAG_COMPRESSION, comp);

  if (comment && strlen(comment)>0) {
    TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, comment);
  }

  if (comp == COMPRESSION_CCITTFAX3)
      TIFFSetField(tif, TIFFTAG_GROUP3OPTIONS,
	  GROUP3OPT_2DENCODING+GROUP3OPT_FILLBITS);

  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, h);

  TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, (int)2);
  TIFFSetField(tif, TIFFTAG_XRESOLUTION, (float)1200.0);
  TIFFSetField(tif, TIFFTAG_YRESOLUTION, (float)1200.0);


  /* write the image data */

  if (ptype == PIC24) {  /* only have to deal with FULLCOLOR or GREYSCALE */
    if (colorstyle == F_FULLCOLOR) {
      TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   8);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_RGB);
      
      TIFFWriteEncodedStrip(tif, 0, pic, w*h*3);
    }

    else {  /* colorstyle == F_GREYSCALE */
      byte *tpic, *tp, *sp;

      TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE,   8);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC,    PHOTOMETRIC_MINISBLACK);

      tpic = (byte *) malloc((size_t) w*h);
      if (!tpic) FatalError("unable to malloc in WriteTIFF()");

      for (i=0, tp=tpic, sp=pic; i<w*h; i++, sp+=3) 
	*tp++ = MONO(sp[0],sp[1],sp[2]);
      
      TIFFWriteEncodedStrip(tif, 0, tpic, w*h);

      free(tpic);
    }
  }

  else {  /* PIC8 */
    if (colorstyle == F_FULLCOLOR) {                  /* 8bit Palette RGB */
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
      setupColormap(tif, rmap, gmap, bmap);
      TIFFWriteEncodedStrip(tif, 0, pic, w*h);
    }

    else if (colorstyle == F_GREYSCALE) {             /* 8-bit greyscale */
      byte rgb[256];
      byte *tpic = (byte *) malloc((size_t) w*h);
      byte *tp = tpic;
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
      for (i=0; i<numcols; i++) rgb[i] = MONO(rmap[i],gmap[i],bmap[i]);
      for (i=0, pix=pic; i<w*h; i++,pix++) {
	/*	if ((i&0x7fff)==0) WaitCursor(); */
	*tp++ = rgb[*pix];
      }
      TIFFWriteEncodedStrip(tif, 0, tpic, w*h);
      free(tpic);
    }

    else if (colorstyle == F_BWDITHER) {             /* 1-bit B/W stipple */
      int bit,k,flipbw;
      byte *tpic, *tp;

      flipbw = (MONO(rmap[0],gmap[0],bmap[0]) > MONO(rmap[1],gmap[1],bmap[1]));
      TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 1);
      TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
      tpic = (byte *) malloc((size_t) TIFFStripSize(tif));
      tp = tpic;
      for (i=0, pix=pic; i<h; i++) {
	/*	if ((i&15)==0) WaitCursor(); */
	for (j=0, bit=0, k=0; j<w; j++, pix++) {
	  k = (k << 1) | *pix;
	  bit++;
	  if (bit==8) {
	    if (flipbw) k = ~k;
	    *tp++ = (byte) (k&0xff);
	    bit = k = 0;
	  }
	} /* j */
	if (bit) {
	  k = k << (8-bit);
	  if (flipbw) k = ~k;
	  *tp++ = (byte) (k & 0xff);
	}
      }
      TIFFWriteEncodedStrip(tif, 0, tpic, TIFFStripSize(tif));
      free(tpic);
    }
  }

  TIFFClose(tif);

  return 0;
}


/* dummy routines to minimize tiff library calls and to print
   error message */

int FatalError(char *in)
{
  fprintf(stderr,"*** TIFF error '%s'\n",in);
  exit(-1);
}

int __eprintf(char *in)
{
  fprintf(stderr,"*** TIFF error 1 '%s'\n",in);
  return(0);
}

int _fxstat()
{
  return(0);
}


