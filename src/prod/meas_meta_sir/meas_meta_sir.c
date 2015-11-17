/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_meta_sir.c    MEaSURES project

  program to generate standard SIR products from .setup file

  Written by DGL at BYU 02/22/2014 + modified from ssmi_meta_sir3.c
  Revised by DGL at BYU 05/15/2014 + use intermediate dump file output
  Revised by DGL at BYU 06/21/2014 + AVE start of SIR
  Revised by DGL at BYU 08/16/2014 + revised error handling, optional INFOfile out

******************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef JANUSicc
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <string.h>
#include <time.h>
#include <netcdf.h>

#include "utils.h"
#include "cetb_file.h"
#include "ezdump.h"
#include "sir3.h"

#define VERSION 1.2

#define file_savings 1.00     /* measurement file savings ratio */
#define REL_EOF   2           /* fseek relative to end of file */
#define REL_BEGIN 0           /* fseek relative to end of file */

#define CREATE_NON 1          /* set to 1 to create NON images, 0 to not create */

#define INFOFILE 1            /* define to create info file, undefine to not create */
#undef INFOFILE               /* do not create info file */

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


/* some error print shortcuts 
   errors output to stderr */

void eprintf(char *s)
{ 
  fprintf(stderr,"%s",s);
  fflush(stderr);
  return;
}

void eprintfi(char *s, int a)
{
  fprintf(stderr,s,a);
  fflush(stderr);
  return;
}

void eprintfc(char *s, char *a)
{
  fprintf(stderr,s,a);
  fflush(stderr);
  return;
}

void Ferror(int i)
{
  fprintf(stderr,"*** Error reading input file at %d ***\n",i);
  fflush(stderr);
  return;
}


/* function prototypes */

int get_measurements(char *store, char *store2, float *tbval, float *ang, int *count,
		     int *ktime, int *iadd, int *nrec);

void get_updates(float tbval, float ang, int count, int *fill_array,
		 short int *response_array, int its );

void compute_ave(float tbval, float ang, int count, int *fill_array, short int *response_array);

void time_updates(float tbval, float ktime, float ant, int count,
		  int *fill_array, short int *response_array);

void stat_updates(float tbval, float ang, int count, int *fill_array,
		  short int *response_array);

void filter(float *val, int size, int opt, int nsx, int nsy, float
	    *temp, float thres);

void no_trailing_blanks(char *s);

char *addpath(char *outpath, char *name, char *temp);

/****************************************************************************/

/* global array variables used for storing images*/

float *a_val, *b_val, *a_temp, *sxy, *sx, *sx2, *sy, *tot;
unsigned char *num_samples;

/* other global variables */

FILE *imf, *omf;
long int nspace;

/****************************************************************************/

/* main program */

int main(int argc, char **argv)
{

  char *file_in;
  char outpath[FILENAME_MAX], tstr[350];

  float latl, lonl, lath, lonh;
  char regname[11], *s;
  int dumb, nrec, ncnt, i, j, n, ii, iii, nsize;
  long int nls, nbyte;
  float ratio, fn, ninv;
  char *space, *store, *store2;
  float tbval, ang, azang=0.0;
  int count, ktime, iadd, end_flag;
  char *x;
  int irecords;
  int non_size_x, non_size_y, nsx2, nsy2, ix, iy, nsize2;
  float xdeg2, ydeg2, ascale2, bscale2, a02, b02;

  /* define no-data values */
  float anodata_A=CETB_FILE_TB_FILL_VALUE;
  float anodata_C=CETB_FILE_TB_NUM_SAMPLES_FILL_VALUE;
  float anodata_I=CETB_FILE_THETA_FILL_VALUE;
  float anodata_Ia=CETB_FILE_THETA_FILL_VALUE;
  float anodata_P=CETB_FILE_TB_TIME_FILL_VALUE;
  float anodata_V=CETB_FILE_TB_STDDEV_FILL_VALUE;
  float anodata_E=-15.0;

  int Nfiles_out;  
  
  int nsx, nsy, iyear, isday, ismin, ieday, iemin;
  int iregion, ipol, iopt;
  char pol, sensor[41], crproc[101], crtime[29];
  float xdeg, ydeg, ascale, bscale, a0, b0;

  time_t tod;

  int its, irec, year, keep;  
  float total, tmax;
  float amin, amax, bmin, bmax, weight, temp, old_amin, old_amax;
  float old_bmin, old_bmax, denom;

  char a_name[100], b_name[100], c_name[100], p_name[100], 
    v_name[100], e_name[100], i_name[100], j_name[100],
    a_name_ave[100], b_name_ave[100], non_aname[100], grd_iname[100], grd_jname[100],
    non_vname[100], grd_aname[100], grd_bname[100], grd_vname[100], 
    grd_pname[100], grd_cname[100], 
    info_name[100], line[100];

  cetb_file_class *cetb_sir;
  cetb_file_class *cetb_grd;
  cetb_swath_producer_id swath_producer_id;
  cetb_platform_id platform_id;
  cetb_sensor_id sensor_id;
  cetb_direction_id direction_id=CETB_NO_DIRECTION;
  unsigned short tb_fill_value=CETB_FILE_TB_FILL_VALUE;
  unsigned short tb_missing_value=CETB_FILE_TB_MISSING_VALUE;
  unsigned short tb_valid_range[ 2 ] = { CETB_FILE_TB_MIN, CETB_FILE_TB_MAX };
  short tb_time_fill_value=CETB_FILE_TB_TIME_FILL_VALUE;
  short tb_time_valid_range[ 2 ] = { CETB_FILE_TB_TIME_MIN, CETB_FILE_TB_TIME_MAX };
  unsigned short tb_stddev_fill_value=CETB_FILE_TB_STDDEV_FILL_VALUE;
  unsigned short tb_stddev_missing_value=CETB_FILE_TB_STDDEV_MISSING_VALUE;
  unsigned short tb_stddev_valid_range[ 2 ] = { CETB_FILE_TB_STDDEV_MIN, CETB_FILE_TB_STDDEV_MAX };
  unsigned char tb_num_samples_fill_value=CETB_FILE_TB_NUM_SAMPLES_FILL_VALUE;
  unsigned char tb_num_samples_valid_range[ 2 ] = { CETB_FILE_TB_NUM_SAMPLES_MIN,
						    CETB_FILE_TB_NUM_SAMPLES_MAX };
  short theta_fill_value=CETB_FILE_THETA_FILL_VALUE;
  short theta_valid_range[ 2 ] = { CETB_FILE_THETA_MIN, CETB_FILE_THETA_MAX };
  float error_valid_range[ 2 ] = { 0.0, NC_MAX_FLOAT };
  int ncerr;

  int storage = 0;
  long head_len;
  int errors = 0, gerr;
  char polch;

  int median_flag = 0;  /* default: no median filter in SIRF algorithm */
  int ibeam = 0;

  char *error_msg;

  /*
   * Set output_debug=1 to get all output images
   * These files will be very large and should only be generated
   * for debug purposes.
   * We should eventually make this a command-line switch.
   */
  int output_debug = 0; 

  /* begin program */

  printf("BYU SSM/I meta SIR/SIRF program: C version %f\n",VERSION);

  if (argc < 2) {
    printf("\nusage: %s setup_in outpath storage_option\n\n",argv[0]);
    printf(" input parameters:\n");
    printf("   setup_in        = input setup file\n");
    printf("   outpath         = output path\n");
    printf("   storage_option  = (0=mem only [def], 1=file only, 2=mem then file\n");
    return(0);
  }
  file_in=argv[1];

  imf = fopen(file_in,"r"); 
  if (imf == NULL) {
     eprintfc("ERROR: cannot open input setup file: %s\n",argv[1]); 
     exit(-1);
  }

  strcpy(outpath,"./"); /* default output path */
  if (argc > 2) 
    sscanf(argv[2],"%s",outpath);
  printf("Output path %s: ",outpath);

  if (argc > 3) 
    sscanf(argv[3],"%d",&storage);
  printf("Storage option %d: ",storage);
  if (storage == 1)
    printf(" File only\n");
  else if (storage == 2)
    printf(" Memory then File\n");
  else
    printf(" Memory only\n");
  
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
   printf("\nInput file header info: '%s'\n",file_in);
   printf("  Year, day range: %d %d - %d\n",iyear,isday,ieday);
   printf("  Image size: %d x %d = %d   Projection: %d\n",nsx,nsy,nsx*nsy,iopt);
   printf("  Origin: %f,%f  Span: %f,%f\n",a0,b0,xdeg,ydeg);
   printf("  Scales: %f,%f  Pol: %d  Reg: %d\n",ascale,bscale,ipol,iregion);
   printf("  Region: '%s'   Records: %d\n",regname,irecords);
   printf("  Corners: LL %f,%f UR %f,%f\n",latl,lonl,lath,lonh);
   printf("  Grid size: %d x %d = %d  Scales %d %d\n",nsx2,nsy2,nsx2*nsy2,non_size_x,non_size_y);
   printf("  Grid Origin: %f,%f  Grid Span: %f,%f\n",a02,b02,xdeg2,ydeg2);
   printf("  Grid Scales: %f,%f\n",ascale2,bscale2);
   printf("\n");

   /* read output file names and misc variables

     In principle, these can be in variable order.  However, if standard
     program is used, order and number is known.  Most variables are info
     only, but some control the internal operations.
   */

   end_flag = 0;
   do {
     
     if (fread(&dumb,   sizeof(int),   1, imf) == 0) Ferror(70);
     if (fread(line,   sizeof(char), 100, imf) == 0) Ferror(71);
     if (fread(&dumb,   sizeof(int),   1, imf) == 0) Ferror(72);
     /* printf("line read '%s'\n",line); */

     if (strstr(line,"A_initialization") != NULL) {
       x = strchr(line,'=');
       a_init=atof(++x);
       printf("A_initialization of %f\n",a_init);
     }

     if (strstr(line,"Beam_code") != NULL) {
       x = strchr(line,'=');
       ibeam=atoi(++x);
       printf("Beam code %d\n",ibeam);
     }

     if (strstr(line,"Max_iterations") != NULL) {
       x = strchr(line,'=');
       nits=atoi(++x);
       printf("Max iterations of %d\n",nits);
     }

     if (strstr(line,"Max_Fill") != NULL) {
       x = strchr(line,'=');
       MAXFILL=atoi(++x);
       printf("Max fill %d\n",MAXFILL);
     }

     if ( strstr( line, " Producer_id" ) != NULL ) {
       x = strchr( line,'=' );
       swath_producer_id = ( cetb_swath_producer_id )atoi(++x);
       printf( "Producer_id %s\n", cetb_swath_producer_id_name[ swath_producer_id ] );
     }

     if ( strstr( line, " Platform_id" ) != NULL ) {
       x = strchr( line,'=' );
       platform_id = ( cetb_platform_id )atoi(++x);
       printf( "Platform_id %s\n", cetb_platform_id_name[ platform_id ] );
     }

     if ( strstr( line, " Sensor_id" ) != NULL ) {
       x = strchr( line,'=' );
       sensor_id = ( cetb_sensor_id )atoi(++x);
       printf( "Sensor_id %s\n", cetb_sensor_id_name[ sensor_id ] );
     }

     if ( strstr( line, " Pass_direction" ) != NULL ) {
       x = strchr( line,'=' );
       direction_id = ( cetb_direction_id )atoi(++x);
       printf( "Direction_id %s\n", cetb_direction_id_name[ direction_id ] );
     }

     if (strstr(line,"Response_Multiplier") != NULL) {
       x = strchr(line,'=');
     }

     if (strstr(line,"Sensor") != NULL) {
       x = strchr(line,'=');
       strncpy(sensor_in,++x,40);
       no_trailing_blanks(sensor_in);
       printf("Sensor '%s'\n",sensor_in);
     }

     if ((x = strchr(line+4,' ')) != NULL) *x='\0'; /* truncate off any trailing spaces */

     if (strstr(line,"Median_flag") != NULL) {
       x = strchr(line,'=')+1;
       if (strstr(x,"T") != NULL || strstr(x,"t") != NULL)
	 median_flag=1;
       if (strstr(x,"F") != NULL || strstr(x,"f") != NULL)
	 median_flag=0;
       printf("Median flag: %d\n",median_flag);       
     }

     if (strstr(line,"Has_Azimuth_Angle") != NULL) {
       x = strchr(line,'=')+1;
       if (strstr(x,"T") != NULL || strstr(x,"t") != NULL) {
	 HASAZANG=1;
	 HS += 4;  /* increase read buffer size */
       }
       if (strstr(x,"F") != NULL || strstr(x,"f") != NULL)
	 HASAZANG=0;
       printf("Has azimuth angle: %d\n",HASAZANG);       
     }

     if (strstr(line,"SIRF_A_file") != NULL) {
       x = strchr(line,'=');
       strncpy(a_name,++x,100);
       no_trailing_blanks(a_name);
     }
     if (strstr(line,"SIRF_C_file") != NULL) {
       x = strchr(line,'=');
       strncpy(c_name,++x,100);
       no_trailing_blanks(c_name);
     }
     if (strstr(line,"SIRF_I_file") != NULL) {
       x = strchr(line,'=');
       strncpy(i_name,++x,100);
       no_trailing_blanks(i_name);
     }
     if (strstr(line,"SIRF_J_file") != NULL) {
       x = strchr(line,'=');
       strncpy(j_name,++x,100);
       no_trailing_blanks(j_name);
       }
     if (strstr(line,"SIRF_E_file") != NULL) {
       x = strchr(line,'=');
       strncpy(e_name,++x,100);
       no_trailing_blanks(e_name);
     }
     if (strstr(line,"SIRF_V_file") != NULL) {
       x = strchr(line,'=');
       strncpy(v_name,++x,100);
       no_trailing_blanks(v_name);
     }
     if (strstr(line,"SIRF_P_file") != NULL) {
       x = strchr(line,'=');
       strncpy(p_name,++x,100);
       no_trailing_blanks(p_name);
     }
     if (strstr(line,"AVE_A_file") != NULL) {
       x = strchr(line,'=');
       strncpy(a_name_ave,++x,100);
       no_trailing_blanks(a_name_ave);
     }
     if (strstr(line,"NON_A_file") != NULL) {
       x = strchr(line,'=');
       strncpy(non_aname,++x,100);
       no_trailing_blanks(non_aname);
     }
     if (strstr(line,"NON_V_file") != NULL) {
       x = strchr(line,'=');
       strncpy(non_vname,++x,100);
       no_trailing_blanks(non_vname);
     }
     if (strstr(line,"GRD_A_file") != NULL) {
       x = strchr(line,'=');
       strncpy(grd_aname,++x,100);
       no_trailing_blanks(grd_aname);
     }
     if (strstr(line,"GRD_V_file") != NULL) {
       x = strchr(line,'=');
       strncpy(grd_vname,++x,100);
       no_trailing_blanks(grd_vname);
     }
     if (strstr(line,"GRD_I_file") != NULL) {
       x = strchr(line,'=');
       strncpy(grd_iname,++x,100);
       no_trailing_blanks(grd_iname);
     }
     if (strstr(line,"GRD_J_file") != NULL) {
       x = strchr(line,'=');
       strncpy(grd_jname,++x,100);
       no_trailing_blanks(grd_jname);
     }
     if (strstr(line,"GRD_P_file") != NULL) {
       x = strchr(line,'=');
       strncpy(grd_pname,++x,100);
       no_trailing_blanks(grd_pname);
       }
     if (strstr(line,"GRD_C_file") != NULL) {
       x = strchr(line,'=');
       strncpy(grd_cname,++x,100);
       no_trailing_blanks(grd_cname);
     }
     if (strstr(line,"Info_file") != NULL) {
       x = strchr(line,'=');
       strncpy(info_name,++x,100);
       no_trailing_blanks(info_name);
     }
      
     if (strstr(line,"End_header") != NULL) {
       end_flag = 1;
     }

   } while (end_flag == 0);

   printf("\n");
   printf("A output file: '%s'\n",a_name);
   printf("I output file: '%s'\n",i_name);
   printf("J output file: '%s'\n",j_name);
   printf("C output file: '%s'\n",c_name);
   printf("P output file: '%s'\n",p_name);
   printf("E output file: '%s'\n",e_name);
   printf("SIR V output file: '%s'\n",v_name);
   printf("AVE A output file: '%s'\n",a_name_ave);
   if (CREATE_NON) {
     printf("NON A output file: '%s'\n",non_aname);
     printf("NON V output file: '%s'\n",non_vname);
     Nfiles_out=16;
   } else {
     Nfiles_out=14;
   }
   printf("GRD A output file: '%s'\n",grd_aname);
   printf("GRD V output file: '%s'\n",grd_vname);
   printf("GRD I output file: '%s'\n",grd_iname);
   printf("GRD J output file: '%s'\n",grd_jname);
   printf("GRD P output file: '%s'\n",grd_pname);
   printf("GRD C output file: '%s'\n",grd_cname);
   printf("Info file: '%s'\n",info_name);
   printf("\n");

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
    * and we need to stop specifying any output filenames in the .meta file
    *
    * Initialize 2 cetb_files one for SIR output, the other for GRD output.
    */
   if ( CETB_NO_DIRECTION == direction_id )
     direction_id = (cetb_direction_id)cetb_get_direction_id_from_info_name( info_name );
   cetb_sir = cetb_file_init( outpath,
			      iregion, ascale, platform_id, sensor_id,
			      iyear, isday, ibeam,
			      direction_id,
			      CETB_SIR,
			      swath_producer_id );
   if ( !cetb_sir ) {
     fprintf( stderr, "%s: Error initializing cetb_file for %s.\n",
	      __FILE__, cetb_reconstruction_id_name[ CETB_SIR ] );
     exit( -1 );
   }

   if ( 0 != cetb_file_open( cetb_sir ) ) {
     fprintf( stderr, "%s: Error opening cetb_file=%s.\n", __FILE__, cetb_sir->filename );
     exit( -1 );
   }
     
   cetb_grd = cetb_file_init( outpath,
			      iregion, CETB_MIN_RESOLUTION_FACTOR, platform_id, sensor_id,
			      iyear, isday, ibeam,
			      direction_id,
			      CETB_GRD,
			      swath_producer_id );
   if ( !cetb_grd ) {
     fprintf( stderr, "%s: Error initializing cetb_file for %s.\n",
	      __FILE__, cetb_reconstruction_id_name[ CETB_GRD ] );
     exit( -1 );
   }

   if ( 0 != cetb_file_open( cetb_grd ) ) {
     fprintf( stderr, "%s: Error opening cetb_file=%s.\n", __FILE__, cetb_grd->filename );
     exit( -1 );
   }
     
   head_len = ftell(imf);
   printf("Input header file length %ld\n",head_len);
   nls=nls-head_len;

/* header read completed, now determine how much program memory to allocate */

  if (storage != 1) { /* allocate memory storage space for measurements */
    nspace = nls * file_savings;/* space to allocate for measurement storage */
    printf("  File size: %ld  Space allocated: %ld\n",nls,nspace);
    if ( 0 != utils_allocate_clean_aligned_memory( ( void ** )&space, ( size_t )nspace*sizeof(char)) ) {
      eprintf("*** Inadequate memory for data file storage\n");
      exit(-1);
    }
  }

/* allocate storage space for image and working arrays
   note: these arrays are re-used multiple times to save memory */

  nsize = nsx * nsy;  
  //  a_val  = (float *) malloc(sizeof(float)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&a_val, (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for a_val\n", __FILE__ );
    exit(-1);
  }
  //  b_val  = (float *) malloc(sizeof(float)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&b_val, (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for b_val\n", __FILE__ );
    exit(-1);
  }
  //  a_temp = (float *) malloc(sizeof(float)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&a_temp, (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for a_temp\n", __FILE__ );
    exit(-1);
  }
  //  sxy    = (float *) malloc(sizeof(float)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&sxy, (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for sxy\n", __FILE__ );
    exit(-1);
  }
  //  sx     = (float *) malloc(sizeof(float)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&sx, (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for sx\n", __FILE__ );
    exit(-1);
  }
  //  sx2    = (float *) malloc(sizeof(float)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&sx2, (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for sx2\n", __FILE__ );
    exit(-1);
  }
  //  sy     = (float *) malloc(sizeof(float)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&sy, (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for sy\n", __FILE__ );
    exit(-1);
  }
  //tot    = (float *) malloc(sizeof(float)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&tot, (size_t)(sizeof(float)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for tot\n", __FILE__ );
    exit(-1);
  }
  //  num_samples = (unsigned char *) calloc( 1, sizeof(unsigned char)*nsize);
  if ( 0 != utils_allocate_clean_aligned_memory( (void**)&num_samples, (size_t)(sizeof(unsigned char)*nsize) ) ) {
    fprintf( stderr, "%s: inadequate memory for num_samples\n", __FILE__ );
    exit(-1);
  }

  /* with storage allocated, copy file into memory if selected */

  if (storage != 1) {   /* read measurement file into memory, 
			   storing only essential information  */

    nrec = 0;         /* number of meaurements in file */
    ncnt = 0;         /* number of useable measurements */
    nbyte = 0;        /* file size in bytes */
    store=space;      /* storage pointer */
  
    printf("Begin setup file copy into memory\n");
    while (fread(&dumb, sizeof(int), 1, imf) != 0) {

    /*	   read (50,err=500,end=500) tbval,ang,count,ktime,iadd,azi
	   read (50,err=500,end=500) (fill_array(i),i=1,count)
	   read (50,err=500,end=500) (response_array(i),i=1,count)
    */

     /*5 items at 4 bytes each: 20 bytes if no azimuth angle */
     /*6 items at 4 bytes each: 24 bytes if azimuth angle */
     if (nbyte+HS < nspace) {
       	if ((dumb=fread(store, sizeof(char), HS, imf)) != HS) {
          eprintfi(" *** Error reading input file data at 180 %d\n", dumb);
	  exit(-1);
        }
        if (fread(&dumb,sizeof(int), 1, imf) == 0) Ferror(100);

        tbval = *((float *) (store+0));
        ang   = *((float *) (store+4));
        count = *((int *)   (store+8));
        ktime = *((int *)   (store+12));
        iadd  = *((int *)   (store+16));
	if (HASAZANG)
	  azang = *((float *) (store+20));

	//printf(" Record %d : %f %f %d %d %d %f\n",nrec,tbval,ang,count,ktime,iadd,azang);

	if (count > MAXFILL) {
	  printf("*** Count error %d  record %d\n",count,nrec);
	  printf("    %f %f %d %d \n",tbval,ang,ktime,iadd);
	  count=MAXFILL;
	}
	/*
	printf("ncnt %d  %f %f  count %d %d %d\n",ncnt,tbval,ang,count,ktime,iadd); 
	if (ncnt > 99) goto label; */

	/* if measurement is "valid" keep it by indexing counters 
           if not, new values will be stored over old values */

	keep=0;
	if (tbval < 350.0 && tbval > 50.0) { 
	  nbyte=nbyte+HS;
	  store=store+HS;
	  ncnt++;
	  keep=1;
	}

	/* read fill_array pixel indices */
	if (nbyte+count*4 < nspace) {
	   if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(110);
	   if (fread(store, sizeof(int), count, imf) != count) {
	      eprintf(" *** Error reading input file data at 111\n");
	      /* exit(-1); */
	      goto label;
	   }
	   if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(112);
	   if (keep == 1) {
	     nbyte=nbyte+count*4;
	     store=store+count*4;
	   }
	} else {
	   eprintfi(" *** out of storage space 1 *** %d\n", ncnt);
	   printf(" *** out of storage space 1 *** %d %ld %ld\n",ncnt,nbyte,nspace);
	   exit(-1);
	}

	/* read response_array weights */
	if (nbyte+count*2 < nspace) {
	   if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(1111);
	   if (fread(store, sizeof(short int), count, imf) != count) {
	      eprintf(" *** Error reading input file data at 1111\n");
	      goto label;
	   }
	   if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(1121);
	   if (keep == 1) {
	     nbyte=nbyte+count*2;
	     if (count % 2 == 1) count=count+1;  /* ensure storage of next record on a 4byte word boundary */
	     store=store+count*2;
	   }
	} else {
	   eprintfi(" *** out of storage space 2 *** %d\n", ncnt);
	   printf(" *** out of storage space 2 *** %d %ld %ld\n",ncnt,nbyte,nspace);
	   exit(-1);
	}

	nrec++;

     } else {
       eprintfi(" *** out of storage space 3 *** %d %ld\n", ncnt);
       printf(" *** out of storage space 3 *** %d %ld\n",ncnt,nspace);
       exit(-1);
     }
    }
  label:
    fclose(imf);


/* print measurement file storage requirements */

    ratio=100.0 * (float) nbyte / (float) nls;
    printf("  Input file read into ram\n");
    printf("  Total storage used: %d %d recs = %ld of %ld (%.1f%% %.1f%%)\n",
	   nrec,ncnt,nbyte,nspace,ratio,100.0*file_savings);
  }


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
  

  if (storage == 1) {  /* for file storage */
    ncnt = 2 * nls * file_savings / HS;  /* high estimate of # of measurements*/
    if (ncnt < 0) ncnt=900000000;
    nrec = 0;         /* will hold actual number of meaurements in file */
  }


  printf("\nSIR parameters: A_init=%f  N=%d\n",a_init,nits);

  /* for each iteration of SIR */

  for (its=0; its < nits; its++) {

    printf("\nSIR iteration %d %d\n",its+1,ncnt);

    if (storage == 1) {  /* file storage: rewind file and skip head */
      fseek(imf, head_len, REL_BEGIN);
    }

    /* for each measurement, accumulate results */

    store=space;
    for (irec = 0; irec < ncnt; irec++) {
    
      if (storage == 1) { /* get measurement info from file */
	store=space;
	gerr=get_measurements(store, store2, &tbval, &ang, &count, &ktime, &iadd, &nrec);
	if (gerr < 0) exit(-1);  /* fatal error */
	if (gerr == 1) {         /* finished with all measurements */
	  if (irec == 0) ncnt = nrec;  /* set count value */
	  goto done;
	}

      } else {  /* get measurement info from memory */
	tbval = *((float *) (store+0));
	ang   = *((float *) (store+4));
	count = *((int *)   (store+8));
	if (its == 0) iadd = *((int *) (store+16));
	if (HASAZANG)
	  azang = *((float *) (store+20));	

	store = store+HS;
	store2 = store + 4*count;
      }

      /* printf("%d %f %f %d\n",irec,tbval,ang,count);
            for (j=0;j<count;j++) printf("%d ",*((int *)(store+4*j)));
      printf("\n");
      */

      get_updates(tbval, ang, count, (int *) store, (short int *) store2, its);

      /* compute AVE image during first iteration */
      if (its == 0) 
	compute_ave(tbval, ang, count,(int *) store, (short int *) store2);

      store = store+4*count;
      store = store+2*count;
      if (count % 2 == 1) store=store+2;  /* ensure word boundary */
    }

done:
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
	      *(sx2+i) = sqrt((double) *(sx2+i));
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

    if (its == 0) printf(" Average weight: %.4f\n",total/nsize);
    printf(" A min max  --> %f %f %d\n",amin,amax,its+1);
    printf(" A change   --> %f %f\n",amin-old_amin,amax-old_amax);

    old_amin=amin;
    old_amax=amax;

    if (median_flag == 1)   /* apply modified median filtering */
      filter(a_val, 3, 0, nsx, nsy, a_temp, anodata_A);  /* 3x3 modified median filter */

    if (its == 0) {  /* output num_samples and AVE image */
      
      if ( output_debug ) {
	
	if ( 0 != cetb_file_add_var( cetb_sir, "TB_ave_image",
				     NC_USHORT, b_val,
				     ( size_t )nsx, ( size_t )nsy,
				     CETB_FILE_TB_STANDARD_NAME,
				     "SIR TB ave image",
				     CETB_FILE_TB_UNIT,
				     &tb_fill_value,
				     &tb_missing_value,
				     &tb_valid_range,
				     CETB_PACK,
				     (float) CETB_FILE_TB_SCALE_FACTOR,
				     (float) CETB_FILE_TB_ADD_OFFSET,
				     NULL ) ) {
	  errors++;
	  fprintf( stderr, "%s: Error writing Tb (ave_image).\n", __FILE__ );
	} else {
	  fprintf( stderr, "> %s: Wrote Tb (ave_image) to %s.\n", __FILE__, cetb_sir->filename );
	}
    
      }
      
    }

  }    /* end of loop for each SIR iteration */
  printf(" weight max --> %f Average weight: %.4f\n",tmax,total/nsize);

  if ( 0 != cetb_file_add_var( cetb_sir, "TB",
			       NC_USHORT, a_val,
			       ( size_t )nsx, ( size_t ) nsy,
			       CETB_FILE_TB_STANDARD_NAME,
			       "SIR TB",
			       CETB_FILE_TB_UNIT,
			       &tb_fill_value,
			       &tb_missing_value,
			       &tb_valid_range,
			       CETB_PACK,
			       (float) CETB_FILE_TB_SCALE_FACTOR,
			       (float) CETB_FILE_TB_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb (A).\n", __FILE__ );
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
    fprintf( stderr, "%s: Error writing Tb num_samples.\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote Tb num_samples to %s.\n", __FILE__, cetb_sir->filename );
  }

  if ( output_debug ) {
    /* output other auxilary product images */
    if ( 0 != cetb_file_add_var( cetb_sir, "i_image",
				 NC_SHORT, sx2,
				 ( size_t )nsx, ( size_t )nsy,
				 NULL,
				 "SIR Incidence Angle stddev",
				 CETB_FILE_ANGULAR_UNIT,
				 &theta_fill_value,
				 NULL,
				 &theta_valid_range,
				 CETB_PACK,
				 (float) CETB_FILE_THETA_SCALE_FACTOR,
				 (float) CETB_FILE_THETA_ADD_OFFSET,
				 NULL ) ) {
      errors++;
      fprintf( stderr, "%s: Error writing Tb (i_image).\n", __FILE__ );
    } else {
      fprintf( stderr, "> %s: Wrote Tb (i_image) to %s.\n", __FILE__, cetb_sir->filename );
    }
    
  }

  if ( 0 != cetb_file_add_var( cetb_sir, "Incidence_angle",
			       NC_SHORT, sx,
			       ( size_t )nsx, ( size_t ) nsy,
			       NULL,
			       "SIR Incidence Angle",
			       CETB_FILE_ANGULAR_UNIT,
			       &theta_fill_value,
			       NULL,
			       &theta_valid_range,
			       CETB_PACK,
			       (float) CETB_FILE_THETA_SCALE_FACTOR,
			       (float) CETB_FILE_THETA_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb incidence angle (j_image).\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote Tb incidence angle (j_image) to %s.\n", __FILE__, cetb_sir->filename );
  }

/* create STD and Err images */

  printf("\nBegin creation of STD images\n");  

  /* initialize arrays */

  for (i=0; i < nsize; i++) {
    *(a_temp+i) = 0.0;
    *(sx+i)  = 0.0;
    *(sy+i)  = 0.0;
    *(sxy+i) = 0.0;
    *(sx2+i) = 0.0;
    *(tot+i) = 0.0;
  }

  if (storage == 1) {  /* file storage: rewind file and skip head */
    fseek(imf, head_len, REL_BEGIN);
  }

  store=space;
  for (irec = 0; irec < ncnt; irec++) {

    if (storage == 1) { /* get measurement info from file */

      store=space;
      gerr=get_measurements(store, store2, &tbval, &ang, &count, &ktime, &iadd, &nrec);      
      if (gerr < 0) exit(-1);  /* fatal error */
      if (gerr==1) goto done1; /* finished with all measurements */
      
    } else {  /* get measurement info from memory */

      tbval = *((float *) (store+0));
      ang   = *((float *) (store+4));
      count = *((int *)   (store+8));
      if (HASAZANG)
	azang = *((float *) (store+20));
      
      store = store+HS;
      store2 = store+4*count;
    }

    stat_updates(tbval, ang, count, (int *) store, (short int *) store2);
    store = store+4*count;
    store = store+2*count;
    if (count % 2 == 1) store=store+2;  /* ensure word boundary */
  }
done1:

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
	  *(sxy+i) = sqrt( (double) *(sxy+i));
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
    
  printf(" Tb STD min   max --> %f %f\n",amin,amax);
  printf(" Tb ERR min   max --> %f %f\n",bmin,bmax);

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
			       (float) CETB_FILE_TB_STDDEV_SCALE_FACTOR,
			       (float) CETB_FILE_TB_STDDEV_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb stddev (V).\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote Tb stddev (V) to %s.\n", __FILE__, cetb_sir->filename );
  }
    
  if ( output_debug ) {
    if ( 0 != cetb_file_add_var( cetb_sir, "e_image",
				 NC_FLOAT, sx,
				 ( size_t )nsx, ( size_t ) nsy,
				 NULL,
				 "SIR TB e_image",
				 "decibel",
				 &anodata_E,
				 NULL,
				 &error_valid_range,
				 CETB_NO_PACK,
				 0.0,
				 0.0,
				 NULL ) ) {
      errors++;
      fprintf( stderr, "%s: Error writing Tb err (e_image).\n", __FILE__ );
    } else {
      fprintf( stderr, "> %s: Wrote Tb err (e_image) to %s.\n", __FILE__, cetb_sir->filename );
    }
  }

/* create time image */

  printf("\nBegin creation of time image\n");  

  /* initialize arrays */

  for (i=0; i < nsize; i++) {
    *(a_temp+i) = 0.0;
    *(sx+i)  = 0.0;
    *(sy+i)  = 0.0;
    *(sxy+i) = 0.0;
    *(tot+i) = 0.0;
  }

  if (storage == 1) {  /* file storage: rewind file and skip head */
    fseek(imf, head_len, REL_BEGIN);
  }

  store=space;
  for (irec = 0; irec < ncnt; irec++) {

    if (storage == 1) { /* get measurement info from file */

      store=space;
      gerr=get_measurements(store, store2, &tbval, &ang, &count, &ktime, &iadd, &nrec);
      if (gerr < 0) exit(-1);  /* fatal error */
      if (gerr==1) goto done2; /* finished with all measurements */
      
    } else {  /* get measurement info from memory */

      tbval = *((float *) (store+0));
      ang   = *((float *) (store+4));
      count = *((int *)   (store+8));
      ktime = *((int *)   (store+12));
      if (ktime < 0) ktime = -ktime;
      if (HASAZANG)
	azang = *((float *) (store+20));
      
      store = store+HS;
      store2 = store+4*count;     
    }

    time_updates(tbval, (float) ktime, ang, count, (int *) store, (short int *) store2);
    store = store+4*count;
    store = store+2*count;
    if (count % 2 == 1) store=store+2;  /* ensure word boundary */
  }
done2:

  amin =  32000.0;            /* for user stats */
  amax = -32000.0;
  total = 0.0;
    
  for (i=0; i<nsize; i++){
    if (*(tot+i) > 0) {    /* update only hit pixels */
      total = total + *(tot+i);

      if (*(tot+i) > 1) 
	if (*(sy+i) != 0.0) 
	  *(sxy+i) = *(sx+i) / *(sy+i);
	else
	  *(sxy+i) =0.0;
      else
	*(sxy+i) = anodata_P;
		
      if (*(sxy+i) > 0.0)
	amin = min(amin, *(sxy+i));
      amax = max(amax, *(sxy+i));
	
    } else
      *(sxy+i) = anodata_P;
  }

  printf(" Time min   max --> %f %f\n",amin,amax);

  if ( 0 != cetb_file_add_var( cetb_sir, "TB_time",
  			       NC_SHORT, sxy,
  			       ( size_t )nsx, ( size_t ) nsy,
  			       NULL,
  			       "SIR TB Time of Day",
  			       cetb_sir->epoch_string,
  			       &tb_time_fill_value,
  			       NULL,
  			       &tb_time_valid_range,
  			       CETB_PACK,
  			       (float) CETB_FILE_TB_TIME_SCALE_FACTOR,
  			       (float) CETB_FILE_TB_TIME_ADD_OFFSET,
  			       "gregorian" ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing Tb time (P).\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote Tb time (P) to %s.\n", __FILE__, cetb_sir->filename );
  }
    
/* create non-enhanced images
   these are grd images pixel replicated to be at the same 
   resolution of the ave and sir images */

  printf("\nBegin creation of non-enhanced GRD images\n");

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


  if (storage == 1) {  /* file storage: rewind file and skip head */
    fseek(imf, head_len, REL_BEGIN);
  }

  store=space;
  for (irec = 0; irec < ncnt; irec++) {

    if (storage == 1) { /* get measurement info from file */
      store=space;

      gerr=get_measurements(store, store2, &tbval, &ang, &count, &ktime, &iadd, &nrec);
      if (gerr < 0) exit(-1);  /* fatal error */
      if (gerr==1) goto done3; /* finished with all measurements */
      
    } else {  /* get measurement info from memory */
      tbval = *((float *) (store+0));
      ang   = *((float *) (store+4));
      count = *((int *)   (store+8));
      iadd  = *((int *)   (store+16));
      if (HASAZANG)
	azang = *((float *) (store+20));

      store = store+HS;
    }
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
	printf("*** Non-enhanced address error: %d %d %d %d %d\n",
	       iadd,ix,iy,non_size_x,non_size_y);
      } else {
	fn = *(tot + iadd);
	*(tot +  iadd) = *(tot +   iadd) + 1.0;                    /* count */
	assert( *(num_samples + iadd) < CETB_FILE_TB_NUM_SAMPLES_MAX );
	*(num_samples + iadd) = *(num_samples + iadd) + 1;         /* num_samples for each pixel */
	ninv = 1./ *(tot + iadd);
	*(sx +   iadd) = (*(sx +   iadd) * fn + ang)*ninv;         /* mean inc angle */
	*(sx2 +  iadd) = (*(sx2 +  iadd) * fn + ang*ang)*ninv;     /* var inc angle */
	*(a_val+ iadd) = (*(a_val+ iadd) * fn + tbval)*ninv;	   /* mean Tb */
	*(sxy +  iadd) = (*(sxy +  iadd) * fn + tbval*ang)*ninv;   /* cross cor TB*inc angle */
	*(sy +   iadd) = (*(sy +   iadd) * fn + tbval*tbval)*ninv; /* var Tb */
	*(a_temp+iadd) = (*(a_temp+iadd) * fn + (float)ktime)*ninv;/* mean time */
      }
    }
  }
done3:

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
	  *(sy+i) = sqrt((double) temp);  /* Tb std */ 
	  old_bmin = min(old_bmin, *(sy+i));
	  old_bmax = max(old_bmax, *(sy+i));
	} else
	  *(sy+i) = 0.0;
      } else
	*(sy+i) = 0.0;
      
      if (*(tot+i) > 1.0) {
	denom = *(sx2+i) - (*(sx+i) * *(sx+i));
	if (denom > 0.0) {
	  *(sx2+i) = sqrt((double) denom);  /* inc std */
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

  printf(" Non-enhanced/Grid A  min   max --> %f %f\n",amin,amax);
  printf(" Non-enhanced/Grid V  min   max --> %f %f\n",old_bmin,old_bmax);
  printf(" Non-enhanced/Grid I  min   max --> %f %f\n",bmin,bmax);
  printf(" Non-enhanced/Grid C        max --> %.1f\n",tmax);

  if ( 0 != cetb_file_add_var( cetb_grd, "TB",
			       NC_USHORT, a_val,
			       ( size_t )nsx2, ( size_t ) nsy2,
			       CETB_FILE_TB_STANDARD_NAME,
			       "GRD TB",
			       CETB_FILE_TB_UNIT,
			       &tb_fill_value,
			       &tb_missing_value,
			       &tb_valid_range,
			       CETB_PACK,
			       (float) CETB_FILE_TB_SCALE_FACTOR,
			       (float) CETB_FILE_TB_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD Tb (A).\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote GRD Tb (A) to %s.\n", __FILE__, cetb_grd->filename );
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
    fprintf( stderr, "%s: Error writing GRD Tb num_samples.\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote GRD Tb num_samples to %s.\n", __FILE__, cetb_grd->filename );
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
			       (float) CETB_FILE_TB_STDDEV_SCALE_FACTOR,
			       (float) CETB_FILE_TB_STDDEV_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD Tb stddev (V).\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote GRD Tb stddev (V) to %s.\n", __FILE__, cetb_grd->filename );
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
			       (float) CETB_FILE_THETA_SCALE_FACTOR,
			       (float) CETB_FILE_THETA_ADD_OFFSET,
			       NULL ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD incidence angle (j_image).\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote GRD incidence angle (j_image) to %s.\n", __FILE__,
	     cetb_grd->filename );
  }

  if ( output_debug ) {

    if ( 0 != cetb_file_add_var( cetb_grd, "i_image",
				 NC_SHORT, sx2,
				 ( size_t )nsx2, ( size_t )nsy2,
				 NULL,
				 "GRD Incidence Angle stddev",
				 CETB_FILE_ANGULAR_UNIT,
				 &theta_fill_value,
				 NULL,
				 &theta_valid_range,
				 CETB_PACK,
				 (float) CETB_FILE_THETA_SCALE_FACTOR,
				 (float) CETB_FILE_THETA_ADD_OFFSET,
				 NULL ) ) {
      errors++;
      fprintf( stderr, "%s: Error writing GRD Tb (i_image).\n", __FILE__ );
    } else {
      fprintf( stderr, "> %s: Wrote GRD Tb (i_image) to %s.\n", __FILE__, cetb_grd->filename );
    }

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
  			       (float) CETB_FILE_TB_TIME_SCALE_FACTOR,
  			       (float) CETB_FILE_TB_TIME_ADD_OFFSET,
  			       "gregorian" ) ) {
    errors++;
    fprintf( stderr, "%s: Error writing GRD Tb time (P).\n", __FILE__ );
  } else {
    fprintf( stderr, "> %s: Wrote GRD Tb time (P) to %s.\n", __FILE__, cetb_grd->filename );
  }
    
  if ( output_debug ) {
    if (CREATE_NON) {

      /* create NON images with same pixel sizes as enhanced resolution images
	 using grid image data */

      for (i=0; i < nsize; i++)
	*(sx + i) = anodata_A;
      for (i=0; i < nsize; i++)
	*(sx2 + i) = anodata_V;
  
      for (i=0; i < nsize2; i++) {
	ix = (i % nsx2) * non_size_x;
	iy = (i / nsx2) * non_size_y;

	for (ii=0; ii < non_size_y; ii++)
	  for (iii=0; iii < non_size_x; iii++) {
	    iadd = nsx*(iy+ii)+ix+iii;
	    *(sx + iadd) = *(a_val + i);
	    *(sx2 + iadd) = *(sy + i);
	  }
      }

      if ( 0 != cetb_file_add_var( cetb_sir, "TB_nonenhanced",
				   NC_USHORT, sx,
				   ( size_t )nsx, ( size_t )nsy,
				   CETB_FILE_TB_STANDARD_NAME,
				   "Non-enhanced TB",
				   CETB_FILE_TB_UNIT,
				   &tb_fill_value,
				   &tb_missing_value,
				   &tb_valid_range,
				   CETB_PACK,
				   (float) CETB_FILE_TB_SCALE_FACTOR,
				   (float) CETB_FILE_TB_ADD_OFFSET,
				   NULL ) ) {
	errors++;
	fprintf( stderr, "%s: Error writing Non-enhanced Tb (A).\n", __FILE__ );
      } else {
	fprintf( stderr, "> %s: Wrote Non-enhanced Tb (A) to %s.\n", __FILE__, cetb_sir->filename );
      }

      if ( 0 != cetb_file_add_var( cetb_sir, "TB_nonenhanced_std_dev",
				   NC_USHORT, sx2,
				   ( size_t )nsx, ( size_t )nsy,
				   NULL,
				   "Non-enhanced TB Std Deviation",
				   CETB_FILE_TB_UNIT,
				   &tb_stddev_fill_value,
				   &tb_stddev_missing_value,
				   &tb_stddev_valid_range,
				   CETB_PACK,
				   (float) CETB_FILE_TB_STDDEV_SCALE_FACTOR,
				   (float) CETB_FILE_TB_STDDEV_ADD_OFFSET,
				   NULL ) ) {
	errors++;
	fprintf( stderr, "%s: Error writing Non-enhanced Tb stddev (V).\n", __FILE__ );
      } else {
	fprintf( stderr, "> %s: Wrote Non-enhanced Tb stddev (V) to %s.\n", __FILE__,
		 cetb_sir->filename );
      }
    
    } /* End of NON data section */
  } /* End of output_debug for NON data */

  if ( 0 != cetb_file_add_grd_parameters( cetb_grd, median_flag ) ) {
    fprintf( stderr, "%s: Error adding GRD parameters to %s.\n", __FILE__, cetb_grd->filename );
    exit( -1 );
  }
  cetb_file_close( cetb_grd );

  if ( 0 != cetb_file_add_sir_parameters( cetb_sir, nits, median_flag ) ) {
    fprintf( stderr, "%s: Error adding SIR parameters to %s.\n", __FILE__, cetb_sir->filename );
    exit( -1 );
  }
  cetb_file_close( cetb_sir );

  if (errors == 0) {
    printf("No errors encountered\n");
#ifdef INFOFILE
    /* write out info file if processing is completed successfully */
    printf("Info file: %s\n",info_name);
    omf = fopen(info_name,"w"); 
    if (omf == NULL) {
      eprintfc("ERROR: cannot open info file: '%s'\n", info_name); 
    } else {
      fprintf(omf,"SIR Processing of '%s' successfully completed\n",file_in);
      fprintf(omf,"A output file: '%s'\n",a_name);
      fprintf(omf,"I output file: '%s'\n",i_name);
      fprintf(omf,"J output file: '%s'\n",j_name);
      fprintf(omf,"C output file: '%s'\n",c_name);
      fprintf(omf,"P output file: '%s'\n",p_name);
      fprintf(omf,"V output file: '%s'\n",v_name);
      fprintf(omf,"E output file: '%s'\n",e_name);
      fprintf(omf,"AVE A output file: '%s'\n",a_name_ave);
      if (CREATE_NON) {
	fprintf(omf,"NON A output file: '%s'\n",non_aname);
	fprintf(omf,"NON V output file: '%s'\n",non_vname);
      }
      fprintf(omf,"GRD A output file: '%s'\n",grd_aname);
      fprintf(omf,"GRD V output file: '%s'\n",grd_vname);
      fprintf(omf,"GRD I output file: '%s'\n",grd_iname);
      fprintf(omf,"GRD J output file: '%s'\n",grd_jname);
      fprintf(omf,"GRD P output file: '%s'\n",grd_pname);
      fprintf(omf,"GRD C output file: '%s'\n",grd_cname);
      fclose(omf);
    }
#endif
  } else
    printf("Processing errors encountered\n");
  
  /* end of program */
  /* printf("De-allocating memory\n");  */

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


/* SIR algorithm update step */

void get_updates(float tbval, float ang, int count, int fill_array[],
		 short int response_array[], int its)
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
  if (ave == 0.0) return;

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
      update = 1.0/((0.5/ave)*(1.0-1.0/scale)+1.0/(*(a_val+n-1) * scale));
    else
      update = 0.5 * ave * (1.0 - scale) + *(a_val+n-1) * scale;

    (*(tot+n-1)) = (*(tot+n-1)) + m;
    *(a_temp+n-1) = (*(a_temp+n-1) * ( *(tot+n-1) - m) + update * m) / *(tot+n-1);
    
    /*
     * Count the number of measurements that hit a given pixel
     * It is a serious error if we accumulate more than NUM_SAMPLES_MAX hits.
     */
    if (its == 0) {
      assert( *(num_samples+n-1) < CETB_FILE_TB_NUM_SAMPLES_MAX );
      (*(num_samples+n-1))++;
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

float median(float *array, int count);
float cmedian(float *array, int count, float center);

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
	sum=sum+(180.0-abs(180.0-abs(array[i]-array[j])));
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
    temp=temp/5.0;
  }
  return(temp);
}



/* routine to compute the spatial response function weighted variance and error from measurements */

void stat_updates(float tbval, float ang, int count, int fill_array[],
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
  if (num == 0) return;
  ave =(double) (total/num);
  
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

void time_updates(float tbval, float ktime, float ang __attribute__ ((unused)), int count,
		  int fill_array[], short int response_array[])
{
  float ave;
  int i, n;
  
  for (i=0; i < count; i++) {
    n=fill_array[i];
    *(tot+n-1) = *(tot+n-1) + 1;
    ave = tbval;
    /* weight time average 
    *(sx+n-1) = *(sx+n-1) + ktime / ave;
    *(sy+n-1) = *(sy+n-1) + 1.0 / ave;    */
    /* unweighted time average */
    *(sx+n-1) = *(sx+n-1) + ktime;
    *(sy+n-1) = *(sy+n-1) + 1.0;
  }
  return;
}


int get_measurements(char *store, char *store2, float *tbval, float *ang, int *count,
		     int *ktime, int *iadd, int *nrec)
{  /* returns the next set of measurement from the file */
  int dumb, flag=1;
  
  while (flag == 1) {
    if (fread(&dumb, sizeof(int), 1, imf) != 0) {  /* fortran record header */
      /*	   read	 (50,err=500,end=500) tbval,ang,count,ktime,iadd
		   read (50,err=500,end=500) (fill_array(i),i=1,count)   */
      if (fread(store, sizeof(char), HS, imf) != HS) Ferror(200);
      if (fread(&dumb,sizeof(int), 1, imf) == 0) Ferror(201); /* fortran 
								record trailer */
      *tbval = *((float *) (store+0));
      *ang   = *((float *) (store+4));
      *count = *((int *)   (store+8));
      *ktime = *((int *)   (store+12));
      *iadd  = *((int *)   (store+16));
      /* if (HASAZANG)
       *azang = *((float *) (store+20)); */
      
      /*
      if (*tbval==0.0 || abs(*tbval) > 400.0)
	printf("record %d %f %f %d %d %d\n",*nrec,*tbval,*ang,*count,*ktime,*iadd);
      */

      /* read fill_array pixel indices */
      if (*count * 4 < nspace) {
	if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(210); 
	if (fread(store, sizeof(int), *count, imf) != *count) Ferror(211);
	if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(212);
      } else {
	eprintfi(" *** fill_array storage error 3 *** %d\n", *nrec);
	fprintf(stderr," *** fill_array storage error 3 *** %d %d %ld\n", *nrec,*count,nspace);
	return(-1);
      }

      /* read response_array values */
      if (*count * 2 < nspace) {
	if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(2101); 
	if (fread(store2, sizeof(short int), *count, imf) != *count) Ferror(2111);
	if (fread(&dumb, sizeof(int), 1, imf) == 0) Ferror(2121);
      } else {
	eprintfi(" *** fill_array storage error 4 *** %d\n", *nrec);
	fprintf(stderr," *** fill_array storage error 4 *** %d %ld\n",*nrec,nspace);
	return(-1);
      }
      (*nrec)++;
	
      if (*tbval < 350.0 && *tbval > 50.0) return(0);
    } else      /* end of file (or file err) encountered */
      return(1);
  }
  return(0);
}


void no_trailing_blanks(char *s)
{  /* remove trailing blanks (spaces) from string */
  int n=strlen(s);
  
  while (n > 0) {
    if (s[n] != ' ' && s[n] != '\0') return;
    if (s[n] == ' ') s[n] = '\0';
    n--;
  }
  return;
}


char *addpath(char *outpath, char *name, char *temp)
{ /* append path to name, return pointer to temp */
  sprintf(temp,"%s/%s",outpath,name);
  return(temp);  
}

  
