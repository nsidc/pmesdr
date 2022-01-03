/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_meta_setup.c

  generates radiometer egg (footprint) setup files for radiometer
  MEaSURES processing

  written by DGL at BYU  02/22/2014 + based on oscat_meta_setup_slice.c and ssmi_meta_setup_v7RSS.f
  further revision are tracked in bitbucket and not via this comment list MAH 05/15/15

******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <err.h>
#if defined(__INTEL__)
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <float.h>
#include <udunits2.h>

#include "calcalcs.h"
#include "cetb.h"
#include "utils.h"
#include "gsx.h"
#include "utils.h"
#include "sir_geom.h"
#include "utCalendar2_cal.h"

#define prog_version 0.3 /* program version */

/* This code can read and process several different data sets.
 * In order to achieve that, the input source files must first be processed into GSX files
 */

#define NSAVE 50            /* maximum number of regions to output */
#define MAXFILL 2000        /* maximum number of pixels in response function */
#define RESPONSEMULT 1000   /* response multiplier */
#define HASAZIMUTHANGLE 1   /* include azimuth angle in output setup file if 1, 
			       set to 0 to not include az ang (smaller file) */
#define DTR ((2.0*(M_PI))/360.0)       /* degrees to radians */

#define AEARTH 6378.1363              /* SEMI-MAJOR AXIS OF EARTH, a, KM */
#define FLAT 3.3528131778969144e-3    /* = 1/298.257 FLATNESS, f, f=1-sqrt(1-e**2) */

#define MINUTES_PER_DEG_LONGITUDE 4
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define HOURS_PER_DAY 24
#define MINUTES_PER_DAY ( HOURS_PER_DAY * MINUTES_PER_HOUR )
#define SECONDS_PER_DAY ( ( MINUTES_PER_DAY ) * SECONDS_PER_MINUTE )

#define min(a,b) (((a) <= (b)) ? (a) : (b))
#define max(a,b) (((a) >= (b)) ? (a) : (b))
#define mod(a,b) ((a) % (b))
#define dmod(a,b) ((a)-floor((a)/(b))*(b))
#define abs(x) (((x) >= 0 ) ? (x) : -(x))

/****************************************************************************/

static int nint(float r)
{
  int ret_val = r;  if (ret_val - r > 0.5) ret_val--;
  if (r - ret_val > 0.5) ret_val++;
  return(ret_val);
}

static void no_trailing_blanks(char *s)
{  /* remove trailing blanks (spaces) and line feeds from string */
  int n=strlen(s);
  
  while (n > 0) {
    if (s[n] == 10) s[n] = '\0';
    if (s[n] != ' ' && s[n] != '\0') return;
    if (s[n] == ' ') s[n] = '\0';
    n--;
  }
  if (n<0) s[n]='\0';  
  return;
}

/****************************************************************************/
typedef enum {
  UNKNOWN_LTOD=-1,
  ASCDES,
  LTOD
} setup_ltod_flag;

typedef struct { /* BYU region information storage */
  int nregions;
  FILE *reg_lu[NSAVE];
  float sav_lath[NSAVE],    sav_latl[NSAVE];
  float sav_lonh[NSAVE],    sav_lonl[NSAVE];
  int   sav_nsx[NSAVE],     sav_nsy[NSAVE];  
  int   sav_projt[NSAVE];
  float sav_ascale[NSAVE],  sav_bscale[NSAVE];
  float sav_a0[NSAVE],      sav_b0[NSAVE];
  float sav_ydeg[NSAVE],    sav_xdeg[NSAVE];
  int   sav_ipolar[NSAVE];
  int   sav_ibeam[NSAVE],   sav_regnum[NSAVE];
  char  sav_regname[NSAVE][11], sav_fname2[NSAVE][180];
  int   sav_dateline[NSAVE],sav_ascdes[NSAVE];
  float sav_tsplit1[NSAVE], sav_tsplit2[NSAVE];
  float sav_km[NSAVE];  
} region_save;  

/********************************************************************/

/* function prototypes */

/* BYU SSM/I approximate spatial response computation */

static float gsx_antenna_response(float x_rel, float y_rel, float theta,
				  float semimajor, float semiminor);
static int write_blanklines_to_header( region_save *save_area );
static int write_filenames_to_header( gsx_class *gsx,
				      region_save *save_area, int *file_flag,
				      unsigned long *position_filename,
				      unsigned long *position_data );
static int write_end_header( region_save *save_area );
static int write_header_info( gsx_class *gsx, region_save *save_area,
			      int year );
static FILE * get_meta(char *mname, char *outpath, int *dstart,
		       int *dend, int *mstart, int *mend, 
		       int *year, char *prog_n, float prog_v,
		       float *response_threshold, int *flatten,
		       int *median_flag, int *inc_correct, float *b_correct,
		       float *angle_ref, int *base_resolution,
		       region_save *save_area, cetb_platform_id *cetb_platform);
static int check_for_consistent_regions( region_save *save_area,
					 setup_ltod_flag *ltdflag );
static void compute_locations(region_save *a, int *nregions, int **noffset,
			      short int **latlon_store);
static int timedecode( double epochTime,
		       ut_unit *epochUnits, calcalcs_cal *calendar,
		       int *year, int *doy, int *month, int *day,
		       int *hour, int *minute, double *second );
static void rel_latlon(float *x_rel, float *y_rel, float alon, float alat,
		       float rlon, float rlat);
static float km2pix(float *x, float *y, int iopt, float ascale, float bscale,
		    int *stat);
static void print_projection(FILE *omf, int iopt, float xdeg, float ydeg, 
			     float ascale, float bscale, float a0, float b0);
static int box_size_by_channel( int ibeam, cetb_sensor_id id,
				int base_resolution, int *box_size );
static void combine_setup_files( char *outpath, region_save *a, int execution_flag,
				 int list_of_channels[], int chanA, int chanB );
static int ltod_split_time( cetb_platform_id platform_id,
			    cetb_region_id region_id,
			    cetb_direction_id direction_id,
			    int year, float *split_time );
static int get_search_period( int year, int dstart, int dend, int mstart, 
			      ut_unit *epochUnits, calcalcs_cal *calendar,
			      double *startEpochTime, double *imageEpochTime,
			      double *endEpochTime );
static int day_offset_from( int year, int month, int day, 
			    int hour, int minute, double second,
			    int dayOffset, ut_unit *epochUnits,
			    calcalcs_cal *calendar,
			    int *offsetYear, int *offsetMonth,
			    int *offsetDay,
			    double *offsetEpochTime );
static int ltod_day_offset( int dstart, int dend, int *midDay,
			    int *startDayOffset, int *endDayOffset,
			    int *imageDayOffset );

/****************************************************************************/

int main(int argc,char *argv[])
{
  region_save save_area;

  int nscans;

  int ret_status=1;

  char fname[250], mname[250];
  char outpath[250];
  char prog_name[250];
  char ftempname[250];
  char *option;
  
  int j,n;
  int nrec, iscan, iasc;
  char *s;
  float cen_lat, cen_lon, ctime;
  double cave;  
  int base_resolution=CETB_BASE_25_RESOLUTION;

  /* output record information */
  float tb,thetai,azang=0.0;
  int count,ktime_minutes,iadd, fill_array[MAXFILL+1];
  short int response_array[MAXFILL+1];  

  int jrec2[NSAVE];  /* measurement counter for each region */
  int dateline;      /* when 1, region crosses dateline */

  FILE *file_id;

  int flag,ascend;
  int dstart,dend,mstart,mend,year,iregion;
  int iday,iyear,imon,ihour,imin,jday;
  int idaye,iyeare,imone,ihoure,imine,jdaye;
  double isec,isece;
  int midDay, startDayOffset, endDayOffset, imageDayOffset;

  float theta;
  
  int ibeam,icc,icmax,icmin,nfile;  
  float cx,cy,lath,latl,lonh,lonl;
  int nsx,nsy,projt;
  float ascale,bscale,a0,b0,xdeg,ydeg,x,y;
  float tbmin=1.e10,tbmax=-1.e10; 
  
  int iadd1, box_size, box_size_flag=0;;
  float b_correct, angle_ref, box_size_km;

  /* file position pointers */
  unsigned long *position_filename;
  unsigned long *position_data;
  int *file_flag=NULL;
  
  /* memory for storage of pixel locations */
  int nregions,*noffset;
  short int *latlon_store;

  int ix1,ix2,ixsize,iysize,ixsize1,ixsize2,iysize1,iysize2,iy1,iy2,ix,iy,cnt;
  float clat,clon,dlat,dlon,sum;
  float eqlon, xhigh_lon, xlow_lon;
  int first_scan_flag[ CETB_NUM_LOCS ];
  int first_Tscan_flag[ CETB_NUM_LOCS ];
  float lat_diff;
  float sc_last_lat[ CETB_NUM_LOCS ];
  double sc_last_scantime[ CETB_NUM_LOCS ];
  int sc_last_ascend[ CETB_NUM_LOCS ];
  float dscale, alat1, alon1,x_rel, y_rel;
  float tsplit1_mins, tsplit2_mins, tsplit;
  float fractional_orbit;

  float response_threshold=-10.0; /* default response threshold in dB */
  int flatten=0;                  /* default: use rounded response function */  
  int median_flag=0;              /* default: do not use median filter */
  int inc_correct=0;              /* default: do not do incidence angle correction */

  int irec = 0; /* input cells counter */
  int jrec = 0; /* output record counter */
  int krec = 0; /* total scans considered */
  int mcnt=0;
  int z;
  char lin[100];

  /*
   * calendar calculation variables
   */
  calcalcs_cal* calendar = ccs_init_calendar( "Standard" );
  ut_system* unitSystem = ut_read_xml( NULL );
  char *unitString;
  ut_unit* epochUnits;
  double searchStartEpochTime;
  double imageStartEpochTime;
  double searchEndEpochTime;

  /* keeping track of LTOD mode */
  setup_ltod_flag ltod_flag;
  
  /* GSX variables */
  gsx_class *gsx=NULL;
  int gsx_count;
  int first_file=0;
  int first_scan_loc;
  int last_scan_loc;
  int status;
  char *gsx_fname[CETB_MAX_INPUT_FILES];
  int infile;
  int loc;
  int imeas;
  int first_measurement;
  cetb_region_id cetb_region;
  cetb_platform_id cetb_platform;

  gsx = NULL;
  for (n=0; n<NSAVE; n++)
    jrec2[n] = 0;  /* measurements for each output region */

  strcpy( prog_name, argv[0] );
  fprintf( stderr, "MEaSUREs Setup Program\nProgram: %s \n\n",prog_name);

  /*
   * optionally get the box size of pixels to use for calculating MRF for each 
   * box size is determined by a function that sets the value based on the channel
   * and the EFOV and the base_resolution for SMAP processing
   * unless the box size is passed in as a cmd line argument
   *
   * NOTE: specifying -b overrides box size setting in the code
   */ 
  while (--argc > 0 && (*++argv)[0] == '-') {
    for (option = argv[0]+1; *option != '\0'; option++) {
      switch (*option) {
      case 'b':
	++argv; --argc;
	if (sscanf(*argv,"%d", &box_size) != 1) {
	  fprintf( stderr, "meas_meta_setup: can't read box size %s\n", *argv);
	  exit(-1);
	}
	fprintf( stderr, "box size is %d\n", box_size );
	box_size_flag = 1;
	break;
      default:
	fprintf(stderr, "meas_meta_setup: Invalid option %c\n", *option);
	exit(-1);
      } /* end switch */
    } /* end loop for each input command option */
  } /* end loop while still input arguments */

  if (argc < 2) {
    printf( "\nusage: meas_meta_setup -b box_size meta_in outpath\n\n");
    printf( " input parameters:\n");
    printf( "   -b box_size is optional input argument to specify box_size for MRF\n");
    printf( "      default box_size is 80 for early regression testing\n");
    printf( "   meta_in     = input meta file\n");
    printf( "   outpath     = output path\n\n");
    exit (-1);
  }

  /* get input meta file name */
  sscanf(*argv++,"%s",mname);
  fprintf( stderr, "\nMetafile name: %s \n",mname);

  /* get output path */
  sscanf(*argv++,"%s",outpath);
  fprintf( stderr, "\nOutput path: %s \n",outpath);

  /* get meta_file region information and open output files */
  file_id = get_meta(mname, outpath, &dstart, &dend, &mstart, &mend, &year,
		     prog_name, prog_version ,&response_threshold, &flatten, &median_flag,
		     &inc_correct, &b_correct, &angle_ref, 
		     &base_resolution, &save_area, &cetb_platform);
  if (file_id == NULL) {
    fprintf(stderr,"*** could not open meta file %s/%s\n", outpath, mname);    
    exit(-1);  
  }
  if (0 != check_for_consistent_regions( &save_area, &ltod_flag ) ) {
    fprintf(stderr,"%s: ERROR setup cannot handle mixed LTOD region types\n",
	    __FILE__);
    exit(-1);  
  }

  fprintf( stderr, "%s: %s metafile number of output setup files %d\n",
	   __FILE__, mname, save_area.nregions);
  
  /* convert response threshold from dB to normal space */
  response_threshold=(float)(pow(10.,0.1*response_threshold));  
 
  /* compute approximate projection grid scale factors for later use */
  cnt = 100;
  for (iregion=0; iregion<save_area.nregions; iregion++) {      
    save_area.sav_km[iregion]=km2pix(&dlon,&dlat,save_area.sav_projt[iregion],
				     save_area.sav_ascale[iregion],
				     save_area.sav_bscale[iregion], &ret_status);
    if ( ret_status != 1 ) {
      fprintf( stderr, "%s: %s metafile: fatal error in routine km2pix\n",
	       __FILE__, mname );
      exit ( -1 );
    }
    fprintf( stderr, "%s: %s metafile: Region %d of %d: nominal km/pixel=%f\n",
	     __FILE__, mname, iregion, save_area.nregions, save_area.sav_km[iregion]);
  /* write out the search box size in km to each setup output file */

    if ( box_size_flag == 0 ) {
      status = box_size_by_channel( save_area.sav_ibeam[iregion],
				      cetb_platform_to_sensor[cetb_platform],
				      base_resolution, &box_size );
      if ( status != 0) {
	exit (-1);
      }
    }
    box_size_km = box_size/save_area.sav_km[iregion];
    fprintf( stderr, "%s: %s metafile:box size in pixels is %d and in km is %f for channel %d\n",
	     __FILE__, mname, box_size, box_size_km, save_area.sav_ibeam[iregion] );
    fwrite( &cnt, 4, 1, save_area.reg_lu[iregion] );
    for( z=0; z < 100; z++ ) lin[z] = ' ';
    sprintf( lin, " Search_box_km=%f", box_size_km );
    fwrite( lin, 100, 1, save_area.reg_lu[iregion] );
    fwrite( &cnt, 4, 1, save_area.reg_lu[iregion] );
  }
  
  /* pre-compute pixel locations for each region */
  compute_locations(&save_area, &nregions, &noffset, &latlon_store);
  fprintf( stderr, "\n");

  /* initialize some pixel counters */
  icc=0;
  cave=0.0;
  icmax=-1;
  icmin=200;

  /* read and process each input swath TB file from meta file input */
  krec=0;              /* total scans read */
  flag=1;              /* end flag */
  nfile=0;             /* L1B input file counter */

  /*
   * Before running through the list of input files,
   * save the file position for each setup output region
   */
  status =
    utils_allocate_clean_aligned_memory( (void**)&position_filename,
					 save_area.nregions*sizeof(long int) );
  if ( 0 != status ) {
    fprintf( stderr,
	     "%s: %s metafile: Couldn't allocate memory for file name position variables\n",
	     __FILE__, mname );
    exit (-1);
  }
  status =
    utils_allocate_clean_aligned_memory( (void**)&position_data,
					 save_area.nregions*sizeof(long int) );
  if ( 0 != status ) {
    fprintf( stderr,
	     "%s: %s: metafile Couldn't allocate memory for file data position variables\n",
	     __FILE__, mname );
    exit (-1);
  }
  for ( iregion = 0; iregion < save_area.nregions; iregion++ ) {
    *(position_filename+iregion) = ftell( save_area.reg_lu[iregion] );
  }
    
  /*
   * Run through all of the input data files so that a list of
   * them can be written out to the setup file
   */
  while(flag) { 
    
    /*
     * before reading in the next file, free the memory from the
     * previous gsx pointer
     */
    if ( NULL != gsx ) {
      gsx_close( gsx );
      gsx = NULL;
    }
    fgets(fname,sizeof(fname),file_id);

    if (ferror(file_id)) {
      fprintf( stderr, "%s: %s metafile: error reading input meta file \n",
	       __FILE__, mname );
      exit(-1);
    }

    if ( strstr(fname,"End_input_file_list") || feof(file_id) ) {
      /* proper end of meta file */
      flag=0;
      if ( NULL != gsx ) { // protects against no files in list
	gsx_close( gsx );
	gsx = NULL;
      }
    } else {
      /* read name of input swath file */
      s=strstr(fname,"Input_file");
      if ( NULL != s ) { /* skip line if not an input file */
	/* find start of file name and extract it */
	s=strchr(s,'=');
	strcpy(ftempname,++s);
	strcpy(fname, ftempname);
	no_trailing_blanks(fname);    
        gsx = gsx_init( fname );
	status = write_blanklines_to_header( &save_area );
	if ( 0 != status ) {
	  fprintf( stderr, "%s: %s metafile: couldn't write out blank lines for filenames\n",
		   __FILE__, mname );
	  exit (-1);
	}
	status = utils_allocate_clean_aligned_memory( (void**)&gsx_fname[nfile], strlen(fname)+1 );
	if ( 0 != status ) {
	  fprintf( stderr, "%s: %s metafile: couldn't allocate space for filename\n",
		   __FILE__, mname );
	  exit (-1);
	}
	strcpy( gsx_fname[nfile], fname );
	nfile++;
	if ( nfile > CETB_MAX_INPUT_FILES ) {
	  flag = 0;
	  gsx_close( gsx );
	  gsx = NULL;
	  fprintf( stderr, "%s: too many input files, only the first %d processed\n",
		   __FILE__, nfile-1 );
	}
      }
    }
  }

  /* check to see if there are no input files in the meta file
   * if that's the case, write out the end header, close the setup files
   * and quit the program
   */
  if ( 0 == nfile ) { 
      status = write_end_header( &save_area );
      if ( 0 != status ) {
	fprintf( stderr, "%s: *** couldn't write out end header information\n", __FILE__ );
	exit (-1);
      }
      fprintf( stderr, "%s: *** There are no input files so exit\n", __FILE__ );
      for (iregion=0; iregion<save_area.nregions; iregion++) {
	fclose(save_area.reg_lu[iregion]);
	save_area.reg_lu[iregion] = NULL;
      }
      exit(0);
  }

  /*
   * Initialize flags to keep track of orbit direction, one for
   * each measurement set; these will keep track of corresponding
   * measurement sets across sequentially ordered swath files
   */
  for ( loc=0; loc<CETB_NUM_LOCS; loc++ ) {
    first_scan_flag[ loc ] = 1;
    first_Tscan_flag[ loc ] = 1;
    sc_last_lat[ loc ] = (float) 0.0;
    sc_last_scantime[ loc ] = (double) 0.0;
    sc_last_ascend[ loc ] = 1;
  }

  for ( infile=0; infile<nfile; infile++ ) { /* input file read loop 1050 */     

  label_330:; /* read next file name from list gsx_fname */
    /* before reading in the next file, free the memory from the previous gsx pointer */
    if ( NULL != gsx ) {
      gsx_close( gsx );
      gsx = NULL;
    }
    strcpy(fname, gsx_fname[infile]);

    /* initialize the flag_file list for each region in this setup run */
    if ( NULL != file_flag ) {
      free( file_flag );
      file_flag = NULL;
    }
    status = utils_allocate_clean_aligned_memory( (void**)&file_flag,
						  sizeof(int)*(save_area.nregions) );
    if ( 0 != status ) {
      fprintf( stderr, "%s: Unable to allocate memory for file_flag array\n", __FILE__ );
      exit (-1);
    } 

    /*
     * read data from file into gsx_class variable
     */
    gsx = gsx_init( fname ); // Read in a GSX file
    if ( NULL == gsx ) {
      fprintf( stderr, "%s: couldn't read file '%s' with infile %d and gsx name %s\n",
	       __FUNCTION__, fname, infile, gsx_fname[infile] );
      free( gsx_fname[infile] );
      infile++;
      goto label_330;  // skip reading file on error
    } else {
      fprintf( stderr, "%s: Swath file: %s\n", __FILE__, fname );
    }

    /*
     * if this is the first file to be read, then write out the
     * final header info for downstream processing
     */
    if ( 0 == first_file ) {
      first_file++;
      status = write_header_info( gsx, &save_area, year );
      if ( 0 != status ) {
	fprintf( stderr, "%s: error: couldn't write out remaining header information\n",
		 __FILE__ );
	exit (-1);
      }
      status = write_end_header( &save_area );
      if ( 0 != status ) {
	fprintf( stderr, "%s: error: couldn't write out end header information\n",
		 __FILE__ );
	exit (-1);
      }
      /*
       * If this is AMSRE or AMSR2, combine the output setup files for a
       * and b scans and close the unneeded output file
       */
      if ( CETB_AQUA == cetb_platform ) {
	combine_setup_files( outpath, &save_area, 1 , (int *)cetb_ibeam_to_cetb_amsre_channel,
			     (int)AMSRE_89H_A, (int)AMSRE_89H_B);
	combine_setup_files( outpath, &save_area, 1 , (int *)cetb_ibeam_to_cetb_amsre_channel,
			     (int)AMSRE_89V_A, (int)AMSRE_89V_B);
      } else if ( CETB_GCOMW1 == cetb_platform ) {
	combine_setup_files( outpath, &save_area, 1 , (int *)cetb_ibeam_to_cetb_amsr2_channel,
			     (int)AMSR2_89H_A, (int)AMSR2_89H_B);
	combine_setup_files( outpath, &save_area, 1 , (int *)cetb_ibeam_to_cetb_amsr2_channel,
			     (int)AMSR2_89V_A, (int)AMSR2_89V_B);
      }
      fprintf( stderr, "%s: First file to be read\n", __FILE__ );
    } else {
      fprintf( stderr, "%s: Subsequent file to be read\n", __FILE__ );
    }

    /*
     * Loop for each measurement set in the file
     * One measurement set is one set of positions
     */
    fprintf( stderr, "%s: Satellite=%s orbit=%d lo scans=%d hi scans=%d\n",
	     __FILE__, cetb_platform_id_name[gsx->short_platform],
	     gsx->orbit, gsx->scans[0], gsx->scans[1] );
    /*
     * The target start/stop times are specified in the meta
     * file.  This logic will ignore stop time and calculate a "gross"
     * search period, outside of which any swath files will just be
     * skipped.  Swath measurement sets within the "gross" period will
     * be examined for each measurement in the later loop.
     *
     * FIXME: unitString should be pulled from gsx variable scan_time
     * units attribute.
     */
    unitString = strdup("seconds since 1987-01-01 00:00:00");
    /* ktime calculation assumes that units are seconds */
    if ( 0 != strncmp( unitString, "seconds", 7 ) ) {
      fprintf( stderr,
	       "%s: ERROR, unexpected time units = %s, exiting\n",
	       __FILE__, unitString );
      exit( -1 );
    }
    
    epochUnits = ut_parse( unitSystem, unitString, UT_ISO_8859_1 );
    if (NULL == epochUnits) {
      fprintf( stderr,
	       "%s: unable to parse unit string=%s\n", 
	       __FILE__, unitString );
    }
    if (0 != get_search_period( year, dstart, dend, mstart,
				epochUnits, calendar,
				&searchStartEpochTime,
				&imageStartEpochTime,
				&searchEndEpochTime ) ) {
      fprintf( stderr,
	       "%s: ERROR in get_search_period, exiting\n",
	       __FILE__ );
      exit( -1 );
    }
    
    for ( loc=0; loc<GSX_MAX_DIMS; loc++ ) {
      
      /* Get the times of the first and last scan in the measurement set */
      fprintf( stderr, "%s: measurement set index =%d\n", __FILE__, loc);
      nscans = gsx->scans[loc];
      if ( 0 != gsx->scans[loc] ) {

	first_scan_loc = loc;
	last_scan_loc = gsx->scans[loc]-1;
	if ( 0 != timedecode( *(gsx->scantime[first_scan_loc]),
			      epochUnits, calendar,
			      &iyear, &jday, &imon, &iday,
			      &ihour, &imin, &isec) ) {
	  fprintf( stderr, "%s: FATAL timedecode error.", __FILE__ );
	  exit(-1);
	}
	if ( 0 != timedecode( *(gsx->scantime[first_scan_loc]+last_scan_loc),
			      epochUnits, calendar,
			      &iyeare, &jdaye, &imone, &idaye,
			      &ihoure, &imine, &isece) ) {
	  fprintf( stderr, "%s: FATAL timedecode error.", __FILE__ );
	  exit(-1);
	}

	fprintf( stderr,
		 "%s: swath start : %.3lf %4d-%02d-%02d jday=%03d "
		 "%02d:%02d:%.3lf\n",
		 __FILE__, *(gsx->scantime[first_scan_loc]),
		 iyear,imon,iday,jday,ihour,imin,isec);
	fprintf( stderr,
		 "%s: swath stop  : %.3lf %4d-%02d-%02d jday=%03d "
		 "%02d:%02d:%.3lf\n",
		 __FILE__, *(gsx->scantime[first_scan_loc]+last_scan_loc),
		 iyeare,imone,idaye,jdaye,ihoure,imine,isece);    

	/*
	 * Reject the whole measurement set and skip to next
	 * swath file if swath ends before search span begins,
	 * or if swath begins later than search span ends
	 */
	if ( *(gsx->scantime[first_scan_loc]+last_scan_loc)
	     < searchStartEpochTime ) {
	  fprintf( stderr,
		   "%s: swath is prior to search period: skipping %s\n",
		   __FILE__, fname );
	  goto label_3501;

	}
	if ( *(gsx->scantime[first_scan_loc])
	     > searchEndEpochTime ) {
	  fprintf( stderr,
		   "%s: swath is after search period: skipping %s\n",
		   __FILE__, fname );
	  goto label_3501;
	}
	  
	/* for each scan in file */
	nrec=0;
	for (iscan=0; iscan<nscans; iscan++) { /*350*/

	  krec=krec+1;	/* count total scans read */
	  nrec=nrec+1;      /* count scans read in file */
      
	  if ((krec%500)==0) {
	    fprintf( stderr,
		     "Scans %7d | Pulses %9d | Output %9d | Day %3d\n",
		     krec, irec, jrec, jday );
	  }

	  /*
	   * Do not process this scan if the time is set to the
	   * scantime fillvalue. It's possible that we may want to
	   * change gsx to simply eliminate bad scan lines rather
	   * than flag them with the fill value
	   */
	  if ( (*(gsx->scantime[loc]+iscan) - gsx->fill_scantime[loc])
	       <= DBL_EPSILON ) {
	    goto label_350;
	  }

	  /*
	   * If gsx knows the pass direction then use it otherwise
	   * Keep track of sequential latitudes/times for scans
	   * This throws away the first scan line in each
	   * measurement set, that's ok because we assume a 3-day
	   * window of inputs.
	   */
	  if ( ASCDES == ltod_flag ) {
	    if ( CETB_NO_DIRECTION == gsx->pass_direction ) {
	      if ( first_scan_flag[loc] ) {
		sc_last_lat[loc] = *(gsx->sc_latitude[loc]+iscan);
		sc_last_scantime[loc] = *(gsx->scantime[loc]+iscan);
		first_scan_flag[loc] = 0;
#ifdef DEBUG	      
		fprintf( stderr,
			 "ASCDES DEBUG: First scan: f=%d, loc=%d, iscan=%d, new time=%.3lf, new lat=%.3f\n",
			 infile, loc, iscan, sc_last_scantime[loc], sc_last_lat[loc] );
#endif	      
		goto label_350;
	      }
	    }
	  }
	  
	  /* Skip scans outside the search period */
	  if ( ( *(gsx->scantime[loc]+iscan) < searchStartEpochTime )
	       || ( *(gsx->scantime[loc]+iscan) > searchEndEpochTime ) ) {
	    goto label_350;
	  }
	  
	  /*
	   * Convert scan time. All measurements in this scan assigned this time
	   */
	  if ( 0 != timedecode( *(gsx->scantime[loc]+iscan),
				epochUnits, calendar,
				&iyear, &jday, &imon, &iday,
				&ihour, &imin, &isec) ) {
	    fprintf( stderr, "%s: FATAL timedecode error.", __FILE__ );
	    exit(-1);
	  }

	  /*
	   * Compute time in mins since start of image data.
	   * This operation assumes that epochUnits are seconds.
	   *
	   * For example, if the observation time is 30 minutes
	   * before midnight UTC, then ktime_minutes will be -30
	   * - this time will later be combined with the
	   * longitude of the observation to get the relative
	   * local time for the measurement at that point to see
	   * the how different UTC values and longitudes are
	   * combined, look at the spreadsheet in the
	   * docs/internal directory in the repo - spreadsheet
	   * allows you to enter day and time of observation and
	   * longitude and then calculates ktime_minutes and
	   * ctime as in the code - the result can be compared
	   * with the split times to see if the time of the
	   * measurement is in the correct range.
	   */
	  ktime_minutes =
	    (int) ( ( ( *(gsx->scantime[loc]+iscan) ) - imageStartEpochTime )
		    / ( SECONDS_PER_MINUTE ) );
	    
	  /* compute the orientation of the nadir track with respect to north */
	  fractional_orbit = ( float ) iscan/nscans;
	  eqlon = (fractional_orbit * 360.f);
	  if (eqlon<0.0) eqlon=(eqlon+360.f);      
	  /*
	    find the longitude of the equator crossing of the
	    middle measurement to use in computing the longitudes
	    that separate ascending and descending for this
	    rev */
	  xhigh_lon=(eqlon+90.f);
	  xlow_lon =(eqlon-90.f);
	   
	  if (xhigh_lon >  180.f) xhigh_lon=(xhigh_lon-360.f);
	  if (xhigh_lon < -180.f) xhigh_lon=(xhigh_lon+360.f);
	  if (xlow_lon  >  180.f) xlow_lon =(xlow_lon -360.f);
	  if (xlow_lon  < -180.f) xlow_lon =(xlow_lon +360.f);

	  /*
	   * For asc/des processing: If gsx has an orbit
	   * direction, use it.  Otherwise, use the running
	   * latitude/scantime to decide
	   */
	  if ( ASCDES == ltod_flag ) {
	    if ( CETB_ASC_PASSES == gsx->pass_direction ) {
	      ascend = 1;
	    } else if ( CETB_DES_PASSES == gsx->pass_direction ) {
	      ascend = 0;
	    } else {
	      /*
	       * Check the time difference between this scan
	       * and the last one that was saved, if it's too large,
	       * we don't have enough information to decide, so
	       * we have to skip this scan
	       */
	      if ( *(gsx->scantime[loc]+iscan) - sc_last_scantime[loc] >
		   SECONDS_PER_MINUTE ) {
		sc_last_lat[loc] = *(gsx->sc_latitude[loc]+iscan);
		sc_last_scantime[loc] = *(gsx->scantime[loc]+iscan);
#ifdef DEBUG	      
		fprintf( stderr, "ASCDES DEBUG: Times too far apart, skipping this scan: "
			 "f=%d, loc=%d, iscan=%d,    time=%.3lf,     lat=%.3f\n",
			 infile, loc, iscan, sc_last_scantime[loc], sc_last_lat[loc] );
#endif		
		goto label_350;
	      } else {
		lat_diff = *(gsx->sc_latitude[loc]+iscan) - sc_last_lat[loc];
		if ( fabs( lat_diff ) < FLT_EPSILON ) {
		  ascend = sc_last_ascend[loc];
		} else if ( lat_diff > 0.0 ) {
		  ascend=1;
		} else { 
		  ascend=0;
		}

#ifdef DEBUG	      
		if (( infile == 13 || infile == 14 || infile == 15) &&
		    loc == 1 ) { 
		  fprintf( stderr, "ASCDES DEBUG: Times close: f=%d, loc=%d, iscan=%d, "
			 " last time=%.3lf, this_time=%.3lf, last lat=%.3f, this lat=%.3f, "
			 " diff=%.8f, ascend=%d\n",
			   infile, loc, iscan, sc_last_scantime[loc], *(gsx->scantime[loc]+iscan),
			   sc_last_lat[loc], *(gsx->sc_latitude[loc]+iscan),
			   *(gsx->sc_latitude[loc]+iscan) - sc_last_lat[loc], ascend );
		}
#endif		
		sc_last_lat[loc] = *(gsx->sc_latitude[loc]+iscan);
		sc_last_scantime[loc] = *(gsx->scantime[loc]+iscan);
		sc_last_ascend[loc] = ascend;
	      }
	    } /* end gsx didn't know the pass direction */

	    /*
	     * Skip this scan if it's not in the search day
	     */
	    if ( jday != dstart ) goto label_350;
		      
	  } /* end ASCDES processing */

	  /* extract TB measurements for each scan */
	  first_measurement = 0;
	  if ( CETB_AMSRE == gsx->short_sensor ) {
	    first_measurement = CETB_AMSRE_FIRST_MEASUREMENT;
	  }
	  
	  /* measurements loop */
	  for (imeas=first_measurement;
	       imeas < gsx->measurements[loc];
	       imeas++) {
	    irec=irec+1;	/* count of pulses examined */

	    if ( ASCDES == ltod_flag ) {
	      if ( first_Tscan_flag[loc] ) {
		first_Tscan_flag[loc] = 0;
#ifdef DEBUG	      
		fprintf( stderr, "ASCDES DEBUG: First T scan for this day: f=%d, loc=%d, "
			 "iscan=%d, new time=%.3lf, new lat=%.3f\n",
			 infile, loc, iscan, sc_last_scantime[loc], sc_last_lat[loc] );
#endif		
		
	      }
	    }

	    /*
	     * regions loop
	     * for each output region and section
	     * loop label_3400
	     */
	    for (iregion=0; iregion<save_area.nregions; iregion++) { 

	      ibeam=save_area.sav_ibeam[iregion];  /* beam number */
	      cetb_region =
		(cetb_region_id)(save_area.sav_regnum[iregion]
				 - CETB_PROJECTION_BASE_NUMBER);

	      if ( CETB_SSMI == gsx->short_sensor )
		gsx_count = cetb_ibeam_to_cetb_ssmi_channel[ibeam];
	      else if ( CETB_AMSRE == gsx->short_sensor )
		gsx_count = cetb_ibeam_to_cetb_amsre_channel[ibeam];
	      else if ( CETB_SSMIS == gsx->short_sensor )
		gsx_count = cetb_ibeam_to_cetb_ssmis_channel[ibeam];
	      else if ( CETB_SMMR == gsx->short_sensor )
		gsx_count = cetb_ibeam_to_cetb_smmr_channel[ibeam];
	      else if ( CETB_SMAP_RADIOMETER == gsx->short_sensor )
		gsx_count = cetb_ibeam_to_cetb_smap_channel[ibeam];
	      else if ( CETB_AMSR2 == gsx->short_sensor )
		gsx_count = cetb_ibeam_to_cetb_amsr2_channel[ibeam];
	      else {
		fprintf( stderr, "%s ****ERROR: Invalid short sensor name %d\n",
			 __FUNCTION__, gsx->short_sensor );
		continue;
	      }
	      
	      /* only get Tb's for channels that use the current
		 set of position coordinates */
	      if ( gsx->channel_dims[gsx_count] == (cetb_loc_id)loc ) {
	      
		/* for this beam, get measurement, geometry, and location */
		/* Note that the gsx_count variable gives you the
		 * offset into the tb array for whichever channel
		 * you are currently looking at - which comes
		 * from the region array the imeas variable
		 * counts measurements across a scanline and the
		 * iscan variable keeps track of which scan line
		 * you are on and gsx->measurements[loc] is the
		 * number of measurements in each scan line
		 */
		tb = *(gsx->brightness_temps[gsx_count]+imeas+iscan*gsx->measurements[loc]);

		/*
		 * Each in turn: nominal incidence angle, nominal azimuth angle,
		 * nominal longitude, nominal latitude
		 */
		thetai = *(gsx->eia[loc]+imeas+iscan*gsx->measurements[loc]);
		azang = *(gsx->eaz[loc]+imeas+iscan*gsx->measurements[loc]); 
		cen_lat = *(gsx->latitude[loc]+imeas+iscan*gsx->measurements[loc]);
		cen_lon = *(gsx->longitude[loc]+imeas+iscan*gsx->measurements[loc]);

		/* skip bad measurements */
		if (tb < *(gsx->validRange[gsx_count])) goto label_3400;
		if (fabs(thetai) < FLT_EPSILON) goto label_3400;
	  
		/*
		 * check ascending/descending orbit pass flag
		 * (see cetb.h for definitions)
		 */
		iasc=save_area.sav_ascdes[iregion];
		if (iasc != (int)CETB_ALL_PASSES) {
		  if (iasc == (int)CETB_ASC_PASSES) {
		    if (!ascend) goto label_3400;
		  } else if (iasc == (int)CETB_DES_PASSES) {
		    if (ascend) goto label_3400;
		  }
		}

		/* only for N and S projections */
		/* extract local-time-of-day split values - these
		   are sensor, year and projection dependent */
		/* a status return of non-zero indicates invalid
		   satellite/year combination */
		status = ltod_split_time(gsx->short_platform, cetb_region,
					 CETB_MORNING_PASSES, year, &tsplit);
		if ( status != 0 ) {
		  exit(-1);
		} else {
		  tsplit1_mins = tsplit * MINUTES_PER_HOUR;
		}
		status = ltod_split_time(gsx->short_platform, cetb_region,
					 CETB_EVENING_PASSES, year, &tsplit);
		if ( status != 0 ) {
		  exit(-1);
		} else {
		  tsplit2_mins = tsplit * MINUTES_PER_HOUR;
		}

		cy=cen_lat;
		cx=cen_lon;
		if (cx >  180.0) cx=(cx-360.f);
		if (cx < -180.0) cx=(cx+360.f);

		/* region lat/lon extent */
		lath=save_area.sav_lath[iregion];
		latl=save_area.sav_latl[iregion];
		lonh=save_area.sav_lonh[iregion];
		lonl=save_area.sav_lonl[iregion];
		dateline=save_area.sav_dateline[iregion];

		/*
		 * Apply LTOD considerations: if a
		 * local-time-of-day image, compute the local
		 * time relative to the current longitude, add in
		 * the UTC time of the observation
		 * (ktime_minutes) which was calculated above and
		 * compare the result to the split times windows.
		 * ctime is the relative local time Note: data
		 * may be next or previous UTC day Note also:
		 * that the length of the window is currently set
		 * to # minutes per 24 hour period this may not
		 * be true for all sensors
		 */
		if (iasc == (int)CETB_MORNING_PASSES
		    || iasc == (int)CETB_EVENING_PASSES) {
		  /* calculate the relative local time of day in minutes */
		  ctime = cx * MINUTES_PER_DEG_LONGITUDE + ktime_minutes; 

		  if ( 0 != ltod_day_offset( dstart, dend, &midDay, &startDayOffset,
					     &endDayOffset, &imageDayOffset ) ) {
		    fprintf( stderr, "%s: Error getting offset days\n", __FUNCTION__ );
		    return 1;
		  }
		  
		  if ( iasc == (int)CETB_MORNING_PASSES ) { /* morning */
		    if (dstart == dend) {
		      if (ctime < tsplit1_mins || ctime >= tsplit2_mins) {
			goto label_3400;
		      }
		    } else {
		      /* we have to get the morning data from the days before and after */
		      for (count = startDayOffset; count <= endDayOffset; count++) {
			//fprintf( stderr, "%s: count = %d", __FUNCTION__, count );
			if (ctime >  (tsplit1_mins + (count * MINUTES_PER_DAY)) &&
			    ctime <= (tsplit2_mins + (count * MINUTES_PER_DAY))) break;
			if (count == endDayOffset) goto label_3400;
		      }
		    }
		  } 
		  if ( iasc == (int)CETB_EVENING_PASSES ) {  /* evening */
		    if (dstart == dend) {
		      if (ctime < tsplit2_mins || ctime >= tsplit1_mins+MINUTES_PER_DAY) {
			goto label_3400;
		      }
		    } else {
		      /* we have to get the evening data from the days before and after */
		      for (count = startDayOffset; count <= endDayOffset; count++) {
			if (ctime >  (tsplit2_mins + (count * MINUTES_PER_DAY)) &&
			    ctime <= (tsplit1_mins + ((count+1) * MINUTES_PER_DAY))) break;
			if (count == endDayOffset) goto label_3400;
		      }
		    }
		  }
		}

		if (dateline) { /* convert lon to ascending order */
		  if (lonl < 0.0) lonl=(lonl+360.f);
		  if (cx < -180.0) cx=(cx+360.f);
		} else {	/* convert lon to -180 to 180 range */
		  if (cx > 180.0) cx=(cx-360.f);
		  if (cx < -180.0) cx=(cx+360.f);
		  if (cx > 180.0) cx=(cx-360.f);
		}

		/* check to see if center is within region */
		if (!((cx > lonl) && (cx < lonh) &&
		      (cy < lath) && (cy > latl))) goto label_3400;
	  
		/* put region projection information into easy to access variables */
		nsx=save_area.sav_nsx[iregion];
		nsy=save_area.sav_nsy[iregion];
		projt=save_area.sav_projt[iregion];
		ascale=save_area.sav_ascale[iregion];
		bscale=save_area.sav_bscale[iregion];
		a0=save_area.sav_a0[iregion];
		b0=save_area.sav_b0[iregion];
		ydeg=save_area.sav_ydeg[iregion];
		xdeg=save_area.sav_xdeg[iregion];
		dscale=save_area.sav_km[iregion];

		/* transform center lat/lon location of measurement to image pixel location */
		latlon2pix(cx, cy, &x, &y, projt, xdeg, ydeg, ascale, bscale, a0, b0);
		/* quantize pixel location to 1-based integer pixel indices */
		f2ipix(x, y, &ix2, &iy2, nsx, nsy);  /* note: ix2,iy2 are 1-based pixel address of center */

		/* skip measurements whose center falls outside of region */
		if (ix2==0 || iy2==0) goto label_3400; 
		iadd=nsx*(iy2-1)+ix2-1;  /* pixel array address (zero based) */
		if (iadd < 0) goto label_3400;
		if (iadd >= nsx*nsy) goto label_3400;

		mcnt++;  /* count measurements with centers in area */

		/* assign the center of the pixel containing the measurement location to
		   be the "new" measurement center lat/lon.  this "quantizes" the measurement
		   centers to the center of the output pixel */
		x=(ix2+0.5f);
		y=(iy2+0.5f);
		pixtolatlon(x, y, &clon, &clat, projt, xdeg, ydeg, ascale, bscale, a0, b0);

		/* define size of box centered at(ix2,iy2) in which the gain response 
		   is computed for each pixel in the box and tested to see if
		   the response exceeds a threshold.  if so, it is used */

		status = box_size_by_channel( ibeam, gsx->short_sensor,
					      base_resolution, &box_size ); 
		if ( status != 0 ) {
		  exit (-1);
		}

		ixsize=dscale*box_size; 
		iysize=dscale*box_size;
		if (ixsize<1) ixsize=1;
		if (iysize<1) iysize=1;

		/* define search box limits */
		ixsize1=-ixsize;
		ixsize2= ixsize;
		if (ix2+ixsize1<1) ixsize1=1-ix2;
		if (ix2+ixsize2>nsx) ixsize2=nsx-ix2;
		iysize1=-iysize;
		iysize2= iysize;
		if (iy2+iysize1<0) iysize1=1-iy2; 
		if (iy2+iysize2>=nsy) iysize2=nsy-iy2;

		theta = azang;
		/* for each pixel in the search box compute the normalized 
		   footprint gain response function
		   keep only those which exceed specified threshold */
		count=0;
		for (iy1=iysize1; iy1<=iysize2; iy1++) {
		  iy=iy1+iy2;  /* 1-based address */
		  for (ix1=ixsize1; ix1<=ixsize2; ix1++) {
		    ix=ix1+ix2; /* 1-based address */
		    iadd1=nsx*(iy-1)+ix-1; /* zero-based address of pixel */
		    if ( ( iadd1 >= 0 ) & ( iadd1 < nsx*nsy ) ) {		  
		      /* get pre-computed lat/lon of pixel */
		      alat1=(latlon_store[iadd1*2+  noffset[iregion]]/200.f);
		      alon1=(latlon_store[iadd1*2+1+noffset[iregion]]/175.f);

		      /* compute antenna pattern response at each pixel based on beam number, 
			 location, and projection rotation and scaling */
		      rel_latlon(&x_rel,&y_rel,alon1,alat1,clon,clat);
		      //		      gsx_count = (int)cetb_ibeam_to_cetb_ssmi_channel[ibeam];
		      sum = gsx_antenna_response( x_rel, y_rel, theta,
						  *(gsx->efov[gsx_count]),
						  *(gsx->efov[gsx_count]+1) );
		      if (sum > response_threshold) {
			if (flatten) sum=1.0;    /* optionally flatten response */
			fill_array[count]=iadd1; /* address of pixel */
			response_array[count]=(short int) nint(sum*RESPONSEMULT); /* quantized response */
			count++;
			if (count >= MAXFILL) {
			  fprintf(stderr,"*** count overflow has occurred\n");		  
			  count=MAXFILL;
			}
		
		      }
		    }
		  }
		}
	  
		/* write measurement and addresses to setup output file */
		if (count > 1) {
		  /* if there is a measurement, then set the value of file_flag to be
		     1 for this file and projection */
		  *(file_flag+iregion) = 1;
		  jrec++; /* a count of total records written */
		  jrec2[iregion]++; /* records/region */
		  if (count >= MAXFILL) { /* error handling -- this should not occur! */
		    fprintf( stderr, "%s: %s metafile: count %d overflow=%d at %d\n",
			     __FILE__, mname, count, MAXFILL, jrec );
		    fprintf( stderr, "%s: %s metafile: center %f %f  %d %d  count %d\n",
			     __FILE__, mname, cen_lat, cen_lon, iscan, ibeam, count );
		    count=MAXFILL;
		  }

		  /* apply incidence angle correction to tb measurements if selected */
		  if (inc_correct)
		    tb=tb-b_correct*(thetai-angle_ref);

		  /* compute statistics of count */
		  icmax=max(icmax,count);
		  icmin=min(icmin,count);
		  cave=cave+count;
		  icc=icc+1;
		  if (tb>0.0) {
		    tbmax=max(tbmax,tb);
		    tbmin=min(tbmin,tb);
		  }	    

		  /* write measurement information record to output .setup file */
		  cnt=4*5;
		  if (HASAZIMUTHANGLE) cnt += 4;
		  fwrite(&cnt,   4,1,save_area.reg_lu[iregion]);
		  fwrite(&tb,    4,1,save_area.reg_lu[iregion]); /* TB (K) */
		  fwrite(&thetai,4,1,save_area.reg_lu[iregion]); /* incidence angle (deg) */
		  fwrite(&count, 4,1,save_area.reg_lu[iregion]); /* number of pixels in list */
		  fwrite(&ktime_minutes, 4,1,save_area.reg_lu[iregion]); /* time of measurement */
		  fwrite(&iadd,  4,1,save_area.reg_lu[iregion]); /* address of center pixel location */
		  if (HASAZIMUTHANGLE)
		    fwrite(&azang, 4,1,save_area.reg_lu[iregion]); /* azimuth angle relative to north (deg) */
		  fwrite(&cnt,   4,1,save_area.reg_lu[iregion]);
#ifdef DEBUGCOUNT
		  if ( iregion == 1 ) {
		    fprintf( stderr, "COUNT DEBUG: jrec2=%d, cnt=%d, tb=%.2f, thetai=%.2f, "
			     " count=%d, ktime=%d, iadd=%d, cnt=%d\n",
			     jrec2[iregion], cnt, tb, thetai, count, ktime_minutes, iadd, cnt );
		  }
#endif	      

		  /* first write list of pixels covered by response pattern */
		  cnt=4*count;
		  fwrite(&cnt,      4,    1,save_area.reg_lu[iregion]);
		  fwrite(fill_array,4,count,save_area.reg_lu[iregion]);
		  fwrite(&cnt,      4,    1,save_area.reg_lu[iregion]);
		  /* then write response value for each pixel */
		  cnt=2*count;
		  fwrite(&cnt,      4,    1,save_area.reg_lu[iregion]);
		  fwrite(response_array,2,count,save_area.reg_lu[iregion]);
		  fwrite(&cnt,      4,    1,save_area.reg_lu[iregion]);		  
	      }
	      }
	      label_3400:; /* end of regions loop */
	    }
	    /* At the end of the regions loop, check to see if any file names need to be written out */
	    
	  }  /* end of measurements loop */
	label_350:; /* end of scan loop */
	}
      } /* end of locs loop */
    }
    label_3501:;  /* end of input file */

    /* input file has been processed */
    fprintf( stderr, "\nTotal input scans: %d  Total input pulses: %d\n",krec,irec);
    fprintf( stderr, "Region counts: ");
    for (j=0; j<save_area.nregions; j++)
      fprintf( stderr, " %d",jrec2[j]);
    fprintf( stderr, "\n");
    fprintf( stderr, "Input File Completed:  %s\n",fname);
    fprintf( stderr, "Last Day %d in Range: %d - %d\n\n",jday,dstart,dend);
    fprintf( stderr, "Number of measurements: %d\n",icc);
    fprintf( stderr, "IPR count average:  %lf\n",(icc>0? cave/(float) icc: cave));
    fprintf( stderr, "IPR count max,min:  %d %d \n",icmax,icmin);
    fprintf( stderr, "Tb max,min:  %f %f \n\n",tbmax,tbmin);
  
    if (jday <= dend)
      fprintf( stderr, "*** DAY RANGE IS NOT FINISHED ***\n");
    fprintf( stderr, "End of day period reached %d %d \n",jday,dend);

    /* input file loop */
    fprintf( stderr, "Done with setup records %d %d\n",irec,krec);
    free( gsx_fname[infile] );
    status = write_filenames_to_header( gsx, &save_area, file_flag, position_filename,
					position_data );
    if ( 0 != status ) {
      fprintf( stderr, "%s: couldn't write %s filename to output setup file\n",
	       __FILE__, gsx->source_file );
      exit (-1);
    }
  } /* input file read loop 1050 */

  /* close output setup files */
  for (j=0; j<save_area.nregions; j++) {
    fprintf( stderr, "\nRegion %d %s beam %d records %d\n",save_area.sav_regnum[j],
	   save_area.sav_regname[j],save_area.sav_ibeam[j],jrec2[j]);
    no_trailing_blanks(save_area.sav_fname2[j]);
    fprintf( stderr, "Output data written to %s\n",save_area.sav_fname2[j]);
    if ( j == 0 ) {
      fclose(save_area.reg_lu[j]);
      save_area.reg_lu[j] = NULL;
    } else {
        if ( CETB_AQUA == cetb_platform || CETB_GCOMW1 == cetb_platform ) {
	  /* now check to see if you have b channels for 89H or 89V and if you also
	     have A channels then combine */
	  if ( CETB_AQUA == cetb_platform ) {
	    combine_setup_files( outpath, &save_area, 2 , (int *)cetb_ibeam_to_cetb_amsre_channel,
				 (int)AMSRE_89H_A, (int)AMSRE_89H_B);
	    combine_setup_files( outpath, &save_area, 2 , (int *)cetb_ibeam_to_cetb_amsre_channel,
				 (int)AMSRE_89V_A, (int)AMSRE_89V_B);
	  }
	    
	  if ( CETB_GCOMW1 == cetb_platform ) {
	    combine_setup_files( outpath, &save_area, 2 , (int *)cetb_ibeam_to_cetb_amsr2_channel,
				 (int)AMSR2_89H_A, (int)AMSR2_89H_B);
	    combine_setup_files( outpath, &save_area, 2 , (int *)cetb_ibeam_to_cetb_amsr2_channel,
				 (int)AMSR2_89V_A, (int)AMSR2_89V_B);
	  }
	    
	  if ( NULL != save_area.reg_lu[j] ) {
	    fprintf( stderr, "%s: back from combin with fileid != NULL", __FUNCTION__ );
	    fclose( save_area.reg_lu[j] );
	    save_area.reg_lu[j] = NULL;
	  }
	} else {
	  fclose(save_area.reg_lu[j]);
	  save_area.reg_lu[j] = NULL;
	}
    }
  }
  fprintf( stderr, "\n");

  /* Cleanup memory and close files */
  free( unitString );
  ut_free_system( unitSystem );
  ccs_free_calendar( calendar );
  fclose(file_id);
  fprintf( stderr, "Setup program successfully completed\n");

  return(0); /* successful termination */
}

/* *********************************************************************** */

FILE * get_meta(char *mname, char *outpath, 
		int *dstart, int *dend, int *mstart, int *mend, int *year, 
		char *prog_n, float prog_v,
		float *response_threshold, int *flatten, int *median_flag,
		int *inc_correct, float *b_correct, float *angle_ref, 
		int *base_resolution, region_save *a, cetb_platform_id *cetb_platform)
{
  /* read meta file, open output .setup files, write .setup file headers, and 
     store key parameters in memory */
 
  FILE *file_id, *ftemp;  

  int ireg;
  char line[100], lin[100];
  int asc_des;
  float lath,latl,lonh,lonl;
  int regnum,projt=0;
  float aorglat,aorglon;
  int nsx,nsy;
  float ascale,bscale,a0,b0,xdeg,ydeg,xdim,ydim;
  int nsx2,nsy2,non_size_x,non_size_y;
  float ascale2,bscale2,a02,b02,ydeg2,xdeg2;
  int iregion=0,ipolar=0;
  char regname[11];
  int dateline;
  char fname2[180];
  char outname[350];
  char sensor[40]="SSMI something";
  int ibeam;  

  int flag, flag_out, flag_region, flag_section, flag_files;
  float a_init,a_offset;
  int nits;

  char *x;
  int z, nsection, isection, cnt;
  float tsplit1=1.0, tsplit2=13.0;

  int count;

  iregion=0;
  ireg=0;

  fprintf( stderr, "%s: open meta file %s\n", __FUNCTION__, mname);
  file_id=fopen(mname,"r");
  if (file_id == NULL) {
    fprintf( stderr, "%s: *** could not open input meta file %s\n", __FUNCTION__, mname);
    return(file_id);
  }

  /* read meta file header info (applies to all regions) */

  flag=1;
  while (flag) {
    fgets(line,sizeof(line),file_id);
    no_trailing_blanks(line);
    if (ferror(file_id)) {
      fprintf( stderr, "%s: *** error reading meta file\n", __FUNCTION__ );
      flag=0;
    } else {
      
      if (strstr(line,"End_description") != NULL)
	flag=0;

      if (strstr(line,"Sensor") != NULL) {
	x = strchr(line,'=');
	strncpy(sensor,++x,40);
	/* Here is where you can get the platform ENUM */
	for ( count=0; count < CETB_NUM_PLATFORMS; count++ ) {
	  if ( strcmp( sensor, cetb_platform_id_name[count] ) == 0 ) {
	    *cetb_platform = (cetb_platform_id) count;
	  }
	}
	fprintf( stderr, "%s: **** cetb_platform_id *** is %d\n", __FUNCTION__, *cetb_platform );

      }

      if (strstr(line,"Start_Year") != NULL) {
	x = strchr(line,'=');
	*year=atoi(++x);
      }
      
      if (strstr(line,"Start_day") != NULL) {
	x = strchr(line,'=');
	*dstart=atoi(++x);
      }
      
      if (strstr(line,"End_day") != NULL) {
	x = strchr(line,'=');
	*dend=atoi(++x);
      }
      
      if (strstr(line,"Start_minute") != NULL) {
	x = strchr(line,'=');
	*mstart=atoi(++x);
      }
      
      if (strstr(line,"End_minute") != NULL) {
	x = strchr(line,'=');
	*mend=atoi(++x);
      }
      
      if (strstr(line,"A_initialization") != NULL) {
	x = strchr(line,'=');
	a_init=(float)atof(++x);
      }      
      
      if (strstr(line,"A_offset") != NULL) {
	x = strchr(line,'=');
	a_offset=(float)atof(++x);
      }
      
      if (strstr(line,"Max_iterations") != NULL) {
	x = strchr(line,'=');
	nits=atoi(++x);
      }
      
      if (strstr(line,"Reference_incidence_angle") != NULL) {
	x = strchr(line,'=');
	*angle_ref=(float)atof(++x);
      }
      
      if (strstr(line,"Incidence_ang_correct") != NULL) {
	x = strchr(line,'=');
	*inc_correct=(float)atof(++x);
      }
      
      if (strstr(line,"B_correct_value") != NULL) {
	x = strchr(line,'=');
	*b_correct=(float)atof(++x);
      }
      
      if (strstr(line,"Response_threshold") != NULL) {
	x = strchr(line,'=');
	*response_threshold=(float)atof(++x);
      }
      
      if (strstr(line,"Flat_response") != NULL) {
	x = strchr(line,'='); x++;	
	if (*x== 'F' || *x== 'f') *flatten=0;
	else *flatten=1;	
      }

      if (strstr(line,"Median_filter") != NULL) {
	x = strchr(line,'='); x++;	
	if (*x== 'F' || *x== 'f') *median_flag=0;
	else *median_flag=1;	
      }      
      
      if (strstr(line,"Base_resolution") != NULL) {
	x = strchr(line,'=');
	*base_resolution=atoi(++x);
      }
      
      if (strstr(line,"Num_Regions") != NULL) {
	x = strchr(line,'=');
	a->nregions=atoi(++x);
	fprintf( stderr, "%s: Regions in meta file: %d\n", __FUNCTION__, a->nregions);
      }

      if (strstr(line,"Begin_region_description") != NULL) {
	/* new region started set some default values */
	asc_des=CETB_ALL_PASSES;	/* use both asc/desc orbits */
	ireg=ireg+1;
	fprintf( stderr, "%s: Region %d of %d  Total regions: %d\n", __FUNCTION__, ireg,a->nregions,iregion);

	/* read region information */
	flag_region=1;
	while(flag_region) {
	  fgets(line,sizeof(line),file_id);
	  no_trailing_blanks(line);
	  if (ferror(file_id)) {
	    fprintf( stderr, "%s: *** error reading meta file at region \n", __FUNCTION__ );
	    flag_region=0;
	  } else {
	    if (strstr(line,"End_region_description") != NULL)
	      flag_region=0;
      
	    if (strstr(line,"Region_id") != NULL) {
	      x = strchr(line,'=');
	      regnum=atoi(++x);
	    }
      
	    if (strstr(line,"Latitude_low") != NULL) {
	      x = strchr(line,'=');
	      latl=(float)atof(++x);
	    }
      
	    if (strstr(line,"Latitude_high") != NULL) {
	      x = strchr(line,'=');
	      lath=(float)atof(++x);
	    }
      
	    if (strstr(line,"Longitude_low") != NULL) {
	      x = strchr(line,'=');
	      lonl=(float)atof(++x);
	    }
      
	    if (strstr(line,"Longitude_high") != NULL) {
	      x = strchr(line,'=');
	      lonh=(float)atof(++x);
	    }

	    if (strstr(line,"Dateline_crossing") != NULL) {
	      x = strchr(line,'='); x++;	
	      if (*x== 'F' || *x== 'f') dateline=0;
	      else dateline=1;	
	    }

	    if (strstr(line,"AscDesc_flag") != NULL) {
	      x = strchr(line,'=');
	      asc_des=atoi(++x);
	    }

	    if (strstr(line,"Region_name") != NULL) {
	      x = strchr(line,'=');
	      strncpy(regname,++x,10);
	    }      
      
	    if (strstr(line,"Polarization") != NULL) {
	      x = strchr(line,'=');
	      ipolar=atoi(++x);
	    }
      
	    if (strstr(line,"Beam_index") != NULL) {
	      x = strchr(line,'=');
	      ibeam=atoi(++x);
	    }
      
	    if (strstr(line,"Max_iterations") != NULL) {
	      x = strchr(line,'=');
	      nits=atoi(++x);
	    }
            
	    if (strstr(line,"Sectioning_code") != NULL) {
	      x = strchr(line,'=');
	      nsection=atoi(++x);
	      nsection=nsection%100;	      
	    }

	    if (strstr(line,"Begin_section_description") != NULL) {
	      /* read section information */
	      flag_section=1;
	      while(flag_section) {
		fgets(line,sizeof(line),file_id);
		no_trailing_blanks(line);
		if (ferror(file_id)) {
		  fprintf( stderr, "%s: *** error reading meta file at section \n", __FUNCTION__ );
		  flag_section=0;
		} else {
		  if (strstr(line,"End_section_description") != NULL)
		    flag_section=0;
      
		  if (strstr(line,"Section_id") != NULL) {
		    x = strchr(line,'=');
		    isection =atoi(++x);
		    fprintf( stderr, "%s: Section %d image count %d\n", __FUNCTION__, isection,iregion);		    
		  }

		  if (strstr(line,"Project_type") != NULL) {
		    x = strchr(line,'=');
		    projt=atoi(++x);
		  }
      
		  if (strstr(line,"Projection_origin_x") != NULL) {
		    x = strchr(line,'=');
		    aorglat =(float)atof(++x);
		    ydeg=aorglat;
		  }

		  if (strstr(line,"Projection_origin_y") != NULL) {
		    x = strchr(line,'=');
		    aorglon =(float)atof(++x);
		    xdeg=aorglon;
		  }

		  if (strstr(line,"Projection_offset_x") != NULL) {
		    x = strchr(line,'=');
		    a0=(float)atof(++x);
		  }

		  if (strstr(line,"Projection_offset_y") != NULL) {
		    x = strchr(line,'=');
		    b0=(float)atof(++x);
		  }

		  if (strstr(line,"Projection_scale_x") != NULL) {
		    x = strchr(line,'=');
		    ascale=(float)atof(++x);
		  }

		  if (strstr(line,"Projection_scale_y") != NULL) {
		    x = strchr(line,'=');
		    bscale=(float)atof(++x);
		  }

		  if (strstr(line,"Projection_dim_x") != NULL) {
		    x = strchr(line,'=');
		    xdim=(float)atof(++x);
		  }

		  if (strstr(line,"Projection_dim_y") != NULL) {
		    x = strchr(line,'=');
		    ydim=(float)atof(++x);
		  }

		  if (strstr(line,"Image_size_x") != NULL) {
		    x = strchr(line,'=');
		    nsx=atoi(++x);
		  }

		  if (strstr(line,"Image_size_y") != NULL) {
		    x = strchr(line,'=');
		    nsy=atoi(++x);
		  }

		  if (strstr(line,"Grid_projection_origin_x") != NULL) {
		    x = strchr(line,'=');
		    ydeg2=(float)atof(++x);
		  }

		  if (strstr(line,"Grid_projection_origin_y") != NULL) {
		    x = strchr(line,'=');
		    xdeg2=(float)atof(++x);
		  }

		  if (strstr(line,"Grid_projection_offset_x") != NULL) {
		    x = strchr(line,'=');
		    a02=(float)atof(++x);
		  }

		  if (strstr(line,"Grid_projection_offset_y") != NULL) {
		    x = strchr(line,'=');
		    b02=(float)atof(++x);
		  }

		  if (strstr(line,"Grid_projection_scale_x") != NULL) {
		    x = strchr(line,'=');
		    ascale2=(float)atof(++x);
		  }

		  if (strstr(line,"Grid_projection_scale_y") != NULL) {
		    x = strchr(line,'=');
		    bscale2=(float)atof(++x);
		  }

		  if (strstr(line,"Grid_scale_x") != NULL) {
		    x = strchr(line,'=');
		    non_size_x=(float)atof(++x);
		  }

		  if (strstr(line,"Grid_scale_y") != NULL) {
		    x = strchr(line,'=');
		    non_size_y =(float)atof(++x);
		  }

		  if (strstr(line,"Grid_size_x") != NULL) {
		    x = strchr(line,'=');
		    nsx2=atoi(++x);
		  }

                  if (strstr(line,"Local_time_split1") != NULL) {
		    x = strchr(line,'=');
		    tsplit1=(float)atof(++x);
		  }

		  if (strstr(line,"Local_time_split2") != NULL) {
		    x = strchr(line,'=');
		    tsplit2=(float)atof(++x);
		  }

		  if (strstr(line,"Grid_size_y") != NULL) {
		    x = strchr(line,'=');
		    nsy2=atoi(++x);
		  }

		  if (strstr(line,"Setup_file_EXTENSION") != NULL) {
		    x = strchr(line,'=');
		  }      

		  if (strstr(line,"Setup_file") != NULL) {
		    x = strchr(line,'=');
		    strncpy(fname2,++x,40);
		    no_trailing_blanks(fname2);   
		  }      
      
		  if (strstr(line,"Begin_product_file_names") != NULL) {
		    /* do not save section 0 when sectioning */		    
		    flag_out=1;
		    if (nsection > 0 && isection < 1) flag_out=0;
		    if (flag_out) {
		      /* section input information input done
			 begin dumping data to setup files */
		      iregion++;
		      
		      /* print out region information summary */
		      fprintf( stderr, "\nSIR file header information: %d %d %d %d\n",iregion,a->nregions,isection,nsection);
		      fprintf( stderr, "  Year, day range: %d %d %d %d %d\n",*year,*dstart,*dend,*mstart,*mend);
		      fprintf( stderr, "  Image size: %d x %d   Projection: %d\n",nsx,nsy,projt);
		      fprintf( stderr, "  Origin: %f %f  Span: %f %f\n",a0,b0,xdeg,ydeg);
		      fprintf( stderr, "  Scales: %f %f  Pol (0=h,1=v): %d\n",ascale,bscale,ipolar);
		      fprintf( stderr, "  Region: %s  Num %d\n",regname,regnum);
		      fprintf( stderr, "  Reg LL corner: %f %f   UR corner: %f %f\n",latl,lonl,lath,lonh);
		      fprintf( stderr, "  Origin (lat,lon): %f %f\n",aorglat,aorglon);
		      fprintf( stderr, "  Array Dimensions in Km: x=%f y=%f\n",xdim,ydim);
		      fprintf( stderr, "  AscDesc flag (0=both,1=asc,2=dsc,3=morn,4=eve,5=mid): %d\n",asc_des);
		      fprintf( stderr, "  Grid size: %f %f  Span: %f %f\n",xdeg,ydeg,aorglat,aorglon);
		      fprintf( stderr, "  Scales: %f %f   Origin: %f %f\n",ascale,bscale,a0,b0);
		      fprintf( stderr, "  GRD image size: %d %d  Size: %d %d\n",nsx2,nsy2,non_size_x,non_size_y);
		      fprintf( stderr, "  GRD image span: %f %f  Orig: %f %f\n",xdeg2,ydeg2,a02,b02);		      
		      fprintf( stderr, "  GRD image scale: %f %f\n",ascale2,bscale2);		      
		      fprintf( stderr, "  Median filter %d  Ref Inc angle %f\n",*median_flag,*angle_ref);
		      fprintf( stderr, "  Incidence angle correction %d  b_correct %f\n",*inc_correct,*b_correct); 
                      fprintf( stderr, "  Time split: %f %f\n\n",tsplit1,tsplit2);
		      
		      /* open output setup file for this section of this region */
		      sprintf(outname,"%s/%s",outpath,fname2);		      
		      ftemp = fopen( outname, "wb" );

		      if ( ftemp != NULL ) {
			a->reg_lu[iregion-1] = ftemp;
			fprintf( stderr, "Opened setup output file '%s'  %d\n",outname,iregion);
		      } else {
			fprintf( stderr, "Couldnot open setup output file '%s' \n", outname );
			return ( NULL );
		      }

		      /* write line header info */
		      /* all values are assumed to be 4 byte long */
		      cnt=4;
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      cnt=0;
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]); /* irecords */
		      cnt=4;
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      /* write region projection information */
		      cnt=4*9;
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      fwrite(&nsx,4,1,a->reg_lu[iregion-1]);
		      fwrite(&nsy,4,1,a->reg_lu[iregion-1]);
		      fwrite(&ascale,4,1,a->reg_lu[iregion-1]);
		      fwrite(&bscale,4,1,a->reg_lu[iregion-1]);
		      fwrite(&a0,4,1,a->reg_lu[iregion-1]);
		      fwrite(&b0,4,1,a->reg_lu[iregion-1]);
		      fwrite(&xdeg,4,1,a->reg_lu[iregion-1]);
		      fwrite(&ydeg,4,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      /* write region time and location information */
		      cnt=4*11+10;
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      fwrite(dstart,4,1,a->reg_lu[iregion-1]);
		      fwrite(dend,4,1,a->reg_lu[iregion-1]);
		      fwrite(mstart,4,1,a->reg_lu[iregion-1]);
		      fwrite(mend,4,1,a->reg_lu[iregion-1]);
		      fwrite(year,4,1,a->reg_lu[iregion-1]);
		      fwrite(&regnum,4,1,a->reg_lu[iregion-1]);
		      fwrite(&projt,4,1,a->reg_lu[iregion-1]); //
		      fwrite(&ipolar,4,1,a->reg_lu[iregion-1]);
		      fwrite(&latl,4,1,a->reg_lu[iregion-1]);
		      fwrite(&lonl,4,1,a->reg_lu[iregion-1]);
		      fwrite(&lath,4,1,a->reg_lu[iregion-1]);
		      fwrite(&lonh,4,1,a->reg_lu[iregion-1]);
		      fwrite(&regname,10,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      /* write region grd projection information */
		      cnt=4*10;
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      fwrite(&nsx2,4,1,a->reg_lu[iregion-1]);
		      fwrite(&nsy2,4,1,a->reg_lu[iregion-1]);
		      fwrite(&non_size_x,4,1,a->reg_lu[iregion-1]);
		      fwrite(&non_size_y,4,1,a->reg_lu[iregion-1]);
		      fwrite(&ascale2,4,1,a->reg_lu[iregion-1]);
		      fwrite(&bscale2,4,1,a->reg_lu[iregion-1]);
		      fwrite(&a02,4,1,a->reg_lu[iregion-1]);
		      fwrite(&b02,4,1,a->reg_lu[iregion-1]);
		      fwrite(&xdeg2,4,1,a->reg_lu[iregion-1]);
		      fwrite(&ydeg2,4,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      /* now add optional 100 char records
			 with tagged variable values */
		      cnt=100;	      
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Setup_program_name=%s version %f",prog_n,prog_v);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Input metafile=%s", mname );
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Setup output directory=%s", outpath );
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Beam_code=%d",ibeam);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," A_initialization=%f",a_init);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," A offset value=%f",a_offset);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Max_iterations=%d",nits);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin,"Reference_incidence_angle=%f",*angle_ref);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Response_threshold=%f",*response_threshold);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      if (*median_flag)
			sprintf(lin," Median_flag=T");
		      else
			sprintf(lin," Median_flag=F");
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      if (flatten)
			sprintf(lin," Flat_response=T");
		      else
			sprintf(lin," Flat_response=F");
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      if (HASAZIMUTHANGLE)
			sprintf(lin," Has_Azimuth_Angle=T");
		      else
			sprintf(lin," Has_Azimuth_Agnle=F");
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Max_Fill=%d",MAXFILL);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Response_Multiplier=%d",RESPONSEMULT);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Sensor=%s",sensor);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
		      for(z=0;z<100;z++)lin[z]=' ';
		      sprintf(lin," Base_resolution=%d", *base_resolution);
		      fwrite(lin,100,1,a->reg_lu[iregion-1]);
		      fwrite(&cnt,4,1,a->reg_lu[iregion-1]);

		    }

		    /* Now read output file names and write to file */
		    flag_files=1;
		    while(flag_files) {
		      fgets(line,sizeof(line),file_id);
		      no_trailing_blanks(line);
		      if (ferror(file_id)) {
			fprintf( stderr, "*** error reading meta file at product files \n");
			flag_files=0;
		      } else {
			if (strstr(line,"End_product_file_names") != NULL) {
			  flag_files=0;
			  /*  don't write out the End_header line until gsx info is inserted into header */
			} else 
			  if (flag_out) {
			    fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
			    for(z=0;z<100;z++)lin[z]=' ';
			    sprintf(lin,"%s",line);
			    fwrite(lin,100,1,a->reg_lu[iregion-1]);
			    fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
			  }
		      }
		    }
		    fprintf( stderr, "Done with setup header for image\n");

		    /* save region information */
		    if (flag_out) {
		      a->sav_lath[iregion-1]=lath;
		      a->sav_latl[iregion-1]=latl;
		      a->sav_lonh[iregion-1]=lonh;
		      a->sav_lonl[iregion-1]=lonl;
		      a->sav_nsx[iregion-1]=nsx;
		      a->sav_nsy[iregion-1]=nsy;
		      a->sav_ipolar[iregion-1]=ipolar;
		      a->sav_ibeam[iregion-1]=ibeam;
		      a->sav_projt[iregion-1]=projt;
		      a->sav_ascale[iregion-1]=ascale;
		      a->sav_bscale[iregion-1]=bscale;
		      a->sav_a0[iregion-1]=a0;
		      a->sav_b0[iregion-1]=b0;
		      a->sav_ydeg[iregion-1]=ydeg; //aorglat;
		      a->sav_xdeg[iregion-1]=xdeg; //aorglon;
		      strncpy(a->sav_fname2[iregion-1],fname2,180);
		      a->sav_regnum[iregion-1]=regnum;
		      strncpy(a->sav_regname[iregion-1],regname,10);
		      a->sav_dateline[iregion-1]=dateline;
		      a->sav_ascdes[iregion-1]=asc_des;
		      a->sav_tsplit1[iregion-1]=tsplit1;
		      a->sav_tsplit2[iregion-1]=tsplit2;
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

  /* override number of regions to the number of sections */
  a->nregions=iregion;  

  return(file_id);
}

void compute_locations(region_save *a, int *nregions, int **noffset, short int **latlon_store) 
{  
  /*
   *   compute the lat,lon of each pixel in the image regions and store in
   *  global arrays.  This reduces the computational load
   *  Additionally save the value of the previous projection and resolution
   *  which will save recalculating them if they're the same
   *  eg if 3 of the projections are N2ES at 6.25 km then you only need
   *  to calculate those positions once
   */

  int iregion, nspace;
  char *p, local[]="./";
  int iadd,ix,iy,nsize,iadd0;
  float x,y,clat,clon;
  int projection, resolution;
  int dumb;

  p=getenv("SIR_areas");
  if (p==NULL) p=local;

  *nregions=a->nregions;
  projection = 0;
  resolution = 0;

  /* determine how much memory is required */
  nspace=0;
  for (iregion=0; iregion < a->nregions; iregion++)
    nspace=nspace+2*a->sav_nsx[iregion]*a->sav_nsy[iregion];

  /* allocate memory for storage of location arrays */
  //  *noffset=malloc(sizeof(int)*(a->nregions+1));
  dumb = utils_allocate_clean_aligned_memory( (void**)&(*noffset), sizeof(int)*(a->nregions+1) );
  if ( 0 != dumb ) {
    fprintf( stderr, "*** Inadequate memory for data file storage 'noffset' \n" );
    exit ( -1 );
  }
  //*latlon_store=malloc(sizeof(short int)*nspace);
  dumb = utils_allocate_clean_aligned_memory( (void**)&(*latlon_store), sizeof(short int)*nspace );
  if ( 0 != dumb ) {
    fprintf(stderr, "*** pixel location buffer allocation error  %d %d\n",*nregions,nspace);
    exit(-1);
  }

  /* for each region stored in a */
  (*noffset)[0]=0;  /* intialize pointer */
  for (iregion=0; iregion < a->nregions; iregion++) {
    nsize=a->sav_nsx[iregion]*a->sav_nsy[iregion];
    
    (*noffset)[iregion+1] = (*noffset)[iregion]+2*nsize;  /* update pointer */

    fprintf( stderr, "\nRegion %d of %d: %dx%d=%d\n",iregion+1,a->nregions, 
	     a->sav_nsx[iregion], a->sav_nsy[iregion],nsize);
    print_projection(stdout, a->sav_projt[iregion], 
		    a->sav_xdeg[iregion], a->sav_ydeg[iregion],
		    a->sav_ascale[iregion], a->sav_bscale[iregion],
		    a->sav_a0[iregion],     a->sav_b0[iregion]);

    if (( projection == a->sav_projt[iregion] ) &&
	( resolution == a->sav_nsx[iregion] )) {

      /* save time and I/O re-use prior load or computation */
      for (iy=0; iy<a->sav_nsy[iregion]; iy++) {
	for (ix=0; ix<a->sav_nsx[iregion]; ix++) {	  
	  iadd=a->sav_nsx[iregion]*iy+ix; /* zero-based lexicographic pixel address */
	  iadd0=2*iadd+(*noffset)[iregion-1];
	  iadd=2*iadd+(*noffset)[iregion];	  
	  (*latlon_store)[iadd]=  (*latlon_store)[iadd0];	  
	  (*latlon_store)[iadd+1]=(*latlon_store)[iadd0+1];
	}
      }      

    } else {  /* different projection and resolution */

      /* reset values for projection and resolution */
      projection = a->sav_projt[iregion];
      resolution = a->sav_nsx[iregion];

      /* compute pixel locations */
      for (iy=0; iy<a->sav_nsy[iregion]; iy++) {
	y=(iy+1.5f); /* center of pixel, 1-based */
	for (ix=0; ix<a->sav_nsx[iregion]; ix++) {
	  x=(ix+1.5f); /* center of pixel, 1-based */
	  pixtolatlon(x, y, &clon, &clat, a->sav_projt[iregion], 
		      a->sav_xdeg[iregion], a->sav_ydeg[iregion],
		      a->sav_ascale[iregion], a->sav_bscale[iregion],
		      a->sav_a0[iregion],     a->sav_b0[iregion]);
	  iadd=a->sav_nsx[iregion]*iy+ix; /* zero-based lexicographic pixel address */
	  if (iadd<0) iadd=0;
	  if (iadd>=nsize) iadd=0;
	  iadd=2*iadd+(*noffset)[iregion];

	  (*latlon_store)[iadd] = (short int)nint(clat*200.f);
	  (*latlon_store)[iadd+1] = (short int)nint(clon*175.f);
	}
      }
    }    
  }
}

/* *********************************************************************** */

/* *********************************************************************** 
 * Compute an estimate of the ssmi beam response (weight) in normal space
 *     given a location (in km, N-E=(x,y)), azimuth angle (theta),
 *   3dB antenna pattern scale factor (bigang), and the footprint
 *   sizes in cross track and along track.  
 *
 *   inputs:
 *     x_rel,y_rel : relative offset from beam center in km
 *     theta : pattern rotation in deg
 *     thetai : incidence angle in deg (not used)
 *     semimajor : semi major axis in km
 *     semiminor : semi minor axis in km
 *
 *   Convert km location to coordinate system with axis
 *   lined up with the elliptical antenna pattern
 *
 *              The rotation matrix looks like this where theta is a
 *		CCW rotation of the input coordinates x,y
 *
 *		--  --   ---                      ---  --   --
 *		|    |   |                          |  |     |
 *		| x1 |   | cos(theta)   -sin(theta) |  |  x  |
 *		|    | = |                          |  |     |
 *		| y1 |   | sin(theta)    cos(theta) |  |  y  |
 *		|    |   |                          |  |     |
 *		-- --    ---                      ---  --   --
 *
 ************************************************************************/

float gsx_antenna_response(float x_rel, float y_rel, float theta, float semimajor, float semiminor)
{
  static float lnonehalf=-0.6931471;  /* ln(0.5) */
  float x, y, cross_beam_size, along_beam_size, t1, t2, weight;

  /* rotate coordinate system to align with look direction */
  x=(float) ( ( (cos(theta*DTR) ) * x_rel ) - ( (sin(theta*DTR) ) * y_rel ) );
  y=(float) ( ( (sin(theta*DTR) ) * x_rel ) + ( (cos(theta*DTR) ) * y_rel ) );
  
  /* compute approximate antenna response
     Antenna weighting is estimation from SSMI Users Guide 21-27 */
  along_beam_size=semimajor;
  cross_beam_size=semiminor;
  t1=2*x/cross_beam_size;
  t2=2*y/along_beam_size;
  
  weight=expf((t1*t1+t2*t2)*lnonehalf);
   
  return(weight);
}

/* *********************************************************************** */

void rel_latlon(float *x_rel, float *y_rel, float alon, float alat, float rlon, float rlat)
{
  /*  compute relative x,y in km of alat,alon with respect to rlat,rlon
      using a locally-tangent plane approximation
      x is aligned East while y is aligned North

      a fancier map projection could be used, but high precision is 
      not really required for this calculation

      written: DGL  1/12/99
  */

  float r,r2,rel_rlat,rel_rlon;

  r=(float) ( ( 1.0 - ( ( (sin(rlat*DTR) ) * (sin(rlat*DTR) ) ) * FLAT ) ) * AEARTH );

  rel_rlat=alat-rlat;
  rel_rlon=alon-rlon;
  if (abs(rel_rlon) > 180.0) {
    if (rel_rlon > 0.0)
      rel_rlon=rel_rlon-360.f;
    else
      rel_rlon=rel_rlon+360.f;
  }
  r2=r*(float)(cos(rlat*DTR));
  *x_rel=(float)(r2*(sin(rel_rlon*DTR)));
  *y_rel=(float)((r*(sin(rel_rlat*DTR)))+((1.-(cos(rel_rlon*DTR)))*(sin(rlat*DTR))*r2));
}

/* *********************************************************************** */

float km2pix(float *x, float *y, int iopt, float ascale, float bscale, int *stat)
{ 
  /*
    determine the approximate "nominal" conversion coefficients for
    converting a distance in km to pixels.  since this is typically
    variable over the projection, the returned scale factors r,x,y
    correspond to the reference point/plane (or center) of the
    transformation and are only approximate.  This approximate
    computation is useful primarily for defining a bounding box
    in which to do more precise computation.
    
    written by:	dgl 6 feb 1995
    revised by: dgl 2 Feb 2014 + converted to c
    revised by: dgl 7 Mar 2014 + added EASE2

    given the projection parameters (xdeg..iopt) returns the (approximate)
    nominal scale factors x,y in pixels/km where x is horizontal and
    y is vertical.  function output r is the rms of x and y.

    stat is a returned status variable - set to 1 upon success and to 0 upon failure
    a failure will terminate the application upon returning to the main{}

  */

  float r=0.0;

  double map_equatorial_radius_m,map_eccentricity, e2,
    map_reference_latitude, map_reference_longitude, 
    map_second_reference_latitude, sin_phi1, cos_phi1, kz,
    map_scale, r0, s0, epsilon;
  int bcols, brows, nease, ind;

  *stat = 1;  /* success return is the default */
  
  switch(iopt) {
  case  8: /* EASE2 N */
  case  9: /* EASE2 S */
  case 10: /* EASE2 T */
    nease=ascale;
    ind=bscale;    
    ease2_map_info(iopt, nease, ind, &map_equatorial_radius_m, 
		   &map_eccentricity, &e2,
		   &map_reference_latitude, &map_reference_longitude, 
		   &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		   &map_scale, &bcols, &brows, &r0, &s0, &epsilon);
    *x=(1.f/(float)(map_scale*0.001)); /* km/pixel rather than m/pixel */
    *y=(1.f/(float)(map_scale*0.001));
    r= (1.f/(float)(map_scale*0.001));
    break;
  default: /* unknown transformation type */
    *x=0.0;
    *y=0.0;
    *stat = 0;
    fprintf( stderr, "%s: Unknown transformation type - %d region id\n", __FUNCTION__, iopt );
  }
  return(r);
}

void print_projection(FILE *omf, int iopt, float xdeg, float ydeg, 
		      float ascale, float bscale, float a0, float b0)
{ 
  /* print standard projection information */

  switch(iopt) {
   case  8:
   case  9:
     fprintf(omf,"  EASE2 polar azimuthal form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Scale factor, ind:    %f , %f\n",ascale,bscale);
     fprintf(omf,"   Map origin (col,row): %f , %f\n",a0,b0);
     break;

   case 10:
     fprintf(omf,"  EASE2 cylindrical form: \n");
     fprintf(omf,"   Map center (col,row): %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Scale factor, ind:    %f , %f\n",ascale,bscale);
     fprintf(omf,"   Map origin (col,row): %f , %f\n",a0,b0);
     break;

   default:
     fprintf(omf,"  Unrecognized SIR file option: \n");
     fprintf(omf,"   Xspan,  Yspan:  %f , %f\n",xdeg,ydeg);
     fprintf(omf,"   Xscale, Yscale: %f , %f\n",ascale,bscale);
     fprintf(omf,"   Xorg,   Yorg:   %f , %f\n",a0,b0);
     break;
  }

  return;
}

/* *********************************************************************** */

/* ***********************************************************************
 * timedecode - convert an epoch time value to Gregorian and day-of-year
 *    equivalent
 *
 *  Input:
 *    epochTime - double, epoch time to convert
 *    epochUnits - pointer to ut_units, epoch information
 *    calendar - pointer to calcalcs_cal calendar information
 *
 *  Output:
 *    year - integer, year
 *    doy - integer, day of year
 *    month - integer month
 *    day - integer, day of month
 *    hour - integer, hour
 *    minute - integer, minutes
 *    second - double, seconds
 *
 *  Result:
 *    status variable indicates success (0) or failure (1)
 *
 *  N.B. The originial timedecode function was truncating fractions
 *  of seconds.
 */
static int timedecode(double epochTime,
		      ut_unit *epochUnits, calcalcs_cal *calendar,
		      int *year, int *doy, int *month, int *day,
		      int *hour, int *minute, double *second) {

  /* epoch time to Gregorian */
  if ( 0 != utCalendar2_cal( epochTime, epochUnits,
			     year, month, day, hour, minute, second,
			     calendar->name ) ) {
    fprintf( stderr, "%s: unable to convert epochTime=%.3lf to Gregorian\n",
	     __FUNCTION__, epochTime );
    return 1;
  }

  /* also get day of year */
  if ( 0 != ccs_date2doy( calendar, *year, *month, *day, doy ) ) {
    fprintf( stderr, "%s: unable to convert %4d-%02d-%02d to day of year\n",
	     __FUNCTION__, *year, *month, *day );
    return 1;
  }

  return 0;

}
 

/*
 * box_size_by_channel - returns the box size to use based on the channel and sensor
 *
 * input :
 *   ibeam : channel number
 *   sensor : short_sensor id
 *   base resolution : CETB_BASE_[25/24/36]_RESOLUTION
 *
 * output :
 *   box size is returned as an argument
 *
 * result :
 *   Status is -1 in case of failure or zero if OK
 *
 *
 * a discussion of the process by which the optimum box_size was chosen can be
 * found in the project on bitbucket.org in the directory docs/internal
 *
 * a discussion document as well as a spread sheet with the data used to make the
 * box size determination are located there
 *
 */
int box_size_by_channel( int ibeam, cetb_sensor_id id, int base_resolution, int *box_size ) {

  if ( CETB_SSMI == id ) {
    
    switch ( cetb_ibeam_to_cetb_ssmi_channel[ibeam] ) {
    case SSMI_19H:
    case SSMI_19V:
    case SSMI_22V:
      *box_size = 100;
      break;
    case SSMI_37H:
    case SSMI_37V:
      *box_size = 60;
      break;
    case SSMI_85H:
    case SSMI_85V:
      *box_size = 20;
      break;
    default:
      *box_size = -1;
      fprintf( stderr, "%s: bad channel number %d\n", __FUNCTION__, ibeam );
    }
  } else if ( CETB_AMSRE == id ) {
    switch ( cetb_ibeam_to_cetb_amsre_channel[ibeam] ) {
    case AMSRE_06H:
    case AMSRE_06V:
      *box_size = 24;
      break;
    case AMSRE_10H:
    case AMSRE_10V:
      *box_size = 20;
      break;
    case AMSRE_18H:
    case AMSRE_18V:
      *box_size = 22;
      break;
    case AMSRE_23H:
    case AMSRE_23V:
      *box_size = 26;
      break;
    case AMSRE_36H:
    case AMSRE_36V:
      *box_size = 22;
      break;
    case AMSRE_89H_A:
    case AMSRE_89V_A:
      *box_size = 10;
      break;
    case AMSRE_89H_B:
    case AMSRE_89V_B:
      *box_size = 12;
      break;
    default:
      *box_size = -1;
      fprintf( stderr, "%s: bad channel number %d\n", __FUNCTION__, ibeam );
    }
  } else if ( CETB_SSMIS == id ) {
    switch ( cetb_ibeam_to_cetb_ssmis_channel[ibeam] ) {
    case SSMIS_19H:
    case SSMIS_19V:
    case SSMIS_22V:
      *box_size = 100;
      break;
    case SSMIS_37H:
    case SSMIS_37V:
      *box_size = 60;
      break;
    case SSMIS_91H:
    case SSMIS_91V:
      *box_size = 20;
      break;
    default:
      *box_size = -1;
      fprintf( stderr, "%s: bad channel number %d\n", __FUNCTION__, ibeam );
    }
  } else if ( CETB_SMMR == id ) {
    switch ( cetb_ibeam_to_cetb_smmr_channel[ibeam] ) {
    case SMMR_06H:
    case SMMR_06V:
      *box_size = 120;
      break;
    case SMMR_10H:
    case SMMR_10V:
      *box_size = 100;
      break;
    case SMMR_18H:
    case SMMR_18V:
      *box_size = 100;
      break;
    case SMMR_21H:
    case SMMR_21V:
      *box_size = 100;
      break;
    case SMMR_37H:
    case SMMR_37V:
      *box_size = 60;
      break;
    default:
      *box_size = -1;
      fprintf( stderr, "%s: bad channel number %d\n", __FUNCTION__, ibeam );
    }
  } else if ( CETB_SMAP_RADIOMETER == id ) {
    if ( CETB_BASE_36_RESOLUTION == base_resolution ) {
      *box_size = 28;
    } else {
      *box_size = 40;
    }
  } else if ( CETB_AMSR2 == id ) {
    switch ( cetb_ibeam_to_cetb_amsr2_channel[ibeam] ) {
    case AMSR2_10H:
    case AMSR2_10V:
      *box_size = 20;
      break;
    case AMSR2_18H:
    case AMSR2_18V:
      *box_size = 22;
      break;
    case AMSR2_23H:
    case AMSR2_23V:
      *box_size = 26;
      break;
    case AMSR2_36H:
    case AMSR2_36V:
      *box_size = 24;
      break;
    case AMSR2_89H_A:
    case AMSR2_89V_A:
      *box_size = 20;
      break;
    case AMSR2_89H_B:
    case AMSR2_89V_B:
      *box_size = 20;
      break;
    default:
      *box_size = -1;
      fprintf( stderr, "%s: bad channel number %d\n", __FUNCTION__, ibeam );
    }
  } else {
    *box_size = -1;
    fprintf( stderr, "%s: bad sensor id %d box size not known\n", __FUNCTION__, id );
  }

  if ( -1 == *box_size ) {
    return (-1);
  } else {
    return (0);
  }
      
}

/*
 * write_header_info - writes out the info required for CETB files
 *
 * Input:
 *   gsx - pointer to gsx_class struct
 *   save_area - contains info on open output setup files
 *   year - the year of the date being processed - used to determine ltod times
 *
 * Return:
 *   none
 *
 */
int write_header_info( gsx_class *gsx, region_save *save_area, int year ) {
  int cnt=100;
  char lin[100];
  int z;
  int iregion, status;
  float ltod_morning, ltod_evening;
  
  /* Writing out CETB required information to setup file */
  if ( gsx != NULL ) {
    for ( iregion=1; iregion<=save_area->nregions; iregion++ ) {
      fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
      for(z=0;z<100;z++)lin[z]=' ';
      sprintf(lin," Platform_id=%d ", gsx->short_platform);
      fwrite(lin,100,1,save_area->reg_lu[iregion-1]);
      fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);

      fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
      for(z=0;z<100;z++)lin[z]=' ';
      sprintf(lin," Sensor_id=%d ", gsx->short_sensor);
      fwrite(lin,100,1,save_area->reg_lu[iregion-1]);
      fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);

      fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
      for(z=0;z<100;z++)lin[z]=' ';
      sprintf(lin," Producer_id=%d ", gsx->input_provider);
      fwrite(lin,100,1,save_area->reg_lu[iregion-1]);
      fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);

      fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
      for(z=0;z<100;z++)lin[z]=' ';
      sprintf(lin," Pass_direction=%d ", save_area->sav_ascdes[iregion-1]);
      fwrite(lin,100,1,save_area->reg_lu[iregion-1]);
      fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);

      if ( ((save_area->sav_regnum[iregion-1]-CETB_PROJECTION_BASE_NUMBER) !=
	    (int)CETB_EASE2_T ) &&
	   ((save_area->sav_regnum[iregion-1]-CETB_PROJECTION_BASE_NUMBER) !=
	    (int)CETB_EASE2_M36) &&
	   ((save_area->sav_regnum[iregion-1]-CETB_PROJECTION_BASE_NUMBER) !=
	    (int)CETB_EASE2_M24) ) {
	fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
	for(z=0;z<100;z++)lin[z]=' ';
	status = ltod_split_time(gsx->short_platform,
				 (cetb_region_id)
				 (save_area->sav_regnum[iregion-1]-CETB_PROJECTION_BASE_NUMBER),
				 CETB_MORNING_PASSES, year, &ltod_morning);
	if ( status != 0 ) {
	  ltod_morning = -1.;
	}
	sprintf(lin," Ltod_morning=%f ", ltod_morning);
	fwrite(lin,100,1,save_area->reg_lu[iregion-1]);
	fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);

	fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
	for(z=0;z<100;z++)lin[z]=' ';
	status = ltod_split_time(gsx->short_platform,
				 (cetb_region_id)
				 (save_area->sav_regnum[iregion-1]-CETB_PROJECTION_BASE_NUMBER),
				 CETB_EVENING_PASSES, year, &ltod_evening);
	if ( status != 0 ) {
	  ltod_evening = -1.;
	}
	sprintf(lin," Ltod_evening=%f ", ltod_evening);
	fwrite(lin,100,1,save_area->reg_lu[iregion-1]);
	fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
      }

    }
  } else {
    return (-1);
  }
  return (0);
}

/* write_filenames_to_header - writes out the input data files that were used to
 * create the setup files as well as the GSX version used in the processing
 *
 * Input:
 *   gsx structure - holds the filename and the version
 *   save_area - contains the information on the open output setup files
 *   file_flag - keeps track of whether or not a filename was used for that projection
 *   position_filename - keeps track of the place to write the next filename in each region's setup file
 *   position_data - keeps track of the place from which to continue writing out data in each region's setup file
 *
 * Return:
 *   status - 0 on success
 */
int write_filenames_to_header( gsx_class *gsx, region_save *save_area, int *file_flag,
			       unsigned long *position_filename, unsigned long *position_data ) {
  int cnt=100;
  char lin[100];
  int z;
  int iregion;

  for ( iregion=0; iregion<save_area->nregions; iregion++ ) {
    if ( 1 == *(file_flag+iregion) ) { //this file has been used for the setup file
      *(position_data+iregion) = ftell( save_area->reg_lu[iregion]);
      fseek( save_area->reg_lu[iregion], *(position_filename+iregion), SEEK_SET );
      fwrite(&cnt,4,1,save_area->reg_lu[iregion]); 
      for(z=0;z<100;z++)lin[z]=' '; 
      sprintf(lin," Input_file=%s (GSX_version:%s)", gsx->source_file, gsx->gsx_version);
      fwrite(lin,100,1,save_area->reg_lu[iregion]); 
      fwrite(&cnt,4,1,save_area->reg_lu[iregion]);
      *(position_filename+iregion) = ftell( save_area->reg_lu[iregion]);
      fseek( save_area->reg_lu[iregion], *(position_data+iregion), SEEK_SET );
    }
  }
  return (0);
}

/* write_blanklines_to_header - writes a blank line for each filename in the metafile
 * during processing we loop through each file and each output region and only then go back
 * to write out the filename if we use measurements from the file.
 *
 * Input:
 *   gsx structure - holds the filename and the version
 *   save_area - contains the information on the open output setup files
 *
 * Return:
 *   0 on success, 1 on failure
 */
int write_blanklines_to_header( region_save *save_area ) {
  int cnt=100;
  char lin[100];
  int z;
  int iregion;

  for ( iregion=0; iregion<save_area->nregions; iregion++ ) { 
     fwrite(&cnt,4,1,save_area->reg_lu[iregion]); 
     for(z=0;z<100;z++)lin[z]=' '; 
     fwrite(lin,100,1,save_area->reg_lu[iregion]); 
     fwrite(&cnt,4,1,save_area->reg_lu[iregion]);
   }
  return (0);
}
/*
 * write_end_header - writes out the End_file line that is used in sir and bgi
 *
 * Input:
 *   save_area - contains info on open output setup files
 *
 * Return:
 *   none
 *
 */
int write_end_header( region_save *save_area ){
  int cnt=100;
  char lin[100];
  int z;
  int iregion;

  for ( iregion=1; iregion<=save_area->nregions; iregion++ ) {
    fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
    for(z=0;z<100;z++)lin[z]=' ';
    sprintf(lin," End_header");
    fwrite(lin,100,1,save_area->reg_lu[iregion-1]);
    fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
  }
  return (0);
}

/*
 * combine_setup_files - called if we have to combine channels into a single output setup file
 *
 * Input:
 *   path to setup file to be deleted or combined
 *   region_save pointer holds the file-ids to be manipulated
 *   execution flag tells what operation should be performed
 *   The ibeam to channel array for decoding the region_save pointer - cast to int
 *   The enum for the channels to be checked - cast to int
 *
 */
void combine_setup_files( char *outpath, region_save *a, int execution_flag, int *ibeam_channel, int chanA, int chanB ) {

  int count;
  int sub_count;
  char outname[350];
  
  /* Here is where you check to see if both AMSRE/AMSR2 89 a and b scans are requested, if so
     they need to be written into only 1 output setup file, i.e. 89Ha and 89Hb go into the same file
     and 89Va and 89Vb go into the same file.  The check needs to be done here so that you don't
     have to rely on the regions in the file going in a specific order - also note that this only works
     in the first place if you put all of the 89 channels into the same def file
     Use the save_area (a in this routine) structure to check for a and b scans if AMSRE/AMSR2
     Note - call the function separately for H and V channels
  */
  fprintf( stderr, "%s: into manipulating fileids with %d execution flag\n", __FUNCTION__, execution_flag );
    for ( count=0; count < a->nregions; count++ ) {
      /* now check to see if you have b channels for 89H or 89V and if you also have A channels
	 then combine if they use the same projection */
      if ( ibeam_channel[a->sav_ibeam[count]] == chanB ) {
	for ( sub_count=0; sub_count < a->nregions; sub_count++ ) {
	  if ( ( ibeam_channel[a->sav_ibeam[sub_count]] == chanA )
	       && ( a->sav_regnum[sub_count] == a->sav_regnum[count] )
	       && ( a->sav_ascdes[sub_count] == a->sav_ascdes[count] ) ) {
	    /* depending on the execution_flag either
	       - close and delete the file that won't be used and save file id
	         for the setup file for AMSRE_89H_A to the file id for
		 AMSRE_89H_B or
	       - set the file id to NULL */
	    if ( execution_flag == 1 ) {
	      fclose( a->reg_lu[count] );
	      sprintf( outname, "%s/%s", outpath, a->sav_fname2[count] );
	      if ( remove( outname ) == 0 ) {
		fprintf( stderr, "%s: %s successfully closed\n", __FUNCTION__,
			 a->sav_fname2[count] );
	      } else {
		fprintf( stderr, "%s: %s NOT closed\n", __FUNCTION__,
			 a->sav_fname2[count] );
	      }
	      a->reg_lu[count] = a->reg_lu[sub_count];
	      fprintf( stderr, "%s: closed region file count %s in favor of "
		       "sub_count %s\n", __FUNCTION__,
		       a->sav_fname2[count], a->sav_fname2[sub_count] );
	    }
	    if ( execution_flag == 2 ) {
	      a->reg_lu[count] = NULL;
	    }
	  }
	}
      }
    }
}

/* *********************************************************************** */
/* ltod_split_time - used to look up the ltod split times by platform,
 * hemisphere and year
 *
 *  Input:
 *    platform_id - from cetb.h is CETB_F08, CETB_AMSRE etc
 *    region_id - from cetb.h is CETB_EASE2_N or _S or _T
 *    direction_id - from cetb.h is CETB_MORNING_PASSES etc
 *
 *  Output:
 *    status variable indicating success (0) or failure (1)
 *    ltod start or end time in decimal UTC hours (0.0 - 24.0)
 *         - note that there are special cases where ltod times
 *           might start before midnight - see eg F15, 2012-2016
 *
 *  Method:
 *    LTOD = Local Time of Day and is the method by which measurements are
 *           separated into morning and evening images for the N and S
 *           projections
 *
 *   The actual times used in this function are derived from the
 *   Jupyter notebook LTOD calculations.ipynb that is in the
 *   ipython directory of this project
 *
 */
static int ltod_split_time( cetb_platform_id platform_id, cetb_region_id region_id,
			    cetb_direction_id direction_id, int year, float *split_time ) {

  int negative_flag;
  float cetb_ltod_split_times[CETB_NUM_PLATFORMS][2][2] = {
    { {-6.0, 6.0}, {-6.0, 6.0} }, /* CETB_NIMBUS7 platform, N or S projection */
    { {-5.0, 7.0}, {-4.0, 8.0} }, /* CETB_AQUA platform, N or S projection */
    { {0.0, 12.0}, {0.0, 12.0} }, /* CETB_F08 platform,  N or S projection */
    { {2.0, 14.0}, {2.0, 14.0} }, /* CETB_F10 platform,
				     1990-1993,
				     1994 + 1 hour,
				     1995-1997 + 2 hours, N or S projection */
    { {0.0, 12.0}, {0.0, 12.0} }, /* CETB_F11 platform, N or S projection */
    { {0.0, 12.0}, {0.0, 12.0} }, /* CETB_F13 platform, N or S projection */
    { {3.0, 15.0}, {3.0, 15.0} }, /* CETB_F14 platform, N or S projection */
    { {3.0, 15.0}, {3.0, 15.0} }, /* CETB_F15 platform, N or S projection */
    { {3.0, 15.0}, {3.0, 15.0} }, /* CETB_F16 platform,
				     2005-2007,
				     2008-2009 -1 hour,
				     2010-2011 -1 hour,
				     2012-2013 -1 hour,
				     2014      -1 hour,
				     2015-2017 -1 hour, N or S projection */
    { {0.0, 12.0}, {0.0, 12.0} }, /* CETB_F17 platform, N or S projection */
    { {0.0, 12.0}, {0.0, 12.0} }, /* CETB_F18 platform, N or S projection */
    { {0.0, 12.0}, {0.0, 12.0} }, /* CETB_F19 platform, N or S projection */
    { {0.0, 12.0}, {0.0, 12.0} }, /* CETB_SMAP platform, N or S projection */
    { {-4.0, 8.0}, {-4.0, 8.0} }  /* CETB_GCOMW1 platform, N or S projection */
  };

  /* note that the degenerative case of the satellite/year
     combination for ltod not being set results in the split time
     being set to -1.0 However, there are some cases where,
     because of drift of the equator crossing time, the ltod
     start time needs to be set to hours before midnight, i.e. a
     negative value.  In those cases, the negative_flag is set to
     1 and the negative ltod time is NOT flagged as an error
  */
  if ( platform_id == CETB_NIMBUS7 || platform_id == CETB_AQUA ||
       platform_id == CETB_GCOMW1 ) {
    negative_flag = 1;
  } else {
    /*
     * the remaining cases for this also depend on year and hemisphere,
     * and will be handled in the following case statement
     */
    negative_flag = 0;
  }

  if ( region_id == CETB_EASE2_T ||
       region_id == CETB_EASE2_M36 ||
       region_id == CETB_EASE2_M24 ) {
    *split_time = -1.0;
    return (0);
  }

  if ( platform_id == CETB_F10 ) {
    switch ( year ) {
    case 1990:
    case 1991:
    case 1992:
    case 1993:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 2.0;
      } else {
	*split_time = 14.0;
      }
      break;
    case 1994:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 3.0;
      } else {
	*split_time = 15.0;
      }
      break;
    case 1995:
    case 1996:
    case 1997:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 4.0;
      } else {
	*split_time = 16.0;
      }
      break;
    default:
      *split_time = -1;
    }
  } else if ( platform_id == CETB_F14 ) {
    switch ( year ) {
    case 1997:
    case 1998:
    case 1999:
    case 2000:
    case 2001:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 3.0;
      } else {
	*split_time = 15.0;
      }
      break;
    case 2002:
    case 2003:
    case 2004:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 2.0;
      } else {
	*split_time = 14.0;
      }
      break;
    case 2005:
    case 2006:
    case 2007:
    case 2008:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 0.0;
      } else {
	*split_time = 12.0;
      }
      break;
    default:
      *split_time = -1;
    }
  } else if ( platform_id == CETB_F15 ) {
    switch ( year ) {
    case 2000:
    case 2001:
    case 2002:
    case 2003:
    case 2004:
    case 2005:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 3.0;
      } else {
	*split_time = 15.0;
      }
      break;
    case 2006:
    case 2007:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 2.0;
      } else {
	*split_time = 14.0;
      }
      break;
    case 2008:
    case 2009:
    case 2010:
    case 2011:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 0.0;
      } else {
	*split_time = 12.0;
      }
      break;
    case 2012:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = -2.0; 
      } else {
	*split_time = 10.0; 
      }
      negative_flag = 1;
      break;
    case 2013:
    case 2014:
    case 2015:
    case 2016:
    case 2017:
    case 2018:
    case 2019:
    case 2020:
    case 2021:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = -3.0;
      } else {
	*split_time = 9.0;
      }
      negative_flag = 1;
      break;
    default:
      *split_time = -1;
    }
  } else if ( platform_id == CETB_F16 ) {
    switch ( year ) {
    case 2005:
    case 2006:
    case 2007:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 3.0;
      } else {
	*split_time = 15.0;
      }
      break;
    case 2008:
    case 2009:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 2.0;
      } else {
	*split_time = 14.0;
      }
      break;
    case 2010:
    case 2011:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 1.0;
      } else {
	*split_time = 13.0;
      }
      break;
    case 2012:
    case 2013:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = 0.0;
      } else {
	*split_time = 12.0;
      }
      break;
    case 2014:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = -1.0;
      } else {
	*split_time = 11.0;
      }
      negative_flag = 1;
      break;
    case 2015:
    case 2016:
    case 2017:
    case 2018:
    case 2019:
    case 2020:
    case 2021:
    case 2022:
      if ( direction_id == CETB_MORNING_PASSES ) {
	*split_time = -2.0;
      } else {
	*split_time = 10.0;
      }
      negative_flag = 1;
      break;
    default:
      *split_time = -1;
    }
  } else {
    *split_time =
      cetb_ltod_split_times[platform_id][region_id][direction_id-CETB_MORNING_PASSES];
  }

  if ( ((*split_time + 1.0) < FLT_EPSILON) && (negative_flag == 0) ) {
    fprintf( stderr, "%s: ERROR: Bad satellite, year combination for platform %d and year %d\n",
	     __FUNCTION__, platform_id, year );
    return (1);
  } else {
    return (0);
  }

} 

/* ***********************************************************************
 * get_search_period - get the search period window we are looking for,
 *    relative to the start year, day-of-year and minutes-of-day
 *    from the meta file; returns the search period (start/end) and
 *    the image date, relative to the requested epoch time
 *
 *  Input:  Note that these inputs come from the input metafile read in get_meta
 *    year - integer, year for target start
 *    dstart - integer, day for target start
 *    dend - integer, end day for period
 *    mstart - integer, minutes of day for target start
 *    epochUnits - pointer to ut_units, epoch information
 *    calendar - pointer to calcalcs_cal calendar information
 *
 *  Output:
 *    startEpochTime - double, begin of search time in requested epochUnits
 *    imageEpochTime - double, begin of image time in requested epochUnits
 *    endEpochTime - double, end of search time in requested epochUnits
 *
 *  Result:
 *    status variable indicates success (0) or failure (1)
 * 
 *  Method:
 *    if LTOD, set search span to target day +/- 1 full day
 *    else, set search span to target day
 */
static int get_search_period( int year, int dstart, int dend, int mstart,
			      ut_unit *epochUnits, calcalcs_cal *calendar,
			      double *startEpochTime, double *imageEpochTime,
			      double *endEpochTime) {

  int month;
  int day;
  int hour = 0;
  double second = 0.0;
  int startDayOffset, imageDayOffset, endDayOffset;
  int midDay, startYear, imageYear, endYear;
  int startMonth, imageMonth, endMonth;
  int startDay, imageDay, endDay;


  /*
   * Set search start and end to 1 day on either side of image date
   * depending on the range of days required
   */
  if ( 0 != ltod_day_offset( dstart, dend, &midDay, &startDayOffset,
			     &endDayOffset, &imageDayOffset ) ) {
    fprintf( stderr, "%s: Error getting offset days\n", __FUNCTION__ );
    return 1;
  }
  
  /* Convert yyyydoy to yyyymmdd */
  if ( 0 != ccs_doy2date( calendar, year, midDay, &month, &day) ) {
    fprintf( stderr, "%s: Error converting yyyydoy=%4d%03d to yyyymmdd\n",
	     __FUNCTION__, year, dstart );
    return 1;
  }
  /* Get search start, relative to requested epoch time */
  if ( 0 != day_offset_from( year, month, day, 
			     hour, mstart, second,
			     startDayOffset,
			     epochUnits, calendar,
			     &startYear, &startMonth, &startDay,
			     startEpochTime ) ) {
    return 1;
  }

  /*
   * Get image start, relative to requested epoch time
   */
  if ( 0 != day_offset_from( year, month, day, 
			     hour, mstart, second,
			     imageDayOffset,
			     epochUnits, calendar,
			     &imageYear, &imageMonth, &imageDay,
			     imageEpochTime ) ) {
    return 1;
  }

  /* Get search end, relative to requested epoch time */
  if ( 0 != day_offset_from( year, month, day, 
			     hour, mstart, second,
			     endDayOffset,
			     epochUnits, calendar,
			     &endYear, &endMonth, &endDay,
			     endEpochTime ) ) {
    return 1;
  }

  fprintf( stderr, "%s: search start: %.3lf %4d-%02d-%02d\n",
	   __FILE__, *startEpochTime, startYear, startMonth, startDay );
  fprintf( stderr, "%s: image  start: %.3lf %4d-%02d-%02d\n",
	   __FILE__, *imageEpochTime, imageYear, imageMonth, imageDay );
  fprintf( stderr, "%s: search   end: %.3lf %4d-%02d-%02d\n",
	   __FILE__, *endEpochTime, endYear, endMonth, endDay );
  
  return 0;
  
}

/* ***********************************************************************
 * ltod_day_offset - given the start and end days, calculates the search
 *    offset days - used in determining the range for input scans and
 *    also the ctime ltod calculation when doing multi-day processing
 *
 *  Input:
 *    dstart - integer, start doy
 *    dend - integer, end doy
 *
 *  Output:
 *    midDay - integer, doy in middle of range
 *    start doy offset
 *    end doy offset
 *    image doy offset
 *
 */
static int ltod_day_offset( int dstart, int dend, int *midDay,
			    int *startDayOffset, int *endDayOffset,
			    int *imageDayOffset ) {

  *midDay = dstart + round((dend-dstart)/2);
  
  if ( dstart == dend ) {
    *startDayOffset = -1;
    *imageDayOffset = 0;
    *endDayOffset = 2;
  } else {
    *startDayOffset = dstart - *midDay - 1;
    *imageDayOffset = 0;
    *endDayOffset = dend - *midDay + 1;
  }
  return 0;
}

/* ***********************************************************************
 * day_offset_from - given a starting date/time and an integer day offset,
 *    calculates the offset time relative to the requested epoch
 *
 *  Input:
 *    year - integer, year
 *    month - integer, month
 *    day - integer, day
 *    hour - integer, hour
 *    minute - integer, minutes
 *    second - integer, seconds
 *    dayOffset - integer, offset in days away from input data
 *    epochUnits - pointer to ut_units, epoch information
 *    calendar - pointer to calcalcs_cal calendar information
 *
 *  Output:
 *    offsetYear - integer, offset year
 *    offsetMonth - integer, offset month
 *    offsetDay - integer, offset day
 *    offsetEpochTime - double, the offset date, in requested epochUnits
 *
 *  Result:
 *    status variable indicates success (0) or failure (1)
 * 
 */
static int day_offset_from( int year, int month, int day, 
			    int hour, int minute, double second,
			    int dayOffset, ut_unit *epochUnits,
			    calcalcs_cal *calendar,
			    int *offsetYear, int *offsetMonth,
			    int *offsetDay,
			    double *offsetEpochTime) {

  if ( 0 != ccs_dayssince( calendar, year, month, day, dayOffset,
			   calendar, offsetYear, offsetMonth,
			   offsetDay) ) {
    fprintf( stderr,
	     "%s: Error dayssince yyyymmdd=%4d%02d%02d for offset=%d\n",
	     __FUNCTION__, year, month, day, dayOffset );
    return 1;
  }
    
  if ( 0 != utInvCalendar2_cal( *offsetYear, *offsetMonth, *offsetDay,
				hour, minute, second, epochUnits,
				offsetEpochTime, calendar->name) ) {
    fprintf( stderr, "%s: Error converting yyyymmdd=%4d%02d%02d to epoch\n",
	     __FUNCTION__, *offsetYear, *offsetMonth, *offsetDay );
    return 1;
  }

  return 0;

}

/* ***********************************************************************
 * check_for_consistent_regions - setup logic depends on processing
 * all regions of the same LTOD-type (so NS regions or T regions, but
 * not both in the same setup execution).  Check the regions that
 * have just been read from the meta file and quit if they are not all
 * the same type.
 * If the regions are not consistent, close and delete the setup files
 * 
 *  Input:
 *    save_area - pointer to region information
 *
 *  Output:
 *    ltdflag - flag indicating the region type
 *
 *  Result:
 *    status variable indicates success (0) or failure (1)
 * 
 */
static int check_for_consistent_regions( region_save *save_area,
					 setup_ltod_flag *ltdflag ) {

  int i;
  setup_ltod_flag last, next;
  int consistent_flag;

  consistent_flag = 1;

  last = UNKNOWN_LTOD;
  for ( i=0; i<save_area->nregions; i++ ) {
    if ( cetb_region_number[0][CETB_EASE2_N] == save_area->sav_regnum[i]
	 || cetb_region_number[0][CETB_EASE2_S] == save_area->sav_regnum[i] ) {
      next = LTOD;
    } else if ( cetb_region_number[0][CETB_EASE2_T] == save_area->sav_regnum[i] ) {
      next = ASCDES;
    }

    if ( 0 == i ) {
      last = next;
    } else if ( last != next ) {
      consistent_flag = 0;
      break;
    } /* end first time through region loop */
  } /* end region loop */

  if ( consistent_flag ) {
    
    *ltdflag = last;
    
  } else {

    /*
     * Clean up the bogus output files and return error exit
     */
    for ( i=0; i<save_area->nregions; i++ ) {
      no_trailing_blanks(save_area->sav_fname2[i] );
      fprintf( stderr, "%s: Closing and deleting incomplete %s\n",
	       __FILE__, save_area->sav_fname2[i] );
      fclose( save_area->reg_lu[i] );
      remove( save_area->sav_fname2[i] );
    } /* end of region loop */

    *ltdflag = UNKNOWN_LTOD;
    return 1;

  }
     
  return 0;
  
}
  
