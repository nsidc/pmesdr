/* copyright 1997, 2000, 2003, 2011 by David Long, BYU all rights reserved */
/*
   program to read BYU SIR image files and create a .gif file

   Written by DGL March 26, 1997
   Revised by DGL Sept. 8, 1997
   + added color table file and did clean up
   Revised by DGL Jan. 11, 2000
   + added optional scale factor, made output file name optional
   Revised by DGL Jan. 13, 2000
   + added image plotting capability
   Revised by DGL Mar. 11, 2000
   + modified for generalized plotting capability
   Revised by DGL June 26, 2000
   + modified for compatibility with xsir6.c 
   Revised by DGL Feb. 1, 2003
   + sir file header version 3.0
   Revised by DGL Sep. 21, 2011
   + fixed a few printf format statements
   Revised by DGL Feb. 28, 2012
   + properly support EASE grid resizing (alter call to plot_region_resize)

*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* define templates for routines from csir library (sir_io.c) */

#include "sir3.h"

/* writegif template (below) */

int writegif(char *name_in, int *len_in, char *pic, int *pw, int *ph,
	     char *rmap, char *gmap, char *bmap, 
	     int *pnumcols, int *pcolorstyle);


/* get routine templates for sir image plotting library and global variables */

#include "sirllplt3.h"


int class, SF, dsize, depth;           /* X display properties */

/* main program */

int main(int argc, char **argv)
{
  FILE *imf;
  int i, j, k;
  char outname[200], optname[200], line[200], *c, *c2, *cp;  
  FILE *fid;
  int base = 0;
  float smax1,smin1,smin2;

/* other SIR file header information */

  float v_min, v_max, anodata;
  int ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc;
  int ioff, iscale, iyear, isday, ismin, ieday, iemin;
  int iregion, itype, nhead, ndes, nhtype, idatatype, ldes, nia;
  int ipol, ifreqhm, ispare1;
  char title[101], sensor[41], crproc[101], type[139], tag[101], crtime[29];

#define MAXDES 1024
  char descrip[MAXDES+1];
#define MAXI 128
  short iaopt[MAXI];

  int ierr, in, ninputs;
  char *data;

  int nsx1, nsy1, iopt1;
  float xdeg1, ydeg1, ascale1, bscale1, a01, b01;  /* SIR header location info */
  float anodata1;
 
 
  fprintf(stdout,"BYU SIR to GIF conversion program w/scaling & plotting\n") ;
  if (argc < 2) {
    fprintf(stdout,"\nusage: %s <options> sir_in <gif_out> <dmin> <dmax> <ctab>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   options =  dashed options (see below)\n");
    fprintf(stdout,"   sir_in  =  input SIR file\n");
    fprintf(stdout,"   gif_out =  output gif file (optional,def='sir_in'.gif)\n");
    fprintf(stdout,"   dmin    =  min saturation value (optional,def=vmin)\n");
    fprintf(stdout,"   dmax    =  max saturation value (optional,def=vmax)\n");
    fprintf(stdout,"   ctab    =  ascii color table file (optional,def=greyscale)\n");

    print_options_info(stdout);
    fprintf(stdout,"\nNote: output is 8 bits.\n");
    
    return(0);
  }

  /* check for optional initial arguments */

  base=0;
  i=1;
  while ( i < argc && argv[i] != NULL && *argv[i] != '\0' && *(argv[i]) == '-') {   /* optional argument */
    base=i;
    if (*(argv[i]+1) == 'd') {   /* debug on */
      printf("Option debug ON\n");
      debug = 1;
    }
    if (*(argv[i]+1) == 'f') {   /* check for file if optional argument is file input */
      sscanf(argv[i],"-f%s",optname);
      fprintf(stdout,"Opening options input file '%s'\n",optname);
      fid=fopen(optname,"r");
      if (fid == NULL) {
	fprintf(stderr,"*** Could not open options file '%s' ***\n",optname);
	exit(-1);
      }
      fclose(fid);
    }
    i++;
  }
  
  if (debug) printf(" debug: Base input file name %s\n",argv[1+base]);  
  
  /* get input file names */

  ninputs = get_input_files(argv,argc,argv[1+base],names);
  if (ninputs == 0 || names[0] == NULL) {
    fprintf(stdout,"*** Error: no input file \n");
    return(0);
  }
  if (debug) {
    printf("debug: Got a total of %d input files\n",ninputs);
    for (i=0; i< ninputs; i++) 
      printf("debug: input file %d: %s\n",i,names[i]);
  }
  
  /* open and read input files */

  for (in = 0; in < ninputs; in++){

   if (names[in] != NULL) {
    fid = fopen(names[in],"r"); 
    if (fid == NULL) {
      fprintf(stdout,"*** ERROR: cannot open input file %d: %s\n",in,names[in]); 
      exit(-1);
    }

  /* get SIR image header information */

    ierr = read_sir_header3(fid, &nhead, &ndes, &nhtype,
			    &idatatype, &nsx1, &nsy1, 
			    &xdeg1, &ydeg1, &ascale1, &bscale1, &a01, &b01, 
			    &ixdeg_off, &iydeg_off, &ideg_sc,
			    &iscale_sc, &ia0_off, &ib0_off, &i0_sc,
			    &ioff, &iscale, &iyear, &isday, &ismin, &ieday, &iemin, 
			    &iregion, &itype, &iopt1, &ipol, &ifreqhm, &ispare1,
			    &anodata1, &v_min, &v_max,
			    sensor, title, type, tag, crproc, crtime, 
			    MAXDES, descrip, &ldes, MAXI, iaopt, &nia);  
    if (ierr < 0) {
      fprintf(stdout,"*** Error reading SIR header from file '%s'\n",names[in]);
      exit(-1);
    }

    if (debug) {    /* print SIR header information */      
      fprintf(stdout,"\nSIR file header: '%s'\n",names[in]);
      print_head3(stdout, nhead, ndes, nhtype, idatatype, nsx1, nsy1, 
		  xdeg1, ydeg1, ascale1, bscale1, a01, b01, 
		  ixdeg_off, iydeg_off, ideg_sc,
		  iscale_sc, ia0_off, ib0_off, i0_sc,
		  ioff, iscale, iyear, isday, ismin, ieday, iemin, 
		  iregion, itype, iopt1, ipol, ifreqhm, ispare1,
		  anodata1, v_min, v_max,
		  sensor, title, type, tag, crproc, crtime, 
		  descrip, ldes, iaopt, nia);  
    }

    if (in == 0) {  /* standard image size/projection */
      nsx = nsx1;
      nsy = nsy1;
      iopt = iopt1;
      xdeg = xdeg1;
      ydeg = ydeg1;
      ascale = ascale1;
      bscale = bscale1;
      a0 = a01;
      b0 = b01;
      anodata = anodata1;
      for (i=0; i < OMAX; i++) {
	smin[i] = v_min;
	smax[i] = v_max;
	nc_opt[i] = 256;
      }
    }

    in_anodata[in] = anodata1;
    
    if (nsx != nsx1 || nsy != nsy1) {
      /* || xdeg != xdeg1 || ydeg != ydeg1 || 
	 ascale != ascale1 || bscale != bscale1 || a0 != a01 || b0 != b01) { */
      fprintf(stdout,"*** Error: input images not size compatible *** \n");
      exit(-1);
    }

    /* allocate image storage */

    in_stval[in] = (float *) malloc(sizeof(float) * nsx1 * nsy1);
    if (in_stval[in] == NULL) {
      fprintf(stderr,"*** ERROR: image %d memory allocation failure...\n",in);
      exit(-1);
    }

  /* read image data from file */

    if (read_sir_data(fid, nhead, idatatype, nsx1, nsy1, ioff, iscale, in_stval[in]) != 0) {
      fprintf(stdout,"*** Error reading SIR image data from file ***\n");
      exit(-1);
    }
    fprintf(stdout,"Successfully read SIR file %s (%d of %d) %f %f %f\n",names[in], in+1, ninputs, in_stval[in],in_anodata[in],anodata1);
   }
  }
  

  /* generate output file name */

  f_opt = names[0];
  while ((c = strrchr(f_opt,'/')) != NULL) /* strip path off input file */
    f_opt = c+1;
  k=sprintf(outname,"%s.gif",f_opt);       /* default output name */
  if (argc > 2+base)                       /* override output name */
    sscanf(argv[2+base],"%s",outname);

  if (outname[0] == '-') {
    printf("*** ERROR in output name, contains leading dash: %s\n",outname);
    printf("*** program terminated ***\n");
    exit(-1);
  }

  /* get image pre-processing options */

  nchans = get_input_preprocess_lines(argv,argc,command_lines,out_anodata,anodata,debug);

  if (nchans == 0) {
    stval[0] = in_stval[0];
    if (debug) printf(" debug: only one channel input\n");    
  } else {
    if (debug) printf(" debug: preprocess lines: %d  Beginning codify\n",nchans);
    nchans = set_image_preprocess(command_lines,nchans,rpn_depth,rpn_list,
				  ninputs,in_stval,in_anodata,out_anodata,debug);
    if (debug) printf(" debug: preprocess codify complete: %d\n",nchans);
    for (i=0; i < nchans; i++) {
      stval[i] = in_stval[i];  /* default copy */
    }
    for (i=0; i < nchans; i++) {
      if (rpn_depth[i] > 0) {
	if (debug) printf(" debug: preprocessing channel %d\n",i+1);
	stval[i] = image_preprocess(rpn_list[i], rpn_depth[i], nsx * nsy, anodata, debug);
	if (debug) printf(" debug: done preprocessing channel %d\n",i+1);
      }
    }
    if (debug) printf(" debug: preprocessing complete\n\n");
    free_input_arrays(stval, nchans, in_stval, ninputs);
    if (debug) {
      printf(" debug: extra memory freed up\n");
      printf(" debug: channel arrays: %d %d\n",nchans, nsx * nsy);
      for (i=0; i <= nchans; i++)
	if (stval[i] != NULL)
	  printf("   %d: %f %f\n",i+1,stval[i],in_anodata[i]);
    }
  }

  if (debug) {  /* preprocessing results */
    for (j=0; j < nchans; j++) {
      if (stval[j] != NULL) {
	smin1 = 1.e25;
	smin2 = 1.e25;
	smax1 = -1.e25;
	for (i=0; i < nsx * nsy; i++)
	  if (*(stval[j]+i) > anodata) {
	    if (*(stval[j]+i) > smax1) smax1 = *(stval[j]+i);
	    if (*(stval[j]+i) < smin1) smin1 = *(stval[j]+i);
	    if (*(stval[j]+i) < smin2) smin2 = *(stval[j]+i);
	  } else
	    if (*(stval[j]+i) < smin2) smin2 = *(stval[j]+i);
	printf(" debug: channel %d (%f) data range: [%f] %f to %f\n",j+1,stval[j],smin2,smin1,smax1);
      }
    }
  }
  

  /* initialize color tables, plotting */
  
  plot_initialize(rtab, gtab, btab, &ncol, nchans);

  SF = 1;     /* multi-byte scale factor */
  class = 0;  
  depth = 8;  /* bits/pixel in output */
  dsize =1;   /* output bytes/pixel */


  /* pass through plot options to determine sub-sizing, scale changing, etc. */
  
  if (plot_pass_one(argv) != 0) {
     fprintf(stdout,"*** Error processing options pass one ***\n");
     exit(-1);
  }

  /* resize image if selected */

  if (npix != 0 && npix != 1) {
    if (plot_region_resize(out_anodata, stval, nchans, npix, 
			   &nsx, &nsy, &ascale, &bscale, 
			   iopt, &xdeg, &ydeg, &a0, &b0) != 0) {
      fprintf(stdout,"*** Error resizing image ***\n");
      exit(-1);
    }
  }
  
  /* override initial options if later options are present */

  if (argc > 3+base) sscanf(argv[3+base],"%f",&smin[0]); 
  if (argc > 4+base) sscanf(argv[4+base],"%f",&smax[0]); 
  if (argc > 5+base) /* read user input color table file */
    read_colortable(argv[5+base],rtab,gtab,btab);

  /* allocate byte image */

  data = (char *) malloc(sizeof(char) * nsx * nsy * dsize);
  if (data == NULL) {
     fprintf(stderr,"*** ERROR: byte image memory allocation failure...\n");
     exit(-1);
  }

  /* create byte image */
  printf(" scaling: no data=%d [%f,%f] to [%d,%d] %d\n",
	     z_opt,smin[0],smax[0],s_opt,e_opt,nchans);
  /* 
  make_byte_image(data, stval, smin, smax, nsx, nsy, nchans,
		  s_opt, e_opt, nc_opt, z_opt, out_anodata, debug);
  */
  make_byte_image2(data, stval, smin, smax, nsx, nsy, nchans,
		   s_opt, e_opt, nc_opt, z_opt, out_anodata, debug, SF, dsize,
		   rtab, gtab, btab);

  /* second pass through options to plot */

  if (plot_pass_two(argv, data, stval[0], nsx, nsy) !=0) {
     fprintf(stdout,"*** Error processing options pass two ***\n");
     exit(-1);
  }

  /* rotate output image */

  if (rot_opt != 0) 
    rotate_image(rot_opt, data, &nsx, &nsy);
  

  /* write output gif image */

  fprintf(stdout,"\nImage range Min, Max: %f , %f\n",smin[0],smax[0]);
  fprintf(stdout,"Color table scaling: [%f, %f] -> [%d, %d]\n",smin[0],smax[0],s_opt,e_opt);
  if (z_opt >= 0)
    fprintf(stdout,"Color table nodata value: %f -> %d\n",out_anodata[0],z_opt);
  fprintf(stdout,"Pixel reduction factor: %d\n",npix);
  fprintf(stdout,"Output image size in pixels: %d x %d\n",nsx,nsy);

  fprintf(stdout,"Writing output gif file to '%s'\n", outname);
  k=strlen(outname);
  i=0;
  ierr=writegif(outname, &k, data, &nsx, &nsy, rtab, gtab, btab, &j, &i);
  if (ierr < 0) {
    fprintf(stderr,"*** ERROR writing output file ***\n");
    fflush(stderr);
  } else
    fprintf(stdout,"gif output file successfully written %d\n",ierr);
    
  return(0);
}



/*
 * xvgifwr.c  -  handles writing of GIF files.  based on flgife.c and
 *               flgifc.c from the FBM Library, by Michael Maudlin
 *
 * Contains: 
 *   WriteGIF(fp, pic, w, h, rmap, gmap, bmap, numcols, colorstyle)
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


typedef long int        count_int;

static int  Width, Height;
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
  int ColorMapSize, InitCodeSize, Background, BitsPerPixel;
  int i,j,w,h,numcols,colorstyle;

  char fname[120];

  w = *pw;
  h = *ph;
  numcols = *pnumcols;
  colorstyle = *pcolorstyle;
  
  for (i=0; i < *len_in; i++) fname[i] = *(name_in+i);	
  fname[*len_in] = 0;
  fp = fopen(&fname[0],"w");
  /* printf("Writing output file '%s'\n",&fname[0]); */

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
	
  if (BitsPerPixel <= 1) InitCodeSize = 2;
                    else InitCodeSize = BitsPerPixel;

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

/* #define min(a,b)        ((a>b) ? b : a) */

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


