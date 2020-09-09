/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_meta_sir.c    MEaSURES project

  program to generate standard SIR products from .setup file

  Written by DGL at BYU 02/22/2014 + modified from ssmi_meta_sir3.c

******************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef JANUSicc
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <float.h>
#include <libgen.h>
#include <string.h>
#include <time.h>
#include <netcdf.h>

#include "utils.h"
#include "cetb_ncatts.h"
#include "cetb_file.h"

#define VERSION 1.2

#define file_savings 1.00     /* measurement file savings ratio */
#define REL_EOF   2           /* fseek relative to end of file */

#define CREATE_NON 1          /* set to 1 to create NON images, 0 to not create */

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

/****************************************************************************/

/* some global variables and their default values */

float a_init=230.0;           /* initial A (TB) value, the mean expected TB (K) */
int   nits=30;                /* number of SIR iterations */
char  sensor_in[40];          /* sensor description string */
int   MAXFILL=1000;           /* maximum number of pixels in response */
int   HASAZANG=0;             /* azimuth angle data not included */
int   HS=20;                  /* measurement headersize in bytes */
int   AVE_INIT=1;             /* use AVE to start SIR iteration if set to 1 */

/****************************************************************************/

static void Ferror(int i)
{
  fprintf( stderr, "*** Error reading input file at %d ***\n", i );
  fflush( stderr );
  return;
}


/* function prototypes */

static void get_updates(float tbval, int count, int *fill_array,
		 short int *response_array, int its );

static void compute_ave(float tbval, float ang, int count, int *fill_array,
			short int *response_array);

static void time_updates(float ktime, int count, int *fill_array);

static void stat_updates(float tbval, int count, int *fill_array,
		  short int *response_array);

static void filter(float *val, int size, int opt, int nsx, int nsy, float *temp,
		   float thres);

static void get_vars_from_store( char *store, float *tbval, float *ang,
				 int *count, float tb_or_stokes_offset,
				 int *ktime, int *add, int HASAZANG, float *azang );

/****************************************************************************/

/* global array variables used for storing images*/

float *a_val, *b_val, *a_temp, *sxy, *sx, *sx2, *sy, *tot;
unsigned char *num_samples;

/* other global variables */

FILE *imf;
long int nspace;

/****************************************************************************/

/* main program */

int main(int argc, char **argv)
{

  char *file_in;
  char outpath[FILENAME_MAX];
  char cetb_sir_filename[FILENAME_MAX]; // Required to save the name for consistency checking at end
  char cetb_grd_filename[FILENAME_MAX]; // Required to save the name for consistency checking at end

  float latl, lonl, lath, lonh;
  char regname[11];
  int dumb, nrec, ncnt, i, nsize;
  long int nls, nbyte;
  float ratio, fn, ninv;
  char *space, *store, *store2;
  float tbval, ang, azang;
  int count, ktime, dummy_ktime, iadd, dummy_iadd, end_flag, input_file_total;
  char *list_of_input_files[CETB_MAX_INPUT_FILES];
  char *x, *stopstring;
  int irecords;
  int non_size_x, non_size_y, nsx2, nsy2, ix, iy, nsize2;
  float xdeg2, ydeg2, ascale2, bscale2, a02, b02;
  float rthreshold, box_size_km;

  /* define no-data values */
  float anodata_A;
  float anodata_C=CETB_NCATTS_TB_NUM_SAMPLES_FILL_VALUE;
  float anodata_I=CETB_NCATTS_THETA_FILL_VALUE;
  float anodata_Ia=CETB_NCATTS_THETA_FILL_VALUE;
  float anodata_P=(float)(CETB_NCATTS_TB_TIME_FILL_VALUE*CETB_NCATTS_TB_TIME_SCALE_FACTOR);
  float anodata_V;
  float anodata_E=-15.0;

  int nsx, nsy, iyear, isday, ismin, ieday, iemin;
  int iregion, ipol, iopt;
  float xdeg, ydeg, ascale, bscale, a0, b0;

  int its, irec, keep;  
  float total, tmax;
  float amin, amax, bmin, bmax, temp, old_amin, old_amax;
  float old_bmin, old_bmax, denom;

  float ltod_morning, ltod_evening;

  char line[100];

  cetb_file_class *cetb_sir;
  cetb_file_class *cetb_grd;
  cetb_swath_producer_id swath_producer_id;
  cetb_platform_id platform_id;
  cetb_sensor_id sensor_id;
  cetb_direction_id direction_id=CETB_NO_DIRECTION;
  unsigned short tb_fill_value=CETB_NCATTS_TB_FILL_VALUE;
  unsigned short stokes_fill_value=CETB_NCATTS_STOKES_FILL_VALUE;
  unsigned short tb_or_stokes_fill_value;
  unsigned short tb_missing_value=CETB_NCATTS_TB_MISSING_VALUE;
  unsigned short stokes_missing_value=CETB_NCATTS_STOKES_MISSING_VALUE;
  unsigned short tb_or_stokes_missing_value;
  unsigned short tb_valid_range[ 2 ] = { CETB_NCATTS_TB_MIN, CETB_NCATTS_TB_MAX };
  unsigned short stokes_valid_range[ 2 ] = { CETB_NCATTS_STOKES_MIN, CETB_NCATTS_STOKES_MAX };
  unsigned short tb_or_stokes_valid_range[ 2 ];
  short tb_time_fill_value=CETB_NCATTS_TB_TIME_FILL_VALUE;
  short tb_time_valid_range[ 2 ] = { CETB_NCATTS_TB_TIME_MIN, CETB_NCATTS_TB_TIME_MAX };
  unsigned short tb_stddev_fill_value=CETB_NCATTS_TB_STDDEV_FILL_VALUE;
  unsigned short tb_stddev_missing_value=CETB_NCATTS_TB_STDDEV_MISSING_VALUE;
  unsigned short tb_stddev_valid_range[ 2 ] = { CETB_NCATTS_TB_STDDEV_MIN, CETB_NCATTS_TB_STDDEV_MAX };
  unsigned char tb_num_samples_fill_value=CETB_NCATTS_TB_NUM_SAMPLES_FILL_VALUE;
  unsigned char tb_num_samples_valid_range[ 2 ] = { CETB_NCATTS_TB_NUM_SAMPLES_MIN,
						    CETB_NCATTS_TB_NUM_SAMPLES_MAX };
  short theta_fill_value=CETB_NCATTS_THETA_FILL_VALUE;
  short theta_valid_range[ 2 ] = { CETB_NCATTS_THETA_MIN, CETB_NCATTS_THETA_MAX };
  float error_valid_range[ 2 ] = { 0.0, NC_MAX_FLOAT };
  float tb_or_stokes_scaled_min, tb_or_stokes_scaled_max, tb_or_stokes_scale_factor;
  float tb_or_stokes_stddev_scale_factor, tb_or_stokes_SIR_offset;
  int tb_or_stokes_add_offset, tb_or_stokes_stddev_add_offset;
  char *tb_or_stokes_SIR_long_name, *tb_or_stokes_GRD_long_name;

  long head_len;
  int errors = 0;
  int status;

  int median_flag = 0;  /* default: no median filter in SIRF algorithm */
  int ibeam = 0;
  int resolution;
  cetb_resolution_id base_resolution = 0;

  /* begin program */

  printf("BYU SSM/I meta SIR/SIRF program: C version %f\n",VERSION);

  if (argc < 2) {
    printf("\nusage: %s setup_in outpath \n\n",argv[0]);
    printf(" input parameters:\n");
    printf("   setup_in        = input setup file\n");
    printf("   outpath         = output path\n");
    return(0);
  }
  file_in=argv[1];

  imf = fopen(file_in,"r"); 
  if (imf == NULL) {
    fprintf( stderr, "%s: ERROR: cannot open input setup file: %s\n", __FILE__, argv[1] ); 
     exit(-1);
  }

  strcpy(outpath,"./"); /* default output path */
  if (argc > 2) 
    sscanf(argv[2],"%s",outpath);
  fprintf( stderr, "%s: Output path %s: ", __FILE__, outpath );

  /* get input file size */
  fseek(imf, 0L, REL_EOF);
  nls=ftell(imf);
  rewind(imf);

/* read setup file header info 
   note: setup file was original written by a fortran program and so has extra
         record information.  This must be skipped when reading */

/*   read (50) irecords  ! Get number of measurement records in file */

   if (fread(&dumb,  sizeof(int)  , 1, imf) == 0) Ferror(0);  /* record header */
   if (fread(&irecords,sizeof(int)  , 1, imf) == 0) Ferror(1);
   if (fread(&dumb,  sizeof(int)  , 1, imf) == 0) Ferror(2); /* record trailer */

/*   read (50) nsx,nsy,ascale,bscale,a0,b0,xdeg,ydeg ! Get header info */

   if (fread(&dumb,  sizeof(int)  , 1, imf) == 0) Ferror(3);  /* record header */
   if (fread(&nsx,   sizeof(int)  , 1, imf) == 0) Ferror(4);
   if (fread(&nsy,   sizeof(int)  , 1, imf) == 0) Ferror(5);
   if (fread(&ascale,sizeof(float), 1, imf) == 0) Ferror(6);
   if (fread(&bscale,sizeof(float), 1, imf) == 0) Ferror(7);
   if (fread(&a0,    sizeof(float), 1, imf) == 0) Ferror(8);
   if (fread(&b0,    sizeof(float), 1, imf) == 0) Ferror(9);
   if (fread(&xdeg,  sizeof(float), 1, imf) == 0) Ferror(10);
   if (fread(&ydeg,  sizeof(float), 1, imf) == 0) Ferror(11);

   if (fread(&dumb,  sizeof(int)  , 1, imf) == 0) Ferror(12); /* record trailer */

/*   read (50) dstart,dend,mstart,mend,year,regnum,projt,npol,latl,lonl,lath,lonh,regname */

   if (fread(&dumb,   sizeof(int),   1, imf) == 0) Ferror(21);
   if (fread(&isday,  sizeof(int),   1, imf) == 0) Ferror(22);
   if (fread(&ieday,  sizeof(int),   1, imf) == 0) Ferror(23);
   if (fread(&ismin,  sizeof(int),   1, imf) == 0) Ferror(24);
   if (fread(&iemin,  sizeof(int),   1, imf) == 0) Ferror(25);
   if (fread(&iyear,  sizeof(int),   1, imf) == 0) Ferror(26);
   if (fread(&iregion,sizeof(int),   1, imf) == 0) Ferror(27);
   if (fread(&iopt ,  sizeof(int),   1, imf) == 0) Ferror(28);
   if (fread(&ipol,   sizeof(int),   1, imf) == 0) Ferror(29);
   if (fread(&latl,   sizeof(float), 1, imf) == 0) Ferror(30);
   if (fread(&lonl,   sizeof(float), 1, imf) == 0) Ferror(31);
   if (fread(&lath,   sizeof(float), 1, imf) == 0) Ferror(32);
   if (fread(&lonh,   sizeof(float), 1, imf) == 0) Ferror(33);
   if (fread(regname, sizeof(char), 10, imf) == 0) Ferror(34);
   regname[10]='\0';
   if (fread(&dumb,   sizeof(int),   1, imf) == 0) Ferror(35);

/*   read (50) nsx2,nsy2,non_size_x,non_size_y, ascale2,bscale2,a02,b02,xdeg2,ydeg2 */

   if (fread(&dumb,   sizeof(int)  , 1, imf) == 0) Ferror(43);/* record header */
   if (fread(&nsx2,   sizeof(int)  , 1, imf) == 0) Ferror(44);
   if (fread(&nsy2,   sizeof(int)  , 1, imf) == 0) Ferror(45);
   if (fread(&non_size_x,sizeof(int)  , 1, imf) == 0) Ferror(46);
   if (fread(&non_size_y,sizeof(int)  , 1, imf) == 0) Ferror(47);
   if (fread(&ascale2,sizeof(float), 1, imf) == 0) Ferror(48);
   if (fread(&bscale2,sizeof(float), 1, imf) == 0) Ferror(49);
   if (fread(&a02,    sizeof(float), 1, imf) == 0) Ferror(50);
   if (fread(&b02,    sizeof(float), 1, imf) == 0) Ferror(51);
   if (fread(&xdeg2,  sizeof(float), 1, imf) == 0) Ferror(52);
   if (fread(&ydeg2,  sizeof(float), 1, imf) == 0) Ferror(53);

   if (fread(&dumb,   sizeof(int)  , 1, imf) == 0) Ferror(54);/* record trailer */

   /* file header read completed, summarize */
   fprintf( stderr, "\n%s:   Input file header info: '%s'\n", __FILE__, file_in );
   fprintf( stderr, "\n" );

   /* read output file names and misc variables

     In principle, these can be in variable order.  However, if standard
     program is used, order and number is known.  Most variables are info
     only, but some control the internal operations.
   */

   end_flag = 0;
   input_file_total = 0;
   do {
     
     if (fread(&dumb,   sizeof(int),   1, imf) == 0) {
       fprintf( stderr, "%s: reached Ferror(70), Error in setup file %s\n",
		__FILE__, file_in );
       exit (-1);
     }
     if (fread(line,   sizeof(char), 100, imf) == 0) {
       fprintf( stderr, "%s: reached Ferror(71), Error in setup file %s\n",
		__FILE__, file_in );
       exit (-1);
     }
     if (fread(&dumb,   sizeof(int),   1, imf) == 0) { 
       fprintf( stderr, "%s: reached Ferror(72), Error in setup file %s\n",
		__FILE__, file_in );
       exit (-1);
     }

     if (strstr(line,"A_initialization") != NULL) {
       x = strchr(line,'=');
       a_init=strtof(++x, &stopstring);
       fprintf( stderr, "%s: A_initialization of %f\n", __FUNCTION__, a_init );
     }

     if (strstr(line,"Response_threshold") != NULL) {
       x = strchr(line,'=');
       rthreshold=strtof(++x, &stopstring);
       fprintf( stderr, "%s: Response_threshold of %f\n", __FUNCTION__, rthreshold );
     }

     if (strstr(line,"Beam_code") != NULL) {
       x = strchr(line,'=');
       ibeam=strtol(++x, NULL, 10);
       fprintf( stderr, "%s: Beam code %d\n", __FUNCTION__, ibeam );
     }

     if (strstr(line,"Max_iterations") != NULL) {
       x = strchr(line,'=');
       nits=strtol(++x, NULL, 10);
       fprintf( stderr, "%s: Max iterations of %d\n", __FUNCTION__, nits );
     }

     if (strstr(line,"Max_Fill") != NULL) {
       x = strchr(line,'=');
       MAXFILL=strtol(++x, NULL, 10);
       fprintf( stderr, "%s: Max fill %d\n", __FUNCTION__, MAXFILL);
     }

     if (strstr(line," Base_resolution") != NULL) {
       x = strchr(line,'=');
       resolution=strtol(++x, NULL, 10);
       fprintf( stderr, "%s: Base resolution %d\n", __FUNCTION__,
		resolution );
       switch (resolution) {
       case 25:
	 base_resolution = CETB_25KM;
	 break;
       case 24:
	 base_resolution = CETB_24KM;
	 break;
       case 36:
	 base_resolution = CETB_36KM;
	 break;
       default:
	 base_resolution = CETB_NO_RESOLUTION;
       }
     }

     if ( strstr( line, " Producer_id" ) != NULL ) {
       x = strchr( line,'=' );
       swath_producer_id = ( cetb_swath_producer_id )strtol(++x, NULL, 10);
       fprintf( stderr,  "%s: Producer_id %s\n",  __FUNCTION__,
		cetb_swath_producer_id_name[ swath_producer_id ] );
     }

     if ( strstr( line, " Platform_id" ) != NULL ) {
       x = strchr( line,'=' );
       platform_id = ( cetb_platform_id )strtol(++x, NULL, 10);
       fprintf( stderr,  "%s: Platform_id %s\n",  __FUNCTION__,
		cetb_platform_id_name[ platform_id ] );
     }

     if ( strstr( line, " Sensor_id" ) != NULL ) {
       x = strchr( line,'=' );
       sensor_id = ( cetb_sensor_id )strtol(++x, NULL, 10);
       fprintf( stderr,  "%s: Sensor_id %s\n",  __FUNCTION__,
		cetb_sensor_id_name[ sensor_id ] );
     }

     if ( strstr( line, " Pass_direction" ) != NULL ) {
       x = strchr( line,'=' );
       direction_id = ( cetb_direction_id )strtol(++x, NULL, 10);
       fprintf( stderr,  "%s: Direction_id %s\n",  __FUNCTION__,
		cetb_direction_id_name[ direction_id ] );
     }

     if ( strstr( line, " Ltod_morning" ) != NULL ) {
       x = strchr( line,'=' );
       ltod_morning = strtof(++x, &stopstring);
       fprintf( stderr,  "%s: Ltod_morning %f\n",  __FUNCTION__, ltod_morning );
     }

     if ( strstr( line, " Ltod_evening" ) != NULL ) {
       x = strchr( line,'=' );
       ltod_evening = strtof(++x, &stopstring);
       fprintf( stderr,  "%s: Ltod_evening %f\n",  __FUNCTION__, ltod_evening );
     }

     if (strstr(line,"Search_box_km") != NULL) {
       x = strchr(line,'=');
       box_size_km=strtof(++x, &stopstring);
       fprintf( stderr, "%s: Search_box_km %f\n", __FUNCTION__, box_size_km );
     }

     if (strstr(line,"Response_Multiplier") != NULL) {
       x = strchr(line,'=');
     }

     if (strstr(line,"Sensor") != NULL) {
       x = strchr(line,'=');
       strncpy(sensor_in,++x,40);
       fprintf( stderr, "%s: Sensor '%s'\n", __FUNCTION__, sensor_in);
     }

     if (strstr(line,"Input_file") != NULL) {
       x = strchr(line,'=');
       status = utils_allocate_clean_aligned_memory( (void**)&list_of_input_files[input_file_total],
						     FILENAME_MAX );
       if ( 0 != status ) {
	 fprintf( stderr, "%s: couldn't allocate space for list of input files, "
		  "setup file=%s\n", __FILE__, file_in );
	 exit (-1);
       }
       strcpy( list_of_input_files[input_file_total], ++x );
       fprintf( stderr, "%s: Input file '%s'\n", __FUNCTION__,
		list_of_input_files[input_file_total] );
       input_file_total++;
     }

     if ((x = strchr(line+4,' ')) != NULL) *x='\0'; /* truncate off any trailing spaces */

     if (strstr(line,"Has_Azimuth_Angle") != NULL) {
       x = strchr(line,'=')+1;
       if (strstr(x,"T") != NULL || strstr(x,"t") != NULL) {
	 HASAZANG=1;
	 HS += 4;  /* increase read buffer size */
       }
       if (strstr(x,"F") != NULL || strstr(x,"f") != NULL)
	 HASAZANG=0;
       fprintf( stderr, "%s: Has azimuth angle: %d\n", __FUNCTION__, HASAZANG);       
     }

     if (strstr(line,"End_header") != NULL) {
       end_flag = 1;
     }

   } while (end_flag == 0);

   /*
    * USING gsx:
    *
    * ascale is the factor we really want (FACTOR 1=12.k km...)
    * resolution factor - is read from ascale
    *
    * setup saves:
    *  - platform_id
    *  - sensor_id
    *  - pass direction
    *  - swath_producer_id (CSU or RSS)
    *  - list of actual gsx source files used as input
    *  - list of GSX version used to create each gsx file used as input
    *
    * Initialize 2 cetb_files one for SIR output, the other for GRD output.
    * Also set the appropriate constants in case this is a Stokes var rather than TB
    */

   tb_or_stokes_scaled_min = CETB_NCATTS_TB_SCALED_MIN;
   tb_or_stokes_scaled_max = CETB_NCATTS_TB_SCALED_MAX;
   tb_or_stokes_scale_factor = CETB_NCATTS_TB_SCALE_FACTOR;
   tb_or_stokes_stddev_scale_factor = CETB_NCATTS_TB_STDDEV_SCALE_FACTOR;
   tb_or_stokes_add_offset = CETB_NCATTS_TB_ADD_OFFSET;
   tb_or_stokes_stddev_add_offset = CETB_NCATTS_TB_STDDEV_ADD_OFFSET;
   tb_or_stokes_SIR_offset = CETB_NCATTS_TB_ADD_OFFSET;
   tb_or_stokes_valid_range[0] = tb_valid_range[0];
   tb_or_stokes_valid_range[1] = tb_valid_range[1];
   tb_or_stokes_SIR_long_name = strdup( CETB_NCATTS_TB_SIR_LONG_NAME );
   tb_or_stokes_GRD_long_name = strdup( CETB_NCATTS_TB_GRD_LONG_NAME );
   anodata_A = CETB_NCATTS_TB_FILL_VALUE;
   anodata_V = (float)(CETB_NCATTS_TB_STDDEV_FILL_VALUE*CETB_NCATTS_TB_STDDEV_SCALE_FACTOR);

   if ( ( sensor_id == CETB_SMAP_RADIOMETER ) &&
	( SMAP_1d41F == cetb_ibeam_to_cetb_smap_channel[ibeam] ) ) {
     tb_or_stokes_scaled_min = CETB_NCATTS_STOKES_SCALED_MIN;
     tb_or_stokes_scaled_max = CETB_NCATTS_STOKES_SCALED_MAX;
     tb_or_stokes_scale_factor = CETB_NCATTS_STOKES_SCALE_FACTOR;
     tb_or_stokes_stddev_scale_factor = CETB_NCATTS_STOKES_STDDEV_SCALE_FACTOR;
     tb_or_stokes_add_offset = CETB_NCATTS_STOKES_ADD_OFFSET;
     tb_or_stokes_stddev_add_offset = CETB_NCATTS_STOKES_STDDEV_ADD_OFFSET;
     tb_or_stokes_SIR_offset = CETB_NCATTS_STOKES_ADD_OFFSET;
     tb_or_stokes_valid_range[0] = stokes_valid_range[0];
     tb_or_stokes_valid_range[1] = stokes_valid_range[1];
     tb_or_stokes_SIR_long_name = strdup( CETB_NCATTS_STOKES_SIR_LONG_NAME );
     tb_or_stokes_GRD_long_name = strdup( CETB_NCATTS_STOKES_GRD_LONG_NAME );
     anodata_A = CETB_NCATTS_STOKES_FILL_VALUE;
     anodata_V = (float)((CETB_NCATTS_STOKES_STDDEV_FILL_VALUE*CETB_NCATTS_STOKES_STDDEV_SCALE_FACTOR)
			 + CETB_NCATTS_STOKES_STDDEV_ADD_OFFSET);
   }
   
   cetb_sir = cetb_file_init( outpath,
			      iregion, base_resolution, ascale, platform_id,
			      sensor_id,
			      iyear, isday, ibeam,
			      direction_id,
			      CETB_SIR,
			      swath_producer_id,
			      basename( argv[0] ) );
   if ( !cetb_sir ) {
     fprintf( stderr, "%s: Error initializing cetb_file for %s.\n",
	      __FILE__, cetb_reconstruction_id_name[ CETB_SIR ] );
     exit( -1 );
   } 

   if ( 0 != cetb_file_open( cetb_sir ) ) {
     fprintf( stderr, "%s: Error opening cetb_file=%s.\n", __FILE__,
	      cetb_sir->filename );
     exit( -1 );
   } 

   cetb_grd = cetb_file_init( outpath, iregion, base_resolution,
			      CETB_MIN_RESOLUTION_FACTOR,
			      platform_id, sensor_id,
			      iyear, isday, ibeam,
			      direction_id,
			      CETB_GRD,
			      swath_producer_id,
			      basename( argv[0] ) );
   if ( !cetb_grd ) {
     fprintf( stderr, "%s: Error initializing cetb_file for %s.\n",
	      __FILE__, cetb_reconstruction_id_name[ CETB_GRD ] );
     exit( -1 );
   }

   if ( 0 != cetb_file_open( cetb_grd ) ) {
     fprintf( stderr, "%s: Error opening cetb_file=%s.\n", __FILE__,
	      cetb_grd->filename );
     exit( -1 );
   }
     
   head_len = ftell(imf);
   fprintf( stderr, "%s: Input header file length %ld\n", __FILE__, head_len );
   nls=nls-head_len;

/* header read completed, now determine how much program memory to allocate */

   nspace = nls * file_savings;/* space to allocate for measurement storage */
   fprintf( stderr, "%s: File size: %ld  Space allocated: %ld\n", __FILE__, nls,
	    nspace );
   if ( 0 != utils_allocate_clean_aligned_memory( ( void ** )&space,
						  ( size_t )nspace*sizeof(char)) ) {
     fprintf( stderr, "%s: inadequate memory for setup file=%s \n", __FILE__,
	      file_in );
     exit(-1);
   }

/* allocate storage space for image and working arrays
   note: these arrays are re-used multiple times to save memory */

  nsize = nsx * nsy;
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&a_val,
						 (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for a_val, setup file=%s\n", __FILE__,
	     file_in );
    exit(-1);
  }
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&b_val,
						 (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for b_val, setup file=%s\n",
	     __FILE__, file_in );
    exit(-1);
  }
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&a_temp,
						 (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for a_temp, setup file=%s\n",
	     __FILE__, file_in );
    exit(-1);
  }
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&sxy,
						 (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for sxy, setup file=%s\n",
	     __FILE__, file_in );
    exit(-1);
  }
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&sx,
						 (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for sx, setup file=%s\n",
	     __FILE__, file_in );
    exit(-1);
  }
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&sx2,
						 (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for sx2, setup file=%s\n",
	     __FILE__, file_in );
    exit(-1);
  }
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&sy,
						 (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for sy, setup file=%s\n",
	     __FILE__, file_in );
    exit(-1);
  }
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&tot,
						 (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for tot, setup file=%s\n",
	     __FILE__, file_in );
    exit(-1);
  }
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&num_samples,
						 (size_t)(sizeof(unsigned char)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for num_samples, setup file=%s\n",
	     __FILE__, file_in );
    exit(-1);
  }

  /* with storage allocated, copy file into memory */
  /* read measurement file into memory, storing only essential information  */

  nrec = 0;         /* number of meaurements in file */
  ncnt = 0;         /* number of useable measurements */
  nbyte = 0;        /* file size in bytes */
  store=space;      /* storage pointer */
  
  fprintf( stderr, "%s: Begin setup file copy into memory\n", __FILE__ );
  while (fread(&dumb, sizeof(int), 1, imf) != 0) {

    /*5 items at 4 bytes each: 20 bytes if no azimuth angle */
    /*6 items at 4 bytes each: 24 bytes if azimuth angle */
    if (nbyte+HS < nspace) {
      if ((dumb=fread(store, sizeof(char), HS, imf)) != HS) {
	fprintf ( stderr, "%s: Error reading input file data at 180 %d, input file %s\n",
		  __FILE__, dumb, file_in);
	exit(-1);
      }
      if (fread(&dumb,sizeof(int), 1, imf) == 0) Ferror(100);
      get_vars_from_store( store, &tbval, &ang, &count, 0.0, &ktime, &iadd,
			   HASAZANG, &azang );

      if (count > MAXFILL) {
	fprintf( stderr, "%s: *** Count error %d  record %d\n", __FILE__, count, nrec );
	fprintf( stderr, "%s: %f %f %d %d \n", __FILE__, tbval, ang, ktime, iadd );
	count=MAXFILL;
      }

      /* if measurement is "valid" keep it by indexing counters 
	 if not, new values will be stored over old values */

      keep=0;
      if (tbval < tb_or_stokes_scaled_max && tbval > tb_or_stokes_scaled_min) { 
	nbyte=nbyte+HS;
	store=store+HS;
	ncnt++;
	keep=1;
      }

      /* read fill_array pixel indices */
      if (nbyte+count*4 < nspace) {
	if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(110);
	if (fread(store, sizeof(int), count, imf) != count) {
	  fprintf( stderr, "%s: Error reading input file data at 111 file %s\n",
		   __FILE__, file_in );
	  /* exit(-1); */
	  goto label;
	}
	if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(112);
	if (keep == 1) {
	  nbyte=nbyte+count*4;
	  store=store+count*4;
	}
      } else {
	fprintf( stderr, "%s: out of storage space 1 %d for file %s\n",
		 __FILE__, ncnt, file_in );
	fprintf( stderr, "%s: out of storage space 1 %d %ld %ld for file %s\n",
		 __FILE__, ncnt, nbyte, nspace, file_in );
	exit(-1);
      }

      /* read response_array weights */
      if (nbyte+count*2 < nspace) {
	if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(1111);
	if (fread(store, sizeof(short int), count, imf) != count) {
	  fprintf( stderr, "%s: Error reading input file data at 1111 for file %s\n",
		   __FILE__, file_in );
	  goto label;
	}
	if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(1121);
	if (keep == 1) {
	  nbyte=nbyte+count*2;
	  if (count % 2 == 1) count=count+1;  /* ensure storage of next record on a 4byte word boundary */
	  store=store+count*2;
	}
      } else {
	fprintf( stderr, "%s: out of storage space 2 %d for file %s\n",
		 __FILE__, ncnt, file_in );
	fprintf( stderr, "%s: out of storage space 2 %d %ld %ld for file %s\n",
		 __FILE__, ncnt, nbyte, nspace, file_in );
	exit(-1);
      }

      nrec++;

    } else {
      fprintf( stderr, "%s: out of storage space 3 %d for file %s\n",
	       __FILE__, ncnt, file_in );
      fprintf( stderr, "%s: out of storage space 3 %d %ld for file %s\n",
	       __FILE__, ncnt, nspace, file_in );
      exit(-1);
    }
  }
 label:
  fclose(imf);

  /* print measurement file storage requirements */
  ratio=(float)(100.0 * (float) nbyte / (float) nls);
  fprintf( stderr, "%s:  Input file %s read into ram\n", __FILE__, file_in );
  fprintf( stderr, "%s:  Total storage used: %d %d recs = %ld of %ld (%.1f%% %.1f%%)"
	   " for input file %s\n",
	   __FILE__, nrec, ncnt, nbyte, nspace, ratio, 100.0*file_savings, file_in );

/* Begin SIR/SIRF processing.  First initialize working arrays. */

  for (i=0; i < nsize; i++)
    *(a_val+i) = a_init;
  for (i=0; i < nsize; i++)
    *(b_val+i) = 0.0;
  for (i=0; i < nsize; i++)
    *(a_temp+i) = 0.0;
  for (i=0; i < nsize; i++)
    *(sx+i)  = 0.0;
  for (i=0; i < nsize; i++)
    *(sy+i)  = 0.0;
  for (i=0; i < nsize; i++)
    *(sxy+i) = 0.0;
  for (i=0; i < nsize; i++)
    *(sx2+i) = 0.0;
  for (i=0; i < nsize; i++)
    *(tot+i) = 0.0;

  old_amin=a_init;
  old_amax=a_init;
  
  fprintf( stderr, "%s: \nSIR parameters: A_init=%f  N=%d for file %s\n",
	   __FILE__, a_init, nits, file_in );

  /* for each iteration of SIR */

  for (its=0; its < nits; its++) {

    fprintf( stderr, "%s: \nSIR iteration %d %d for file %s\n",
	     __FILE__, its+1, ncnt, file_in );

    /* for each measurement, accumulate results */

    store=space;
    for (irec = 0; irec < ncnt; irec++) {

      if (its == 0) {
	get_vars_from_store( store, &tbval, &ang, &count, tb_or_stokes_SIR_offset,
			   &dummy_ktime, &iadd, HASAZANG, &azang );
      } else {
	get_vars_from_store( store, &tbval, &ang, &count, tb_or_stokes_SIR_offset,
			   &dummy_ktime, &dummy_iadd, HASAZANG, &azang );
      }

      store = store+HS;
      store2 = store + 4*count;

      get_updates(tbval, count, (int *) store, (short int *) store2, its);

      /* compute AVE image during first iteration */
      if (its == 0) 
	compute_ave(tbval, ang, count,(int *) store, (short int *) store2);

      store = store+4*count;
      store = store+2*count;
      if (count % 2 == 1) store=store+2;  /* ensure word boundary */
    }

    /* after processing all measurements for this iteration and
       updating the A image, clear arrays */

    amin =  20000.0;            /* for user stats */
    amax = -20000.0;
    tmax = -1.0;
    total = 0.0;
    
    for (i=0; i<nsize; i++){
      if (*(tot+i) > 0) {    /* update only hit pixels */
	total = total + *(tot+i);
	*(a_val+i) = *(a_temp+i);

	if (its == 0) {        /* first iteration */
	  if (*(sy+i) > 0) {      
	    *(sx+i) = *(sx+i) / *(sy+i);
	    *(sx2+i) = *(sx2+i) / *(sy+i);
	    *(sx2+i) = *(sx2+i) - *(sx+i) * *(sx+i);
	    if (*(sx2+i) > 0.0) 
	      *(sx2+i) = (float)sqrt((double) *(sx2+i));
	    else
	      *(sx2+i) = 0.;
	  } else {
	    *(sx2+i) = anodata_I;
	    *(sx+i) = anodata_Ia;
	  }
	  *(sxy+i) = *(tot+i); 
	  if (*(sy+i) > 0)  /* first iteration, compute AVE */
	    *(b_val+i) = *(b_val+i) / *(sy+i);
	  else
	    *(b_val+i) = anodata_A;
	  if (AVE_INIT)
	    *(a_val+i)=*(b_val+i); /* copy AVE to sir iteration buffer */
	}
	if (its+1 != nits) {          /* clean up */
	  *(a_temp+i) = 0.0;
	  *(tot+i) = 0.0; 
	} else                       /* on last iteration */
	  tmax = max(tmax, *(tot+i));

	amin = min(amin, *(a_val+i));
	amax = max(amax, *(a_val+i));
	
      } else {
	*(a_val+i) = anodata_A;
	*(b_val+i) = anodata_A;
	*(sx2+i) = anodata_I;
	*(sx+i) = anodata_Ia;
      }	
    }

    if (its == 0) fprintf( stderr, "%s:  Average weight: %.4f for file %s\n",
			   __FILE__, total/nsize, file_in );
    fprintf( stderr, "%s:  A min max  --> %f %f %d for file %s\n",
	     __FILE__, amin, amax, its+1, file_in );
    fprintf( stderr, "%s:  A change   --> %f %f for file %s\n", __FILE__,
	     amin-old_amin, amax-old_amax, file_in );

    old_amin=amin;
    old_amax=amax;

    if (median_flag == 1)   /* apply modified median filtering */
      filter(a_val, 3, 0, nsx, nsy, a_temp, anodata_A);  /* 3x3 modified median filter */

  }    /* end of loop for each SIR iteration */

  fprintf( stderr, "%s: weight max --> %f Average weight: %.4f for file %s\n",
	   __FILE__, tmax, total/nsize, file_in );

  /*
   * if this is a Stokes variable then the SIR offset must be added back in before
   * the variable is written and packed into the netCDF output file
  */

  for ( i = 0; i < nsx*nsy; i++ ) {
    *(a_val+i) += tb_or_stokes_SIR_offset;
  }
  
  if ( 0 != cetb_file_add_var( cetb_sir, "TB",
			       NC_USHORT, a_val,
			       ( size_t )nsx, ( size_t ) nsy,
			       CETB_FILE_TB_STANDARD_NAME,
			       tb_or_stokes_SIR_long_name,
			       CETB_FILE_TB_UNIT,
			       &tb_fill_value,
			       &tb_missing_value,
			       &tb_or_stokes_valid_range,
			       CETB_PACK,
			       (float) tb_or_stokes_scale_factor,
			       (float) tb_or_stokes_add_offset,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb (A) to %s.\n", __FILE__,
	     cetb_sir->filename );
  } else {
    fprintf( stderr, "> %s: Wrote Tb (A) to %s.\n", __FILE__, cetb_sir->filename );
  }
    
  /* Save the number of measurement samples that hit each pixel */
  if ( 0 != cetb_file_add_var( cetb_sir, "TB_num_samples",
			       NC_UBYTE, num_samples,
			       nsx, nsy,
			       NULL,
			       "SIR TB Number of Measurements",
			       "count",
			       &tb_num_samples_fill_value,
			       NULL,
			       &tb_num_samples_valid_range,
			       CETB_NO_PACK,
			       0.0,
			       0.0,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb num_samples to %s.\n", __FILE__,
	     cetb_sir->filename );
  } else {
    fprintf( stderr, "> %s: Wrote Tb num_samples to %s.\n", __FILE__, cetb_sir->filename );
  }

  if ( 0 != cetb_file_add_var( cetb_sir, "Incidence_angle",
			       NC_SHORT, sx,
			       ( size_t )nsx, ( size_t ) nsy,
			       CETB_FILE_INCIDENCE_ANGLE_STANDARD_NAME,
			       "SIR Incidence Angle",
			       CETB_FILE_ANGULAR_UNIT,
			       &theta_fill_value,
			       NULL,
			       &theta_valid_range,
			       CETB_PACK,
			       (float) CETB_NCATTS_THETA_SCALE_FACTOR,
			       (float) CETB_NCATTS_THETA_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb incidence angle (j_image) to %s.\n",
	     __FILE__, cetb_sir->filename );
  } else {
    fprintf( stderr, "> %s: Wrote Tb incidence angle (j_image) to %s.\n",
	     __FILE__, cetb_sir->filename );
  }

/* create STD and Err images */
  /* initialize arrays */

  for (i=0; i < nsize; i++) {
    *(a_temp+i) = 0.0;
    *(sx+i)  = 0.0;
    *(sy+i)  = 0.0;
    *(sxy+i) = 0.0;
    *(sx2+i) = 0.0;
    *(tot+i) = 0.0;
  }

  store=space;
  for (irec = 0; irec < ncnt; irec++) {

    get_vars_from_store( store, &tbval, &ang, &count, tb_or_stokes_SIR_offset,
			 &dummy_ktime, &dummy_iadd, HASAZANG, &azang );
    //    tbval = *((float *) (store+0));
    //    tbval = tbval - tb_or_stokes_add_offset;
    //    ang   = *((float *) (store+4));
    //    count = *((int *)   (store+8));
    //    if (HASAZANG)
    //      azang = *((float *) (store+20));
      
    store = store+HS;
    store2 = store+4*count;

    stat_updates(tbval, count, (int *) store, (short int *) store2);
    store = store+4*count;
    store = store+2*count;
    if (count % 2 == 1) store=store+2;  /* ensure word boundary */
  }

  amin =  32000.0;            /* for user stats */
  amax = -32000.0;
  bmin =  300.0;
  bmax = -300.0;
  total = 0.0;
    
  for (i=0; i<nsize; i++){
    if (*(tot+i) > 0) {    /* update only hit pixels */
      total = total + *(tot+i);
      *(sx+i) = *(sx+i) / *(tot+i); /* first moment (mean) */
      *(sy+i) = *(sy+i) / *(tot+i); /* second moment */
      
      if (*(sy+i) > 0.0) {
	*(sxy+i) = *(sy+i) - *(sx+i) * *(sx+i);
	if (*(sxy+i) > 0.0) 
	  *(sxy+i) = (float)sqrt( (double) *(sxy+i));
	else
	  *(sxy+i) = 0.0;
      } else
	  *(sxy+i) =0.0;
		
      amin = min(amin, *(sxy+i));
      amax = max(amax, *(sxy+i));
      bmin = min(bmin, *(sx+i));
      bmax = max(bmax, *(sx+i));
	
    } else {
      *(sxy+i) = anodata_V;
      *(sx+i) = anodata_E;
    }
  }
    
  fprintf( stderr, "%s: Tb STD min   max --> %f %f for file %s\n",
	   __FILE__, amin, amax, file_in );
  fprintf( stderr, "%s: Tb ERR min   max --> %f %f for file %s\n",
	   __FILE__, bmin, bmax, file_in );

  if ( 0 != cetb_file_add_var( cetb_sir, "TB_std_dev",
			       NC_USHORT, sxy,
			       ( size_t )nsx, ( size_t ) nsy,
			       NULL,
			       "SIR TB Std Deviation",
			       CETB_FILE_TB_UNIT,
			       &tb_stddev_fill_value,
			       &tb_stddev_missing_value,
			       &tb_stddev_valid_range,
			       CETB_PACK,
			       (float) tb_or_stokes_stddev_scale_factor,
			       (float) tb_or_stokes_stddev_add_offset,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb stddev (V)to %s.\n", __FILE__,
	     cetb_sir->filename );
  } else {
    fprintf( stderr, "> %s: Wrote Tb stddev (V) to %s.\n", __FILE__,
	     cetb_sir->filename );
  }
    
/* create time image */
  /* initialize arrays */

  for (i=0; i < nsize; i++) {
    *(a_temp+i) = 0.0;
    *(sx+i)  = 0.0;
    *(sy+i)  = 0.0;
    *(sxy+i) = 0.0;
    *(tot+i) = 0.0;
  }

  store=space;
  for (irec = 0; irec < ncnt; irec++) {
    
    get_vars_from_store( store, &tbval, &ang, &count, tb_or_stokes_SIR_offset,
			 &ktime, &dummy_iadd, HASAZANG, &azang );
    if (ktime < 0) ktime = -ktime;
    
    store = store+HS;
    store2 = store+4*count;     

    time_updates((float) ktime, count, (int *) store);
    store = store+4*count;
    store = store+2*count;
    if (count % 2 == 1) store=store+2;  /* ensure word boundary */
  }

  amin =  32000.0;            /* for user stats */
  amax = -32000.0;
  total = 0.0;
    
  for (i=0; i<nsize; i++){
    if (*(tot+i) > 0) {    /* update only hit pixels */
      total = total + *(tot+i);

      if (*(tot+i) > 1) 
	if (fabs(*(sy+i)) >= FLT_EPSILON) 
	  *(sxy+i) = *(sx+i) / *(sy+i);
	else
	  *(sxy+i) = anodata_P;
      else
	*(sxy+i) = anodata_P;
		
      if (*(sxy+i) > 0.0)
	amin = min(amin, *(sxy+i));
      amax = max(amax, *(sxy+i));
	
    } else
      *(sxy+i) = anodata_P;
  }

  fprintf( stderr, "%s:  Time min   max --> %f %f for file %s\n",
	   __FILE__, amin, amax, file_in );

  if ( 0 != cetb_file_add_var( cetb_sir, "TB_time",
  			       NC_SHORT, sxy,
  			       ( size_t )nsx, ( size_t ) nsy,
  			       CETB_FILE_TB_TIME_STANDARD_NAME,
  			       "SIR TB Time of Day",
  			       cetb_sir->epoch_string,
  			       &tb_time_fill_value,
  			       NULL,
  			       &tb_time_valid_range,
  			       CETB_PACK,
  			       (float) CETB_NCATTS_TB_TIME_SCALE_FACTOR,
  			       (float) CETB_NCATTS_TB_TIME_ADD_OFFSET,
  			       "gregorian" ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb time (P) to %s.\n",
	     __FILE__, cetb_sir->filename );
  } else {
    fprintf( stderr, "> %s: Wrote Tb time (P) to %s.\n",
	     __FILE__, cetb_sir->filename );
  }

/* create non-enhanced images
   these are grd images pixel replicated to be at the same 
   resolution of the ave and sir images */
  /* note that this should be the case!: (grid*non_size = sir size)
     nsx2 = nsx/non_size_x;
     nsy2 = nsy/non_size_y; */

  nsize2=nsx2*nsy2;

  /* initialize arrays */

  for (i=0; i < nsize2; i++)
    *(a_val+i) = 0.0;
  for (i=0; i < nsize2; i++)
    *(a_temp+i) = 0.0;
  for (i=0; i < nsize2; i++)
    *(tot+i) = 0.0;
  for (i=0; i < nsize2; i++)
    *(sx+i)  = 0.0;
  for (i=0; i < nsize2; i++)
    *(sy+i)  = 0.0;
  for (i=0; i < nsize2; i++)
    *(sxy+i) = 0.0;
  for (i=0; i < nsize2; i++)
    *(sx2+i) = 0.0;
  for (i=0; i < nsize2; i++)
    *(num_samples+i) = 0;

  store=space;
  for (irec = 0; irec < ncnt; irec++) {

    get_vars_from_store( store, &tbval, &ang, &count, tb_or_stokes_SIR_offset,
			 &ktime, &iadd, HASAZANG, &azang );

    store = store+HS;
    store = store+4*count;
    store = store+2*count;
    if (count % 2 == 1) store=store+2;  /* ensure word boundary */

    /* compute address in shrunk workspace */

    if (iadd < 0) iadd=-iadd;  /* remove asc/des flag (if used) */
    if (iadd > 0) {            /* skip out-of-area measurements */

      /* compute location of measurement within lo-res grid */
      ix = (iadd) % nsx;
      iy = (iadd) / nsx;
      ix = ix / non_size_x;
      iy = iy / non_size_y;
      iadd = nsx2*iy+ix; 

    /* compute unweighted, normalized stats for measurements hitting grid element */

      if (iadd >= nsx2*nsy2 || iadd < 0) {  /* keep only in-image measurements */
	fprintf( stderr, "%s: *** Non-enhanced address error: %d %d %d %d %d file %s\n",
		 __FILE__, iadd, ix, iy, non_size_x, non_size_y, file_in );
      } else {
	fn = *(tot + iadd);
	*(tot +  iadd) = *(tot +   iadd) + 1;                    /* count */
	if ( *(num_samples + iadd) < CETB_NCATTS_TB_NUM_SAMPLES_MAX ) {
	  *(num_samples + iadd) = (unsigned char)(*(num_samples + iadd) + 1);         /* num_samples for each pixel */
	}
	ninv = 1/ *(tot + iadd);
	*(sx +   iadd) = (*(sx +   iadd) * fn + ang)*ninv;         /* mean inc angle */
	*(sx2 +  iadd) = (*(sx2 +  iadd) * fn + ang*ang)*ninv;     /* var inc angle */
	*(a_val+ iadd) = (*(a_val+ iadd) * fn + tbval)*ninv;	   /* mean Tb */
	*(sxy +  iadd) = (*(sxy +  iadd) * fn + tbval*ang)*ninv;   /* cross cor TB*inc angle */
	*(sy +   iadd) = (*(sy +   iadd) * fn + tbval*tbval)*ninv; /* var Tb */
	*(a_temp+iadd) = (*(a_temp+iadd) * fn + (float)ktime)*ninv;/* mean time */
      }
    }
  }

  amin =  32000.0;            /* for user stats */
  amax = -32000.0;
  bmin =  32000.0;
  bmax = -32000.0;
  old_bmax = -32000.0;
  old_bmin =  32000.0;
  tmax = -1.0;
  total = 0.0;
    
  for (i=0; i<nsize2; i++){
    if (*(tot+i) > 0) {    /* update only hit pixels */
      total = total + *(tot+i);
      
      amin = min(amin, *(a_val+i));
      amax = max(amax, *(a_val+i));

      if (*(tot+i) > 1.0) {
	temp =  *(sy+i) - (*(a_val+i) * *(a_val+i));
	if (temp > 0.0) {
	  *(sy+i) = (float)sqrt((double) temp);  /* Tb std */ 
	  old_bmin = min(old_bmin, *(sy+i));
	  old_bmax = max(old_bmax, *(sy+i));
	} else
	  *(sy+i) = 0.0;
      } else
	*(sy+i) = 0.0;
      
      if (*(tot+i) > 1.0) {
	denom = *(sx2+i) - (*(sx+i) * *(sx+i));
	if (denom > 0.0) {
	  *(sx2+i) = (float)sqrt((double) denom);  /* inc std */
	  bmin = min(bmin, *(sx2+i));
	  bmax = max(bmax, *(sx2+i));
	} else
	  *(sx2+i) = 0.0;
      } else
	*(sx2+i) = 0.0;

      tmax = max(tmax, *(tot+i));

    } else {
      *(a_val+i) = anodata_A;
      *(a_temp+i) = anodata_P;
      *(sxy+i) = anodata_V;
      *(sx+i) = anodata_Ia;
      *(sx2+i) = anodata_I;
      *(sy+i) = anodata_V;
      *(tot+i) = anodata_C;
    }
  }

  fprintf( stderr, "%s:  Non-enhanced/Grid A  min   max --> %f %f in file %s\n",
	   __FILE__, amin, amax, file_in );
  fprintf( stderr, "%s:  Non-enhanced/Grid V  min   max --> %f %f in file %s\n",
	   __FILE__, old_bmin, old_bmax, file_in );
  fprintf( stderr, "%s:  Non-enhanced/Grid I  min   max --> %f %f in file %s\n",
	   __FILE__, bmin, bmax, file_in );
  fprintf( stderr, "%s:  Non-enhanced/Grid C        max --> %.1f in file %s\n",
	   __FILE__, tmax, file_in );

  for ( i=0; i < nsx2*nsy2; i++ ) {
    *(a_val+i) += tb_or_stokes_SIR_offset;
  }
  if ( 0 != cetb_file_add_var( cetb_grd, "TB",
			       NC_USHORT, a_val,
			       ( size_t )nsx2, ( size_t ) nsy2,
			       CETB_FILE_TB_STANDARD_NAME,
			       tb_or_stokes_GRD_long_name,
			       CETB_FILE_TB_UNIT,
			       &tb_fill_value,
			       &tb_missing_value,
			       &tb_or_stokes_valid_range,
			       CETB_PACK,
			       (float) tb_or_stokes_scale_factor,
			       (float) tb_or_stokes_add_offset,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD Tb (A) to %s.\n",
	     __FILE__, cetb_grd->filename );
  } else {
    fprintf( stderr, "> %s: Wrote GRD Tb (A) to %s.\n",
	     __FILE__, cetb_grd->filename );
  }
    
  /* Save the number of measurement samples that hit each GRD pixel */
  if ( 0 != cetb_file_add_var( cetb_grd, "TB_num_samples",
			       NC_UBYTE, num_samples,
			       ( size_t )nsx2, ( size_t )nsy2,
			       NULL,
			       "GRD TB Number of Measurements",
			       "count",
			       &tb_num_samples_fill_value,
			       NULL,
			       &tb_num_samples_valid_range,
			       CETB_NO_PACK,
			       0.0,
			       0.0,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD Tb num_samples to %s.\n",
	     __FILE__, cetb_grd->filename );
  } else {
    fprintf( stderr, "> %s: Wrote GRD Tb num_samples to %s.\n",
	     __FILE__, cetb_grd->filename );
  }

  if ( 0 != cetb_file_add_var( cetb_grd, "TB_std_dev",
			       NC_USHORT, sy,
			       ( size_t )nsx2, ( size_t )nsy2,
			       NULL,
			       "GRD TB Std Deviation",
			       CETB_FILE_TB_UNIT,
			       &tb_stddev_fill_value,
			       &tb_stddev_missing_value,
			       &tb_stddev_valid_range,
			       CETB_PACK,
			       (float) CETB_NCATTS_TB_STDDEV_SCALE_FACTOR,
			       (float) CETB_NCATTS_TB_STDDEV_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD Tb stddev (V) to %s.\n", __FILE__,
	     cetb_grd->filename );
  } else {
    fprintf( stderr, "> %s: Wrote GRD Tb stddev (V) to %s.\n", __FILE__,
	     cetb_grd->filename );
  }
    
  if ( 0 != cetb_file_add_var( cetb_grd, "Incidence_angle",
			       NC_SHORT, sx,
			       ( size_t )nsx2, ( size_t )nsy2,
			       NULL,
			       "GRD Incidence Angle",
			       CETB_FILE_ANGULAR_UNIT,
			       &theta_fill_value,
			       NULL,
			       &theta_valid_range,
			       CETB_PACK,
			       (float) CETB_NCATTS_THETA_SCALE_FACTOR,
			       (float) CETB_NCATTS_THETA_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD incidence angle (j_image).\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote GRD incidence angle (j_image) to %s.\n", __FILE__,
	     cetb_grd->filename );
  }

  if ( 0 != cetb_file_add_var( cetb_grd, "TB_time",
  			       NC_SHORT, a_temp,
  			       ( size_t )nsx2, ( size_t )nsy2,
  			       NULL,
  			       "GRD TB Time of Day",
  			       cetb_sir->epoch_string,
  			       &tb_time_fill_value,
  			       NULL,
  			       &tb_time_valid_range,
  			       CETB_PACK,
  			       (float) CETB_NCATTS_TB_TIME_SCALE_FACTOR,
  			       (float) CETB_NCATTS_TB_TIME_ADD_OFFSET,
  			       "gregorian" ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD Tb time (P) to file %s.\n",
	     __FILE__, cetb_grd->filename );
  } else {
    fprintf( stderr, "> %s: Wrote GRD Tb time (P) to file %s.\n",
	     __FILE__, cetb_grd->filename );
  }

  if ( 0 != cetb_file_add_grd_parameters( cetb_grd, median_flag ) ) {
    fprintf( stderr, "%s: Error adding GRD parameters to %s.\n", __FILE__,
	     cetb_grd->filename );
    exit( -1 );
  }
  if ( 0 != cetb_file_add_filenames( cetb_grd, input_file_total,
				     list_of_input_files ) ) {
    fprintf( stderr, "%s: Error adding list of files to %s.\n", __FILE__,
	     cetb_grd->filename );
    exit( -1 );
  }
  if ( 0 != cetb_file_add_TB_parameters( cetb_grd, rthreshold, box_size_km,
					 ltod_morning, ltod_evening ) ) {
    fprintf( stderr, "%s: Error adding TB parameters to %s.\n", __FILE__,
	     cetb_grd->filename );
    exit( -1 );
  }
  strcpy( cetb_grd_filename, cetb_grd->filename );
  cetb_file_close( cetb_grd );

  /* Now check to make sure that there are no OOR temps and set to MISSING if so */
  if ( 0 != cetb_file_check_consistency( cetb_grd_filename ) ) {
    fprintf( stderr, "%s: Error running file consistency check file %s\n",
	     __FILE__, cetb_grd_filename );
    exit( -1 );
  }

  if ( 0 != cetb_file_add_sir_parameters( cetb_sir, nits, median_flag ) ) {
    fprintf( stderr, "%s: Error adding SIR parameters to %s.\n", __FILE__,
	     cetb_sir->filename );
    exit( -1 );
  }
  if ( 0 != cetb_file_add_filenames( cetb_sir, input_file_total,
				     list_of_input_files ) ) {
    fprintf( stderr, "%s: Error adding input file names to %s.\n", __FILE__, 
	     cetb_sir->filename );
    exit( -1 );
  }
  if ( 0 != cetb_file_add_TB_parameters( cetb_sir, rthreshold, box_size_km,
					 ltod_morning, ltod_evening ) ) {
    fprintf( stderr, "%s: Error adding TB parameters to %s.\n", __FILE__,
	     cetb_sir->filename );
    exit( -1 );
  }
  strcpy( cetb_sir_filename, cetb_sir->filename );
  
  cetb_file_close( cetb_sir );

  /* Now check to make sure that there are no OOR temps and set to MISSING if so */
  if ( 0 != cetb_file_check_consistency( cetb_sir_filename ) ) {
    fprintf( stderr, "%s: Error running file consistency check file %s\n", __FILE__,
	     cetb_sir_filename );
    exit( -1 );
  }

  if (errors == 0) {
    fprintf( stderr, "%s: Processing successfully completed for %s\n", __FILE__,
	     cetb_sir_filename );
  } else {
    fprintf( stderr, "%s: Processing errors encountered in %s and azang set to %f\n",
	     __FILE__, cetb_sir_filename, azang );
  }
  
  /* end of program */
  /* free malloc'ed memory (not strictly necessary, but good to be explicit) */
  free(space);
  free(a_val);
  free(b_val);
  free(a_temp);
  free(sxy);
  free(sx);
  free(sx2);
  free(sy);
  free(tot);
  free(num_samples);

  return(errors);
}

/*
 * Function to get tbval, ang and count from store array - this happens 4 times in the code
 */

void get_vars_from_store( char *store, float *tbval, float *ang, int *count,
			  float tb_or_stokes_offset, int *ktime, int *iadd,
			  int hasazang, float *azang )
{

  *tbval = *((float *) (store+0));
  *ang   = *((float *) (store+4));
  *count = *((int *)   (store+8));
  *ktime = *((int *)  (store+12));
  *iadd  = *((int *)  (store+16));
  if ( hasazang )
    *azang = *((float *) (store+20));
  *tbval = *tbval - tb_or_stokes_offset;

}

/* SIR algorithm update step */

void get_updates(float tbval, int count, int fill_array[], short int response_array[], int its)
{
  float total = 0.0, num=0.0;
  float ave, scale, update;
  int i, n, m;

  /* compute back projection */ 
  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    num=num+m;
    total=total + m * *(a_val+n-1);
  }
  if (num > 0)
    ave=total/num;
  else
    return;
  if (fabs(ave) <= FLT_EPSILON) return;

  /* for each measurement hitting a pixel calculate updates */

  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    scale = tbval / ave;
    if (scale > 0.0)       /* damp scale factor */
      scale=(float) sqrt((double) scale);
    else                   /* unless scale is negative */
      scale=1.0;
    if (scale > 1.0)       /* constraining function */
      update = (float)(1.0/((0.5/ave)*(1.0-1.0/scale)+1.0/(*(a_val+n-1) * scale)));
    else
      update = (float)(0.5 * ave * (1.0 - scale) + *(a_val+n-1) * scale);

    (*(tot+n-1)) = (*(tot+n-1)) + m;
    *(a_temp+n-1) = (*(a_temp+n-1) * ( *(tot+n-1) - m) + update * m) / *(tot+n-1);
    
    /*
     * Count the number of measurements that hit a given pixel
     * Because this variable is stored as a byte, we only increment up to NC_MAX_CHAR
     */
    if (its == 0) {
      if ( *(num_samples+n-1) < CETB_NCATTS_TB_NUM_SAMPLES_MAX ) {
	(*(num_samples+n-1))++;
      }
    }
    
  }
  return;
}

/* compute contribution of measurement to AVE image */ 

void compute_ave(float tbval, float ang, int count, int fill_array[],
		 short int response_array[])
{
  int i, n, m;

  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    *(b_val+n-1) = *(b_val+n-1) + m * tbval;
    *(sy+n-1) = *(sy+n-1) + m;
    *(sx+n-1) = *(sx+n-1) + ang * m;    
    *(sx2+n-1) = *(sx2+n-1) + ang * ang * m;
  }
  return;
}

/* modified median, circular median, or smoothing filter routine */

static float median(float *array, int count);
static float cmedian(float *array, int count, float center);

void filter(float *val, int size, int mode, int nsx, int nsy, 
	    float *temp, float thres)
{
  float array[100], total;
  int i,j,x,y,size2,count,x1,x2,y1,y2;
  
  if ((size % 2) == 0) size=size+1;   /* force window size odd */
  size2 = (size-1)/2;                 /* window center */

  for (x=1; x<= nsx; x++)
    for (y=1; y<= nsy; y++) {
      count=0;
      y1=max(y-size2,1);
      y2=min(y+size2,nsy);
      x1=max(x-size2,1);
      x2=min(x+size2,nsx);
      for (i=x1; i <= x2; i++)
	for (j=y1; j <= y2; j++)
	  if (*(val+i+(j-1)*nsx-1) > thres) {
	    array[count]=*(val+i+(j-1)*nsx-1);
	    count++;
	  };
      
      if (count > 0 && *(val+x+(y-1)*nsx-1) > thres) {
	if (mode == 0)  /* Mode 0 = median */
	  *(temp+(y-1)*nsx+x-1) = median(array,count);
	else if (mode == 2) /* Mode 2 = circular median */
	  *(temp+(y-1)*nsx+x-1) = cmedian(array,count,*(val+x+(y-1)*nsx-1));
	else {		    /* Mode 1 = average */
	  total=0.0;
	  for (i=0; i<count; i++)
	    total=total+array[i];
	  *(temp+(y-1)*nsx+x-1) = total/count;
	}
      } else
	*(temp+(y-1)*nsx+x-1) = thres;
    };
  
  for (i=0; i < nsx*nsy; i++) {
    *(val+i) = *(temp+i);
    *(temp+i)=0.0;
  }
  return;
}

/* compute circular median of an array of directions (0..360)
   i.e., find the angle which has the smallest average distance
   from the others */

float cmedian(float array[], int count, float center)
{
  int i,j,k;
  float temp,sum;

  if (count < 3)
    return(center);
  
  temp = 1.e25;
  k=-1;
  for (i=0; i < count; i++) {
    sum = 0.0;
    for (j=0; j < count; j++)
      if (i != j)
	sum=sum+(180-abs(180-abs(array[i]-array[j])));
    if (sum < temp) {
      temp = sum;
      k = i;
    }
  }
  if (k > -1)
    return(array[k]);
  else
    return(center);
}

/* compute modified median of an array of values */

float median(float array[], int count)
{
  int i,j;
  float temp;

  for (i=count; i >= 2; i--)
    for (j=1; j <= i-1; j++)
      if (array[i-1] < array[i-j-1]) {
	temp=array[i-1];
	array[i-1]=array[i-j-1];
	array[i-j-1]=temp;
      };
  temp=array[count/2];

  if (array[count-2]-array[1] < 0.25 && count > 5) {
    temp=0.0;
    for (i=count/2-1; i <= count/2+3; i++)
      temp=temp+array[i-1];
    temp=temp/5;
  }
  return(temp);
}

/* routine to compute the spatial response function weighted variance and error from measurements */

void stat_updates(float tbval, int count, int fill_array[],
		  short int response_array[])
{
  float ave, sigv;
  float total=0.0, num=0.0;
  int i, n, m;
  
  /* compute forward projection of measurement */
  
  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    sigv = *(a_val+n-1);
    total = total + m * sigv;
    num = num + m;
  }
  if (abs(num) < FLT_EPSILON) return;
  ave =(total/num);
  
  for (i=0; i < count; i++) {
    n=fill_array[i];
    m=response_array[i];
    *(tot+n-1) += m;
    sigv = (tbval - ave);      /* difference */

    *(sx+n-1) += m * sigv; 
    *(sy+n-1) += m * sigv * sigv;
  } 

  return;
}

/* routine to compute time estimates from measurements */

void time_updates(float ktime, int count, int fill_array[])
{
  int i, n;
  
  for (i=0; i < count; i++) {
    n=fill_array[i];
    *(tot+n-1) = *(tot+n-1) + 1;
    *(sx+n-1) = *(sx+n-1) + ktime;
    *(sy+n-1) = *(sy+n-1) + 1;
  }
  return;
}

