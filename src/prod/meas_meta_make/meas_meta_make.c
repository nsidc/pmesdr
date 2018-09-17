/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_meta_make.c

  generates SIR meta file for the MEaSUREs project

  translated to C by DGL at BYU 03/01/2014 from ssmi_meta_make3.f 

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef JANUSicc
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <time.h>

#include <cetb.h>
#include <sir_geom.h>

#define prog_version 1.2 /* program version */
#define prog_name "meas_meta_make"

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define mod(a,b) ((a) % (b))
#define abs(x) ((x) >= 0 ? (x) : -(x))

#define TRUE 1
#define FALSE 0

/****************************************************************************/
/* default location of the SIR standard region definition */

char rname[] = "regiondef1.dat";  /* file defining region codes */

/********************************************************************/

/* function prototypes */

static int get_region_parms( FILE *mout, int *argn, char *argv[], int F_num,
			     float response_threshold_dB, int resolution_ind );

static int get_file_names(FILE *mout, int *argn, char *argv[]);

static void getregdata_fromcetb(int regnum, int resolution_ind, int *iproj, int *dateline, float *latl,
				float *lonl, float *lath, float *lonh, char *regname);

/****************************************************************************/
/* note: program is designed to be run from command line and NOT interactively
*/

int main(int argc,char *argv[])
{
 
  char mname[256];  
  time_t tod;
  char ltime[29];
  int argn=0;
  char platform[28];
  int Fn=0;
  int resolution_ind=0;
  cetb_platform_id F_num=CETB_NO_PLATFORM;
  float response_threshold_dB;
  char *option;
  FILE *mout;

  /*
   * Default response pattern minimum threshold, in dB.
   * Command-line option -t will override this.
   */
  response_threshold_dB = -8.0;

  /* Check for command line options */
  while ( --argc > 0 && (*++argv)[0] == '-' ) {
    for ( option = argv[0]+1; *option != '\0'; option++ ) {
      switch (*option) {
	case 't':
	  ++argv; --argc;
	  if ( sscanf( *argv, "%f", &response_threshold_dB ) != 1 ) {
	    fprintf( stderr, "%s: ERROR reading -t value\n", __FILE__ );
	    exit( -1 );
	  }
	  if ( response_threshold_dB >= 0. ) {
	    fprintf( stderr, "%s: ERROR -t option should be < 0.\n",
		     __FILE__ );
	    exit( -1 );
	  }
	  break;
	case 'r':
	  ++argv; --argc;
	  if ( sscanf( *argv, "%d", &resolution_ind ) != 1 ) {
	    fprintf( stderr, "%s: ERROR reading -r value\n", __FILE__ );
	    exit( -1 );
	  }
	  if ( resolution_ind < 0 || resolution_ind > 3 ) {
	    fprintf( stderr, "%s: ERROR -r option should be 0, 1 or 2 %d\n",
		     __FILE__, resolution_ind );
	    exit( -1 );
	  }
	  break;
      default:
	fprintf( stderr, "%s: ERROR invalid option %c\n",
		 __FILE__, *option );
	exit( -1 );
      }
    }
  }
  
  fprintf( stderr,
	   "%s: MEaSures Meta_Make Program\nProgram: %s  Version: %f\n\n",
	   __FILE__, prog_name,prog_version);

  if (argc < 7) {
    fprintf( stderr,
	     "\n%s: incorrect number of input arguments\n",
	     __FILE__);
    fprintf( stderr,
	     "\n%s: usage: meas_meta_make [-t threshold] [-r resolution] "
	     "meta_name platform start_day stop_day year def in_list\n\n",
	     __FILE__);
    fprintf( stderr, " %s: options:\n", __FILE__ );
    fprintf( stderr, "   %s: -t threshold = response threshold, dB, "
	     "default is -8. dB\n", __FILE__ );
    fprintf( stderr, "   %s: -r resolution flag = 0 (25km) 1 (24 km) 2 (36 km), "
	     "default is 0 for 25 km base resolution\n", __FILE__ );
    fprintf( stderr, " %s: input parameters:\n", __FILE__ );
    fprintf( stderr, "   %s: meta_name   = meta file output name\n", __FILE__ );
    fprintf( stderr, "   %s: platform    = name of the platform as "
	     "cetb_platform_id (from cetb.h)\n", __FILE__ );
    fprintf( stderr, "   %s: start_day   = start day\n", __FILE__ );
    fprintf( stderr, "   %s: end_day     = end day\n", __FILE__ );
    fprintf( stderr, "   %s: year        = year input\n", __FILE__ );
    fprintf( stderr, "   %s: def         = region def file \n", __FILE__ );
    fprintf( stderr, "   %s: in_list     = name of input file containing list "
	     "of swath files\n\n", __FILE__ );
    exit (-1);
  }

  /* get local time/date */
  (void) time(&tod);
  (void) strftime(ltime,28,"%X %x",localtime(&tod));

  /* get meta file name */
  sscanf(argv[argn],"%s",mname);
  fprintf( stderr, "%s: Metafile name: %s \n", __FILE__,mname);
  argn++;  

  /* get satellite number as a cetb_platform_id enum */
  sscanf(argv[argn],"%s",platform);
  argn++;
  /* decode platform number */
  while( Fn < (int)CETB_NUM_PLATFORMS ) {
    if ( 0 == strcmp( cetb_platform_id_name[Fn], platform ) ) {
	F_num = (cetb_platform_id)Fn;
	Fn = CETB_NUM_PLATFORMS;
    }
    Fn++;
  }
  if (F_num<=CETB_NO_PLATFORM || F_num>=CETB_NUM_PLATFORMS) {
    fprintf( stderr, "%s: *** error decoding platform number: %s \n", __FILE__, platform );
    exit(-1);
  }
  fprintf( stderr, "%s: Platform number: %s %s \n", __FILE__, platform, cetb_platform_id_name[F_num] );

  /* open output meta file and write header */
  fprintf( stderr, "%s: Opening meta output file %s\n", __FILE__, mname);
  mout=fopen(mname,"w");
  if (mout==NULL) {
    fprintf( stderr, "%s: *** could not open output meta file: %s\n", __FILE__, mname);
    exit(-1);
  }
  fprintf(mout,"File=%s\n",mname);
  fprintf(mout,"Generated by=%s\n",prog_name);
  fprintf(mout,"Version=B%f\n",prog_version);
  fprintf(mout,"Meta_file_date=%s\n",ltime);
  fprintf(mout,"Sensor=%s\n",platform);

  /* get rest of input region parameters and write to file */
  get_region_parms( mout, &argn, argv, F_num, response_threshold_dB, resolution_ind );

  /* get list of input files and save to file */
  get_file_names(mout,&argn,argv);

  /* close output files */
  fclose(mout);
  fprintf( stderr, "%s: Finished writing meta file %s\n", __FILE__, mname);
  fprintf( stderr, "%s: All done\n", __FILE__ );

  exit( 0 );

}

  

/****************************************************************************/

static int get_file_names(FILE *mout, int *argn, char *argv[])
{  /* read swath data files from list file and write to meta file */

  char lname[1000], line[1000];
  FILE *Lfile;
  int last=1, count=0;

  fprintf(mout,"Begin_input_file_list\n");

  /* get list file name */
  sscanf(argv[*argn],"%s",lname);
  (*argn)++;

  if (lname[0]!='\0') { /* read file names from input list file */
    /* open input list file */
    fprintf( stderr, "%s: Opening input list file %s\n", __FUNCTION__, lname);
    Lfile=fopen(lname,"r");
    if (Lfile==NULL) {
      fprintf( stderr, "%s: *** could not open input list file %s\n", __FUNCTION__, lname);
      exit(-1);
    }

    last=1;
    do {
      /* read line of input list file and write to meta file */
      if (fgets(line,sizeof(line),Lfile) == NULL) {
	last=0;
	break;
      } else if (line[0]!='\0' && strlen(line)>1) {
	fprintf(mout," Input_file=%s",line);
	count++;
      } else
	last=0;

    } while (last);
    fclose(Lfile);

  } 

  fprintf(mout,"End_input_file_list\n");

  fprintf( stderr, "%s: Total number of input files: %d\n", __FUNCTION__, count);
  return(0);
}

/*
 * getregdata_fromcetb
 *   this function replaces the original getregdata by retrieving the info
 *   on the regions from cetb.h
 *
 *   input:
 *         int region number from def file
 *         int resolution_ind from command line
 *
 *   output:
 *         int proj - short version of region id
 *         int dateline - this is always zero for our projections - might not be needed
 *         float *latl, *lonl - lower left latitude and longitude
 *         float *lath, *lonh - upper right latitude and longitude
 *         char *regname
 */
static void getregdata_fromcetb(int regnum, int resolution_ind, int *iproj, int *dateline, float *latl,
				float *lonl, float *lath, float *lonh, char *regname)
{
  cetb_region_id cetb_region;
  int proj;
  int index_offset;

  proj = regnum - CETB_REGION_BASE_NUMBER;
  index_offset = CETB_NUMBER_BASE_RESOLUTIONS * resolution_ind;
  cetb_region = (cetb_region_id)( proj + index_offset );
  dateline = 0;

  fprintf( stderr, "%s: proj %d, index_offset %d, cetb_region %d \n", __FUNCTION__,
	   proj, index_offset, cetb_region );
  regname = strdup( cetb_region_id_name[ proj + index_offset ] );
  *latl = cetb_latitude_extent[(int)cetb_region][0];
  *lath = cetb_latitude_extent[(int)cetb_region][1];
  *lonl = cetb_longitude_extent[(int)cetb_region][0];
  *lonh = cetb_longitude_extent[(int)cetb_region][1];

  switch ( cetb_region ) {
  case CETB_EASE2_N:
  case CETB_EASE2_N36:
  case CETB_EASE2_N24:
    *iproj = 8;
    break;
  case CETB_EASE2_S:
  case CETB_EASE2_S36:
  case CETB_EASE2_S24:
    *iproj = 9;
    break;
  case CETB_EASE2_T:
  case CETB_EASE2_M36:
  case CETB_EASE2_M24:
    *iproj = 10;
    break;
  default:
    *iproj = 0;
  }

  fprintf( stderr, "%s: regname=%s, latl=%f, lath=%f, lonl=%f, lonh=%f, proj=%d\n",
	   __FUNCTION__, regname, *latl, *lath, *lonl, *lonh, *iproj );
}
    
/* *********************************************************************** */

static void getregdata(int regnum, int *iproj, int *dateline, float *latl,
		       float *lonl, float *lath, float *lonh, char *regname)
{
  char line[180], *s, *p;  
  int regnum1=1, last;

  /* try to get environment variable */
  p=getenv("SIR_region");
  if (p==NULL) {
    fprintf( stderr, "%s: *** standard regions environment variable 'SIR_region'"
	     "not defined!\n", __FUNCTION__ );    
    p=rname; /* use default if environment variable not available */
  }  

  FILE *rid=fopen(p,"r");
  if (rid==NULL) {
    fprintf( stderr, "%s: *** could not open standard regions file %s\n",
	     __FUNCTION__, p);
    exit(-1);
  }

  /* skip first two input file header lines*/
  fgets( line, 180, rid );
  fgets( line, 180, rid );  

  last=1;
  do {
    /* read line of input file and check */
    if (fgets(line,180,rid) == NULL || regnum1==9999 || regnum1==0) {
      fprintf( stderr, "%s: *** region %d not found in %s\n",
	       __FUNCTION__, regnum,p);
      exit(-1);
      last=0;
    }	
    sscanf(line,"%d %d %d %f %f %f %f %s\n",
	   &regnum1, iproj, dateline, latl, lonl, lath,  lonh, regname);
    s=strstr( line, regname );
    strncpy( regname, s, 10 );
    regname[10]='\0';
    if (regnum1 == 9999 || regnum1 == 0) {
      fprintf( stderr, "%s: *** region %d not found in %s\n",
	       __FUNCTION__, regnum, p );
      exit(-1);
      last=0;
    }	
    if (regnum1==regnum) {
      last=0;
      break;
    }
  } while (last);

  fclose(rid);
  return;  
}

/* utility routines */

/* ***********************************************************************
 * get_region_parms - generates region parameters section of .meta file.
 * 
 *  Input:
 *    mout - FILE pointer to write to
 *    argn - integer pointer index to next argument to use (dstart)
 *    argv - char pointer array with command-line arguments
 *    F_num - integer with platform_id
 *    threshold_response_dB - float, response pattern minimum threshold, in dB
 *    resolution_ind - int, resolution index 0, 1 or 2 for 25, 30 or 36 km base res
 *
 *  Output: n/a
 *    
 *  Result:
 *    status variable indicates success (0) or failure (1)
 *
 *  All output is written to file pointed to by mout
 * 
 */
static int get_region_parms( FILE *mout, int *argn, char *argv[], int F_num,
			     float response_threshold_dB, int resolution_ind ) {
  
  int err=0;  
  int negg=2; /* only do eggs */
  int nsection;
  char regname[11], reg[4], cpol, sen, cegg, chan;
  char TF[]={'F', 'T'};
  int dstart, dend, year, mstart, mend;
  float a_init, a_offset, b_init, b_weight, angle_ref;
  int nits, flatten, median_flag;
  char rfile[250];  
  int pfile;
  FILE *pid;
  int nregions, poleflag, dateline, iproj, regnum, iregion, ircnt;
  float latl, lonl, lath, lonh;
  int projt, nsx, nsy, xdim, ydim, nease;
  float ascale, bscale, xdeg, ydeg, a0, b0, aorglon, aorglat;
  float maplxlon, maprxlon;
  int iasc, ibeam, ipolar;
  int non_size;
  int nsect, nsx2, nsy2;
  float ascale2, bscale2;
  int isection, iy;
  float tsplit1, tsplit2;
  char setname[120];
  double map_equatorial_radius_m,map_eccentricity, e2,
    map_reference_latitude, map_reference_longitude, 
    map_second_reference_latitude, sin_phi1, cos_phi1, kz,
    map_scale, r0, s0, epsilon;
  int bcols, brows, ind;
  int base_resolution[] = { CETB_SMAP_BASE_25_RESOLUTION,
			    CETB_SMAP_BASE_24_RESOLUTION,
			    CETB_SMAP_BASE_36_RESOLUTION };
  
  fprintf(mout,"Egg_or_slice=%d\n",negg);

  /* read time period information */
  sscanf( argv[*argn], "%d", &dstart );
  (*argn)++;
  sscanf( argv[*argn], "%d", &dend );
  (*argn)++;
  sscanf( argv[*argn], "%d", &year );
  (*argn)++;
  mstart=0;  /* start minute of day */
  mend=1440; /* end minute of day */

  fprintf( stderr, "%s: Input day range %d to %d year %d\n", __FUNCTION__, dstart, dend, year );
  
  /* write to meta file */ 	
  fprintf(mout,"Begin_description\n");	 
  fprintf(mout," Start_Year=%5d\n",year);	
  fprintf(mout," Start_day=%4d\n",dstart);
  fprintf(mout," End_day=%4d\n",dend);
  fprintf(mout," Start_minute=%4d\n",mstart);
  fprintf(mout," End_minute=%4d\n",mend);

  /* set SIR imaging options */
  a_init=200.0;   /* initial TB valu for SIR */
  a_offset=0.0;   /* processing offset -- should be 0 */
  b_init=0.0;     /* initial slope of TB vs incidence angle */
  b_weight=1;     /* slope weighting */
  nits=20;        /* SIR iterations */
  angle_ref=53.0; /* reference incidence angle (if used) */
  flatten=FALSE;     /* flatten antenna response to 1,0 if TRUE */
  median_flag=FALSE; /* include median filter in SIR processing if TRUE */

  /* write to meta file */
  fprintf(mout," A_initialization=%10.5f\n", a_init);  
  fprintf(mout," A_offset_value=%10.5f\n", a_offset);
  fprintf(mout," B_initialization=%10.5f\n", b_init);
  fprintf(mout," B_weight=%10.5f\n", b_weight);
  fprintf(mout," Max_iterations=%3d\n", nits);
  fprintf(mout," Reference_incidence_angle=%10.5f\n", angle_ref);
  fprintf(mout," Response_threshold=%10.5f\n", response_threshold_dB);
  fprintf(mout," Flat_response=%c\n", TF[flatten]);
  fprintf(mout," Median_filter=%c\n", TF[median_flag]);

  /* read region parameters definition file name */
  strncpy( rfile, argv[*argn], sizeof( rfile ) );
  (*argn)++;
  fprintf( stderr, "%s: rfile=%s \n", __FUNCTION__, rfile );  
 
  pfile=TRUE;
  fprintf(mout," Region_parameters_file=%s\n",rfile);
  pid=fopen(rfile,"r");
  if (pid==NULL) {
    fprintf( stderr, "%s: *** could not open region parameters file: %s\n",
	     __FUNCTION__, rfile );
    exit(-1);      
  }
  fscanf(pid,"%d",&nregions);
  fprintf( stderr, "%s: Base resolution: %d\n", __FUNCTION__, resolution_ind );
  fprintf( mout, " Base_resolution=%2d\n", base_resolution[resolution_ind] );
  fprintf( stderr, "%s: Number of regions: %d\n", __FUNCTION__, nregions);
  fprintf( mout, " Num_Regions=%2d\n", nregions );
  
  /* for each region, read in the parameters that define the region 
     size and projection and data selection criteria */

  ircnt=0;  /* count total regions */
  for (iregion=0; iregion<nregions; iregion++) {
    dateline=FALSE;
    
    /* region ID number */ 

    fscanf( pid, "%d", &regnum );
    fprintf( stderr, "%s: Region %d number: %d\n", __FUNCTION__, iregion, regnum );
    
    /* define region, using auto definition if possible */
    strncpy( regname, "Custom", 10);
    if (regnum > 0) { /* get region definition from contents of cetb.h */
      fprintf( stderr, "%s: about to call fromcetb, regnum=%d, resolution_ind%d\n",
	       __FUNCTION__, regnum, resolution_ind );
      getregdata_fromcetb( regnum, resolution_ind, &iproj, &dateline, &latl,
			   &lonl, &lath, &lonh, regname );
      if (((regnum >= 0) && (regnum < 100)) || (regnum >= 120)) poleflag=0; /* non-polar area */
      fprintf( stderr, "%s: Region name: '%s'  Def Proj %d  Dateline %d\n",
	       __FUNCTION__, regname, iproj, dateline );
    } else {
      fprintf( stderr, "%s: Region is not defined and out of bounds\n", __FUNCTION__ );
      exit(-1);
    }
    
    /* print region ID number and bounding box info */
    fprintf( stderr, "%s: Region definition information\n", __FUNCTION__ );
    fprintf( stderr, "%s:  Latitude range:  %f %f\n", __FUNCTION__, latl, lath );
    fprintf( stderr, "%s:  Longitude range: %f %f\n", __FUNCTION__, lonl, lonh );
    fprintf( stderr, "%s:  Region polar code (0=arbitrary, 1=N pol, 2=S pole: %d\n",
	     __FUNCTION__, poleflag );
    if (dateline) {
      fprintf( stderr, "%s:  Region crosses dateline\n", __FUNCTION__ );
      maplxlon=min(lonh,lonl);
      maprxlon=max(lonh,lonl);
      lonl=maplxlon;
      lonh=maprxlon;
      fprintf( stderr, "%s:  Corrected longitude range: %f %f\n",
	       __FUNCTION__, lonl, lonh );
    }
     
    /* write region ID number and bound box info to meta file */
    fprintf(mout," Begin_region_description\n");
    fprintf(mout,"  Region_id=%4d\n", regnum);
    fprintf(mout,"  Latitude_low=%16.9f\n", latl);
    fprintf(mout,"  Latitude_high=%16.9f\n", lath);
    fprintf(mout,"  Longitude_low=%16.9f\n", lonl);
    fprintf(mout,"  Longitude_high=%16.9f\n", lonh);
    fprintf(mout,"  Dateline_crossing=%c\n", TF[dateline]);
    fprintf(mout,"  Polar_flag=%c\n", TF[poleflag]);
    fprintf(mout,"  Region_name=%s\n", regname);
    
    /* transformation information */

    /* projection */
    projt=iproj;  /* default value from region definition file */
    /* This version of meas_meta_make only does EASE2-N, -S, -T/M */
    if ( projt > 10 || projt < 8 ) {
      fprintf( stderr, "%s: Only acceptable projections are 308, 309, 310, "
	       "EASE2-N, -S, -T\n", __FUNCTION__ );
      exit(-1);
    }

    fprintf( stderr, "%s:  Projection code ( 8=EASE2N,9=EASE2S,10=EASE2T/M ): %d\n",
	     __FUNCTION__, projt);

    /* define the origin, size, scale, offset for each projection */
    switch (projt) {

    case  8:  /* ease2 grid N */
    case  9:  /* ease2 grid S */
    case 10:  /* ease2 grid T */
      fscanf(pid,"%d",&nease);
      /* projt=regnum-300; */ 
      /* define projection parameters for particular EASE2 case */
      ind=resolution_ind;  /* standard base resolution */
      fprintf( stderr, "%s: EASE2 parameters: proj=%d  nease=%d  ind=%d\n",
	       __FUNCTION__, projt, nease, ind);      
      ease2_map_info(projt, nease, ind, &map_equatorial_radius_m, 
		     &map_eccentricity, &e2,
		     &map_reference_latitude, &map_reference_longitude, 
		     &map_second_reference_latitude, &sin_phi1, &cos_phi1, &kz,
		     &map_scale, &bcols, &brows, &r0, &s0, &epsilon);

      nsx=bcols;             /* X dim (horizontal=cols) pixels */
      nsy=brows;             /* Y dim (vertical=rows) pixels */
      ascale=(float) nease;  /* base grid scale factor (0..5) */
      bscale=(float) ind;    /* base grid scale index (0..2) */
      a0=0.0;                /* X origin pixel - 1 */
      b0=0.0;                /* Y origin pixel - 1 */
      xdeg=(float) (nsx/2);  /* map center X pixel - 1 */
      ydeg=(float) (nsy/2);  /* map center Y pixel - 1 */
      xdim=xdeg*2;
      ydim=ydeg*2;
      aorglon=xdeg;
      aorglat=ydeg;
      break;

    default:
      fprintf( stderr,"%s: *** Error selecting projection type %d ***\n",
	      __FUNCTION__, projt );
      exit(-1);      
    }
  
    /* select ascending/descending data */
    iasc=0;
    fscanf(pid,"%d",&iasc);
    fprintf( stderr, "%s: Asc/Desc flag: (0=both,1=asc,2=dsc,3=morn,4=eve) %d\n",
	     __FUNCTION__, iasc );
    fprintf( mout,"  AscDesc_flag=%2d\n",iasc );

    /* Selection is based on whichever platform is specified on the input */
    fscanf( pid, "%d", &ibeam );
    fprintf( stderr, "%s: Beam index (eg for SSM/I 1=19H,2=19V,3=22V,etc): %d\n",
	     __FUNCTION__, ibeam );
    ipolar=0;   /* h pol */
    if (ibeam==2 || ibeam==3 || ibeam==5 || ibeam==7) ipolar=1; /* v pol */
    fprintf( mout, "  Polarization=%d\n", ipolar );
    fprintf( mout, "  Beam_index=%d\n", ibeam );

    /* read number iterations from .def file */
    fscanf( pid, "%d", &nits );
    fprintf( mout, " Max_iterations=%d\n", nits );

    /* set grid image size parameters */
    /* number of enhanced resolution pixels/non-enhanced pixels */
    /* for this project non-enhanced size is always 25 km grid */
    non_size=(1 << (nease) );
 
    if (non_size*(nsx/non_size) != nsx || non_size*(nsy/non_size) != nsy) {
      fprintf( stderr,"%s: *** warning: non grid size parameter %d does not evenly"
	       "divide image size: %d %d\n", __FUNCTION__, non_size, nsx, nsy );
    }

    /* optionally section region in to smaller images for processing.  They will be recombined later */
    nsection=0;
    fprintf(mout,"  Sectioning_code=%d\n",nsection);

    nsect=(nsection % 100);    /* number of sections */

    /* for each output section (0=unsection region) */
    for (isection=0; isection<=nsect; isection++) {
      fprintf(mout,"  Begin_section_description\n");
      fprintf(mout,"  Section_id=%d\n",isection);


      fprintf( stderr, "\nSectioning: (x,y) %d %d\n",isection,nsect);

      /* write map projection info to output file */
      fprintf(mout,"   Project_type=%d\n",projt);
      fprintf(mout,"   Projection_origin_x=%16.9f\n", aorglat);
      fprintf(mout,"   Projection_origin_y=%16.9f\n", aorglon);
      fprintf(mout,"   Projection_offset_x=%16.9f\n", a0);
      fprintf(mout,"   Projection_offset_y=%16.9f\n", b0);
      fprintf(mout,"   Projection_scale_x=%16.9f\n", ascale);
      fprintf(mout,"   Projection_scale_y=%16.9f\n", bscale);
      fprintf(mout,"   Projection_dim_x=%d\n", xdim);
      fprintf(mout,"   Projection_dim_y=%d\n", ydim);
      fprintf(mout,"   Image_size_x=%d\n", nsx);
      fprintf(mout,"   Image_size_y=%d\n", nsy);

      /* generate parameters for non-enhanced gridded images */
      nsx2=nsx/non_size;
      nsy2=nsy/non_size;
      if (non_size*nsx2 != nsx || non_size*nsy2 != nsy) {
	fprintf( stderr, "%s: *** WARNING: non grid size %d does not evenly divide"
		 "image size %d %d\n", __FUNCTION__, non_size, nsx, nsy );
      }
      ascale2=ascale;
      bscale2=bscale;
      if (projt==8 || projt==9 || projt==10) { /* EASE2 */
	ascale2=ascale-2;
      } else {
	nsx2=nsx;
	nsy2=nsy;
	fprintf( stderr, "%s: *** WARNING: Projection type can not generate "
		 "Non-enhanced parameters ***\n", __FUNCTION__ );
      }

      /* write grid projection info to meta file */

      fprintf(mout,"   Grid_scale_x=%d\n", non_size);
      fprintf(mout,"   Grid_scale_y=%d\n", non_size);
      fprintf(mout,"   Grid_size_x=%d\n", nsx2);
      fprintf(mout,"   Grid_size_y=%d\n", nsy2);
      fprintf(mout,"   Grid_projection_origin_x=%16.9f\n", aorglat);
      fprintf(mout,"   Grid_projection_origin_y=%16.9f\n", aorglon);
      fprintf(mout,"   Grid_projection_offset_x=%16.9f\n", a0);
      fprintf(mout,"   Grid_projection_offset_y=%16.9f\n", b0);
      fprintf(mout,"   Grid_projection_scale_x=%16.9f\n", ascale2);
      fprintf(mout,"   Grid_projection_scale_y=%16.9f\n", bscale2);
	      
      /* for this region, generate standard product data file names abreviation */
      switch(regnum) {
      case 308:
	strncpy(reg,"E2N",4);
	break;
      case 309:
	strncpy(reg,"E2S",4);
	break;
      case 310:
	strncpy(reg,"E2T",4);
	break;
      default:
	strncpy(reg,regname,3);
	reg[3]='\0';	
	break;	
      }
            
      /* write LTOD split time to file */
      /* note: tsplit values not 0 & 12 require more than one UTC as daily input */   
      if (iasc > 2) {
	/* northern hemisphere */
	tsplit1=0.0;
	tsplit2=12.0;
	/* southern hemisphere, use slightly different numbers */
	if ( regnum == 309 )  { /* E2S */
	  tsplit1=0.0;
	  tsplit2=12.0;
	}
	fprintf(mout,"   Local_time_split1=%16.9f\n", tsplit1);
	fprintf(mout,"   Local_time_split2=%16.9f\n", tsplit2);
      }
  
      /* create the full set of BYU naming standard file names -- whether used or not */
      iy=(year % 100);
      /* code for the correct names from cetb.h */
      if ( CETB_NIMBUS7 == F_num ) sen='R';
      if ( CETB_AQUA == F_num ) sen='A';
      if ( CETB_F08 <= F_num && F_num <= CETB_F15 ) sen='F';
      if ( CETB_F16 <= F_num && F_num < CETB_NUM_PLATFORMS ) sen='I';
      if ( CETB_SMAP == F_num ) sen='S';
      /* F=ssmi, A=AMSRE, R=SMMR, I=SSMIS, S=SMAP */
      if (F_num < 10.0) /* code the sensor number based on cetb_platform_id*/
	cegg=(char) (F_num+48);  /* 0...9 */
      else
	cegg=(char) (F_num-10+65); /* A...Z */
      if ( ibeam < 10 )
	chan=(char) (ibeam+48);
      else
	chan=(char) (ibeam-10+97); // a...z
      cpol='b';  /* both asc and desc (all data) used */

      /* modify file name if LTOD ascending/descending or morn/even */

      if (iasc != 0) {
	cpol='a';  /* asc */
	if (iasc==2) cpol='d'; /* desc */
	if (iasc==3) cpol='m'; /* morning/midnight */
	if (iasc==4) cpol='e'; /* evening */
	if (iasc==5) cpol='n'; /* noon/night */
      }

     /* Only need to save the name of the setup file */
      sprintf(setname,"%c%c%c%c-%3s%02d-%03d-%03d.setup",sen,cegg,chan,cpol,reg,iy,dstart,dend);
      /* there are no longer product file names to write out to the meta file, however, the logic of the
       * setup program requries the Begin_ and End_product_file_names tags */
    fprintf(mout,"  Setup_file=%s\n",setname);
    fprintf(mout,"  Begin_product_file_names\n");
    fprintf(mout,"  End_product_file_names\n");

      /* add SIRF commands to job script */
    fprintf(mout," End_section_description\n");
    ircnt++;
    }
    /* end section loop */

    fprintf(mout," End_region_description\n");
  }      /* end region loop */
  fprintf(mout,"End_description\n");
  
  if (pfile) fclose(pid);
	      
  fprintf( stderr, "%s: Total regional images: %d Regions: %d\n", __FUNCTION__, ircnt, nregions );
  return(err);  

}


