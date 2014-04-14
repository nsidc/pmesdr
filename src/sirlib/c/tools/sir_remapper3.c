/* (c) 2001, 2005, 2014 BYU MERS Laboratory

   This program remaps BYU SIR images into a different projection

   Written by DGL 30 April 2001
   Revised by DGL  1 Dec 2001 + add option for selecting processing type
   Revised by DGL 10 Dec 2001 + added additional command line options
   Revised by DGL 17 Dec 2001 + added standard regions
   Revised by DGL 25 Jul 2005 + added standard region clipping in copy
   Revised by DGL  7 Mar 2014 + include EASE2

   This simple program reads one or more BYU SIR-format input 
   files using "EASY SIR" routines.  The image pixel values are
   remapped onto a different sir projection and written to an output 
   file.  Remapping requires computing the lat/lon coordinates of each
   and so may require considerable CPU resources.  A variety of command line
   options enable setting of header variables.  The recommended approach is
   to use a template file.

   Remapping algorithm summary: Option 1 (averaging + nearest-neighbor)

   1. For each input pixel, compute lat/lon of pixel center using the SIR
       pixel -> lat/lon routines
   2. For each input pixel lat/lon, determine the output pixel in the output
       image using SIR lat/lon -> pixel routines.  If multiple input pixels
       hit the same output pixel, they are averaged.
   4. Scan output image.  For each empty output pixel, compute the
       corresponding pixel center lat/lon
   5. Compute the input pixel location from the lat/lon and copy the value.

   Remapping algorithm summary: Option 2 (nearest-neighbor)

   1. For each pixel in the output image, compute lat/lon of pixel center 
       using the SIR pixel -> lat/lon routines
   2. Compute the input pixel location from the lat/lon and copy the value.


   This program should be linked with 
    sir_ez.c
    sir_geom.c
    sir_io.c
    sir_ez.h
    sir3.h

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "sir_ez.h"   /* get easy sir routine interface */

#define MAXN 20       /* limit to number of input file names */
#define MAXp 50       /* maximum number of -P arguments */
#define MAXLINE 1024  /* maximum line length of input */

#define STD_REGION_FILE "regiondef1.dat"  /* name of standard region def file */

#define DEBUG 0       /* set to 1 to print extra debugging lines, 0 to suppress */


/* function prototypes */

void define_sir_head(sir_head *out_head, int reset_flag, FILE *, 
		     int nSarg, char **Sarg_param, char **Sarg_value,
		     int use_standard_region, char *std_region_num, char *pix_size);

void Sarg_process(sir_head *out_head, int nSarg, char **Sarg_param, char **Sarg_value);

int getline1 (FILE *imf, char *line, int max_length);

void list_header_variables(FILE *imf);

void copy_sir_head_args(sir_head *out_head, int nxarg, char **xarg_param, sir_head *in_head);

void process_standard_region(sir_head *head, char *std_region_num, char *pix_size);

void reset_header(sir_head *head);

void set_header_projection_scale_factors(sir_head *head);

void clip_extent(float *ll_lat, float *ll_lon, float *ur_lat, float *ur_lon, sir_head *head);

/* main program */

int main(int argc, char **argv)
{
  FILE  *imf;
  int   i, j, k, n, nsx, nsy, ix, iy, x, y;
  int   xmin[MAXN], xmax[MAXN], ymin[MAXN], ymax[MAXN];
  float x1, y1, alon, alat;
  char  *out = NULL, *template = NULL, fname[120];
  char  *in[MAXN], *Sarg_param[MAXp], *Sarg_value[MAXp], *xarg_param[MAXp];
  char  *std_region_num, *pix_size;
  int   ierr, Nfiles = 0, nSarg = 0, nxarg = 0, last_iopt = -100;
  int   processing_option = 0, print_headers = 0, revise_head_flag = 0;
  int   Dset = 0, use_standard_region = 0, clip_option = 0;
  int   nDarg = 10;
  char  *Darg_param[][7] = {"IYEAR", "ISDAY", "ISMIN", "IEDAY", "IEMIN", 
			    "ITYPE", "IPOL", "IFREQHM", "SENSOR", "TYPE"};
  float *stval[MAXN];  /* pointers to input image storage */
  float *outval;       /* pointer to output storage */
  short *cnt;          /* pointer to count image storage */
  float ll_lat[MAXN], ll_lon[MAXN], ur_lat[MAXN], ur_lon[MAXN];
  
  sir_head *in_head, out_head;


  fprintf(stdout,"BYU SIR remapping program\n");

  sir_init_head(&out_head);   /* initialize working header */
  reset_header(&out_head);    /* set default header values */
  

  /* count input files and check for optional args */

  for (i=1; i < argc; i++) {
    if (*argv[i] == '-') {
      if (*(argv[i]+1) == 'b' || *(argv[i]+1) == 'B')
	if (i+2 < argc) {
	  std_region_num = argv[++i];
	  pix_size = argv[++i];
	  use_standard_region = 1;
	}
      if (*(argv[i]+1) == 'c' || *(argv[i]+1) == 'C')
	clip_option = 1;
      if (*(argv[i]+1) == 'n' || *(argv[i]+1) == 'N')
	processing_option = 1;
      if (*(argv[i]+1) == 'h' || *(argv[i]+1) == 'H')
	list_header_variables(stdout);
      if (*(argv[i]+1) == 'p' || *(argv[i]+1) == 'P')
	print_headers = 1;
      if (*(argv[i]+1) == 'o' || *(argv[i]+1) == 'O')
	if (i+1 < argc)
	  out = argv[++i];
      if (*(argv[i]+1) == 'i' || *(argv[i]+1) == 'I')
	revise_head_flag = 1;
      if (*(argv[i]+1) == 's' || *(argv[i]+1) == 'S')
	if (i+2 < argc) {
	  Sarg_param[nSarg] = argv[++i];
	  Sarg_value[nSarg] = argv[++i];
	  nSarg++;
	}
      if (*(argv[i]+1) == 'd' || *(argv[i]+1) == 'D')
	Dset = 1;
      if (*(argv[i]+1) == 'x' || *(argv[i]+1) == 'X')
	if (i+1 < argc)
	  xarg_param[nxarg++] = argv[++i];
      if (*(argv[i]+1) == 't' || *(argv[i]+1) == 'T')
	if (i+1 < argc)
	  template = argv[++i];
    } else
      in[Nfiles++] = argv[i];
  }
  if (DEBUG) fprintf(stdout,"Nfiles=%d  nSarg=%d\n",Nfiles,nSarg);

  if (argc < 2 || Nfiles == 0) {
    fprintf(stdout,"\nusage: %s <options> in_file1 in_file2 ...\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   <options> (upper or lower case accepted for option letter code)\n");
    fprintf(stdout,"     -b <std region num> <pix size> = use standard region definition\n");
    fprintf(stdout,"        read from local file '%s' with pixel size factor\n",STD_REGION_FILE);
    fprintf(stdout,"        pix size: 0=SSM/I,1=ERS,2=SASS/NSCAT/QSCAT_Egg,3=QSCAT_slice\n");
    fprintf(stdout,"        (roughly 0=17.8, 1=8.9, 2=4.45, 3=2.225, 4=1.1125 km/pix) to create new header\n");
    fprintf(stdout,"     -c = clip input image from standard region ('%s') to reduce edge effects\n",STD_REGION_FILE);
    fprintf(stdout,"     -d = copy key date/time header variables from 1st input SIR to output\n");
    fprintf(stdout,"          equiv to -x p for p={IYEAR...IEMIN ITYPE IPOL IFREQHM SENSOR TYPE} \n");
    fprintf(stdout,"     -h = list file header variables\n");
    fprintf(stdout,"     -i = interactively modify output SIR header\n");
    fprintf(stdout,"     -o out = name of output SIR file [default=out.sir]\n");
    fprintf(stdout,"     -p = print out file headers\n");
    fprintf(stdout,"     -s <name> <value> = set header parameter <name> to <value>\n");
    fprintf(stdout,"     -t template = name of SIR file used as header template\n");
    fprintf(stdout,"     -x <name> = copy header value <name> from 1st input SIR to output\n");
    fprintf(stdout,"     -A = forward average+nearest neighbor pixel copying [default]\n");
    fprintf(stdout,"     -N = nearest neighbor pixel copying (over rides -A)\n");
    fprintf(stdout,"   in_file1  =  input SIR file\n");
    fprintf(stdout,"   in_file2  =  input SIR file, etc.\n");
    return(0);
  }
 
  if (Nfiles == 0) exit(0);

  /* allocate storage space for input file headers */
  
  in_head = (sir_head *) malloc(Nfiles * sizeof(sir_head));
  if (in_head == NULL) {
     fprintf(stderr,"*** ERROR: input image header memory allocation failure...\n");
     exit(-1);
  }


  /* determine output file name if not specified */

  if (out == NULL) {  
    (void) strncpy(fname,"out.sir",120);
    out = fname;
  }
  if (DEBUG) fprintf(stdout,"Output file: %s\n",out);


  /* define output file header */

  if (template != NULL) {
    fprintf(stdout,"Using SIR template file '%s'\n",template);
    ierr = get_sir_head_name(template, &out_head, &imf);
    if (ierr < 0) {
      fprintf(stderr,"*** ERROR: can not read SIR template file... %d\n",ierr);
      exit(-1);
    }
    fclose(imf);

    last_iopt = out_head.iopt;
    if (use_standard_region == 1) {
      process_standard_region(&out_head, std_region_num, pix_size);
      if (out_head.iopt != last_iopt) {
	set_header_projection_scale_factors(&out_head);
	fprintf(stdout,"Projection parameter scaling reset\n");
      }
    }

    if (nSarg > 0)
      Sarg_process(&out_head, nSarg, Sarg_param, Sarg_value);

    if (revise_head_flag == 1)
      define_sir_head(&out_head, 1, stdin, nSarg, Sarg_param, Sarg_value,
		      use_standard_region, std_region_num, pix_size);

  } else

    define_sir_head(&out_head, 0, stdin, nSarg, Sarg_param, Sarg_value,
		      use_standard_region, std_region_num, pix_size);
  
  /* allocate storage space for output image data */
  
  outval = sir_data_alloc(&out_head);
  if (outval == NULL) {
     fprintf(stderr,"*** ERROR: output image memory allocation failure...\n");
     exit(-1);
  }


  /* initialize output image */

  for (i=0; i < out_head.nsx * out_head.nsy; i++)
    *(outval+i) = 0.0;

  
  /* allocate and initialize counter array */

  cnt = (short *) malloc(sizeof(short) * out_head.nsx * out_head.nsy);

  for (i=0; i < out_head.nsx * out_head.nsy; i++)
    *(cnt+i) = 0;


  /* use easy SIR interface routines to read in input files.  This routine
     allocates array storage for each input file as needed.  Will fail and
     return zero on failure. */

  for (n=0; n < Nfiles; n++) {
    fprintf(stdout,"Input data file %d '%s'\n",n+1,in[n]);
    if (get_sir(in[n], &in_head[n], &stval[n]) != 0) {
      fprintf(stdout,"*** ERROR: cannot open/read input file: %s\n",in[n]);
      fprintf(stdout,"...exiting...\n");
      exit(-1);
    }
    
    if (print_headers == 1) {      /* print input SIR file header(s) */
      fprintf(stdout,"\nInput SIR file %d header: %s\n",n+1,in[n]);
      print_sir_head(stdout, &in_head[n]);
    }

    if (clip_option == 1) /* read standard region clipping extent */
      clip_extent(&ll_lat[n], &ll_lon[n], &ur_lat[n], &ur_lon[n], &in_head[n]);

  }
  

  /* copy any desired header values from 1st input SIR file to output header */

  if (Dset == 1)  /* a standard list */
    copy_sir_head_args(&out_head, nDarg, (char **) Darg_param, &in_head[0]);

  if (nxarg > 0)  /* user specified list */
    copy_sir_head_args(&out_head, nxarg, xarg_param, &in_head[0]);

  if (print_headers == 1) {     /* print output SIR file header */
    fprintf(stdout,"\nOutput SIR file header: %s\n",out);
    print_sir_head(stdout, &out_head);
  }


  /* based on processing option, copy pixels into output image array */

  if (processing_option == 0) {

    /* Step 1, copy input pixels into output image, averaging multiple hits */

    for (n=0; n < Nfiles; n++) {   /* for each input file */

      nsx=in_head[n].nsx;
      nsy=in_head[n].nsy;    

      xmin[n]=out_head.nsx;   /* intialize output pixel range for each input */
      xmax[n]=1;
      ymin[n]=out_head.nsy;
      ymax[n]=1;

      fprintf(stdout,"Forward Average file %d: %d x %d -> %d x %d\n",n+1,nsx,nsy,out_head.nsx,out_head.nsy);
      k=0;

      for (iy = 1; iy <= nsy; iy++)          /* for all input image pixels */
	for (ix = 1; ix <= nsx; ix++) {
	  i = sir_lex(ix, iy, &in_head[n]);  /* get image pixel index  0 ... nsx*nxy-1 */

	  if (*(stval[n]+i) > in_head[n].anodata + 0.00001) { /* if valid pixel value */

	    sir_pix2latlon(0.5+(float) ix, 0.5+(float) iy, &alon, &alat, &in_head[n]);  /* (ix,iy) -> (lat,lon) */

	    if (clip_option == 0 ||
		clip_option == 1 && alat >= ll_lat[n] && alat <= ur_lat[n] &&
		                    alon >= ll_lon[n] && alon <= ur_lon[n]) {

	      j = sir_latlon2pix(alon, alat, &x1, &y1, &out_head); /* (lat,lon) -> (x,y) */
	      
	      if (j >= 0) {
		
		k++;           /* count of pixels copied */
		(*(cnt+j))++;  /* increment counter image */
		*(outval+j) = *(outval+j) + *(stval[n]+i);  /* output image */
		
		/* determine the range of output image pixels covered */
		
		(void) isir_lex(&x, &y, j, &out_head);
		
		xmin[n] = (x < xmin[n] ? x : xmin[n]);	  
		xmax[n] = (x > xmax[n] ? x : xmax[n]);	  
		ymin[n] = (y < ymin[n] ? y : ymin[n]);	  
		ymax[n] = (y > ymax[n] ? y : ymax[n]);	  
	      }
	    }
	  }
	}
      if (DEBUG) fprintf(stdout," Pixels copied: %d of %d\n",k,nsx*nsy);
    
    /* enlarge search area slightly to ensure full coverage in the inverse mapping

     note: these range values are used to minimize computation when the output image
     is larger than the input -- only some output pixels need to be checked in
     nearest neighbor inverse algorithm section */

      xmin[n] = (xmin[n]-4 > 0 ? xmin[n]-4 : 1);	  
      xmax[n] = (xmax[n]+4 < out_head.nsx+1 ? xmax[n]+4 : out_head.nsx);	  
      ymin[n] = (ymin[n]-4 > 0 ? ymin[n]-4 : 1);	  
      ymax[n] = (ymax[n]+4 < out_head.nsy+1 ? ymax[n]+4 : out_head.nsy);

    }
    
  } else {

    /* when the forward average is not used, search all output pixels for nearest 
       neighbor inverse algorithm section */

    for (n=0; n < Nfiles; n++) {
      xmin[n] = 1;
      xmax[n] = out_head.nsx;
      ymin[n] = 1;
      ymax[n] = out_head.nsy;
    }
    
  }


  /* Nearest neighbor pixel copying.  Fill output array with nearest input
     array pixel from each input array, averaging as needed.  (note: a negative
     counter is used here to simplify the test for filling a pixel with the 
     nearest-neighbor from multiple source files*/

  nsx = out_head.nsx;
  nsy = out_head.nsy;

  for (n=0; n < Nfiles; n++) {
    k = 0;

    fprintf(stdout,"Nearest neighbor file %d: %d x %d range (%d to %d x %d to %d) -> %d x %d\n",
	 n+1,nsx,nsy,xmin[n],xmax[n],ymin[n],ymax[n],in_head[n].nsx,in_head[n].nsy);
    
    for (iy = ymin[n]; iy <= ymax[n]; iy++)   /* for all output image pixels in range */
      for (ix = xmin[n]; ix <= xmax[n]; ix++) {
    
	j = sir_lex(ix, iy, &out_head);  /* get image pixel index  0 ... nsx*nxy-1 */

	if (*(cnt+j) <= 0) {
	  sir_pix2latlon(0.5+(float) ix, 0.5+(float) iy, &alon, &alat, &out_head);  /* (ix,iy) -> (lat,lon) */

	  if (clip_option == 0 ||
	      clip_option == 1 && alat >= ll_lat[n] && alat <= ur_lat[n] &&
	                          alon >= ll_lon[n] && alon <= ur_lon[n]) {

	    i = sir_latlon2pix(alon, alat, &x1, &y1, &in_head[n]); /* (lat,lon) -> (x	,y) */
	    if (i >= 0)
	      if (*(stval[n]+i) > in_head[n].anodata + 0.00001) {
		k++;           /* count of pixels copied */
		(*(cnt+j))--;  /* deccrement counter image */
		*(outval+j) = *(outval+j) + *(stval[n]+i);  /* average into output image */
	      }
	  }
	}
      }
    if (DEBUG) fprintf(stdout,"Pixels copied: %d of %d\n", k, nsx * nsy);
  }

  /* normalize any pixels which were averaged */

  for (i=0; i < out_head.nsx * out_head.nsy; i++)
    if (*(cnt+i) > 1) 
      *(outval+i) = *(outval+i) / (float) *(cnt+i);
    else
      if (*(cnt+i) < -1)
	*(outval+i) = - *(outval+i) / (float) *(cnt+i);
      else 
	if (*(cnt+i) == 0)
	  *(outval+i) = out_head.anodata;
 

  /* write output sir file */

  fprintf(stdout,"Output SIR file '%s'\n", out);

  ierr = put_sir(out, &out_head, outval);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output SIR file ***\n");
     fflush(stderr);
     exit(-1);
  }

  return(0);
}



/* support routines */


void dupdate(char *prompt, int *d, FILE *imf)
{
  char line[MAXLINE];

  fprintf(stdout,prompt, *d);
  (void) getline1(imf, line, MAXLINE);
  if (strlen(line) > 0) sscanf(line,"%d",d);
}

void fupdate(char *prompt, float *f, FILE *imf)
{
  char line[MAXLINE];

  fprintf(stdout,prompt, *f);
  (void) getline1(imf, line, MAXLINE);
  if (strlen(line) > 0) sscanf(line,"%f",f);
}

void supdate(char *prompt, char *s, int l, FILE *imf)
{
  char line[MAXLINE];

  fprintf(stdout,prompt, s);
  (void) getline1(imf, line, MAXLINE);
  if (strlen(line) > 0) strncpy(s,line,l);
}


void get_projection_name(char *ptype, int iopt, int n);


#define PSIZE 22

void define_sir_head(sir_head *head, int reset, FILE *imf, int nSarg, char **param, char **value,
		     int use_standard_region, char *std_region_num, char *pix_size)
{
  int last_iopt=-100;
  
  char line[MAXLINE], ptype[PSIZE+1];
  
  fprintf(stdout,"\nCAUTION:\n");
  fprintf(stdout,"Without a template to define the SIR header variables, you must manually\n");
  fprintf(stdout,"select header parameters.  Caution: error checking is very limited.   It is\n");
  fprintf(stdout,"very easy to create headers with errors which cause problems with the file\n");
  fprintf(stdout,"storage accuracy and/or file processing programs.\n\n");

  head->nhtype = 30;
  head->nhead = 1;
  head->nia = 0;       /* no extra header shorts */
  head->ldes = 0;      /* no extra header descriptions */

  if (reset == 0)     /* set header defaults */
    reset_header(head);

  last_iopt = head->iopt;  
  if (use_standard_region == 1) {
    process_standard_region(head, std_region_num, pix_size);
    if (head->iopt != last_iopt) {
      set_header_projection_scale_factors(head);
      fprintf(stdout,"Projection parameter scaling reset\n");
    }
  }
  
  if (nSarg > 0)
    Sarg_process(head, nSarg, param, value);


  /* interactively modify SIR header parameters */
    
 LOOP:

  /* output image size in pixels */

  dupdate("Enter NSX X dimension image size in pixels: [def=%d] ",&head->nsx,imf);
  dupdate("Enter NSY Y dimension image size in pixels: [def=%d] ",&head->nsy,imf);

  /* map projection type (option) */

  get_projection_name(ptype,head->iopt,PSIZE);
  
  fprintf(stdout,"Enter IOPT projection type (-1 = image only [remapping not possible],\n");
  fprintf(stdout,"                             0 = rect lat/lon,\n");
  fprintf(stdout,"                             1 = Lambert fixed radius,\n");
  fprintf(stdout,"                             2 = Lambert local radius,\n");
  fprintf(stdout,"                             5 = Polar sterographic,\n");
  fprintf(stdout,"                             8 = EASE2 North Hem,\n");
  fprintf(stdout,"                             9 = EASE2 South Hem,\n");
  fprintf(stdout,"                            10 = EASE2 Tropical/Global\n");
  fprintf(stdout,"                            11 = EASE1 North Hem,\n");
  fprintf(stdout,"                            12 = EASE1 South Hem,\n");
  fprintf(stdout,"                            13 = EASE1 Global\n");
  fprintf(stdout,"    IOPT: [Def=%d (%s)] ",head->iopt,ptype);
  (void) getline1(stdin, line, MAXLINE);  
  if (strlen(line) > 0) sscanf(line,"%d",&head->iopt);
  switch(head->idatatype){
  case -1:
  case 0:
  case 1:
  case 2:
  case 5:
  case 11:
  case 12:
  case 13:
    break;
  default:
    head->iopt = 0;
    fprintf(stdout,"*** Invalid input.  IOPT set to %d\n",head->iopt);
    goto LOOP;
  }


  /* map projection parameters */

  fupdate("Enter XDEG (x axis length) projection parameter: [def=%f]  ",&head->xdeg,imf);
  fupdate("Enter YDEG (y axis length) projection parameter: [def=%f]  ",&head->ydeg,imf);
  fupdate("Enter ASCALE (x axis scale) projection parameter: [def=%f] ",&head->ascale,imf);
  fupdate("Enter BSCALE (y axis scale) projection parameter: [def=%f] ",&head->bscale,imf);
  fupdate("Enter A0 (x axis origin) projection parameter: [def=%f]    ",&head->a0,imf);
  fupdate("Enter B0 (y axis origin) projection parameter: [def=%f]    ",&head->b0,imf);

  if (head->iopt != last_iopt) {  /* set default header scale factors based on 
				     projection type.  Note: this will override
				     previous user inputs for this parameters */
    last_iopt = head->iopt;
    set_header_projection_scale_factors(head);
    fprintf(stdout,"Projection parameter scaling reset\n");
    
  }

  /* Data storage scaling parameters */

  dupdate("Enter data form (1=one byte, 2=two byte integer, 4=float)/pixel: [def=%d] ",&head->idatatype,imf);
  switch(head->idatatype){
  case 1:
  case 2:
  case 4:
    break;
  default:
    head->idatatype = 2;
    fprintf(stdout,"*** Input overridden.  IDATATYPE set to %d\n",head->idatatype);
  }
  dupdate("Enter IOFF data storage offset factor: [def=%d]  ",&head->ioff,imf);
  dupdate("Enter ISCALE data storage scale factor: [def=%d] ",&head->iscale,imf);
  fupdate("Enter ANODATA no-data value: [def=%f] ",&head->anodata,imf);
  fupdate("Enter V_MIN default minimum viewable data value: [def=%f] ",&head->v_min,imf);
  fupdate("Enter V_MAX default maximum viewable data value: [def=%f] ",&head->v_max,imf);

  /* Various descriptive header values */

  dupdate("Enter IYEAR data start year (4 digits): [def=%d] ",&head->iyear,imf);
  dupdate("Enter ISDAY data start day-of-year: [def=%d] ",&head->isday,imf);
  dupdate("Enter ISMIN data start min-of-day: [def=%d]  ",&head->ismin,imf);
  dupdate("Enter IEDAY data end day-of-year: [def=%d] ",&head->ieday,imf);
  dupdate("Enter IEMIN data end min-of-day: [def=%d]  ",&head->iemin,imf);
  dupdate("Enter IREGION region number code: [def=%d] ",&head->iregion,imf);
  dupdate("Enter ITYPE image type code: [def=%d] ",&head->itype,imf);
  dupdate("Enter IPOL polarization code (0=n/a, 1=h, 2=v, 3=both): [def=%d] ",&head->ipol,imf);
  dupdate("Enter IFREQHM sensor frequency in hundreds of MHz (e.g. 13GHz->130): [def=%d] ",&head->ifreqhm,imf);

  /* Various header strings */
  
  supdate("Enter TITLE image title: [def='%s'] ",head->title,100,imf);
  supdate("Enter SENSOR name/description: [def='%s'] ",head->sensor,40,imf);
  supdate("Enter TYPE: type description [def='%s'] ",head->type,138,imf);
  supdate("Enter TAG description: [def='%s'] ",head->tag,100,imf);
  supdate("Enter CRPROC creation process: [def='%s'] ",head->crproc,100,imf);
  supdate("Enter CRTIME creation date: [def='%s'] ",head->crtime,29,imf);

  /* review results */

  fprintf(stdout,"\nOutput header variable preview:\n");
  print_sir_head(stdout, head);    

  fprintf(stdout,"Accept as is or Revise (A/R) [default=A]:");
  (void) getline1(stdin, line, MAXLINE);
  if (*line == 'R' || *line == 'r') goto LOOP;

}


int getline1(FILE *imf, char *line, int max_length)
{  /* read a newline-terminated character string from a file stream */
  char *s, ch;
  int n=0;
  
  s=line;
  while ((ch = getc(imf)) != '\n' && ch != -1 && n++ < max_length)
    *(s++) = ch;

  *s = '\0';    /* ensure string is null terminated */
  if (ch == -1) 
    return(-1);
  else
    return(n);
}


void get_projection_name(char *ptype, int iopt, int n)  /* returns short text description */
{                                                       /* corresponding to projection code */
  switch(iopt) {
  case -1:   /* Image Only */
    strncpy(ptype,"Image Only",n);
    break;

  case 0:   /* Rectangular Lat/Lon */
    strncpy(ptype,"Rectangular Lat/Lon",n);
    break;

  case 1:   /* Lambert Fixed Radius */
    strncpy(ptype,"Lambert Fixed Radius",n);
    break;
    
  case 2:   /* Lambert Local Radius */
    strncpy(ptype,"Lambert Local Radius",n);
    break;

  case 5:   /* Polar Stereographic */
    strncpy(ptype,"Polar Stereographic",n);
    break;

  case 8:   /* EASE2 North Hem */
    strncpy(ptype,"EASE2 North Hem",n);
    break;

  case 9:   /* EASE2 South Hem */
    strncpy(ptype,"EASE2 South Hem",n);
    break;

  case 10:   /* EASE2 Global */
    strncpy(ptype,"EASE2 Tropical/Global",n);
    break;

  case 11:   /* EASE1 North Hem */
    strncpy(ptype,"EASE1 North Hem",n);
    break;

  case 12:   /* EASE1 South Hem */
    strncpy(ptype,"EASE1 South Hem",n);
    break;

  case 13:   /* EASE1 Global */
    strncpy(ptype,"EASE1 Global",n);
    break;

  default:   /* unknown & therefore invalid */
    strncpy(ptype,"Invalid",n);
    break;
  }
}


void set_header_projection_scale_factors(sir_head *head)  /* set default projection */
{                                                         /* parameter scale factors */ 
  switch(head->iopt) {
  case -1:   /* image only */
    head->ixdeg_off=0;
    head->iydeg_off=0;
    head->ideg_sc=10;
    head->iscale_sc=1000;
    head->ia0_off=0;
    head->ib0_off=0; 
    head->i0_sc=100;
    break;

  case 0:   /* rectangular lat/lon */
    head->ixdeg_off=-100;
    head->iydeg_off=0;
    head->ideg_sc=100;
    head->iscale_sc=1000;
    head->ia0_off=0;
    head->ib0_off=0; 
    head->i0_sc=100;
    break;

  case 1:   /* Lambert */
  case 2:
    head->ixdeg_off=0;
    head->iydeg_off=0;
    head->ideg_sc=100;
    head->iscale_sc=1000;
    head->ia0_off=0;
    head->ib0_off=0; 
    head->i0_sc=1;
    break;

  case 5:   /* Polar Stereographic */
    head->ixdeg_off=-100;
    head->iydeg_off=0;
    head->ideg_sc=100;
    head->iscale_sc=1000;
    head->ia0_off=0;
    head->ib0_off=0; 
    head->i0_sc=1;
    break;

  case 8:    /* EASE2 */
  case 9:    /* EASE2 */
  case 10:   /* EASE2 */
    head->ixdeg_off=0;
    head->iydeg_off=0;
    head->ideg_sc=10;
    head->iscale_sc=100;
    head->ia0_off=0;
    head->ib0_off=0; 
    head->i0_sc=10;
    break;

  case 11:   /* EASE1 */
  case 12:   /* EASE1 */
  case 13:   /* EASE1 */
    head->ixdeg_off=0;
    head->iydeg_off=0;
    head->ideg_sc=10;
    head->iscale_sc=1000;
    head->ia0_off=0;
    head->ib0_off=0; 
    head->i0_sc=10;
    break;

  default:
    head->ixdeg_off=0;
    head->iydeg_off=0;
    head->ideg_sc=10;
    head->iscale_sc=1000;
    head->ia0_off=0;
    head->ib0_off=0; 
    head->i0_sc=100;
    break;
  }
}


#include <time.h>
void reset_header(sir_head *head)   /* set header defaults */
{
  time_t tod;

  head->nhtype = 30;
  head->nhead = 1;
  head->nia = 0;       /* no extra header shorts */
  head->ldes = 0;      /* no extra header descriptions */

  head->nsx = 1;
  head->nsy = 1;
  head->iopt = -1;  /* image only */
  head->xdeg = head->nsx;
  head->ydeg = head->nsy;
  head->ascale = 1;
  head->bscale = 1;
  head->a0 = 1;
  head->b0 = 1;

  set_header_projection_scale_factors(head);

  head->idatatype = 2;
  head->ioff = -33;
  head->iscale = 1000;
  head->anodata = -33.0;
  head->v_min = -32.0;
  head->v_max = -0.0;

  head->iyear = 1970;
  head->isday = 1;
  head->ismin = 1;
  head->ieday = 1;
  head->iemin = 1440;
  head->iregion = 0;
  head->itype = 1;
  head->ipol = 0;
  head->ifreqhm = 0;
  head->ispare1 = 0;

  head->title[0] = '\0';
  head->sensor[0] = '\0';
  head->type[0] = '\0';
  head->tag[0] = '\0';
  strncpy(head->crproc,"BYU sir_remapper",100);

  head->crtime[0] = '\0';
  (void) time(&tod);
  (void) strftime(head->crtime,28,"%X %x",localtime(&tod));
}



char *strupr(char *s)
{ char *out = s;
 
  for (;*s != '\0' ; s++)    
    if (*s > 96 && *s < 123)    
      *s = *s - 32;
  return(out);
  
}

void Sarg_process(sir_head *head, int nSarg, char **param, char **value)
{
  int i;
  char *s, line[MAXLINE];

  if (nSarg <= 0) return;

  for (i=0; i < nSarg; i++) {
    if (DEBUG) fprintf(stdout, "set header parameter %d <%s> = <%s>\n",i+1,param[i],value[i]);

    s = strupr(param[i]);
    
    if (strncmp(s,"NSX",3) == 0)
      sscanf(value[i],"%d",&head->nsx);
    if (strncmp(s,"NSY",3) == 0)
      sscanf(value[i],"%d",&head->nsy);

    if (strncmp(s,"IOPT",3) == 0)
      sscanf(value[i],"%d",&head->iopt);
    switch(head->idatatype){
    case -1:
    case 0:
    case 1:
    case 2:
    case 5:
    case 11:
    case 12:
    case 13:
      break;
    default:
      head->iopt = 0;
      fprintf(stderr,"*** Invalid value.  IOPT reset to %d\n",head->iopt);
    }

    if (strncmp(s,"A0",3) == 0)
      sscanf(value[i],"%f",&head->a0);
    if (strncmp(s,"B0",3) == 0)
      sscanf(value[i],"%f",&head->b0);
    if (strncmp(s,"ASCALE",3) == 0)
      sscanf(value[i],"%f",&head->ascale);
    if (strncmp(s,"BSCALE",3) == 0)
      sscanf(value[i],"%f",&head->bscale);
    if (strncmp(s,"XDEG",3) == 0)
      sscanf(value[i],"%f",&head->xdeg);
    if (strncmp(s,"YDEG",3) == 0)
      sscanf(value[i],"%f",&head->ydeg);
    if (strncmp(s,"IXDEG_OFF",3) == 0)
      sscanf(value[i],"%d",&head->ixdeg_off);
    if (strncmp(s,"IYDEG_OFF",3) == 0)
      sscanf(value[i],"%d",&head->iydeg_off);
    if (strncmp(s,"IDEG_SC",3) == 0)
      sscanf(value[i],"%d",&head->ideg_sc);
    if (strncmp(s,"ISCALE_SC",3) == 0)
      sscanf(value[i],"%d",&head->iscale_sc);
    if (strncmp(s,"IA0_OFF",3) == 0)
      sscanf(value[i],"%d",&head->ia0_off);
    if (strncmp(s,"IB0_OFF",3) == 0)
      sscanf(value[i],"%d",&head->ib0_off);
    if (strncmp(s,"I0_SC",3) == 0)
      sscanf(value[i],"%d",&head->i0_sc);

    if (strncmp(s,"IDATATYPE",3) == 0)
      sscanf(value[i],"%d",&head->idatatype);
    switch(head->idatatype){
    case 1:
    case 2:
    case 4:
      break;
    default:
      head->idatatype = 2;
      fprintf(stdout,"*** Input overridden.  IDATATYPE set to %d\n",head->idatatype);
    }

    if (strncmp(s,"IOFF",3) == 0)
      sscanf(value[i],"%d",&head->ioff);
    if (strncmp(s,"ISCALE",3) == 0)
      sscanf(value[i],"%d",&head->iscale);
    if (strncmp(s,"ANODATA",3) == 0)
      sscanf(value[i],"%f",&head->anodata);
    if (strncmp(s,"V_MIN",3) == 0)
      sscanf(value[i],"%f",&head->v_min);
    if (strncmp(s,"V_MAX",3) == 0)
      sscanf(value[i],"%f",&head->v_max);
    if (strncmp(s,"IYEAR",3) == 0)
      sscanf(value[i],"%d",&head->iyear);
    if (strncmp(s,"ISDAY",3) == 0)
      sscanf(value[i],"%d",&head->isday);
    if (strncmp(s,"IEDAY",3) == 0)
      sscanf(value[i],"%d",&head->ieday);
    if (strncmp(s,"ISMIN",3) == 0)
      sscanf(value[i],"%d",&head->ismin);
    if (strncmp(s,"IEMIN",3) == 0)
      sscanf(value[i],"%d",&head->iemin);
    if (strncmp(s,"IREGION",3) == 0)
      sscanf(value[i],"%d",&head->iregion);
    if (strncmp(s,"ITYPE",3) == 0)
      sscanf(value[i],"%d",&head->itype);
    if (strncmp(s,"IPOL",3) == 0)
      sscanf(value[i],"%d",&head->ipol);
    if (strncmp(s,"IFREQHM",3) == 0)
      sscanf(value[i],"%d",&head->ifreqhm);
    if (strncmp(s,"TITLE",3) == 0)
      strncpy(head->title,value[i],100);
    if (strncmp(s,"SENSOR",3) == 0)
      strncpy(head->sensor,value[i],40);
    if (strncmp(s,"TYPE",3) == 0)
      strncpy(head->type,value[i],138);
    if (strncmp(s,"TAG",3) == 0)
      strncpy(head->tag,value[i],100);
    if (strncmp(s,"CRPROC",3) == 0)
      strncpy(head->crproc,value[i],100);
    if (strncmp(s,"CRTIME",3) == 0)
      strncpy(head->crtime,value[i],29);
  }
  
}


void list_header_variables(FILE *imf)
{
  fprintf(stdout,"\nSetable SIR file header variable names:\n");
  fprintf(stdout," NSX NSY IOPT A0 B0 ASCALE BSCALE XDEG YDEG\n");
  fprintf(stdout," IXDEG_OFF IYDEG_OFF IDEG_SC ISCALE_SC IA0_OFF IB0_OFF I0_SC\n");
  fprintf(stdout," IDATATYPE IOFF ISCALE ANODATA V_MIN V_MAX\n");
  fprintf(stdout," IYEAR ISDAY ISMIN IEDAY IEMIN IREGION ITYPE IPOL IFREQHM\n");
  fprintf(stdout," TITLE SENSOR TYPE TAG CRPROC CRTIME\n");
}


void copy_sir_head_args(sir_head *head, int nxarg, char **param, sir_head *in)
{
  int i;
  char *s;

  if (nxarg <= 0) return;

  for (i=0; i < nxarg; i++) {

    s = strupr(param[i]);
    
    if (strncmp(s,"NSX",3) == 0)
      head->nsx = in->nsx;
    if (strncmp(s,"NSY",3) == 0)
      head->nsy = in->nsy;
    if (strncmp(s,"IOPT",3) == 0)
      head->iopt = in->iopt;
    if (strncmp(s,"A0",3) == 0)
      head->a0 = in ->a0;
    if (strncmp(s,"B0",3) == 0)
      head->b0 = in->a0;
    if (strncmp(s,"ASCALE",3) == 0)
      head->ascale = in->ascale;
    if (strncmp(s,"BSCALE",3) == 0)
      head->bscale = in->bscale;
    if (strncmp(s,"XDEG",3) == 0)
      head->xdeg = in->xdeg;
    if (strncmp(s,"YDEG",3) == 0)
      head->ydeg = in->ydeg;
    if (strncmp(s,"IXDEG_OFF",3) == 0)
      head->ixdeg_off = in->ixdeg_off;
    if (strncmp(s,"IYDEG_OFF",3) == 0)
      head->iydeg_off = in->iydeg_off;
    if (strncmp(s,"IDEG_SC",3) == 0)
      head->ideg_sc = in->ideg_sc;
    if (strncmp(s,"ISCALE_SC",3) == 0)
      head->iscale_sc = in->iscale_sc;
    if (strncmp(s,"IA0_OFF",3) == 0)
      head->ia0_off = in->ia0_off;
    if (strncmp(s,"IB0_OFF",3) == 0)
      head->ib0_off = in->ib0_off;
    if (strncmp(s,"I0_SC",3) == 0)
      head->i0_sc = in->i0_sc;
    if (strncmp(s,"IDATATYPE",3) == 0)
      head->idatatype = in->idatatype;
    if (strncmp(s,"IOFF",3) == 0)
      head->ioff = in->ioff;
    if (strncmp(s,"ISCALE",3) == 0)
      head->iscale = in->iscale;
    if (strncmp(s,"ANODATA",3) == 0)
      head->anodata = in->anodata;
    if (strncmp(s,"V_MIN",3) == 0)
      head->v_min = in->v_min;
    if (strncmp(s,"V_MAX",3) == 0)
      head->v_max = in->v_max;
    if (strncmp(s,"IYEAR",3) == 0)
      head->iyear = in->iyear;
    if (strncmp(s,"ISDAY",3) == 0)
      head->isday = in->isday;
    if (strncmp(s,"IEDAY",3) == 0)
      head->ieday = in->ieday;
    if (strncmp(s,"ISMIN",3) == 0)
      head->ismin = in->ismin;
    if (strncmp(s,"IEMIN",3) == 0)
      head->iemin = in->iemin;
    if (strncmp(s,"IREGION",3) == 0)
      head->iregion = in->iregion;
    if (strncmp(s,"ITYPE",3) == 0)
      head->itype = in->itype;
    if (strncmp(s,"IPOL",3) == 0)
      head->ipol = in->ipol;
    if (strncmp(s,"IFREQHM",3) == 0)
      head->ifreqhm = in->ifreqhm;
    if (strncmp(s,"TITLE",3) == 0)
      strncpy(head->title,in->title,100);
    if (strncmp(s,"SENSOR",3) == 0)
      strncpy(head->sensor,in->sensor,40);
    if (strncmp(s,"TYPE",3) == 0)
      strncpy(head->type,in->type,138);
    if (strncmp(s,"TAG",3) == 0)
      strncpy(head->tag,in->tag,100);
    if (strncmp(s,"CRPROC",3) == 0)
      strncpy(head->crproc,in->crproc,100);
    if (strncmp(s,"CRTIME",3) == 0)
      strncpy(head->crtime,in->crtime,29);

  }
}


#define abs(a) ((a) > 0 ? (a) : - (a))

/* these internal routines are not generally called by a user program
   but are used in the following routine to help define the standard
   geometric projection information */

extern void lambert1(float, float, float*, float*, float, float, int);
extern void polster(float, float, float*, float*, float, float);
extern void ease2_map_info(int, int, int, double*, double*, double*,double*,
			   double*, double*, double*, double*, double*,
			   double*, int*, int*, double*, double*, double*);

void process_standard_region(sir_head *head, char *std_region_num, char *pix_size)
{
  char fname[] = STD_REGION_FILE;
  FILE *imf;
  int region_num, psize, end_flag = 0, n;
  int reg, prj, DtLn;
  float LLlat, LLlon, URlat, URlon;
  char regname[25];
  float xdim, ydim, lmostx, rmostx, lmosty, rmosty, x0, y0, aorglat, aorglon;
  float maplylat, maplylon, mapuylon, maprxlon, maprxlat, mapuylat, maplxlon, maplxlat;
  char line[MAXLINE];

  int polar_view = 0;  /* Lambert default: non-polar view */

  double map_equatorial_radius_m,map_eccentricity, e2,
    map_reference_latitude, map_reference_longitude, 
    map_second_reference_latitude, sin_phi1, cos_phi1, kz,
    map_scale, r0, s0, epsilon;
  int bcols, brows, bs;

  /* reads the file with the name STD_REGION_FILE containing geometry definitions of
     standard regions to fill in the default geometry of these standard regions */

  sscanf(std_region_num,"%d",&region_num);
  sscanf(pix_size,"%d",&psize);
  if (DEBUG) fprintf(stdout,"Standard region requested: %d %d\n", region_num, psize);
  
  if (region_num < 0 || region_num > 9999) {
    fprintf(stdout,"*** Invalid Std Region number specified %d, reset to 1\n", region_num);
    region_num = 1;
  }
  
  if (psize < 0 || psize > 5) {
    fprintf(stdout,"*** Invalid pix_size specified %d, reset to 1\n", psize);
    psize = 1;
  }

  /* open standard region definition file */

  if (DEBUG) fprintf(stdout,"Opening standard region definition file %s\n",fname);
  imf = fopen(fname,"r");
  if (imf == NULL) {
    fprintf(stdout,"*** ERROR: could not open standard region def file '%s'\n",fname);
    return;
  }

  /* skip header lines */

  (void) getline1(imf, line, MAXLINE);
  (void) getline1(imf, line, MAXLINE);

  while (end_flag == 0 && getline1(imf,line, MAXLINE) > 0) {
    /* printf("%s\n",line); */
    sscanf(line,"%d %d %d %f %f %f %f %s", &reg, &prj, &DtLn, &LLlat, &LLlon, &URlat, &URlon, regname);
    
    if (reg == region_num)
      end_flag = 1;
  }
  if (end_flag == 0) {
    fprintf(stdout,"*** Region number %d not found in standard region file\n",region_num);
    return;
  }
  

  if (DEBUG) {
    fprintf(stdout,"Standard region %d parameters\n",region_num);
    fprintf(stdout,"%s\n",line);
    fprintf(stdout," Projection: %d   DateLine: %d\n", prj, DtLn);
    fprintf(stdout," LL lat,lon: %f, %f  UR lat,lon: %f, %f  Region %s\n",LLlat, LLlon, URlat, URlon, regname);
  }
  
  switch(prj) {
  case -1:   /* Image Only */
    head->iopt = prj;  
    head->a0=1;
    head->b0=1;
    head->xdeg=URlon-LLlon;
    head->ydeg=URlat-LLlat;
    head->ascale=1;
    head->bscale=head->ascale;
    head->nsx=head->xdeg;
    head->nsy=head->ydeg;
    break;

  case 0:   /* Rectangular Lat/Lon */
    head->iopt = prj;  
    head->a0=LLlon;
    head->b0=LLlat;
    head->xdeg=URlon-LLlon;
    head->ydeg=URlat-LLlat;
    switch(psize) {
    case 0:
      head->ascale=6.0;
      break;
    case 1:
      head->ascale=12.0;
      break;
    case 2:
    default:
      head->ascale=24.0;
      break;
    case 3:
      head->ascale=48.0;
      break;
    case 4:
      head->ascale=96.0;
      break;
    }
    head->bscale=head->ascale;
    head->nsx=head->xdeg*head->ascale;
    head->nsy=head->ydeg*head->bscale;
    break;

  case 1:   /* Lambert Fixed Radius */
  case 2:   /* Lambert Local Radius */
    head->iopt = prj;  
    switch(psize) {
    case 0:
      head->ascale=0.5/8.9;
      break;
    case 1:
      head->ascale=1./8.9;
      break;
    case 2:
    default:
      head->ascale=2./8.9;
      break;
    case 3:
      head->ascale=4./8.9;
      break;
    case 4:
      head->ascale=8./8.9;
      break;
    }
    head->bscale=head->ascale;

    if (polar_view == 1) {  /* polar Lambert proj not generally used */
      mapuylon = 0.0;
      maplylon = 180.0;
      if (URlat > 0.0) { /* north pole */
	aorglat = URlat;
	aorglon = 180.0;
	maplxlon = 90.0;
	maplxlat = LLlat;
	maprxlon = -90.0;
	maprxlat = LLlat;
	mapuylat = LLlat;
	maplylat = LLlat;
      } else {          /* south pole */
	aorglat = LLlat;
	aorglon = 0.0;
	maplxlon = -90.0;
	maplxlat = URlat;
	maprxlon = 90.0;
	maprxlat = URlat;
	mapuylat = URlat;
	maplylat = URlat;
      }

    } else { /* Non-polar Lambert projection generally the case */

      aorglat = 0.5 * (URlat + LLlat);
      aorglon = 0.5 * (URlon + LLlon);
      mapuylat = URlat;
      maplylat = LLlat;
      maplxlon = LLlon;
      maprxlon = URlon;
		 
      if (URlat*LLlat < 0.0) { /* region contains equator */
	maplxlat = 0.0;
	maprxlat = 0.0;
	if (URlat > abs(LLlat)) {	/* mostly in nrthrn hemisphere */
	  mapuylon = LLlon;
	  maplylon = aorglon;
	} else {	                /* mostly in sthrn hemisphere */
	  mapuylon = aorglon;
	  maplylon = LLlon;
	}		
      } else {
	if (aorglat > 0.0) { /* region in northern hemisphere */
	  maplxlat = LLlat;
	  maprxlat = LLlat;
	  mapuylon = LLlon;
	  maplylon = aorglon;
	} else {
	  if (aorglat < 0.0) {  /* region in southern hemisphere */
	    maplxlat = URlat;
	    maprxlat = URlat;
	    mapuylon = aorglon;
	    maplylon = LLlon;
	  }
	}
      }
    }

    /* left most x value */
    lambert1(maplxlat, maplxlon, &x0, &y0, aorglat, aorglon, prj);
    lmostx = x0;
    /* right most x value */
    lambert1(maprxlat, maprxlon, &x0, &y0, aorglat, aorglon, prj);
    rmostx = x0;
    /* highest y value */
    lambert1(mapuylat, mapuylon, &x0, &y0, aorglat, aorglon, prj);
    rmosty = y0;
    /* lowest y value */
    lambert1(maplylat, maplylon, &x0, &y0, aorglat, aorglon, prj);
    lmosty = y0;
    
    /* the following quanitzation steps are used to generate
       convenient parameter values and are the BYU standard approach */

    /* quantize to multiple of one hundred */
    lmostx = ((int)((lmostx - 100.0) / 100.0)) * 100.0;
    rmostx = ((int)((rmostx - 100.0) / 100.0)) * 100.0;
    lmosty = ((int)((lmosty - 100.0) / 100.0)) * 100.0;
    rmosty = ((int)((rmosty - 100.0) / 100.0)) * 100.0;
    /* lower left corner */
    head->a0 = lmostx;
    head->b0 = lmosty;
    /* image dimensions */
    xdim = abs(lmostx) + abs(rmostx);
    ydim = abs(lmosty) + abs(rmosty);
    head->nsy = 10 * (int)((ydim * head->bscale + 10.0) / 10.0); 
    head->nsx = 10 * (int)((xdim * head->ascale + 10.0) / 10.0);
    head->xdeg = aorglon;
    head->ydeg = aorglat;

    break;

  case 5:   /* Polar Stereographic */
    head->iopt = prj;  
    switch(psize) {
    case 0:
      head->ascale=2*8.9;
      break;
    case 1:
      head->ascale=8.9;
      break;
    case 2:
    default:
      head->ascale=8.9/2.0;
      break;
    case 3:
      head->ascale=8.9/4.0;
      break;
    case 4:
      head->ascale=8.9/8.0;
      break;
    }
    head->bscale=head->ascale;

    /* polar stereographic views always centered on pole */
    if (URlat < 0.0) { /* south pole */
      head->xdeg = 0.0;
      head->ydeg = -70.0;   /* reference (tangent) latitude */
      maplxlat = URlat;
      maplxlon = -90.0;
      maprxlat = URlat;
      maprxlon = 90.0;
      mapuylat = URlat;
      mapuylon = head->xdeg;
      maplylat = URlat;
      maplylon = 180.0;
    } else {  /* north pole */
      head->xdeg = -45.0;
      head->ydeg = 70.0;    /* reference (tangent) latitude */
      maplxlat = LLlat;
      maplxlon = -135.0;   /* by default, rotate reference longitude */
      maprxlat = URlat;    /* so that Greenland is vertical */
      maprxlon = 45.0;     /* ditto, rotate reference longitude */
      mapuylat = LLlat;
      mapuylon = 135.0;    /* ditto, rotate reference longitude */
      maplylat = LLlat;
      maplylon = head->xdeg;
    }
    aorglon = head->xdeg;
    aorglat = head->ydeg;

    /* left most x value */
    polster(maplxlon, maplxlat, &x0, &y0, aorglon, aorglat);
    lmostx = x0;
    /* right most x value */
    polster(maprxlon, maprxlat, &x0, &y0, aorglon, aorglat);
    rmostx = x0;
    /* highest y value */
    polster(mapuylon, mapuylat, &x0, &y0, aorglon, aorglat);
    rmosty = y0;
    /* lowest y value */
    polster(maplylon, maplylat, &x0, &y0, aorglon, aorglat);
    lmosty = y0;

    /* the following quanitzation steps are used to generate
       convenient parameter values and are the BYU standard approach */

    /* quantize to multiple of one hundred */
    lmostx = ((int)((lmostx - 100.0) / 100.0)) * 100.0;
    rmostx = ((int)((rmostx - 100.0) / 100.0)) * 100.0;
    lmosty = ((int)((lmosty - 100.0) / 100.0)) * 100.0;
    rmosty = ((int)((rmosty - 100.0) / 100.0)) * 100.0;
    /* lower left corner */
    head->a0 = lmostx;
    head->b0 = lmosty;
    /* image dimensions */
    xdim = abs(lmostx) + abs(rmostx);
    ydim = abs(lmosty) + abs(rmosty);
    head->nsy = 10 * (int)((ydim / head->bscale + 10.0) / 10.0);
    head->nsx = 10 * (int)((ydim / head->ascale + 10.0) / 10.0);

    break;

  case 8:   /* EASE2 North Hem */
  case 9:   /* EASE2 South Hem */
  case 10:  /* EASE2 Tropical/Global */
    /* note: base size index is fixed to index 0 in this code */
    head->iopt = prj;
    bs=0;    
    ease2_map_info(prj, psize, bs, &map_equatorial_radius_m, 
		   &map_eccentricity, &e2,
		   &map_reference_latitude, &map_reference_longitude, 
		   &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		   &map_scale, &bcols, &brows, &r0, &s0, &epsilon);
    head->nsx=(int) bcols;
    head->nsy=(int) brows;
    head->ascale=(float) psize;
    head->bscale=(float) bs;
    head->a0=0.0;
    head->b0=0.0;
    head->xdeg=(float) head->nsx;
    head->ydeg=(float) head->nsy; 
    break;

  case 11:   /* EASE1 North Hem */
    head->iopt = prj;  
    switch(psize) {
    case 0:
      n=1;
      break;
    case 1:
      n=2;
      break;
    case 2:
    default:
      n=4;
      break;
    case 3:
      n=8;
      break;
    case 4:
      n=16;
      break;
    }
    head->nsx=720*n+1;
    head->nsy=720*n+1;
    head->ascale=2. * (double) 6371.228/((double) 25.067525 / n);
    head->bscale=(double) 25.067525 / n;
    head->xdeg=360.0*n;
    head->ydeg=360.0*n;
    head->a0=-head->xdeg;
    head->b0=-head->ydeg;
    break;

  case 12:   /* EASE1 South Hem */
    head->iopt = prj;  
    switch(psize) {
    case 0:
      n=1;
      break;
    case 1:
      n=2;
      break;
    case 2:
    default:
      n=4;
      break;
    case 3:
      n=8;
      break;
    case 4:
      n=16;
      break;
    }
    head->nsx=720*n+1;
    head->nsy=720*n+1;
    head->ascale=2. * (double) 6371.228/((double) 25.067525 / n);
    head->bscale=(double) 25.067525 / n;
    head->xdeg=360.0*n;
    head->ydeg=360.0*n;
    head->a0=-head->xdeg;
    head->b0=-head->ydeg;
    break;

   case 13:   /* EASE1 Global */
    head->iopt = prj;  
    switch(psize) {
    case 0:
      n=1;
      break;
    case 1:
      n=2;
      break;
    case 2:
    default:
      n=4;
      break;
    case 3:
      n=8;
      break;
    case 4:
      n=16;
      break;
    }
    head->nsx=1382*n+1;
    head->nsy=586*n;
    head->ascale=(double) 6371.228/((double) 25.067525 / n);
    head->bscale=(double) 25.067525 / n;
    head->xdeg=691.0*n;
    head->ydeg=292.5*n;
    head->a0=-head->xdeg;
    head->b0=-head->ydeg;
    break;

  default:   /* unknown & therefore invalid */
    break;
  }
    
}

void clip_extent(float *ll_lat, float *ll_lon, float *ur_lat, float *ur_lon, sir_head *head)
{
  char fname[] = STD_REGION_FILE;
  FILE *imf;
  int region_num, psize, end_flag = 0, n;
  int reg, prj, DtLn;
  float LLlat, LLlon, URlat, URlon;
  char regname[25];
  char line[MAXLINE];

  /* reads the file with the name STD_REGION_FILE containing geometry definitions of
     standard regions to fill in the default geometry of these standard regions */

  *ll_lat=-90.0;
  *ur_lat=90.0;
  *ll_lon=-180.0;
  *ur_lon=180.0;

  region_num = head->iregion; 
  if (DEBUG) fprintf(stdout,"Standard region clipping: %d\n", region_num);
  if (region_num < 0 || region_num > 9999) {
    fprintf(stdout,"*** Invalid Standard region number specified %d\n", region_num);
    return;
  }
  
  /* open standard region definition file */

  if (DEBUG) fprintf(stdout,"Opening standard region definition file %s\n",fname);
  imf = fopen(fname,"r");
  if (imf == NULL) {
    fprintf(stdout,"*** ERROR: could not open standard region def file '%s'\n",fname);
    return;
  }

  /* skip header lines */

  (void) getline1(imf, line, MAXLINE);
  (void) getline1(imf, line, MAXLINE);

  while (end_flag == 0 && getline1(imf,line, MAXLINE) > 0) {
    sscanf(line,"%d %d %d %f %f %f %f %s", &reg, &prj, &DtLn, &LLlat, &LLlon, &URlat, &URlon, regname);    
    if (reg == region_num)
      end_flag = 1;
  }
  fclose(imf);
  
  if (end_flag == 0) {
    fprintf(stdout,"*** Region number %d not found in standard region file\n",region_num);
    return;
  }

  *ll_lat=LLlat;
  *ur_lat=URlat;
  *ll_lon=LLlon;
  *ur_lon=URlon;
  if (DEBUG) fprintf(stdout," Region limits: (%f, %f) - (%f, %f)\n",LLlat,URlat,LLlon,URlon);
  
  return;
}
