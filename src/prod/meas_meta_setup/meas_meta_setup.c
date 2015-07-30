/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_meta_setup.c

  generates radiometer egg (footprint) setup files for radiometer
  MEaSURES processing

  This version is configured for RSS SSMI V7 binary

  written by DGL at BYU  02/22/2014 + based on oscat_meta_setup_slice.c and ssmi_meta_setup_v7RSS.f
  revised by DGL at BYU  03/07/2014 + added EASE2 capability
  revised by DGL at BYU  04/11/2014 + added response debug output
  revised by DGL at BYU  06/28/2014 + added RSS swath overlap reject
  revised by MAH at NSIDC 10/10/2014 + directives for Intel math library
  revised by DGL at BYU  01/19/2015 + changed LTOD logic
  revised by MAH at NSIDC 01/20/2015 + added DGL's changes in by hand
  revised by DGL at BYU  01/27/2015 + updated convert_time and morn LTOD
  revised by MAH at NSIDC 01/30/2015 + added DGL's changes in by hand
  further revision are tracked in bitbucket and not via this comment list MAH 05/15/15

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef JANUSicc
#include <mathimf.h>
#else
#include <math.h>
#endif

#include <sir3.h>

#define prog_version 0.3 /* program version */
#define prog_name "meas_meta_setup"

/* This code can read and process several different data sets.  To change data sets
   the code should be recompiled using the appropriate define flags.

   The compile script should externally set RSS and CSU flags; otherwise
   uncomment the following block and set the flags as desired

*/
//#define RSS
//#undef RSS   /* uncomment for RSS, uncomment out for CSU */
//#ifndef RSS
//#define CSU
//#endif

#define DEBUG_RESPONSE   /* if defined, outputs response debug info to file */
#undef DEBUG_RESPONSE   /* if not defined (normal case) do not output response debug info to file */

#define NSAVE 50            /* maximum number of regions to output */
#define MAXFILL 2000        /* maximum number of pixels in response function */
#define RESPONSEMULT 1000   /* response multiplier */
#define HASAZIMUTHANGLE 1   /* include azimuth angle in output setup file if 1, 
			       set to 0 to not include az ang (smaller file) */
#define USE_PRECOMPUTE_FILES 1 /* use files to store precomputed locations when 1, 
				  use 0 to not use pre compute files*/ 
#define SHORTLOC /* if defined use short ints to store precomputed locations */
//#undef SHORTLOC  /* undefine to use floats to store precomputed locations  (more memory used) */

#define DTR 1.7453292519943295e-2      /* degrees to radians */
#define RTD 57.29577951308233          /* radians to degrees */
#define PI  3.141592653589793 

#define AEARTH 6378.1363              /* SEMI-MAJOR AXIS OF EARTH, a, KM */
#define FLAT 3.3528131778969144e-3    /* = 1/298.257 FLATNESS, f, f=1-sqrt(1-e**2) */

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define mod(a,b) ((a) % (b))
#define dmod(a,b) ((a)-floor((a)/(b))*(b))
#define abs(x) ((x) >= 0 ? (x) : -(x))

/****************************************************************************/

int nint(float r)
{
  int ret_val = r;  if (ret_val - r > 0.5) ret_val--;
  if (r - ret_val > 0.5) ret_val++;
  return(ret_val);
}

void set_mask(int *quality_mask, int sensor)
{
  /* depending on sensor, set mask to not consider some bits */

  *quality_mask = 0x0FFF;
}

void no_trailing_blanks(char *s)
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

void convert_time(char *time_tag, int *iyear, int *iday, int *ihour, int *imin)
{ /* convert ascii time tag into year, day, hour, minute */
  int imon, mday;
  float secs;
  /* '19970600301015123.150300 '1997 1 3 60 1 51 23 */
  sscanf(time_tag,"%4d%3d%2d%2d%2d%2d%7.4f",iyear,iday,&imon,&mday,ihour,imin,&secs);
  return;                                                                                                                 
}

int isleapyear(int year) 
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


/****************************************************************************/

#ifdef RSS

/* declarations for RSS sensor swath TB files */

#define HI_SCAN 128
#define LO_SCAN  64
#define HSCANS 3600
#define LSCANS 1800
#define NUMCHAR  25

typedef struct { /* ssmiRSS V7 data file organization */
  int    KSAT;                     /* Spacecraft ID */
  int    IORBIT;                   /* Orbit (i.e. granule) number */
  int    NUMSCAN;                  /* Number of high res scans    */
  char   ASTART_TIME[NUMCHAR];     /* pass start date/time */
  double SCAN_TIME[HSCANS];        /* Time in seconds from start of 2000 */
  double ORBIT[HSCANS];            /* orbit number (hi-res)     */
  float  SC_LAT[HSCANS];           /* Spacecraft latitude            */
  float  SC_LON[HSCANS];           /* Spacecraft longitude            */
  float  SC_ALT[HSCANS];           /* Spacecraft altitude            */
  int    IQUAL_FLAG[HSCANS];       /* Quality Flag (hi-res)          */
  float  CEL_LAT[HI_SCAN*HSCANS];  /* Latitude (hi-res)             */
  float  CEL_LON[HI_SCAN*HSCANS];  /* Longitude (hi-res)            */
  float  CEL_EIA[HI_SCAN*HSCANS];  /* Earth incidence angle (hi-res) */
  float  CEL_SUN[HI_SCAN*HSCANS];  /* Sun glint angle (hi-res)       */
  float  CEL_AZM[HI_SCAN*HSCANS];  /* Scan azimuth angle        */
  float  CEL_LND[HI_SCAN*HSCANS];  /* land flag        */
  short int CEL_ICE[HI_SCAN*HSCANS]; /* ice flag */
  float  CEL_85V[HI_SCAN*HSCANS];  /* tb85v : 85 GHz V-Pol TBs (hi-res)      */
  float  CEL_85H[HI_SCAN*HSCANS];  /* tb85h : 85 GHz H-Pol TBs (hi-res)      */
  float  CEL_19V[ LO_SCAN*LSCANS]; /* tb19v : 19 GHz V-Pol TBs (lo-res       */
  float  CEL_19H[ LO_SCAN*LSCANS]; /* tb19h : 19 GHz H-Pol TBs               */
  float  CEL_22V[ LO_SCAN*LSCANS]; /* tb22v : 22 GHz V-Pol TBs               */
  float  CEL_37V[ LO_SCAN*LSCANS]; /* tb37v : 37 GHz V-Pol TBs               */
  float  CEL_37H[ LO_SCAN*LSCANS]; /* tb37h : 37 GHz H-Pol TBs               */
 
  /* fortran array declaration and access from fortran reader
  real*4 CEL_LAT(128,3600)
  do i=1,3600
       do j=1,128
	    CEL_LAT(j,i) */

} ssmiRSSv7;

#else

/* declarations for reading CSU sensor swath TB files */

#define HI_SCAN 128
#define LO_SCAN  64
#define HSCANS 3600
#define LSCANS 1800
#define NUMCHAR  23

typedef struct { /* ssmiCSU data file contents */
  int    KSAT;                     /* scid : Spacecraft ID */
  int    IORBIT;                   /* orbit_number : Orbit (i.e. granule) number */
  int    NUMSCAN;                  /* nhscan : Number of high res scans    */
  int    nlscan;                      /* Number of low res scans        */
  char   title[100];                  /* Title                          */
  char   author[100];                 /* Author                         */
  char   email[100];                  /* Email                          */
  char   url[100];                    /* URL                            */
  char   institution[100];            /* Institution                    */
  char   version[100];                /* Version number                 */
  char   revision_date[100];          /* Revision Date                  */
  char   platform[100];               /* Satellite/Platform ID          */
  char   sensor[100];                 /* Sensor name                    */
  char   startdate[100];              /* Granule start date/time        */
  char   enddate[100];                /* Granule end date/time          */
  char   created[100];                /* File creation date             */
  char   inpfile[100];                /* Input data file name           */
  char   outfile[100];                /* Output data file name          */
  char   ASTART_TIME[NUMCHAR];        /* datetimehr : scan date/time */
  double SCAN_TIME[HSCANS];       /* xtime : Time in seconds from 1/1/87    */
  double ORBIT[HSCANS];           /* rev : orbit number (hi-res)     */
  float  SC_LAT[HSCANS];          /* sclat : Spacecraft latitude            */
  float  SC_LON[HSCANS];          /* sclon : Spacecraft longitude            */
  float  SC_ALT[HSCANS];          /* sclalt : Spacecraft altitude            */
  char   ASTART_TIME_lo[NUMCHAR];         /* datetimehr : scan date/time */
  double SCAN_TIME_lo[LSCANS];       /* xtime : Time in seconds from 1/1/87    */
  double ORBIT_lo[LSCANS];           /* rev : orbit number (lo-res)      */
  float  SC_LAT_lo[LSCANS];          /* sclat : Spacecraft latitude            */
  float  SC_LON_lo[LSCANS];          /* sclon : Spacecraft longitude            */
  float  SC_ALT_lo[LSCANS];          /* sclalt : Spacecraft altitude            */
  int    IQUAL_FLAG[HSCANS];       /* qual : Quality Flag (hi-res)          */
  int    IQUAL_FLAG_lo[LSCANS];      /* qual : Quality Flag (lo-res)          */
  float  CEL_LAT[HI_SCAN*HSCANS];  /* lat : Latitude (hi-res)             */
  float  CEL_LON[HI_SCAN*HSCANS];  /* lon : Longitude (hi-res)            */
  float  CEL_EIA[HI_SCAN*HSCANS];  /* eia : Earth incidence angle (hi-res) */
  float  CEL_SUN[HI_SCAN*HSCANS];  /* glint : Sun glint angle (hi-res)       */
  //float  CEL_AZM[HI_SCAN*HSCANS];  /* Scan azimuth angle        */ 
  //float  CEL_LND[HI_SCAN*HSCANS];  /* land flag        */
  //short int CEL_ICE[HI_SCAN*HSCANS]; /* ice flag */
  float  CEL_LAT_lo[LO_SCAN*LSCANS]; /* lat : Latitude (lo-res)             */
  float  CEL_LON_lo[LO_SCAN*LSCANS]; /* lon : Longitude (lo-res)            */
  float  CEL_EIA_lo[LO_SCAN*LSCANS]; /* eia : Earth incidence angle (lo-res) */
  float  CEL_SUN_lo[LO_SCAN*LSCANS]; /* glint : Sun glint angle (lo-res)       */
  float  CEL_85V[HI_SCAN*HSCANS];  /* tb85v : 85 GHz V-Pol TBs (hi-res)      */
  float  CEL_85H[HI_SCAN*HSCANS];  /* tb85h : 85 GHz H-Pol TBs (hi-res)      */
  float  CEL_19V[ LO_SCAN*LSCANS]; /* tb19v : 19 GHz V-Pol TBs (lo-res)      */
  float  CEL_19H[ LO_SCAN*LSCANS]; /* tb19h : 19 GHz H-Pol TBs               */
  float  CEL_22V[ LO_SCAN*LSCANS]; /* tb22v : 22 GHz V-Pol TBs               */
  float  CEL_37V[ LO_SCAN*LSCANS]; /* tb37v : 37 GHz V-Pol TBs               */
  float  CEL_37H[ LO_SCAN*LSCANS]; /* tb37h : 37 GHz H-Pol TBs               */
 
  /* fortran array declaration and access from fortran reader
  real*4 CEL_LAT(128,3600)
  do i=1,3600
       do j=1,128
	    CEL_LAT(j,i) */

} ssmiCSU;

#endif
/********************************************************************/

/* function prototypes */

/* BYU SSM/I approximate spatial response computation */
float ssmi_response(float x_rel, float y_rel, float theta, float thetai, int ibeam);

#ifdef RSS

/* RMS SSM/I V7 binary reader */
int read_ssmiRSS_TB_V07_binary(char *fname, ssmiRSSv7 *d, int verbose);
void read_ssmiRSS_minmaxlat(float *minlat, float *maxlat);

#else

/* CSU SSM/I netcdf reader */
int read_ssmiCSU_TB(char *fname, ssmiCSU *d, int verbose);
void read_ssmiRSS_minmaxlat(float *minlat, float *maxlat);

#endif

FILE * get_meta(char *mname, char *outpath, int *dstart, 
                int *dend, int *mstart, int *mend, 
		int *year, char *prog_n, float prog_v,
		float *response_threshold, int *flatten, int *median_flag,
		int *inc_correct, float *b_correct, float *angle_ref, 
		int *KSAT, int nfile_select, region_save *save_area);

void compute_locations(region_save *a, int *nregions, int **noffset, short int **latlon_store, float **flatlon_store);

void timedecode(double time, int *iyear, int *jday, int *imon, int *iday, int *ihour, int *imin, int *isec, int refyear);

void rel_latlon(float *x_rel, float *y_rel, float alon, float alat, float rlon, float rlat);

float km2pix(float *x, float *y, int iopt, float xdeg, float ydeg, 
	     float ascale, float bscale, float a0, float b0, int *stat);

void print_projection(FILE *omf, int iopt, float xdeg, float ydeg, 
		      float ascale, float bscale, float a0, float b0);

/****************************************************************************/

int main(int argc,char *argv[])
{
  region_save save_area;

  int nscans, lrev=0;

  int ret_status=1;

  char fname[250], mname[250];
  char line[1025], outpath[250];
  char ftempname[250];
  char *option;
  
  int i,j,k,n;
  int dend2, ilenyear, nrec, iscan, iscan1, iasc, ii, nsum;
  char *s;
  float ant_az, cen_lat, cen_lon, ctime, lata, az_bias;
  double cave;  

  /* output record information */
  float tb,az,thetai,azang=0.0;
  int count,ktime,iadd, fill_array[MAXFILL+1];
  short int response_array[MAXFILL+1];  

  int jrec2[NSAVE];  /* measurement counter for each region */
  int dateline;      /* when 1, region crosses dateline */

  FILE *file_id, *datafile, *fout;

  int ipolar,flag,ascend;
  int dstart,dend,mstart,mend,year,iregion;
  int iday,iyear,imon,ihour,imin,isec,jday;
  int idaye,iyeare,imone,ihoure,imine,isece,jdaye;

  int nbeams=7;
  int inlonrange,ilow,ihigh,nmeas;
  float a,b,rat,theta_orb,scan_ang,esep_ang,ang2,theta;
  
  int ib,ibeam,icc,icmax,icmin,nfile,file_read_error;  
  float cx,cy,lath,latl,lonh,lonl;
  int nsx,nsy,projt,ltdflag;
  float ascale,bscale,a0,b0,xdeg,ydeg,x,y;
  int shortf,offset;
  float tbmin=1.e10,tbmax=-1.e10; 
  
  int nfile_select, iadd1, box_size;
  float b_correct, angle_ref;

  /* memory for storage of pixel locations */
  int nregions,*noffset;
  short int *latlon_store;
  float *flatlon_store;  

  int ix1,ix2,ixsize,iysize,ixsize1,ixsize2,iysize1,iysize2,iy1,iy2,ix,iy,cnt;
  float clat,clon,dlat,dlon,sum;
  float eqlon, xhigh_lon, xlow_lon, sc_last_lat, sc_last_lon;
  float minlat[HI_SCAN], maxlat[HI_SCAN]; 
  float dscale, tsplit1, tsplit2, alat1, alon1,x_rel, y_rel;

  float response_threshold=-10.0; /* default response threshold in dB */
  int flatten=0;                  /* default: use rounded response function */  
  int median_flag=0;              /* default: do not use median filter */
  int inc_correct=0;              /* default: do not do incidence angle correction */
  int KSAT=13;                    /* assign a default value */
  int quality_mask=0;             /* default: no mask applied */

  int irev = 0; /* rev counter */
  int irec = 0; /* input cells counter */
  int jrec = 0; /* output record counter */
  int krec = 0; /* total scans considered */
  int mcnt=0;

#ifdef DEBUG_RESPONSE
  FILE *resp_debug;
  char debug_response_out_name[]="debug_response.out";  
  float plat, plon;
#endif

#ifdef RSS
  char fcdr_input[]="SSM/I RSS V7 binary";  
  ssmiRSSv7 *d;
  d=(ssmiRSSv7 *) malloc(sizeof(ssmiRSSv7));
  if (d==NULL) {
    fprintf(stderr,"*** could not allocate startup\n");
    exit(-1);
  }
#else
  char fcdr_input[]="SSM/I CSU netcdf";  
  ssmiCSU *d;
  d=(ssmiCSU *) malloc(sizeof(ssmiCSU));
  if (d==NULL) {
    fprintf(stderr,"*** could not allocate startup\n");
    exit(-1);
  }
#endif

  for (n=0; n<NSAVE; n++)
    jrec2[n] = 0;  /* measurements for each output region */
  
  printf("Code version: %s\n",fcdr_input);  
  printf("MEaSures Setup Program\nProgram: %s  Version: %f\n\n",prog_name,prog_version);

  /* optionally get the box size of pixels to use for calculating MRF for each */
  /* box size will ultimately be replaced by a function that sets the value based on the channel and the FOV */
  box_size = 80;  // this is the default for the regression tests
    while (--argc > 0 && (*++argv)[0] == '-')
    { for (option = argv[0]+1; *option != '\0'; option++)
	{ switch (*option)
	    { case 'b':
		++argv; --argc;
		if (sscanf(*argv,"%d", &box_size) != 1) {
		  fprintf(stderr,"meas_meta_setup: can't read box size %s\n", *argv);
		  exit(-1);
		}
		fprintf( stderr, "box size is %d\n", box_size );
		break;
	      default:
	        fprintf(stderr,"meas_meta_setup: Invalid option %c\n", *option);
	        exit(-1);
	    } /* end switch */
  	} /* end loop for each input command option */
    } /* end loop while still input arguments */

  if (argc < 2) {
    printf("\nusage: meas_meta_setup -b box_size meta_in outpath singlefile\n\n");
    printf(" input parameters:\n");
    printf("   -b box_size is optional input argument to specify box_size for MRF\n");
    printf("      default box_size is 80 for early regression testing\n");
    printf("   meta_in     = input meta file\n");
    printf("   outpath     = output path\n");
    printf("   singlefile  = for testing, specify a single file. Use zero for all \n\n");
    exit (-1);
  }
 

  /* get input meta file name */
  sscanf(*argv++,"%s",mname);
  printf("\nMetafile name: %s \n",mname);

  /* get output path */
  sscanf(*argv++,"%s",outpath);
  printf("\nOutput path: %s \n",outpath);

  nfile_select = 0; /* parameter only used for debugging, but embedded all over the place */
  if (*argv != NULL) {
    sscanf(*argv,"%d",&nfile_select);
    fprintf( stderr, "nfile_select is %d\n", nfile_select );
  }

  /* get meta_file region information and open output files */
  file_id = get_meta(mname, outpath, &dstart, &dend, &mstart, &mend, &year,
		     prog_name, prog_version ,&response_threshold, &flatten, &median_flag,
		     &inc_correct, &b_correct, &angle_ref, 
		     &KSAT, nfile_select, &save_area);
  if (file_id == NULL) {
    fprintf(stderr,"*** could not open meta file %s/%s\n",outpath,mname);    
    exit(-1);  
  }

  printf("Number of output setup files %d\n",save_area.nregions);
  
  /* convert response threshold from dB to normal space */
  response_threshold=pow(10.,0.1*response_threshold);  
 
  /* set some sensor-specific constants */
  nbeams=7;          /* number of beams */     
  scan_ang=102.0;    /* azimuth angle of the scan from left measurement to right measurement at subsat point */

  /* set sensor specific quality control flag mask */
  set_mask(&quality_mask, 1);  /* sensor=SSM/I=1 */


#ifdef RSS
  /* for ssmi RSS V7 get max/min nominal lats for each A scan measurement */
  read_ssmiRSS_minmaxlat(minlat,maxlat);
#else
  /* for ssmi RSS V7 get max/min nominal lats for each A scan measurement */
  read_ssmiRSS_minmaxlat(minlat,maxlat);
#endif

  /* Set flag for local time of day filtered images */
  ltdflag=0;
  for (i=0; i<save_area.nregions; i++)
    if (save_area.sav_ascdes[i] >= 3 && save_area.sav_ascdes[i] <= 7)
      ltdflag=1;
  
  /* compute approximate projection grid scale factors for later use */
  for (iregion=0; iregion<save_area.nregions; iregion++) {      
    save_area.sav_km[iregion]=km2pix(&dlon,&dlat,save_area.sav_projt[iregion],
				     save_area.sav_xdeg[iregion],   save_area.sav_ydeg[iregion],
				     save_area.sav_ascale[iregion], save_area.sav_bscale[iregion],
				     save_area.sav_a0[iregion],     save_area.sav_b0[iregion], &ret_status);
    if ( ret_status != 1 ) {
      fprintf( stderr, "meas_meta_setup: fatal error in routine\n" );
      exit ( -1 );
    }
    printf("Region %d of %d: nominal km/pixel=%f\n", iregion, save_area.nregions, save_area.sav_km[iregion]);
  }
  
  /* pre-compute pixel locations for each region */
  compute_locations(&save_area, &nregions, &noffset, &latlon_store, &flatlon_store);
  printf("\n");

  /* initialize some pixel counters */
  icc=0;
  cave=0.0;
  icmax=-1;
  icmin=200;

  /* read and process each input swath TB file from meta file input */
    
  krec=0;              /* total scans read */
  flag=1;              /* end flag */
  nfile=0;             /* L1B input file counter */
  while(flag) { /* meta file read loop 1050 */     
    /* printf("Read meta file input file name\n"); */
    
  label_330:; /* read next input line (file name) from meta file */   
    fgets(fname,sizeof(fname),file_id);
    /* printf("file %s\n",fname); */

    if (ferror(file_id)) {
      fprintf( stderr, "*** error reading input meta file encountered\n" );
      exit(-1);
    }

    if (strstr(fname,"End_input_file_list")) { /* proper end of meta file */
      flag=0;
      goto label_3501;
    }

    if (feof(file_id)) { /* input meta file error */
      fprintf(stdout,"*** end of input meta file encountered\n");
      goto label_3501;
    }

    /* read name of input swath file */
    s=strstr(fname,"Input_file");
    if (s == NULL) /* skip line if not an input file */
      goto label_330;    

    /* find start of file name and extract it */
    s=strchr(s,'=');
    strcpy(ftempname,++s);
    strcpy(fname, ftempname);
    no_trailing_blanks(fname);    

    nfile++;
    /* single file select */	   
    if (nfile_select>0 && nfile_select != nfile) /* if single file select */
      goto label_330;  /* skip all files that are not one selected */

    /* initialize last spacecraft position */
    sc_last_lat=-1.e25;
    sc_last_lon=-1.e25;

    /* open and read new input TB file */
    printf("Opening input TB file %d '%s'\n",nfile,fname);

    /* read measurement data from file */    
#ifdef RSS
    file_read_error=read_ssmiRSS_TB_V07_binary(fname, d, 1);
#else
    file_read_error=read_ssmiCSU_TB(fname, d, 1);
#endif
    if (file_read_error < 0) {
      fprintf(stderr,"*** error reading %s\n  skipping...\n",fname);
      goto label_330;   /* skip reading file on error */
    }
    printf("Satellite %d  orbit %d  scans %d\n",d->KSAT,d->IORBIT,d->NUMSCAN);

    /* extract values of interest */
    nscans=d->NUMSCAN;
#ifdef RSS
    convert_time(d->ASTART_TIME,&iyear,&iday,&ihour,&imin);
#else
    convert_time(d->ASTART_TIME,&iyear,&iday,&ihour,&imin);
    sscanf(d->ASTART_TIME,"%4d-%2d-%2dT%2d:%2d",&iyear,&imon,&iday,&ihour,&imin);
#endif

    printf("start time: '%s' %d %d %d %d\n",d->ASTART_TIME,iyear,iday,ihour,imin);

    /* time of first and last scans */
#ifdef RSS
    timedecode(d->SCAN_TIME[0],&iyear,&jday,&imon,&iday,&ihour,&imin,&isec,2000);
    timedecode(d->SCAN_TIME[nscans-1],&iyeare,&jdaye,&imone,&idaye,&ihoure,&imine,&isece,2000);
#else
    timedecode(d->SCAN_TIME[0],&iyear,&jday,&imon,&iday,&ihour,&imin,&isec,1987);
    timedecode(d->SCAN_TIME[nscans-1],&iyeare,&jdaye,&imone,&idaye,&ihoure,&imine,&isece,1987);
#endif    
    printf("* start time: %s %lf  %d %d %d %d %d %d %d\n",d->ASTART_TIME,d->SCAN_TIME[0],iyear,iday,imon,jday,ihour,imin,isec);    
    printf("* stop time:  %s %lf  %d %d %d %d %d %d %d\n",d->ASTART_TIME,d->SCAN_TIME[nscans-1],iyeare,idaye,imone,jdaye,ihoure,imine,isece);    

    printf("first scan:%lf %d %d %d %d %d %d %d\n",d->SCAN_TIME[0],iyear,iday,imon,jday,ihour,imin,isec);
    printf("last scan: %lf %d %d %d %d %d %d %d\n",d->SCAN_TIME[nscans-1],iyeare,idaye,imone,jdaye,ihoure,imine,isece);
    printf("search year: %d dstart,dend: %d %d  mstart: %d\n",year,dstart,dend,mstart);

    iday=jday;    /* use day of year (jday) for day search */
    idaye=jdaye;    

    /* check data range to see if file contains useful time period
       if not, skip reading file */
    shortf=1;
    if (dstart <= dend) {
      if (iyear != year && iyeare != year) goto label_3501;
      if (iday <= dstart) {
	if (idaye < dstart) goto label_3501;
	if (idaye == dstart && ihoure*60+imine < mstart) goto label_3501;
      } else {
	if (iday > dend) {
	  if (!ltdflag) goto label_3501;
	  dend2=dend+1;
	  if (ltdflag && idaye > dend2) goto label_3501;
	} else {
	  if (iday == dend && ihour*60+imin > mend) goto label_3501;
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
	if (idaye == dstart && ihoure*60+imine < mstart) goto label_3501;
      } else {
	if (iday > dend2) goto label_3501;
	if (iday == dend2 && ihour*60+imin > mend) goto label_3501;
      }
      /* shortf=0; */
    }
  
    /* for each scan in file */
    nrec=0;
    for (iscan=0; iscan<nscans; iscan++) { /*350*/
      //printf("Scan %d %s\n",iscan,timetags[iscan]);
      krec=krec+1;	/* count total scans read */
      nrec=nrec+1;      /* count scans read in file */
      
      if ((krec%500)==0) printf("Scans %7d | Pulses %9d | Output %9d | Day %3d\n",krec,irec,jrec,iday);

      if (d->ORBIT[iscan] == 0.0) goto label_350; /* skip further processing of this scan */
      
      if (d->IORBIT != lrev && d->IORBIT > 0) {
	irev++;
	lrev=d->IORBIT;
      }

      /* check scan measurement quality */
      if ((quality_mask & d->IQUAL_FLAG[iscan]) != 0)
	goto label_350; /* skip to next scan if data bad */

#ifdef RSS
      /* reject scans that are not part of current orbit (RSS only) */
      if (d->ORBIT[iscan]-d->IORBIT < 0.0 || d->ORBIT[iscan]-d->IORBIT > 1.0)
	goto label_350; /* skip to next scan */
#endif
   
      //printf("scan %d of %d: %lf %d %d\n",iscan,nscans,d->ORBIT[iscan],d->IQUAL_FLAG[iscan],d->IORBIT);
 
      /* scan time.  All measurements in this scan assigned this time */
#ifdef RSS
      timedecode(d->SCAN_TIME[iscan],&iyear,&jday,&imon,&iday,&ihour,&imin,&isec,2000);
      iday=jday;      
#else
      //timedecode(d->SCAN_TIME_lo[iscan],&iyear,&jday,&imon,&iday,&ihour,&imin,&isec,1987);
      //printf("CSU lo scan:%d %d %d %d %d %d %d\n",iyear,iday,imon,jday,ihour,imin,isec);
      timedecode(d->SCAN_TIME[iscan],&iyear,&jday,&imon,&iday,&ihour,&imin,&isec,1987);
      //printf("CSU hi scan:%d %d %d %d %d %d %d\n",iyear,iday,imon,jday,ihour,imin,isec);
      iday=jday;
#endif    
    
      /* check to see if day is in desired range */
      if (iasc >= 3 && iasc <= 5) { /* if local time of day discrimination is used */
	if (dstart < dend) { /* if dosen't cross year boundary */
	  if (iyear != year) goto label_350; /* skip further processing of this scan */
	  if ( iday < dstart - 1 ) goto label_350; /* skip further processing of this scan */
	}
      } else {
	if (iyear != year) goto label_350; /* skip further processing of this scan */
	if (iday < dstart) goto label_350; /* skip further processing of this scan */
	if (iday > dend)   goto label_350; /* done processing, skip rest of scan */
      }

      /* compute days since start of image data if cross year boundary */
      ktime=(iyear-year)*365;   /* days */
      if (ktime > 0) { /* more than a year */
	if (isleapyear(year)) ktime=ktime+1;
      }  else
	if (ktime<0) printf("*possible bug in ktime\n");
      
      /* compute time in mins since start of image data (assumes mstart=0) */
      ktime=((ktime+iday-dstart)*24+ihour)*60+imin;

      /* compute the orientation of the nadir track with respect to north */
      eqlon=360.0*(d->ORBIT[iscan]-d->IORBIT);
      if (eqlon<0.0) eqlon=eqlon+360.0;      

      /*
        find the longitude of the equator crossing of the middle measurement to use in computing the
        longitudes that separate ascending and descending for this rev */

      xhigh_lon=eqlon+90.0;
      xlow_lon =eqlon-90.0;
	   
      if (xhigh_lon >  180.0) xhigh_lon=xhigh_lon-360.0;
      if (xhigh_lon < -180.0) xhigh_lon=xhigh_lon+360.0;
      if (xlow_lon  >  180.0) xlow_lon =xlow_lon -360.0;
      if (xlow_lon  < -180.0) xlow_lon =xlow_lon +360.0;

      /* set asc/dsc flag for measurements */
      if (d->SC_LAT[iscan]-sc_last_lat < 0.0) 
	ascend=0;
      else
	ascend=1;
      sc_last_lat=d->SC_LAT[iscan];
      sc_last_lon=d->SC_LON[iscan];

      /* extract TB measurements for each scan.  the logic here works through
	 both hi and lo (A and B) scans with a single loop */

      iscan1=(iscan % 2);
      for (i=0; i < HI_SCAN; i++) { /* measurements loop to label_3401 */
	irec=irec+1;	/* count of pulses examined */

	/* for each output region and section */
	for (iregion=0; iregion<save_area.nregions; iregion++) { /* regions loop label_3400 */
	  /* ipolar=save_area.sav_ipolar[iregion]; */  /* 0=h, 1=v */
	  ibeam=save_area.sav_ibeam[iregion];  /* beam number */

	  /* determine indexing for hi and low scans */
	  if (ibeam < 6) { /* ssmi 19..37 GHz */
	    if (i > LO_SCAN-1) goto label_3400; /* skip non-existing low res measurements */
	    if (iscan1 -= 0) goto label_3400; /* skip every other along-track index */
	    ilow=iscan/2;
	    ihigh=i*2;
	    nmeas=LO_SCAN;
	  } else { 	/* ssmi 85 GHz */
	    ilow=iscan;
	    ihigh=i;
	    nmeas=HI_SCAN;	    
	  }

	  /* for this beam, get measurement, geometry, and location */

	  switch (ibeam) {
	  case 1:
	    tb=d->CEL_19H[i+ilow*LO_SCAN];             /*Tb measurement value */
	    break;
	  case 2:
	    tb=d->CEL_19V[i+ilow*LO_SCAN];
	    break;	    
	  case 3:
	    tb=d->CEL_22V[i+ilow*LO_SCAN];
	    break;
	  case 4:
	    tb=d->CEL_37H[i+ilow*LO_SCAN];
	    break;
	  case 5:
	    tb=d->CEL_37V[i+ilow*LO_SCAN];
	    break;
	  case 6:
	    tb=d->CEL_85H[i+iscan*HI_SCAN];
	    break;
	  case 7:
	    tb=d->CEL_85V[i+iscan*HI_SCAN];
	    break;
	  default:
	    printf("**** beam specification error \n");
	  }

#ifdef RSS
	  thetai= d->CEL_EIA[ihigh+iscan*HI_SCAN]; /* nominal incidence angle */
	  azang=  d->CEL_AZM[ihigh+iscan*HI_SCAN]; /* nominal azimuth angle */
	  cen_lon=d->CEL_LON[ihigh+iscan*HI_SCAN]; /* nominal longitude */
	  cen_lat=d->CEL_LAT[ihigh+iscan*HI_SCAN]; /* nominal latitude */
#else
	  if (nmeas==LO_SCAN) {
	    thetai= d->CEL_EIA_lo[i+ilow*LO_SCAN]; /* nominal incidence angle */
	    //azang=  d->CEL_AZM_lo[i+ilow*LO_SCAN]; /* nominal azimuth angle */
	    cen_lon=d->CEL_LON_lo[i+ilow*LO_SCAN]; /* nominal longitude */
	    cen_lat=d->CEL_LAT_lo[i+ilow*LO_SCAN]; /* nominal latitude */
	    //printf("lo Record %d %d %f %f %f %f\n", iscan,i,cen_lon,cen_lat,tb,thetai);    
	  } else {	      
	    thetai= d->CEL_EIA[i+iscan*HI_SCAN]; /* nominal incidence angle */
	    //azang=  d->CEL_AZM[i+iscan*HI_SCAN]; /* nominal azimuth angle */
	    cen_lon=d->CEL_LON[i+iscan*HI_SCAN]; /* nominal longitude */
	    cen_lat=d->CEL_LAT[i+iscan*HI_SCAN]; /* nominal latitude */
	    cen_lat=d->CEL_LAT_lo[i/2+(iscan/2)*LO_SCAN]; /* nominal latitude */
	    //printf("hi Record %d %d %f %f %f %f\n", iscan,i,cen_lon,cen_lat,tb,thetai);	  
	  }
#endif

	  //printf("Record %d %d %f %f %f %f\n", iscan,ihigh,cen_lon,cen_lat,tb,thetai);	  

	  if (tb == 0.0) goto label_3400; /* skip bad measurements */
	  if (thetai == 0.0) goto label_3400; /* skip bad measurements */
	  
	  /* compute angle between satellite nadir track and true north at the measurement position */
	  
	  /* first, determine if measurement is in the longitude range for ascending or descending */
	  inlonrange=0;
	  if (xhigh_lon > 0.0 && xlow_lon < 0.0) {
	    if (cen_lon <= xhigh_lon && cen_lon >= xlow_lon) inlonrange=1;
	  } else {
	    if (cen_lon <= xhigh_lon || cen_lon >= xlow_lon) inlonrange=1;
	  }
	  
	  /* then, compute the angle of the subfootprint ground track with respect to north */
	  a=cos(minlat[ihigh]*DTR);	  
	  b=cos(cen_lat*DTR);
	  if (b > 0.0) b=0.001;
	  if (cen_lat > 0.0) a=cos(maxlat[ihigh]*DTR);
	  rat=a/b;
	  if (rat >  1.0) rat=1.0;
	  if (rat < -1.0) rat=-1.0;  /* was 1.0 */
	  if (ascend && inlonrange || (!ascend) && (!inlonrange))
	    theta_orb=-asin(rat)*RTD; /* ascending */
	  else
	    theta_orb=-180.0+asin(rat)*RTD; /* descending */

	  /* compute the angle between the consecutive ellipses at the subsatellite ground point */
	  esep_ang=scan_ang/(nmeas-1.0);
	  
	  /* compute the angle between the line extending from the sub sat point
	     through the measurement point and the satellite path at the measurement */
	  ang2=scan_ang/2.0-(nmeas-i)*esep_ang;
	         
	  /* compute the estimated orientation of the antenna illumination ellipse
	     on the earth's surface with respect to north */
	  theta=ang2+theta_orb;

	  /* check ascending/descending orbit pass flag (0=both, 1=asc, 2=desc) */
	  iasc=save_area.sav_ascdes[iregion];
	  if (iasc != 0)
	    if (iasc == 1) {
	      if (!ascend) goto label_3400;
	    } else if (iasc == 2)
	      if (ascend) goto label_3400;

	  /* extract local-time-of-day split values */
	  tsplit1=save_area.sav_tsplit1[iregion]*60;
	  tsplit2=save_area.sav_tsplit2[iregion]*60;

	  cy=cen_lat;
	  cx=cen_lon;
	  if (cx >  180.0) cx=cx-360.0;
	  if (cx < -180.0) cx=cx+360.0;

	  /* region lat/lon extent */
	  lath=save_area.sav_lath[iregion];
	  latl=save_area.sav_latl[iregion];
	  lonh=save_area.sav_lonh[iregion];
	  lonl=save_area.sav_lonl[iregion];
	  dateline=save_area.sav_dateline[iregion];

	  //printf("center %d %d %f %f %f %f %f %f\n",i,iscan,cx,cy,lonl,lonh,latl,lath);

	  /* if a local-time-of-day image, compute the local time and see if it fits within LTOD window.
	     Note: data may be next UTC day */

	  if (iasc > 2 && iasc < 6) { /* apply LTOD considerations */
	    ctime = cx *4.0 + ktime; /* calculate the local time of day in minutes */
	    if (iasc == 3) { /* morning */
	      if (ctime < tsplit1+(24*60) || ctime > tsplit2+(24*60)) goto label_3400;
	      } else if (iasc == 5) { /* midday -- not used */
	      // if (ctime < (34*60) || ctime >= (42*60)) goto label_3400;
	      } else /* iasc==4 evening */
	      if (ctime <= tsplit2 || ctime >= tsplit1+(24*60)) goto label_3400;
	  } 


	  if (dateline) { /* convert lon to ascending order */
	    if (lonl < 0.0) lonl=lonl+360.0;
	    if (cx < -180.0) cx=cx+360.0;
	  } else {	/* convert lon to -180 to 180 range */
	    if (cx > 180.0) cx=cx-360.0;
	    if (cx < -180.0) cx=cx+360.0;
	    if (cx > 180.0) cx=cx-360.0;
	  }

	  /* check to see if center is within region */
	  if (!((cx > lonl) && (cx < lonh) &&
		(cy < lath) && (cy > latl))) goto label_3400;
	  
	  //printf("keep center %6.2f %6.2f %6.2f  %6.2f %6.2f %6.2f\n",cx,lonl,lonh,cy,latl,lath);

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
	  //printf("retain center %6.2f %6.2f %6.2f  %6.2f %6.2f %6.2f\n",cx,lonl,lonh,cy,latl,lath);

	  /* assign the center of the pixel containing the measurement location to
	     be the "new" measurement center lat/lon.  this "quantizes" the measurement
	     centers to the center of the output pixel */
	  x=ix2+0.5;
	  y=iy2+0.5;
	  pixtolatlon(x, y, &clon, &clat, projt, xdeg, ydeg, ascale, bscale, a0, b0);

	  //printf(" Check: %6d %4d,%4d %6.2f,%6.2f %6.2f,%6.2f %6.2f,%6.2f\n",iadd,ix2,iy2,x,y,cx,cy,clon,clat);			
	  //printf("    %1d %4d,%4d %1d %f %f %f %f %f %f\n",iregion,nsx,nsy,projt, xdeg, ydeg, ascale, bscale, a0, b0);

	  //printf("Keeping %f %f %d %d %d  %d %d %d\n",x,y,ix2,iy2,iadd,iregion,ibeam,iscan,ii);
	  
	  /* define size of box centered at(ix2,iy2) in which the gain response 
	     is computed for each pixel in the box and tested to see if
	     the response exceeds a threshold.  if so, it is used */
	  ixsize=dscale*box_size; /* hi scan */
	  iysize=dscale*box_size;
	  if (ixsize<1) ixsize=1;
	  if (iysize<1) iysize=1;
	  if (ibeam < 6) {  /* lo scan */
	    ixsize *= 2;
	    iysize *= 2;
	  }

	  /* define search box limits */
	  ixsize1=-ixsize;
	  ixsize2= ixsize;
	  if (ix2+ixsize1<1) ixsize1=1-ix2;
	  if (ix2+ixsize2>nsx) ixsize2=nsx-ix2;
	  iysize1=-iysize;
	  iysize2= iysize;
	  if (iy2+iysize1<0) iysize1=1-iy2; 
	  if (iy2+iysize2>=nsy) iysize2=nsy-iy2;

#ifdef DEBUG_RESPONSE
	  if (mod(mcnt,7) != 1 || mcnt > 535 || mcnt < 500) goto label_3400;  /* reduce count */
	  printf(" Az comp: %d %d %f %f %f %f\n",ix2,iy2,theta,azang,
		 dmod(theta-azang+720.0,360.0),dscale);
	  fprintf(resp_debug,"%d %d %d %d %f %f %f %f\n", iscan, iregion, ibeam, i, clat, clon, sc_last_lat, sc_last_lon);	  
	  fprintf(resp_debug,"%d %d %f %f %f %f %d %d %d\n",
		  ix2,iy2,theta,azang,dmod(theta-azang+720.0,360.0),dscale,nsx,nsy,iadd);
	  fprintf(resp_debug,"%f %d %d %d %d %d %d %d\n",
		  dscale,box_size,ixsize,iysize,ixsize1,ixsize2,iysize1,iysize2);
#endif

#ifdef RSS
	  //  printf(" Azimuth angle comparison: %f %f %f\n",theta,azang,dmod(theta-azang+720.0,360.0));
	  theta=azang;   /* use azimuth angle from file */
#endif

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
#ifdef SHORTLOC
		alat1=latlon_store[iadd1*2+  noffset[iregion]]/200.0;
		alon1=latlon_store[iadd1*2+1+noffset[iregion]]/175.0;
#else
		alat1=flatlon_store[iadd1*2+  noffset[iregion]];
		alon1=flatlon_store[iadd1*2+1+noffset[iregion]];
#endif

		/* compute lat/lon of pixel as a check of the pre-computed location */
		//pixtolatlon(((float) ix)+0.5, ((float) iy)+0.5, &clon, &clat,
		// 	  projt, xdeg, ydeg, ascale, bscale, a0, b0);
		//clat=nint(clat*200.0)/200.0; clon=nint(clon*175.0)/175.0;
		//printf("Loc check: %d %f %f  %f %f\n",iadd1,alat1,clat,alon1,clon);

		/* compute antenna pattern response at each pixel based on beam number, 
		   location, and projection rotation and scaling */
		rel_latlon(&x_rel,&y_rel,alon1,alat1,clon,clat);
		//x_rel=ix1*2.5; y_rel=iy1*5;
		sum=ssmi_response(x_rel,y_rel,theta,thetai,ibeam);

		if (sum > response_threshold) {
		  if (flatten) sum=1.0;    /* optionally flatten response */
		  fill_array[count]=iadd1; /* address of pixel */
		  response_array[count]=(short int) nint(sum*RESPONSEMULT); /* quantized response */
		  count++;
		  if (count >= MAXFILL) {
		    fprintf(stderr,"*** count overflow has occurred\n");		  
		    count=MAXFILL;
		  }
		
#ifdef DEBUG_RESPONSE
		} else
		  sum=-sum;

		x=ix+0.5;  /* compute exact lat/lon of each pixel in response array */
		y=iy+0.5;
		pixtolatlon(x, y, &plon, &plat, projt, xdeg, ydeg, ascale, bscale, a0, b0);
		fprintf(resp_debug,"%d %d %f %f %f %d %f %f %f %f\n",
			ix,iy,x_rel,y_rel,sum,iadd1,alon1,alat1,plon,plat);
#else
		}
#endif

	      }
	    }
	  }
	  
	  
	  /* write measurement and addresses to setup output file */
	  if (count > 1) {
	    jrec++; /* a count of total records written */
	    jrec2[iregion]++; /* records/region */
	    if (count >= MAXFILL) { /* error handling -- this should not occur! */
	      printf("*** count %d overflow=%d at %d\n",count,MAXFILL,jrec);
	      printf("center %f %f  %d %d %d  count %d\n",cen_lat,cen_lon,iscan,ii,ibeam,count);
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

	    //printf("Write %f %f %d %d %d  %d %d %d %d\n",tb,thetai,count,ktime,iadd,iregion,ibeam,iscan,ii);	    

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

	label_3400:; /* end of regions loop */
	}
      /*      label_3401:; /* end of measurements loop */
      }
      
    label_350:; /* end of scan loop */
    }


  label_3501:;  /* end of input file */
    /* printf("end of input file\n"); */

    /* input file has been processed */
    if (shortf) {
      printf("\nTotal input scans: %d  Total input pulses: %d\n",krec,irec);
      printf("Output Cells/Records: %d  Revs: %d  Last rev: %d\n",jrec,irev,lrev);
      printf("Region counts: ");
      for (j=0; j<save_area.nregions; j++)
	printf(" %d",jrec2[j]);
      printf("\n");
      printf("Input File Completed:  %s\n",fname);
      printf("Last Day %d in Range: %d - %d\n\n",iday,dstart,dend);
      printf("Number of measurements: %d\n",icc);
      printf("IPR count average:  %lf\n",(icc>0? cave/(float) icc: cave));
      printf("IPR count max,min:  %d %d \n",icmax,icmin);
      printf("Tb max,min:  %f %f \n\n",tbmax,tbmin);
  
      if (iday <= dend)
	printf("*** DAY RANGE IS NOT FINISHED ***\n");
      printf("End of day period reached %d %d  %d\n",iday,dend,irev);
    }
               /* input file loop */
    printf("Done with setup records %d %d\n",irec,krec);
  }

#ifdef DEBUG_RESPONSE
  fclose(resp_debug);
#endif
  
  /* close output setup files */
  for (j=0; j<save_area.nregions; j++) {
    printf("\nRegion %d %s beam %d records %d\n",save_area.sav_regnum[j],
	   save_area.sav_regname[j],save_area.sav_ibeam[j],jrec2[j]);
    no_trailing_blanks(save_area.sav_fname2[j]);
    printf("Output data written to %s\n",save_area.sav_fname2[j]);
    fclose(save_area.reg_lu[j]);
  }
  printf("\n");

  /* close input meta file */	    
  close(file_id);
  printf("Setup program successfully completed\n");

  return(0); /* successful termination */
}

/* *********************************************************************** */

FILE * get_meta(char *mname, char *outpath, 
		int *dstart, int *dend, int *mstart, int *mend, int *year, 
		char *prog_n, float prog_v,
		float *response_threshold, int *flatten, int *median_flag,
		int *inc_correct, float *b_correct, float *angle_ref, 
		int *KSAT, int nfile_select, region_save *a)
{
  /* read meta file, open output .setup files, write .setup file headers, and 
     store key parameters in memory */
 
  FILE *file_id, *ftemp;  

  int irecords=0,ireg;
  char line[100], lin[100];
  int asc_des;
  float lath,latl,lonh,lonl;
  int poleflag,regnum,projt=0;
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
  int ibeam, ninst=0;  

  int flag, flag_out, flag_region, flag_section, flag_files;
  float a_init,a_offset;
  int nits;

  char *s, *x;
  int z, nsection, isection, cnt;
  float tsplit1=1.0, tsplit2=13.0;

  iregion=0;
  ireg=0;
  ninst=0;

  printf("open meta file %s\n",mname);
  file_id=fopen(mname,"r");
  if (file_id == NULL) {
    printf("*** could not open input meta file %s\n",mname);
    return(file_id);
  }

  /* read meta file header info (applies to all regions) */

  flag=1;
  while (flag) {
    fgets(line,sizeof(line),file_id);
    no_trailing_blanks(line);
    if (ferror(file_id)) {
      printf("*** error reading meta file\n");
      flag=0;
    } else {
      
      if (strstr(line,"End_description") != NULL)
	flag=0;

      if (strstr(line,"Sensor") != NULL) {
	x = strchr(line,'=');
	strncpy(sensor,++x,40);
	printf("Sensor string='%s'\n",sensor);
	if (strncmp(sensor,"SSMI",4)==0) {	    
	  sscanf(&sensor[7],"%2d",KSAT);
	  printf(" SSMI platform %d\n",*KSAT);
	}	
      }

      if (strstr(line,"Instrument") != NULL) {
	x = strchr(line,'=');
	ninst=atoi(++x);
	printf("Instrument code=%d\n",ninst);
	*KSAT=ninst;	
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
	a_init=atof(++x);
      }      
      
      if (strstr(line,"A_offset") != NULL) {
	x = strchr(line,'=');
	a_offset=atof(++x);
      }
      
      if (strstr(line,"Max_iterations") != NULL) {
	x = strchr(line,'=');
	nits=atoi(++x);
      }
      
      if (strstr(line,"Reference_incidence_angle") != NULL) {
	x = strchr(line,'=');
	*angle_ref=atof(++x);
      }
      
      if (strstr(line,"Incidence_ang_correct") != NULL) {
	x = strchr(line,'=');
	*inc_correct=atof(++x);
      }
      
      if (strstr(line,"B_correct_value") != NULL) {
	x = strchr(line,'=');
	*b_correct=atof(++x);
      }
      
      if (strstr(line,"Response_threshold") != NULL) {
	x = strchr(line,'=');
	*response_threshold=atof(++x);
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
	asc_des=0;	/* use both asc/desc orbits */
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
	      latl=atof(++x);
	    }
      
	    if (strstr(line,"Latitude_high") != NULL) {
	      x = strchr(line,'=');
	      lath=atof(++x);
	    }
      
	    if (strstr(line,"Longitude_low") != NULL) {
	      x = strchr(line,'=');
	      lonl=atof(++x);
	    }
      
	    if (strstr(line,"Longitude_high") != NULL) {
	      x = strchr(line,'=');
	      lonh=atof(++x);
	    }

	    if (strstr(line,"Dateline_crossing") != NULL) {
	      x = strchr(line,'='); x++;	
	      if (*x== 'F' || *x== 'f') dateline=0;
	      else dateline=1;	
	    }

	    if (strstr(line,"Polar_flag") != NULL) {
	      x = strchr(line,'='); x++;	
	      if (*x== 'F' || *x== 'f') poleflag=0;
	      else poleflag=1;	
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
		    aorglat =atof(++x);
		    ydeg=aorglat;
		  }

		  if (strstr(line,"Projection_origin_y") != NULL) {
		    x = strchr(line,'=');
		    aorglon =atof(++x);
		    xdeg=aorglon;
		  }

		  if (strstr(line,"Projection_offset_x") != NULL) {
		    x = strchr(line,'=');
		    a0=atof(++x);
		  }

		  if (strstr(line,"Projection_offset_y") != NULL) {
		    x = strchr(line,'=');
		    b0=atof(++x);
		  }

		  if (strstr(line,"Projection_scale_x") != NULL) {
		    x = strchr(line,'=');
		    ascale=atof(++x);
		  }

		  if (strstr(line,"Projection_scale_y") != NULL) {
		    x = strchr(line,'=');
		    bscale=atof(++x);
		  }

		  if (strstr(line,"Projection_dim_x") != NULL) {
		    x = strchr(line,'=');
		    xdim=atof(++x);
		  }

		  if (strstr(line,"Projection_dim_y") != NULL) {
		    x = strchr(line,'=');
		    ydim=atof(++x);
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
		    ydeg2=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_origin_y") != NULL) {
		    x = strchr(line,'=');
		    xdeg2=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_offset_x") != NULL) {
		    x = strchr(line,'=');
		    a02=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_offset_y") != NULL) {
		    x = strchr(line,'=');
		    b02=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_scale_x") != NULL) {
		    x = strchr(line,'=');
		    ascale2=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_scale_y") != NULL) {
		    x = strchr(line,'=');
		    bscale2=atof(++x);
		  }

		  if (strstr(line,"Grid_scale_x") != NULL) {
		    x = strchr(line,'=');
		    non_size_x=atof(++x);
		  }

		  if (strstr(line,"Grid_scale_y") != NULL) {
		    x = strchr(line,'=');
		    non_size_y =atof(++x);
		  }

		  if (strstr(line,"Grid_size_x") != NULL) {
		    x = strchr(line,'=');
		    nsx2=atoi(++x);
		  }

                  if (strstr(line,"Local_time_split1") != NULL) {
		    x = strchr(line,'=');
		    tsplit1=atof(++x);
		  }

		  if (strstr(line,"Local_time_split2") != NULL) {
		    x = strchr(line,'=');
		    tsplit2=atof(++x);
		  }

		  if (strstr(line,"Grid_size_y") != NULL) {
		    x = strchr(line,'=');
		    nsy2=atoi(++x);
		  }

		  if (strstr(line,"Setup_file_EXTENSION") != NULL) {
		    x = strchr(line,'=');
		    //		    if (nfile_select> 0) {
		    //sprintf(fname2,"%3.3d%s",nfile_select,++x);
		    //no_trailing_blanks(fname2);
		    //}		    
		  }      

		  if (strstr(line,"Setup_file") != NULL) {
		    x = strchr(line,'=');
		    //if (nfile_select<=0) {
		      strncpy(fname2,++x,40);
		      no_trailing_blanks(fname2);   
		      //}
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
		      /*
           write (reg_lu[iregion]) irecords
           write (reg_lu[iregion]) nsx,nsy,ascale,bscale,a0,b0,xdeg,ydeg
           write (reg_lu[iregion]) dstart,dend,mstart,mend,year,regnum,projt,
     $          ipolar,latl,lonl,lath,lonh,regname
           write (reg_lu[iregion]) nsx2,nsy2,non_size_x,non_size_y,
     $          ascale2,bscale2,a02,b02,xdeg2,ydeg2
		      */


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
		      sprintf(lin," Response_threshold=%f",response_threshold);
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
		      sprintf(lin," Response_Multiplier=%f",RESPONSEMULT);
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
			  if (flag_out) {
			    fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
			    for(z=0;z<100;z++)lin[z]=' ';
			    sprintf(lin," End_header");
			    fwrite(lin,100,1,a->reg_lu[iregion-1]);
			    fwrite(&cnt,4,1,a->reg_lu[iregion-1]);
			  }
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

void compute_locations(region_save *a, int *nregions, int **noffset, short int **latlon_store, float **flatlon_store) 
{  
  /* compute the lat,lon of each pixel in the image regions and store in
     global arrays.  This reduces the computational load */

  int iregion, nspace;
  char *p, local[]="./";
  int iadd,ix,iy,nsize,iadd0;
  float x,y,clat,clon;
  FILE *f;
  char tempname[180],lastname[180]="\0",line[1024];

  p=getenv("SIR_areas");
  if (p==NULL) p=local;

  *nregions=a->nregions;

  /* determine how much memory is required */
  nspace=0;
  for (iregion=0; iregion < a->nregions; iregion++)
    nspace=nspace+2*a->sav_nsx[iregion]*a->sav_nsy[iregion];

  /* allocate memory for storage of location arrays */
  *noffset=malloc(sizeof(int)*(a->nregions+1));
#ifdef SHORTLOC
  *latlon_store=malloc(sizeof(short int)*nspace);
#else
  *flatlon_store=malloc(sizeof(float)*nspace);
  *latlon_store = (short int *) *flatlon_store;  
#endif
  if (*noffset==NULL || *latlon_store==NULL) {
    fprintf(stderr,"*** pixel location buffer allocation error  %d %d\n",*nregions,nspace);
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
#ifdef SHORTLOC
    sprintf(tempname,"%4.4d-%4.4d-%2.2d-%4.4d-%4.4d-%4.4d-%4.4d.loc",
	    a->sav_nsx[iregion], a->sav_nsy[iregion], a->sav_projt[iregion],
	    abs(nint(a->sav_a0[iregion])), abs(nint(a->sav_b0[iregion])),
	    abs(nint(a->sav_xdeg[iregion])),abs(nint(a->sav_ydeg[iregion])));
#else
    sprintf(tempname,"%4.4d-%4.4d-%2.2d-%4.4d-%4.4d-%4.4d-%4.4d.floc",
	    a->sav_nsx[iregion], a->sav_nsy[iregion], a->sav_projt[iregion],
	    abs(nint(a->sav_a0[iregion])), abs(nint(a->sav_b0[iregion])),
	    abs(nint(a->sav_xdeg[iregion])) ,abs(nint(a->sav_ydeg[iregion])));
#endif

    if (strncmp(lastname,tempname,180)==0) {  /* new file name is same as last */
      /* so save time and I/O re-use prior load or computation */
      for (iy=0; iy<a->sav_nsy[iregion]; iy++) {
	for (ix=0; ix<a->sav_nsx[iregion]; ix++) {	  
	  iadd=a->sav_nsx[iregion]*iy+ix; /* zero-based lexicographic pixel address */
	  iadd0=2*iadd+(*noffset)[iregion-1];
	  iadd=2*iadd+(*noffset)[iregion];	  
#ifdef SHORTLOC	
	  (*latlon_store)[iadd]=  (*latlon_store)[iadd0];	  
	  (*latlon_store)[iadd+1]=(*latlon_store)[iadd0+1];
#else	
	  (*flatlon_store)[iadd]=  (*flatlon_store)[iadd0];	  
	  (*flatlon_store)[iadd+1]=(*flatlon_store)[iadd0+1];
#endif	
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
#ifdef SHORTLOC
	if (fread(&(*latlon_store)[(*noffset)[iregion]], 2, nsize*2, f)!=2*nsize) {
	  printf("*** error reading precompute file %s\n",line);	
	  fclose(f);
	  goto label_skip;
	}
#else
	if (fread(&(*flatlon_store)[(*noffset)[iregion]], 4, nsize*2, f)!=2*nsize) {
	  printf("*** error reading precompute file %s\n",line);	
	  fclose(f);
	  goto label_skip;
	}
#endif
	fclose(f);
	goto label_read;
      }

    label_skip:;

      /* compute pixel locations */
      for (iy=0; iy<a->sav_nsy[iregion]; iy++) {
	y=iy+1.5; /* center of pixel, 1-based */
	for (ix=0; ix<a->sav_nsx[iregion]; ix++) {
	  x=ix+1.5; /* center of pixel, 1-based */
	  pixtolatlon(x, y, &clon, &clat, a->sav_projt[iregion], 
		      a->sav_xdeg[iregion], a->sav_ydeg[iregion],
		      a->sav_ascale[iregion], a->sav_bscale[iregion],
		      a->sav_a0[iregion],     a->sav_b0[iregion]);
	  iadd=a->sav_nsx[iregion]*iy+ix; /* zero-based lexicographic pixel address */
	  if (iadd<0) iadd=0;
	  if (iadd>=nsize) iadd=0;
	  iadd=2*iadd+(*noffset)[iregion];
#ifdef SHORTLOC	
	  (*latlon_store)[iadd]=  nint(clat*200.0);
	  (*latlon_store)[iadd+1]=nint(clon*175.0);
#else	
	  (*flatlon_store)[iadd]=  clat;
	  (*flatlon_store)[iadd+1]=clon;
#endif	
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
#ifdef SHORTLOC
	if (fwrite(&(*latlon_store)[(*noffset)[iregion]], 2, nsize*2, f)!=2*nsize) {
	  fprintf(stderr,"*** error writing location file -- file is now invald\n");
	  fclose(f);
	  exit(-1);
	}
#else
	if (fwrite(&(*flatlon_store)[(*noffset)[iregion]], 4, nsize*2, f)!=2*nsize) {
	  fprintf(stderr,"*** error writing location file -- file is now invald\n");
	  fclose(f);
	  exit(-1);
	}
#endif
	fclose(f);
      }
    }    

    label_read:;
  }
}

/* *********************************************************************** */

#ifdef RSS

void Ferror(int *err, int i)
{
  fprintf(stderr,"*** file read error at %d ***\n",i); fflush(stderr);
  *err=-1;
  return;
}

int Sread(float *val, short int *b, int r, int c, FILE *fid, float sc, float off, int flag)
{ /* binary RSS SSM/I V7 file reader */
  int i, j;

  if (fread(b, 2, r*c, fid) != r*c) return(0);

  for (i=0; i<c; i++)
    for (j=0; j<r; j++)
      if (b[i*r+j] > 0 || flag)
	val[i*r+j]=b[i*r+j]*sc+off;
      else
	val[i*r+j]=0.0;
    return(1);
}


int read_ssmiRSS_TB_V07_binary(char *fname, ssmiRSSv7 *d, int verbose)
{
  FILE *fid;  
  int ierr=0, i, j;
  short int b[HI_SCAN*HSCANS];  /* read buffer */

  fid=fopen(fname,"r");
  if (fid==NULL) {
    fprintf(stderr,"*** could not open TB file %s\n",fname);
    return(-2);   /* skip reading file on error */
  }

  if (fread(&d->KSAT,    4, 1, fid)!=1) Ferror(&ierr, 0);
  if (fread(&d->IORBIT,  4, 1, fid)!=1) Ferror(&ierr, 1);
  if (fread(&d->NUMSCAN, 4, 1, fid)!=1) Ferror(&ierr, 2);
  if (verbose) {
    printf("Input file %s\n",fname);    
    printf(" SSMI Hi rec scans = %d\n\n",d->NUMSCAN);
  }
  if (fread(&d->ASTART_TIME, NUMCHAR-1, 1, fid)!=1) Ferror(&ierr, 3);
  d->ASTART_TIME[NUMCHAR-1]='\0';  
  if (fread(&d->SCAN_TIME, 8, HSCANS, fid)!=HSCANS) Ferror(&ierr, 4);
  if (fread(&d->ORBIT,  8, HSCANS, fid)!=HSCANS) Ferror(&ierr, 5);
  if (fread(&d->SC_LAT, 4, HSCANS, fid)!=HSCANS) Ferror(&ierr, 6);
  if (fread(&d->SC_LON, 4, HSCANS, fid)!=HSCANS) Ferror(&ierr, 7);
  if (fread(&d->SC_ALT, 4, HSCANS, fid)!=HSCANS) Ferror(&ierr, 8);
  if (fread(&d->IQUAL_FLAG, 4, HSCANS, fid)!=HSCANS) Ferror(&ierr, 9);
  /* using address of zero index in call to Sread to make compiler happy */
  if (Sread(&d->CEL_LAT[0], b, HI_SCAN, HSCANS, fid, 0.01,   0., 1)==0) Ferror(&ierr, 10);
  if (Sread(&d->CEL_LON[0], b, HI_SCAN, HSCANS, fid, 0.01, 180., 1)==0) Ferror(&ierr, 11);
  if (Sread(&d->CEL_EIA[0], b, HI_SCAN, HSCANS, fid, 0.002, 45., 1)==0) Ferror(&ierr, 12);
  if (Sread(&d->CEL_AZM[0], b, HI_SCAN, HSCANS, fid, 0.01, 180., 1)==0) Ferror(&ierr, 13);
  if (Sread(&d->CEL_SUN[0], b, HI_SCAN, HSCANS, fid, 0.01,   0., 1)==0) Ferror(&ierr, 14);
  if (Sread(&d->CEL_LND[0], b, HI_SCAN, HSCANS, fid, 0.4,    0., 1)==0) Ferror(&ierr, 15);
  if (fread(&d->CEL_ICE, 2, HI_SCAN*HSCANS, fid)!=HI_SCAN*HSCANS) Ferror(&ierr, 16);
  if (Sread(&d->CEL_85V[0], b, HI_SCAN, HSCANS, fid, 0.01, 100., 0)==0) Ferror(&ierr, 17);
  if (Sread(&d->CEL_85H[0], b, HI_SCAN, HSCANS, fid, 0.01, 100., 0)==0) Ferror(&ierr, 18);
  if (Sread(&d->CEL_19V[0], b, LO_SCAN, LSCANS, fid, 0.01, 100., 0)==0) Ferror(&ierr, 19);
  if (Sread(&d->CEL_19H[0], b, LO_SCAN, LSCANS, fid, 0.01, 100., 0)==0) Ferror(&ierr, 20);
  if (Sread(&d->CEL_22V[0], b, LO_SCAN, LSCANS, fid, 0.01, 100., 0)==0) Ferror(&ierr, 21);
  if (Sread(&d->CEL_37V[0], b, LO_SCAN, LSCANS, fid, 0.01, 100., 0)==0) Ferror(&ierr, 22);
  if (Sread(&d->CEL_37H[0], b, LO_SCAN, LSCANS, fid, 0.01, 100., 0)==0) Ferror(&ierr, 23);

  /* close input file */
  fclose(fid);

  return(ierr);
}

void read_ssmiRSS_minmaxlat(float *minlat, float *maxlat)
{ /* read fine with min and max latitudes for each scan position from file 
     the location of this file is specified in the environment variable 'RSS_path' */

  char *p, line[260];
  //char RSS_path[]="/auto/users/long/research/NSIDC/RSS/";
  char RSS_path[]="../../ref/"; /* default location */
  int i;
  FILE *f;
  
  p=getenv("RSS_path");
  if (p == NULL)
    p=RSS_path;

  sprintf(line,"%s/minmaxlat.dat",p);
    printf("Read ssmiRSS minmax lat file: %s\n",line);
  f=fopen(line,"r");
  if (f==NULL) {
    fprintf(stderr,"*** failed to open file %s\n",line);
    exit(-1);
  }
  for (i=0; i<HI_SCAN; i++)
    fscanf(f,"%f %f",&minlat[i],&maxlat[i]);
  fclose(f);
}

#else

#include <netcdf.h>

#define ERRCODE -1 /* Handle errors by printing an error message */
#define ERR(e, ierr) { printf("Error: %s %d\n", nc_strerror(e), ierr=ERRCODE); }

int read_ssmiCSU_TB(char *inpfile, ssmiCSU *d, int verbose)       
{ /* Read ssmi CSU NetCDF FCDR File */

  char  *p;
  int    i,n;
  int    ncid;
  int    retval;
  char   space[100];
  int    ierr=0;  

  /* netdef varid variables */
  int    recid;
  int    nlscan_dimid;
  int    nhscan_dimid;
  int    revlr_varid;
  int    scantimelr_varid;
  int    scandatelr_varid;
  int    sclatlr_varid;
  int    sclonlr_varid;
  int    scaltlr_varid;
  int    sclathr_varid;
  int    sclonhr_varid;
  int    scalthr_varid;
  int    lat_lr_varid;
  int    lon_lr_varid;
  int    eia_lr_varid;
  int    sun_lr_varid;
  int    qual_lr_varid;
  int    revhr_varid;
  int    scantimehr_varid;
  int    scandatehr_varid;
  int    lat_hr_varid;
  int    lon_hr_varid;
  int    eia_hr_varid;
  int    sun_hr_varid;
  int    qual_hr_varid;
  int    tb19v_varid;
  int    tb19h_varid;
  int    tb22v_varid;
  int    tb37v_varid;
  int    tb37h_varid;
  int    tb85v_varid;
  int    tb85h_varid;

  /* Declare header variables */
  size_t nscan;
  int    scid;
  int    orbitnumber;
  char   title[100];
  char   author[100];
  char   email[100];
  char   url[100];
  char   institution[100];
  char   version[100];
  char   revision_date[100];
  char   platform[100];
  char   sensor[100];
  char   orbit_number[100];
  char   source[100];
  char   filename[100];
  char   startdate[100];
  char   enddate[100];
  char   created[100];

  /* Declare buffer variable */
  unsigned char buffer[HSCANS*HI_SCAN]; /* working buffer larger than largest data element */  

  /* Open NetCDF File and get variable information */
  if (verbose) {
    printf("Open file %s\n",inpfile); 
  }  
  if (retval = nc_open(inpfile, NC_NOWRITE, &ncid))              ERR(retval, ierr);
  //printf("file opened %d\n",retval, &ierr);  
  if (retval = nc_inq_unlimdim(ncid, &recid))                    ERR(retval, ierr);
  if (retval = nc_inq_dimid(ncid, "nscan_lores", &nlscan_dimid)) ERR(retval, ierr);
  if (retval = nc_inq_dimid(ncid, "nscan_hires", &nhscan_dimid)) ERR(retval, ierr);
  if (retval = nc_inq_dimlen(ncid, nlscan_dimid, &nscan))        ERR(retval, ierr);
  d->nlscan = nscan;
  if (retval = nc_inq_dimlen(ncid, nhscan_dimid, &nscan))        ERR(retval, ierr);
  d->NUMSCAN = nscan;
  if (verbose) {
    printf(" SSMI Lo res scans  = %d\n",d->nlscan);
    printf(" SSMI Hi res scans  = %d\n\n",d->NUMSCAN);
  }

  /* Read in Global attribute metadata fields */
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "title",               title))         ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "creator_name",        author))        ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "creator_email",       email))         ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "creator_url",         url))           ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "institution",         institution))   ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "product_version",     version))       ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "revision_date",       revision_date)) ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "platform",            platform))      ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "sensor",              sensor))        ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "orbit_number",        orbit_number))  ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "source",              source))        ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "id",                  filename))      ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "time_coverage_start", startdate))     ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "time_coverage_end",   enddate))       ERR(retval, ierr);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "date_created",        created))       ERR(retval, ierr);

  if (verbose) {
    printf(" Title         = %s\n",title);
    printf(" Author        = %s\n",author);
    printf(" Contact       = %s\n",email);
    printf(" URL           = %s\n",url);
    printf(" Institution   = %s\n",institution);
    printf(" Version       = %s\n",version);
    printf(" Revision Date = %s\n",revision_date);
    printf(" Satellite     = %s\n",platform);
    printf(" Sensor        = %s\n",sensor);
    printf(" Orbit Number  = %s\n",orbit_number);
    printf(" Source File   = %s\n",source);
    printf(" Filename      = %s\n",filename);
    printf(" Granule Start = %s\n",startdate);
    printf(" Granule End   = %s\n",enddate);
    printf(" Creation Date = %s\n\n",created);
  }

  if (d->nlscan > 0) {
    /* Read netcdf varids */
    if (retval = nc_inq_varid(ncid, "orbit_lores",          &revlr_varid))      ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "scan_time_lores",      &scantimelr_varid)) ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "scan_datetime_lores",  &scandatelr_varid)) ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "spacecraft_lat_lores", &sclatlr_varid))    ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "spacecraft_lon_lores", &sclonlr_varid))    ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "spacecraft_alt_lores", &scaltlr_varid))    ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "lat_lores",            &lat_lr_varid))     ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "lon_lores",            &lon_lr_varid))     ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "eia_lores",            &eia_lr_varid))     ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "sun_glint_lores",      &sun_lr_varid))     ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "quality_lores",        &qual_lr_varid))    ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "fcdr_tb19v",           &tb19v_varid))      ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "fcdr_tb19h",           &tb19h_varid))      ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "fcdr_tb22v",           &tb22v_varid))      ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "fcdr_tb37v",           &tb37v_varid))      ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "fcdr_tb37h",           &tb37h_varid))      ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "orbit_hires",          &revhr_varid))      ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "scan_time_hires",      &scantimehr_varid)) ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "scan_datetime_hires",  &scandatehr_varid)) ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "spacecraft_lat_hires", &sclathr_varid))    ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "spacecraft_lon_hires", &sclonhr_varid))    ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "spacecraft_alt_hires", &scalthr_varid))    ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "lat_hires",            &lat_hr_varid))     ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "lon_hires",            &lon_hr_varid))     ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "eia_hires",            &eia_hr_varid))     ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "sun_glint_hires",      &sun_hr_varid))     ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "quality_hires",        &qual_hr_varid))    ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "fcdr_tb85v",           &tb85v_varid))      ERR(retval, ierr);
    if (retval = nc_inq_varid(ncid, "fcdr_tb85h",           &tb85h_varid))      ERR(retval, ierr);

    /* read netcdf data arrays */
    if (retval = nc_get_var_double(ncid,scantimelr_varid, &d->SCAN_TIME_lo[0])) ERR(retval, ierr);
    //if (retval = nc_get_var_text(ncid,  scandatelr_varid, &datetimelr[0][0]))   ERR(retval, ierr);
    if (retval = nc_get_var_double(ncid,revlr_varid,      &d->ORBIT_lo[0]))     ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, sclatlr_varid,    &d->SC_LAT_lo[0]))    ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, sclonlr_varid,    &d->SC_LON_lo[0]))    ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, scaltlr_varid,    &d->SC_ALT_lo[0]))    ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, lat_lr_varid,     &d->CEL_LAT_lo[0]))   ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, lon_lr_varid,     &d->CEL_LON_lo[0]))   ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, eia_lr_varid,     &d->CEL_EIA_lo[0]))   ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, tb19v_varid,      &d->CEL_19V[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, tb19h_varid,      &d->CEL_19H[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, tb22v_varid,      &d->CEL_22V[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, tb37v_varid,      &d->CEL_37V[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, tb37h_varid,      &d->CEL_37H[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_double(ncid,scantimehr_varid, &d->SCAN_TIME[0]))    ERR(retval, ierr);
    //if (retval = nc_get_var_text(ncid,  scandatehr_varid, &datetimehr[0]))   ERR(retval, ierr);
    if (retval = nc_get_var_double(ncid,revhr_varid,      &d->ORBIT[0]))        ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, sclathr_varid,    &d->SC_LAT[0]))       ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, sclonhr_varid,    &d->SC_LON[0]))       ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, scalthr_varid,    &d->SC_ALT[0]))       ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, lat_hr_varid,     &d->CEL_LAT[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, lon_hr_varid,     &d->CEL_LON[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, eia_hr_varid,     &d->CEL_EIA[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, tb85v_varid,      &d->CEL_85V[0]))      ERR(retval, ierr);
    if (retval = nc_get_var_float(ncid, tb85h_varid,      &d->CEL_85H[0]))      ERR(retval, ierr);

    /* read signed char variables into buffer and convert to ints */
    if (retval = nc_get_var_schar(ncid, sun_lr_varid,     buffer))              ERR(retval, ierr);
    for (i=0; i<d->nlscan*LO_SCAN; i++) d->CEL_SUN_lo[i]=buffer[i];
    if (retval = nc_get_var_schar(ncid, qual_lr_varid,    buffer))              ERR(retval, ierr);
    for (i=0; i<d->nlscan*LO_SCAN; i++) d->IQUAL_FLAG_lo[i]=buffer[i];
    if (retval = nc_get_var_schar(ncid, sun_hr_varid,     buffer))              ERR(retval, ierr);
    for (i=0; i<d->NUMSCAN*HI_SCAN; i++) d->CEL_SUN[i]=buffer[i];
    if (retval = nc_get_var_schar(ncid, qual_hr_varid,    buffer))              ERR(retval, ierr);
    for (i=0; i<d->NUMSCAN*HI_SCAN; i++) d->IQUAL_FLAG[i]=buffer[i];
  }

  /* Close the output data file, freeing all netcdf resources. */
  if (retval = nc_close(ncid)) ERR(retval, ierr);

  /* Load header variables into output header data structure */
  p=rindex(platform,'F');
  sscanf(p+1,"%2d",&scid);
  sscanf(orbit_number,"%5d",&orbitnumber);
  d->KSAT   = scid;
  d->IORBIT = orbitnumber;
  if (verbose) {
    printf("Spacecraft ID = %2d\n",scid);
    printf("Orbit Number  = %5d\n",orbitnumber);
  }

  /* copy string header values */
  strncpy(&d->ASTART_TIME[0],startdate,NUMCHAR);
  strncpy(&d->title[0],title,100);
  strncpy(&d->author[0],author,100);
  strncpy(&d->email[0],email,100);
  strncpy(&d->url[0],url,100);
  strncpy(&d->institution[0],institution,100);
  strncpy(&d->version[0],version,100);
  strncpy(&d->revision_date[0],revision_date,100);
  strncpy(&d->platform[0],platform,100);
  strncpy(&d->sensor[0],sensor,100);
  strncpy(&d->startdate[0],startdate,100);
  strncpy(&d->enddate[0],enddate,100);
  strncpy(&d->created[0],created,100);
  strncpy(&d->inpfile[0],source,100);
  strncpy(&d->outfile[0],filename,100);

  /* Return to main program */
  return(ierr);
}

void read_ssmiRSS_minmaxlat(float *minlat, float *maxlat)
{ /* read fine with min and max latitudes for each scan position from file 
     the location of this file is specified in the environment variable 'RSS_path' */

  char *p, line[260];
  //char RSS_path[]="/auto/users/long/research/NSIDC/RSS/";
  char RSS_path[]="../../ref/"; /* default location */
  int i;
  FILE *f;
  
  p=getenv("RSS_path");
  if (p == NULL)
    p=RSS_path;

  sprintf(line,"%s/minmaxlat.dat",p);
    printf("Read ssmiRSS minmax lat file: %s\n",line);
  f=fopen(line,"r");
  if (f==NULL) {
    fprintf(stderr,"*** failed to open file %s\n",line);
    exit(-1);
  }
  for (i=0; i<HI_SCAN; i++)
    fscanf(f,"%f %f",&minlat[i],&maxlat[i]);
  fclose(f);
}

#endif

    
/* *********************************************************************** */

float ssmi_response(float x_rel, float y_rel, float theta, float thetai, int ibeam)
{
  /* Compute an estimate of the ssmi beam response (weight) in normal space
     given a location (in km, N-E=(x,y)), azimuth angle (theta),
     3dB antenna pattern scale factor (bigang), and the footprint
     sizes in cross track and along track.  

     inputs:
       x_rel,y_rel : relative offset from beam center in km
       theta : pattern rotation in deg
       thetai : incidence angle in deg (not used)
       ibeam : beam number

     Convert km location to coordinate system with axis
     lined up with the elliptical antenna pattern

                The rotation matrix looks like this where theta is a
		CCW rotation of the input coordinates x,y

		--  --   ---                      ---  --   --
		|    |   |                          |  |     |
		| x1 |   | cos(theta)   -sin(theta) |  |  x  |
		|    | = |                          |  |     |
		| y1 |   | sin(theta)    cos(theta) |  |  y  |
		|    |   |                          |  |     |
		-- --    ---                      ---  --   --
  */

  static float lnonehalf=-0.6931471;  /* ln(0.5) */
  static int NBEAMS=7;  
  static float geom[]={/* along,cross beam sizes (km) and beamwidth (deg) */
    /* SSM/I 3dB footprint geometries from (Hollinger, 1990) */
    69.0, 43.0, 1.93, 1.86,  /* beam 1 19h */
    69.0, 43.0, 1.93, 1.88,  /* beam 2 19v */
    60.0, 40.0, 1.83, 1.60,  /* beam 3 22  */
    37.0, 28.0, 1.27, 1.00,  /* beam 4 37h */
    37.0, 29.0, 1.31, 1.00,  /* beam 5 37v */
    15.0, 13.0, 0.60, 0.41,  /* beam 6 85h */
    15.0, 13.0, 0.60, 0.42}; /* beam 7 85v */

  float x, y, cross_beam_size, along_beam_size, t1, t2, weight;

  /* rotate coordinate system to align with look direction */
  x=cos(theta*DTR)*x_rel - sin(theta*DTR)*y_rel;
  y=sin(theta*DTR)*x_rel + cos(theta*DTR)*y_rel;
  
  /* compute approximate antenna response
     Antenna weighting is estimation from SSMI Users Guide 21-27 */
  along_beam_size=geom[(ibeam-1)*4  ];
  cross_beam_size=geom[(ibeam-1)*4+1];
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

  r=(1.0-(sin(rlat*DTR)*sin(rlat*DTR))*FLAT)*AEARTH;

  rel_rlat=alat-rlat;
  rel_rlon=alon-rlon;
  if (abs(rel_rlon) > 180.0) {
    if (rel_rlon > 0.0)
      rel_rlon=rel_rlon-360.0;
    else
      rel_rlon=rel_rlon+360.0;
  }
  r2=r*cos(rlat*DTR);
  *x_rel=r2*sin(rel_rlon*DTR);
  *y_rel=r*sin(rel_rlat*DTR)+(1.-cos(rel_rlon*DTR))*sin(rlat*DTR)*r2;
}

/* *********************************************************************** */

float km2pix(float *x, float *y, int iopt, float xdeg, float ydeg, 
	     float ascale, float bscale, float a0, float b0, int *stat)
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

  float radearth=6378.135;       /* radius of the earth in km */
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

/* *********************************************************************** */
