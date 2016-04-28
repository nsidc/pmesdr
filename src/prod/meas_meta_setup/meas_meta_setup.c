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

#include "cetb.h"
#include "utils.h"
#include "gsx.h"
#include "utils.h"
#include <sir3.h>

#define prog_version 0.3 /* program version */
#define prog_name "meas_meta_setup"

/* This code can read and process several different data sets.
 * In order to achieve that, the input source files must first be processed into GSX files
 */

#define MAX_INPUT_FILES 100 /* maximum number of input files to process onto a single grid */
#define NSAVE 50            /* maximum number of regions to output */
#define MAXFILL 2000        /* maximum number of pixels in response function */
#define RESPONSEMULT 1000   /* response multiplier */
#define HASAZIMUTHANGLE 1   /* include azimuth angle in output setup file if 1, 
			       set to 0 to not include az ang (smaller file) */
#define USE_PRECOMPUTE_FILES 1 /* use files to store precomputed locations when 1, 
				  use 0 to not use pre compute files*/ 
#define DTR ((2.0*(M_PI))/360.0)       /* degrees to radians */

#define AEARTH 6378.1363              /* SEMI-MAJOR AXIS OF EARTH, a, KM */
#define FLAT 3.3528131778969144e-3    /* = 1/298.257 FLATNESS, f, f=1-sqrt(1-e**2) */

#define MINUTES_PER_DEG_LONGITUDE 4
#define MINUTES_PER_DAY (24*60)
#define MINUTES_PER_HOUR 60

#define min(a,b) (((a) <= (b)) ? (a) : (b))
#define max(a,b) (((a) >= (b)) ? (a) : (b))
#define mod(a,b) ((a) % (b))
#define dmod(a,b) ((a)-floor((a)/(b))*(b))
#define abs(x) (((x) >= 0 ) ? (x) : -(x))

/****************************************************************************/

extern void ease2_map_info(int iopt, int isc, int ind, 
			   double *map_equatorial_radius_m, double *map_eccentricity, 
			   double *e2, double *map_reference_latitude, 
			   double *map_reference_longitude, 
			   double *map_second_reference_latitude,double * sin_phi1, 
			   double *cos_phi1, double *kz,
			   double *map_scale, int *bcols, int *brows, 
			   double *r0, double *s0, double *epsilon);

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

static void convert_time(char *time_tag, int *iyear, int *iday, int *ihour, int *imin)
{ /* convert ascii time tag into year, day, hour, minute */
  int imon, mday;
  float secs;
  /* '19970600301015123.150300 '1997 1 3 60 1 51 23 */
  sscanf(time_tag,"%4d%3d%2d%2d%2d%2d%7f",iyear,iday,&imon,&mday,ihour,imin,&secs);
  return;                                                                                                                 
}

static int isleapyear(int year) 
{ /* is year a leap year? */
  if (year==4*(year/4))  /* this test is only good for 1904-2096! */
    return(1);
  else
    return(0);
}

/****************************************************************************/

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

static float gsx_antenna_response(float x_rel, float y_rel, float theta, float semimajor, float semiminor);
static void write_filenames_to_header( gsx_class *gsx, region_save *save_area );
static void write_end_header( region_save *save_area );
static void write_header_info( gsx_class *gsx, region_save *save_area );
static FILE * get_meta(char *mname, char *outpath, int *dstart, 
                int *dend, int *mstart, int *mend, 
		int *year, char *prog_n, float prog_v,
		float *response_threshold, int *flatten, int *median_flag,
		int *inc_correct, float *b_correct, float *angle_ref, 
		region_save *save_area, cetb_platform_id *cetb_platform);

static void compute_locations(region_save *a, int *nregions, int **noffset, short int **latlon_store);

static void timedecode(double time, int *iyear, int *jday, int *imon, int *iday, int *ihour, int *imin, int *isec, int refyear);

static void rel_latlon(float *x_rel, float *y_rel, float alon, float alat, float rlon, float rlat);

static float km2pix(float *x, float *y, int iopt, float xdeg, float ydeg, 
	     float ascale, float bscale, int *stat);

static void print_projection(FILE *omf, int iopt, float xdeg, float ydeg, 
		      float ascale, float bscale, float a0, float b0);

static int box_size_by_channel( int ibeam, cetb_sensor_id id );
static void combine_setup_files( region_save *a, int execution_flag );
static int julday(int mm, int id, int iyyy);

/****************************************************************************/

int main(int argc,char *argv[])
{
  region_save save_area;

  int nscans;

  int ret_status=1;

  char fname[250], mname[250];
  char outpath[250];
  char ftempname[250];
  char *option;
  
  int i,j,n;
  int dend2, ilenyear, nrec, iscan, iasc, ii;
  char *s;
  float cen_lat, cen_lon, ctime;
  double cave;  

  /* output record information */
  float tb,thetai,azang=0.0;
  int count,ktime,iadd, fill_array[MAXFILL+1];
  short int response_array[MAXFILL+1];  

  int jrec2[NSAVE];  /* measurement counter for each region */
  int dateline;      /* when 1, region crosses dateline */

  FILE *file_id;

  int flag,ascend;
  int dstart,dend,mstart,mend,year,iregion;
  int iday,iyear,imon,ihour,imin,isec,jday;
  int idaye,iyeare,imone,ihoure,imine,isece,jdaye;

  float theta;
  
  int ibeam,icc,icmax,icmin,nfile;  
  float cx,cy,lath,latl,lonh,lonl;
  int nsx,nsy,projt,ltdflag;
  float ascale,bscale,a0,b0,xdeg,ydeg,x,y;
  int shortf;
  float tbmin=1.e10,tbmax=-1.e10; 
  
  int iadd1, box_size;
  float b_correct, angle_ref;

  /* memory for storage of pixel locations */
  int nregions,*noffset;
  short int *latlon_store;

  int ix1,ix2,ixsize,iysize,ixsize1,ixsize2,iysize1,iysize2,iy1,iy2,ix,iy,cnt;
  float clat,clon,dlat,dlon,sum;
  float eqlon, xhigh_lon, xlow_lon, sc_last_lat;
  float dscale, alat1, alon1,x_rel, y_rel;
  float tsplit1_mins, tsplit2_mins;
  float fractional_orbit;

  float response_threshold=-10.0; /* default response threshold in dB */
  int flatten=0;                  /* default: use rounded response function */  
  int median_flag=0;              /* default: do not use median filter */
  int inc_correct=0;              /* default: do not do incidence angle correction */

  int irec = 0; /* input cells counter */
  int jrec = 0; /* output record counter */
  int krec = 0; /* total scans considered */
  int mcnt=0;

  /*
   * begin to add in GSX variables
   */
  gsx_class *gsx=NULL;
  int gsx_count;
  int first_file=0;
  int first_scan_loc;
  int last_scan_loc;
  int status;
  char *gsx_fname[MAX_INPUT_FILES];
  int infile;
  int loc;
  int imeas;
  int first_measurement;
  cetb_region_id cetb_region;
  cetb_platform_id cetb_platform;  

  gsx = NULL;
  for (n=0; n<NSAVE; n++)
    jrec2[n] = 0;  /* measurements for each output region */
  
  fprintf( stderr, "MEaSUREs Setup Program\nProgram: %s  Version: %f\n\n",prog_name,prog_version);

  /* optionally get the box size of pixels to use for calculating MRF for each */
  /* box size will ultimately be replaced by a function that sets the value based on the channel and the FOV */
  box_size = 80;  // this is the default for the regression tests
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
	break;
      default:
	fprintf(stderr, "meas_meta_setup: Invalid option %c\n", *option);
	exit(-1);
      } /* end switch */
    } /* end loop for each input command option */
  } /* end loop while still input arguments */

  if (argc < 2) {
    fprintf( stderr, "\nusage: meas_meta_setup -b box_size meta_in outpath\n\n");
    fprintf( stderr, " input parameters:\n");
    fprintf( stderr, "   -b box_size is optional input argument to specify box_size for MRF\n");
    fprintf( stderr, "      default box_size is 80 for early regression testing\n");
    fprintf( stderr, "   meta_in     = input meta file\n");
    fprintf( stderr, "   outpath     = output path\n\n");
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
		     &save_area, &cetb_platform);
  if (file_id == NULL) {
    fprintf(stderr,"*** could not open meta file %s/%s\n",outpath,mname);    
    exit(-1);  
  }

  fprintf( stderr, "Number of output setup files %d\n",save_area.nregions);
  
  /* convert response threshold from dB to normal space */
  response_threshold=(float)(pow(10.,0.1*response_threshold));  
 
  /* Set flag for local time of day filtered images */
  ltdflag=0;
  for (i=0; i<save_area.nregions; i++)
    if (save_area.sav_ascdes[i] >= CETB_MORNING_PASSES && save_area.sav_ascdes[i] <= CETB_EVENING_PASSES)
      ltdflag=1;
  
  /* compute approximate projection grid scale factors for later use */
  for (iregion=0; iregion<save_area.nregions; iregion++) {      
    save_area.sav_km[iregion]=km2pix(&dlon,&dlat,save_area.sav_projt[iregion],
				     save_area.sav_xdeg[iregion],   save_area.sav_ydeg[iregion],
				     save_area.sav_ascale[iregion], save_area.sav_bscale[iregion], &ret_status);
    if ( ret_status != 1 ) {
      fprintf( stderr, "meas_meta_setup: fatal error in routine\n" );
      exit ( -1 );
    }
    fprintf( stderr, "Region %d of %d: nominal km/pixel=%f\n", iregion, save_area.nregions, save_area.sav_km[iregion]);
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

  /* Run through all of the input data files so that a list of them can be written out to the setup file */
  while(flag) { 
    
    /* before reading in the next file, free the memory from the previous gsx pointer */
    if ( NULL != gsx ) {
      gsx_close( gsx );
      gsx = NULL;
    }
    fgets(fname,sizeof(fname),file_id);

    if (ferror(file_id)) {
      fprintf( stderr, "*** error reading input meta file encountered\n" );
      exit(-1);
    }

    if ( strstr(fname,"End_input_file_list") || feof(file_id) ) { /* proper end of meta file */
      flag=0;
      gsx_close( gsx );
      gsx = NULL;
    } else {
      /* read name of input swath file */
      s=strstr(fname,"Input_file");
      if ( NULL != s ) {/* skip line if not an input file */
	/* find start of file name and extract it */
	s=strchr(s,'=');
	strcpy(ftempname,++s);
	strcpy(fname, ftempname);
	no_trailing_blanks(fname);    
        gsx = gsx_init( fname );
	write_filenames_to_header( gsx, &save_area );
	status = utils_allocate_clean_aligned_memory( (void**)&gsx_fname[nfile], strlen(fname)+1 );
	if ( 0 != status ) {
	  fprintf( stderr, "%s: *** couldn't allocate space for filename\n", __FILE__ );
	  exit (-1);
	}
	strcpy( gsx_fname[nfile], fname );
	nfile++;
	if ( nfile > MAX_INPUT_FILES ) {
	  flag = 0;
	  gsx_close( gsx );
	  gsx = NULL;
	}
      }
    }
  }

  if ( 0 == nfile ) { /* there are no input files in the meta file */
      write_end_header( &save_area );
  }
  
  for ( infile=0; infile<nfile; infile++ ) { /* input file read loop 1050 */     
    
  label_330:; /* read next file name from list gsx_fname */
    /* before reading in the next file, free the memory from the previous gsx pointer */
    if ( NULL != gsx ) {
      gsx_close( gsx );
      gsx = NULL;
    }
    strcpy(fname, gsx_fname[infile]);
  
    /* initialize last spacecraft latitude */
    sc_last_lat=-1.e25;

    /*
     * read data from file into gsx_class variable
     */
    gsx = gsx_init( fname ); // Read in a GSX file

    if ( NULL == gsx ) {
      fprintf( stderr, "%s: couldn't read file '%s' into gsx variable\n", __FUNCTION__, fname );
      free( gsx_fname[infile] );
      infile++;
      goto label_330;  // skip reading file on error
    }

    /* if this is the first file to be read, then write out the final header info for downstream processing */
    if ( 0 == first_file ) {
      first_file++;
      write_header_info( gsx, &save_area );
      write_end_header( &save_area );
      /* If this is AMSRE, combine the output setup files for a and b scans and close the unneeded output file */
      if ( CETB_AQUA == cetb_platform ) {
	combine_setup_files( &save_area, 1 );
      }
    }

    /* Here is where you loop through all of the different measurement sets in the file */
    fprintf( stderr, "%s: Satellite %s  orbit %d  lo scans %d hi scans %d\n", \
				__FUNCTION__, cetb_platform_id_name[gsx->short_platform], \
	     gsx->orbit, gsx->scans[0], gsx->scans[1] );
    for ( loc=0; loc<GSX_MAX_DIMS; loc++ ) {
      
      /* extract values of interest */
      nscans = gsx->scans[loc];
      if ( 0 != gsx->scans[loc] ) {

	first_scan_loc = loc;
	last_scan_loc = gsx->scans[loc]-1;
	timedecode( *(gsx->scantime[first_scan_loc]), &iyear,&jday,&imon,&iday,&ihour,&imin,&isec,1987);
	timedecode( *(gsx->scantime[first_scan_loc]+last_scan_loc),	\
		    &iyeare,&jdaye,&imone,&idaye,&ihoure,&imine,&isece,1987);
	fprintf( stderr, "* start time:  %lf  %d %d %d %d %d %d %d\n",	\
	       *(gsx->scantime[first_scan_loc]),iyear,iday,imon,jday,ihour,imin,isec);    
	fprintf( stderr, "* stop time:   %lf  %d %d %d %d %d %d %d\n",	\
	       *(gsx->scantime[first_scan_loc]+last_scan_loc),iyeare,idaye,imone,jdaye,ihoure,imine,isece);    
	fprintf( stderr, "search year: %d dstart,dend: %d %d  mstart: %d\n",year,dstart,dend,mstart);

	iday=jday;    /* use day of year (jday) for day search */
	idaye=jdaye;    

	/* check data range to see if file contains useful time period
	   if not, skip reading file */
	shortf=1;
	if (dstart <= dend) {
	  if (iyear != year && iyeare != year) goto label_3501;
	  if (iday <= dstart) {
	    if (idaye < dstart-1) goto label_3501;
	    if (idaye == dstart && ihoure*MINUTES_PER_HOUR+imine < mstart) goto label_3501;
	  } else {
	    if (iday > dend) {
	      if (!ltdflag) goto label_3501;
	      dend2=dend+1;
	      if (ltdflag && idaye > dend2) goto label_3501;
	    } else {
	      if (iday == dend && ihour*MINUTES_PER_HOUR+imin > mend) goto label_3501;
	    }
	  }
	} else {
	  ilenyear=365;
	  if (isleapyear(year)) ilenyear=366;      
	  iday=iday+(iyear-year)*ilenyear;
	  idaye=idaye+(iyeare-year)*ilenyear;
	  if (ltdflag)
	    dend2=dend+ilenyear+1;    
	  else
	    dend2=dend+ilenyear;
	  if (iday <= dstart) {
	    if (idaye < dstart) goto label_3501;
	    if (idaye == dstart && ihoure*MINUTES_PER_HOUR+imine < mstart) goto label_3501;
	  } else {
	    if (iday > dend2) goto label_3501;
	    if (iday == dend2 && ihour*MINUTES_PER_HOUR+imin > mend) goto label_3501;
	  }
	  /* shortf=0; */
	}
  
	/* for each scan in file */
	nrec=0;
	for (iscan=0; iscan<nscans; iscan++) { /*350*/

	  krec=krec+1;	/* count total scans read */
	  nrec=nrec+1;      /* count scans read in file */
      
	  if ((krec%500)==0) fprintf( stderr, "Scans %7d | Pulses %9d | Output %9d | Day %3d\n",krec,irec,jrec,iday);

	  if ( (*(gsx->scantime[loc]+iscan) - gsx->fill_scantime[loc]) <= DBL_EPSILON ) goto label_350; // do not process this scan - until gsx is fixed
	  /* scan time.  All measurements in this scan assigned this time */
	  timedecode(*(gsx->scantime[loc]+iscan),&iyear,&jday,&imon,&iday,&ihour,&imin,&isec,1987);
	  iday = jday;
    
	  /* check to see if scan is in desired range */
	  /* this means that the scan must be either from the day before,
	     the day of or the day after the UTC day being processed */
	  if (iyear != year) goto label_350; /* skip further processing of this scan */
	  if (iday < dstart-1) goto label_350; /* skip further processing of this scan */
	  if (iday > dend+1)   goto label_350; /* done processing, skip rest of scan */

	  /* compute days since start of image data if cross year boundary */
	  ktime=(iyear-year)*365;   /* days */
	  if (ktime > 0) { /* more than a year */
	    if (isleapyear(year)) {
	      ktime=ktime+1;
	    }
	  }  else {
	    if (ktime<0) {
	      fprintf( stderr, "*possible bug in ktime %d iyear %d year %d iscan %d\n", ktime, iyear, year, iscan);
	    }
	  }
      
	  /* compute time in mins since start of image data (assumes mstart=0) */
	  /* compute the time in minutes of the current scan line, wrt to the UTC day being processed
	     for example, if the observation time is 30 minutes before midnight UTC, then ktime will
	     be -30 - this time will later be combined with the longitude of the observation to get the relative
	     local time for the measurement at that point
	     to see the how different UTC values and longitudes are combined, look at the spreadsheet in the docs/internal
	     directory in the repo - spreadsheet allows you to enter day and time of observation and longitude and then
	     calculates ktime and ctime as in the code - the result can be compared with the split times to see if the time
	     of the measurement is in the correct range */
	  ktime=((ktime+iday-dstart)*24+ihour)*MINUTES_PER_HOUR+imin;

	  /* compute the orientation of the nadir track with respect to north */
	  fractional_orbit = ( float ) iscan/nscans;
	  eqlon = (float)(fractional_orbit * 360.0);
	  if (eqlon<0.0) eqlon=(float)(eqlon+360.0);      
	  /*
	    find the longitude of the equator crossing of the middle measurement to use in computing the
	    longitudes that separate ascending and descending for this rev */
	  xhigh_lon=(float)(eqlon+90.0);
	  xlow_lon =(float)(eqlon-90.0);
	   
	  if (xhigh_lon >  180.0) xhigh_lon=(float)(xhigh_lon-360.0);
	  if (xhigh_lon < -180.0) xhigh_lon=(float)(xhigh_lon+360.0);
	  if (xlow_lon  >  180.0) xlow_lon =(float)(xlow_lon -360.0);
	  if (xlow_lon  < -180.0) xlow_lon =(float)(xlow_lon +360.0);

	  /* here test for AMSRE that doesn't have sc_lat and sc_lon and get asc desc flag from file name */
	  /* set asc/dsc flag for measurements */
	  if ( CETB_AMSRE != gsx->short_sensor ) {
	    if (*(gsx->sc_latitude[loc]+iscan)-sc_last_lat < 0.0 ) 
	      ascend=0;  
	    else  
	      ascend=1;

	    sc_last_lat = *(gsx->sc_latitude[loc]+iscan); 
	  } else {
	    if ( CETB_ASC_PASSES == gsx->pass_direction )
	      ascend=1;
	    else
	      ascend=0;
	  }

	  /* extract TB measurements for each scan */

	  first_measurement = 0;
	  if ( CETB_AMSRE == gsx->short_sensor ) first_measurement = CETB_AMSRE_FIRST_MEASUREMENT;
	  for (imeas=first_measurement; imeas < gsx->measurements[loc]; imeas++) { /* measurements loop */
	    irec=irec+1;	/* count of pulses examined */

	    /* for each output region and section */
	    for (iregion=0; iregion<save_area.nregions; iregion++) { /* regions loop label_3400 */

	      ibeam=save_area.sav_ibeam[iregion];  /* beam number */
	      cetb_region = (cetb_region_id)(save_area.sav_regnum[iregion]-cetb_region_number[0]);

	      /* If we are processing a T grid (asc/des), check to make sure the scan line is for the day of processing */
	      if ( cetb_region == CETB_EASE2_T ) {
		if ( iday != dstart ) goto label_3400;
	      }
		      
	      if ( CETB_SSMI == gsx->short_sensor ) gsx_count = cetb_ibeam_to_cetb_ssmi_channel[ibeam];
	      if ( CETB_AMSRE == gsx->short_sensor ) gsx_count = cetb_ibeam_to_cetb_amsre_channel[ibeam];
	      
	      /* only get Tb's for channels that use the current set of position coordinates */

	      if ( gsx->channel_dims[gsx_count] == (cetb_loc_id)loc ) {
	      
		/* for this beam, get measurement, geometry, and location */
		/* Note that the gsx_count variable gives you the offset into the tb array for whichever channel
		 * you are currently looking at - which comes from the region array
		 * the imeas variable counts measurements across a scanline and the
		 * iscan variable keeps track of which scan line you are on and
		 * gsx->measurements[loc] is the number of measurements in each scan line
		 */

		tb = *(gsx->brightness_temps[gsx_count]+imeas+iscan*gsx->measurements[loc]);

		thetai = *(gsx->eia[loc]+imeas+iscan*gsx->measurements[loc]);  /* nominal incidence angle */
		azang = *(gsx->eaz[loc]+imeas+iscan*gsx->measurements[loc]);  /* nominal azimuth angle */
		cen_lat = *(gsx->latitude[loc]+imeas+iscan*gsx->measurements[loc]);  /* nominal longitude */
		cen_lon = *(gsx->longitude[loc]+imeas+iscan*gsx->measurements[loc]);  /* nominal latitude */

		if (tb < *(gsx->validRange[gsx_count])) goto label_3400; /* skip bad measurements */
		if (thetai < FLT_EPSILON) goto label_3400; /* skip bad measurements */
	  
		/* check ascending/descending orbit pass flag (see cetb.h for definitions) */
		iasc=save_area.sav_ascdes[iregion];
		if (iasc != (int)CETB_ALL_PASSES)
		  if (iasc == (int)CETB_ASC_PASSES) {
		    if (!ascend) goto label_3400;
		  } else if (iasc == (int)CETB_DES_PASSES) {
		    if (ascend) goto label_3400;
		  }

		/* extract local-time-of-day split values  - these are sensor and projection dependent */

		tsplit1_mins=(cetb_ltod_split_times[gsx->short_platform][cetb_region][0])*MINUTES_PER_HOUR;
		tsplit2_mins=(cetb_ltod_split_times[gsx->short_platform][cetb_region][1])*MINUTES_PER_HOUR;

		cy=cen_lat;
		cx=cen_lon;
		if (cx >  180.0) cx=(float)(cx-360.0);
		if (cx < -180.0) cx=(float)(cx+360.0);

		/* region lat/lon extent */
		lath=save_area.sav_lath[iregion];
		latl=save_area.sav_latl[iregion];
		lonh=save_area.sav_lonh[iregion];
		lonl=save_area.sav_lonl[iregion];
		dateline=save_area.sav_dateline[iregion];

		/* if a local-time-of-day image, compute the local time relative to the current longitude, add in the
		   UTC time of the observation (ktime) which was calculated above and compare the result to
		   the split times windows.
		   ctime is the relative local time
		   Note: data may be next or previous UTC day
		   Note also: that the length of the window is currently set to # minutes per 24 hour period
		   this may not be true for all sensors */

		if (iasc == (int)CETB_MORNING_PASSES || iasc == (int)CETB_EVENING_PASSES) { /* apply LTOD considerations */
		  ctime = cx * MINUTES_PER_DEG_LONGITUDE + ktime; /* calculate the relative local time of day in minutes */

		  if ( iasc == (int)CETB_MORNING_PASSES ) { /* morning */
		    if (ctime < tsplit1_mins || ctime >= tsplit2_mins) goto label_3400;
		  } 
		  if ( iasc == (int)CETB_EVENING_PASSES ) {  /* evening */
		    if (ctime < tsplit2_mins || ctime >= tsplit1_mins+MINUTES_PER_DAY) goto label_3400;
		  }
		}

		if (dateline) { /* convert lon to ascending order */
		  if (lonl < 0.0) lonl=(float)(lonl+360.0);
		  if (cx < -180.0) cx=(float)(cx+360.0);
		} else {	/* convert lon to -180 to 180 range */
		  if (cx > 180.0) cx=(float)(cx-360.0);
		  if (cx < -180.0) cx=(float)(cx+360.0);
		  if (cx > 180.0) cx=(float)(cx-360.0);
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
		x=(float)(ix2+0.5);
		y=(float)(iy2+0.5);
		pixtolatlon(x, y, &clon, &clat, projt, xdeg, ydeg, ascale, bscale, a0, b0);

		/* define size of box centered at(ix2,iy2) in which the gain response 
		   is computed for each pixel in the box and tested to see if
		   the response exceeds a threshold.  if so, it is used */

		box_size = box_size_by_channel( ibeam, gsx->short_sensor ); 
		if ( box_size < 0 ) {
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
		    if (iadd1 >= 0 & iadd1 < nsx*nsy) {		  
		      /* get pre-computed lat/lon of pixel */
		      alat1=(float)(latlon_store[iadd1*2+  noffset[iregion]]/200.0);
		      alon1=(float)(latlon_store[iadd1*2+1+noffset[iregion]]/175.0);

		      /* compute antenna pattern response at each pixel based on beam number, 
			 location, and projection rotation and scaling */
		      rel_latlon(&x_rel,&y_rel,alon1,alat1,clon,clat);
		      //		      gsx_count = (int)cetb_ibeam_to_cetb_ssmi_channel[ibeam];
		      sum = gsx_antenna_response( x_rel, y_rel, theta, *(gsx->efov[gsx_count]), *(gsx->efov[gsx_count]+1) );
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
		  jrec++; /* a count of total records written */
		  jrec2[iregion]++; /* records/region */
		  if (count >= MAXFILL) { /* error handling -- this should not occur! */
		    fprintf( stderr, "*** count %d overflow=%d at %d\n",count,MAXFILL,jrec);
		    fprintf( stderr, "center %f %f  %d %d %d  count %d\n",cen_lat,cen_lon,iscan,ii,ibeam,count);
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
		  fwrite(&ktime, 4,1,save_area.reg_lu[iregion]); /* time of measurement */
		  fwrite(&iadd,  4,1,save_area.reg_lu[iregion]); /* address of center pixel location */
		  if (HASAZIMUTHANGLE)
		    fwrite(&azang, 4,1,save_area.reg_lu[iregion]); /* azimuth angle relative to north (deg) */
		  fwrite(&cnt,   4,1,save_area.reg_lu[iregion]);

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
	  }  /* end of measurements loop */
	label_350:; /* end of scan loop */
	}
      } /* end of locs loop */
    }
    label_3501:;  /* end of input file */

    /* input file has been processed */
    if (shortf) {
      fprintf( stderr, "\nTotal input scans: %d  Total input pulses: %d\n",krec,irec);
      fprintf( stderr, "Region counts: ");
      for (j=0; j<save_area.nregions; j++)
	fprintf( stderr, " %d",jrec2[j]);
      fprintf( stderr, "\n");
      fprintf( stderr, "Input File Completed:  %s\n",fname);
      fprintf( stderr, "Last Day %d in Range: %d - %d\n\n",iday,dstart,dend);
      fprintf( stderr, "Number of measurements: %d\n",icc);
      fprintf( stderr, "IPR count average:  %lf\n",(icc>0? cave/(float) icc: cave));
      fprintf( stderr, "IPR count max,min:  %d %d \n",icmax,icmin);
      fprintf( stderr, "Tb max,min:  %f %f \n\n",tbmax,tbmin);
  
      if (iday <= dend)
	fprintf( stderr, "*** DAY RANGE IS NOT FINISHED ***\n");
      fprintf( stderr, "End of day period reached %d %d \n",iday,dend);
    }
               /* input file loop */
    fprintf( stderr, "Done with setup records %d %d\n",irec,krec);
    free( gsx_fname[infile] );
  }

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
        if ( CETB_AQUA == cetb_platform ) {
	  /* now check to see if you have b channels for 89H or 89V and if you also have A channels then combine */
	  combine_setup_files( &save_area, 2 );
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

  /* close input meta file */	    
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
		region_save *a, cetb_platform_id *cetb_platform)
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
	/* Here is where you can get the sensor ENUM */
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
      
      if (strstr(line,"Num_Regions") != NULL) {
	x = strchr(line,'=');
	a->nregions=atoi(++x);
	printf("Regions in meta file: %d\n",a->nregions);
      }

      if (strstr(line,"Begin_region_description") != NULL) {
	/* new region started set some default values */
	asc_des=CETB_ALL_PASSES;	/* use both asc/desc orbits */
	ireg=ireg+1;
	printf("Region %d of %d  Total regions: %d\n",ireg,a->nregions,iregion);

	/* read region information */
	flag_region=1;
	while(flag_region) {
	  fgets(line,sizeof(line),file_id);
	  no_trailing_blanks(line);
	  if (ferror(file_id)) {
	    printf("*** error reading meta file at region \n");
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
		  printf("*** error reading meta file at section \n");
		  flag_section=0;
		} else {
		  if (strstr(line,"End_section_description") != NULL)
		    flag_section=0;
      
		  if (strstr(line,"Section_id") != NULL) {
		    x = strchr(line,'=');
		    isection =atoi(++x);
		    printf("Section %d image count %d\n",isection,iregion);		    
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
		      printf("\nSIR file header information: %d %d %d %d\n",iregion,a->nregions,isection,nsection);
		      printf("  Year, day range: %d %d %d %d %d\n",*year,*dstart,*dend,*mstart,*mend);
		      printf("  Image size: %d x %d   Projection: %d\n",nsx,nsy,projt);
		      printf("  Origin: %f %f  Span: %f %f\n",a0,b0,xdeg,ydeg);
		      printf("  Scales: %f %f  Pol (0=h,1=v): %d\n",ascale,bscale,ipolar);
		      printf("  Region: %s  Num %d\n",regname,regnum);
		      printf("  Reg LL corner: %f %f   UR corner: %f %f\n",latl,lonl,lath,lonh);
		      printf("  Origin (lat,lon): %f %f\n",aorglat,aorglon);
		      printf("  Array Dimensions in Km: x=%f y=%f\n",xdim,ydim);
		      printf("  AscDesc flag (0=both,1=asc,2=dsc,3=morn,4=eve,5=mid): %d\n",asc_des);
		      printf("  Grid size: %f %f  Span: %f %f\n",xdeg,ydeg,aorglat,aorglon);
		      printf("  Scales: %f %f   Origin: %f %f\n",ascale,bscale,a0,b0);
		      printf("  GRD image size: %d %d  Size: %d %d\n",nsx2,nsy2,non_size_x,non_size_y);
		      printf("  GRD image span: %f %f  Orig: %f %f\n",xdeg2,ydeg2,a02,b02);		      
		      printf("  GRD image scale: %f %f\n",ascale2,bscale2);		      
		      /*printf("  Egg response threshold %f  Flat %d\n",*response_threshold,*flatten); */
		      printf("  Median filter %d  Ref Inc angle %f\n",*median_flag,*angle_ref);
		      printf("  Incidence angle correction %d  b_correct %f\n",*inc_correct,*b_correct); 
                      printf("  Time split: %f %f\n\n",tsplit1,tsplit2);
		      
		      /* open output setup file for this section of this region */
		      sprintf(outname,"%s/%s",outpath,fname2);		      
		      ftemp = fopen( outname, "wb" );

		      if ( ftemp != NULL ) {
			a->reg_lu[iregion-1] = ftemp;
			printf("Opened setup output file '%s'  %d\n",outname,iregion);
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
		      //fwrite(&projt,4,1,a->reg_lu[iregion-1]); //
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

		    }

		    /* Now read output file names and write to file */
		    flag_files=1;
		    while(flag_files) {
		      fgets(line,sizeof(line),file_id);
		      no_trailing_blanks(line);
		      if (ferror(file_id)) {
			printf("*** error reading meta file at product files \n");
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
		    printf("Done with setup header for image\n");

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
  /* compute the lat,lon of each pixel in the image regions and store in
     global arrays.  This reduces the computational load */

  int iregion, nspace;
  char *p, local[]="./";
  int iadd,ix,iy,nsize,iadd0;
  float x,y,clat,clon;
  FILE *f;
  char tempname[180],lastname[180]="\0",line[1024];
  int dumb;

  p=getenv("SIR_areas");
  if (p==NULL) p=local;

  *nregions=a->nregions;

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

    printf("\nRegion %d of %d: %dx%d=%d\n",iregion+1,a->nregions, 
            a->sav_nsx[iregion], a->sav_nsy[iregion],nsize);
    print_projection(stdout, a->sav_projt[iregion], 
		    a->sav_xdeg[iregion], a->sav_ydeg[iregion],
		    a->sav_ascale[iregion], a->sav_bscale[iregion],
		    a->sav_a0[iregion],     a->sav_b0[iregion]);

    /* hash file name */
    sprintf(tempname,"%4.4d-%4.4d-%2.2d-%4.4d-%4.4d-%4.4d-%4.4d.loc",
	    a->sav_nsx[iregion], a->sav_nsy[iregion], a->sav_projt[iregion],
	    (int)abs(round(a->sav_a0[iregion])), (int)abs(round(a->sav_b0[iregion])),
	    (int)abs(round(a->sav_xdeg[iregion])), (int)abs(round(a->sav_ydeg[iregion])));

    if (strncmp(lastname,tempname,180)==0) {  /* new file name is same as last */
      /* so save time and I/O re-use prior load or computation */
      for (iy=0; iy<a->sav_nsy[iregion]; iy++) {
	for (ix=0; ix<a->sav_nsx[iregion]; ix++) {	  
	  iadd=a->sav_nsx[iregion]*iy+ix; /* zero-based lexicographic pixel address */
	  iadd0=2*iadd+(*noffset)[iregion-1];
	  iadd=2*iadd+(*noffset)[iregion];	  
	  (*latlon_store)[iadd]=  (*latlon_store)[iadd0];	  
	  (*latlon_store)[iadd+1]=(*latlon_store)[iadd0+1];
	}
      }      

    } else {  /* new name differs from last name */

      /* keep last name */
      strncpy(lastname,tempname,180);
    
      if (USE_PRECOMPUTE_FILES) {     /* check to see if pre-computed array 
                                         is available in file */
	sprintf(line,"%s/%s",p,tempname);
	printf("Reading pixel locations file: %s\n",line);
	f=fopen(line,"r");
	if (f==NULL) {
	  printf("... could not open precompute file %s will recompute\n",line);	
	  goto label_skip;
	}

	if (fread(&(*latlon_store)[(*noffset)[iregion]], 2, nsize*2, f)!=2*nsize) {
	  printf("*** error reading precompute file %s\n",line);	
	  fclose(f);
	  goto label_skip;
	}
	fclose(f);
	goto label_read;
      }

    label_skip:;

      /* compute pixel locations */
      for (iy=0; iy<a->sav_nsy[iregion]; iy++) {
	y=(float)(iy+1.5); /* center of pixel, 1-based */
	for (ix=0; ix<a->sav_nsx[iregion]; ix++) {
	  x=(float)(ix+1.5); /* center of pixel, 1-based */
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

      /* write out array to file for next time to save computation*/
      if (USE_PRECOMPUTE_FILES) {
	sprintf(line,"%s/%s",p,tempname);
	printf("Writing pixel locations file: %s\n",line);
	f=fopen(line,"wx");
	if (f==NULL) {
	  printf("	*** error opening output precompute file %s\n",line);	
	  goto label_read;
	}      

	if (fwrite(&(*latlon_store)[(*noffset)[iregion]], 2, nsize*2, f)!=2*nsize) {
	  fprintf(stderr,"*** error writing location file -- file is now invald\n");
	  fclose(f);
	  exit(-1);
	}
	fclose(f);
      }
    }    

    label_read:;
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

float km2pix(float *x, float *y, int iopt, float xdeg, float ydeg, 
	     float ascale, float bscale, int *stat)
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
    *x=1./(map_scale*0.001); /* km/pixel rather than m/pixel */
    *y=1./(map_scale*0.001);
    r= 1./(map_scale*0.001);
    break;
  default: /* unknown transformation type */
    *x=0.0;
    *y=0.0;
    *stat = 0;
    fprintf( stderr, "km2pix: Unknown transformation type - %d region id\n", iopt );
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

/* support routines for date conversion */

int julday(int mm, int id, int iyyy) {
  /* returns the Julian day number that begins on noon of the calendar
     date specifed by month mm, day id and year iyy.
     the opposite of caldat */
  int IGREG=15+31*(10+12*1582);
  int jy=iyyy;
  int juday, jm, ja;
  
  if (jy < 0) 
    jy=jy+1;
  if (mm > 2) 
    jm=mm+1;
  else {
    jy=jy-1;
    jm=mm+13;
  }
  juday=floor(365.25*jy)+floor(30.6001*jm)+id+1720995;
  if (id+31*(mm+12*iyyy) >= IGREG) {
    ja=floor(0.01*jy);
    juday=juday+2-ja+floor(0.25*ja);
  }
  return(juday);
}

void caldat(int julian, int *mm, int *id, int *iyyy) {
  /* given julian day, returns output month, day, year
     the opposite of julday*/
  int IGREG=2299161;
  int jalpha, ja, jb, jc, jd, je;
  
  if (julian >= IGREG) {     
    jalpha=floor(((julian-1867216)-0.25)/36524.25);
    ja=julian+1+jalpha-floor(0.25*jalpha);
  } else
    ja=julian;
  jb=ja+1524;
  jc=floor(6680.+((jb-2439870)-122.1)/365.25);
  jd=365*jc+floor(0.25*jc);
  je=floor((jb-jd)/30.6001);
  *id=jb-jd-floor(30.6001*je);
  *mm=je-1;
  if (*mm > 12)
    *mm=*mm-12;
  *iyyy=jc-4715;
  if (*mm > 2) 
    *iyyy=*iyyy-1;
  if (*iyyy <= 0) 
    *iyyy=*iyyy-1;
}

/* date conversion routine */

void timedecode(double time, int *iyear, int *jday, int *imon, 
		int *iday, int *ihour, int *imin, int *isec, int refyear)
{     
  /* given a time in seconds from the start of 1987 (1/1/87 0Z) determine
     the year, month, day, hour, minute, and second */

  int itime=time;
  int ijd=time/(24*3600);
  int ijd0, ijd1;  
  
  if (ijd < 0) ijd=ijd-1;
  ijd0=julday(1,1,refyear);   /* julian day reference */
  caldat(ijd+ijd0,imon,iday,iyear);
  ijd1=julday(1,1,*iyear);
  *jday=ijd+ijd0-ijd1+1;   /* day of the year */

  itime=time-(julday(*imon,*iday,*iyear)-ijd0)*24*3600;
  *ihour=mod(itime/3600,24);
  *imin=mod(itime-*ihour*3600,60*60)/60;
  *isec=mod(itime-*ihour*3600-*imin*60,60);
}

/*
 * box_size_by_channel - returns the box size to use based on the channel and sensor
 *
 * input :
 *   ibeam : channel number
 *   sensor : short_sensor id
 *
 * result :
 *   box size in pixels for that channel/sensor combination
 *   returns -1 in case of error - e.g. out of range channel or unknown sensor
 *
 * the function only expects SSMI or AMSRE channel data for now and fails otherwise
 *
 * a discussion of the process by which the optimum box_size was chosen can be
 * found in the project on bitbucket.org in the directory docs/internal
 *
 * a discussion document as well as a spread sheet with the data used to make the
 * box size determination are located there
 *
 */
int box_size_by_channel( int ibeam, cetb_sensor_id id ) {
  int box_size;

  if ( CETB_SSMI == id ) {
    
    switch ( cetb_ibeam_to_cetb_ssmi_channel[ibeam] ) {
    case SSMI_19H:
    case SSMI_19V:
    case SSMI_22V:
      box_size = 100;
      break;
    case SSMI_37H:
    case SSMI_37V:
      box_size = 60;
      break;
    case SSMI_85H:
    case SSMI_85V:
      box_size = 20;
      break;
    default:
      box_size = -1;
      fprintf( stderr, "%s: bad channel number %d\n", __FUNCTION__, ibeam );
    }
  } else if ( CETB_AMSRE == id ) {
    switch ( cetb_ibeam_to_cetb_amsre_channel[ibeam] ) {
    case AMSRE_06H:
    case AMSRE_06V:
      box_size = 24;
      break;
    case AMSRE_10H:
    case AMSRE_10V:
      box_size = 20;
      break;
    case AMSRE_18H:
    case AMSRE_18V:
      box_size = 22;
      break;
    case AMSRE_23H:
    case AMSRE_23V:
      box_size = 26;
      break;
    case AMSRE_36H:
    case AMSRE_36V:
      box_size = 22;
      break;
    case AMSRE_89H_A:
    case AMSRE_89V_A:
      box_size = 10;
      break;
    case AMSRE_89H_B:
    case AMSRE_89V_B:
      box_size = 12;
      break;
    default:
      box_size = -1;
      fprintf( stderr, "%s: bad channel number %d\n", __FUNCTION__, ibeam );
    }
  } else {
    box_size = -1;
  }
    
  return box_size;
}

/*
 * write_header_info - writes out the info required for CETB files
 *
 * Input:
 *   gsx - pointer to gsx_class struct
 *   save_area - contains info on open output setup files
 *
 * Return:
 *   none
 *
 */
void write_header_info( gsx_class *gsx, region_save *save_area ) {
  int cnt=100;
  char lin[100];
  int z;
  int iregion;
  
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

    }
  }
}

/* write_filenames_to_header - writes out the input data files that were used to
 * create the setup files as well as the GSX version used in the processing
 *
 * Input:
 *   gsx structure - holds the filename and the version
 *   save_area - contains the information on the open output setup files
 *
 * Return:
 *   0 on success, 1 on failure
 */
void write_filenames_to_header( gsx_class *gsx, region_save *save_area ) {
  int cnt=100;
  char lin[100];
  int z;
  int iregion;

  for ( iregion=1; iregion<=save_area->nregions; iregion++ ) { 
     fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]); 
     for(z=0;z<100;z++)lin[z]=' '; 
     sprintf(lin," Input_file=%s (GSX_version:%s)", gsx->source_file, gsx->gsx_version);
     fwrite(lin,100,1,save_area->reg_lu[iregion-1]); 
     fwrite(&cnt,4,1,save_area->reg_lu[iregion-1]);
   } 
  fprintf( stderr, "****%s, source file %s, gsx_version %s\n", __FUNCTION__, gsx->source_file, gsx->gsx_version );
  fprintf( stderr, "****%s, last line written%s\n", __FUNCTION__, lin );
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
void write_end_header( region_save *save_area ){
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
}

/*
 * manipulate_setup_files - called if we have to combine channels into a single output setup file
 *
 * Input:
 *   region_save pointer holds the file-ids to be manipulated
 *   execution flag tells what operation should be performed
 *
 */
void combine_setup_files( region_save *a, int execution_flag ) {

  int count;
  int sub_count;
  
  /* Here is where you check to see if both AMSRE 89 a and b scans are requested, if so
     they need to be written into only 1 output setup file, i.e. 89Ha and 89Hb go into the same file
     and 89Va and 89Vb go into the same file.  The check needs to be done here so that you don't
     have to rely on the regions in the file going in a specific order - also note that this only works
     in the first place if you put all of the 89 channels into the same def file
     Use the save_area (a in this routine) structure to check for a and b scans if AMSRE */
  fprintf( stderr, "%s: into manipulating fileids with %d execution flag\n", __FUNCTION__, execution_flag );
    for ( count=0; count < a->nregions; count++ ) {
      /* now check to see if you have b channels for 89H or 89V and if you also have A channels
	 then combine if they use the same projection */
      if ( cetb_ibeam_to_cetb_amsre_channel[a->sav_ibeam[count]] == AMSRE_89H_B ) {
	for ( sub_count=0; sub_count < a->nregions; sub_count++ ) {
	  if ( ( cetb_ibeam_to_cetb_amsre_channel[a->sav_ibeam[sub_count]] == AMSRE_89H_A )
	       && ( a->sav_regnum[sub_count] == a->sav_regnum[count] )
	       && ( a->sav_ascdes[sub_count] == a->sav_ascdes[count] ) ) {
	    /* depending on the execution_flag either
	       - close the file that won't be used and save file id for the setup file for
	         AMSRE_89H_A to the file id for AMSRE_89H_B or
	       - set the file id to NULL */
	    if ( execution_flag == 1 ) {
	      fclose( a->reg_lu[count] );
	      a->reg_lu[count] = a->reg_lu[sub_count];
	      fprintf( stderr, "%s: closed region file count %d in favor of sub_count %d\n", __FUNCTION__,
		       count, sub_count );
	    }
	    if ( execution_flag == 2 ) {
	      a->reg_lu[count] = NULL;
	    }
	  }
	}
      }
      if ( cetb_ibeam_to_cetb_amsre_channel[a->sav_ibeam[count]] == AMSRE_89V_B ) {
	for ( sub_count=0; sub_count < a->nregions; sub_count++ ) {
	  if ( ( cetb_ibeam_to_cetb_amsre_channel[a->sav_ibeam[sub_count]] == AMSRE_89V_A )
	       && ( a->sav_regnum[sub_count] == a->sav_regnum[count] )
	       && ( a->sav_ascdes[sub_count] == a->sav_ascdes[count] ) )  {
	    /* depending on the execution_flag either
	       - close the file that won't be used and save file id for the setup file for
	         AMSRE_89H_A to the file id for AMSRE_89H_B or
	       - set the file id to NULL */
	    if ( execution_flag == 1 ) {
	      fclose( a->reg_lu[count] );
	      a->reg_lu[count] = a->reg_lu[sub_count];
	      fprintf( stderr, "%s: closed region file count %d in favor of sub_count %d\n", __FUNCTION__,
		       count, sub_count );
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
