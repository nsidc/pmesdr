/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_meta_make.c

  generates SIR meta file for the MEaSUREs project

  translated to C by DGL at BYU 03/01/2014 from ssmi_meta_make3.f 
   note: while fortran was well-tested, not all C program options
   have been fully tested
  Modified by DGL at BYU 3/07/2014 + added EASE2 capability
  Modified by DGL at BYU 8/16/2014 + added error message when required environment variable not defined
  Modified by MAH at NSIDC 10/10/2014 - compile directive for Intel math library
  Modified by DGL at BYU 1/18/2015 + modified local time for LTOD, name scheme
  Modified by MAH at NSIDC 01/20/2015 - added DGL's changes into meas_meta_make.c for repo

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

#include <sir3.h>

#define prog_version 1.2 /* program version */
#define prog_name "meas_meta_make"

#define MAKEJOB 0 /* create job script if 1, do not create job script if 0 */
#define ENABLE_SYSTEM_CALL 0 /* enable system call to make job script executable */

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define mod(a,b) ((a) % (b))
#define abs(x) ((x) >= 0 ? (x) : -(x))

int nint(float r)
{
  int ret_val = r;
  if (ret_val - r > 0.5) ret_val--;
  if (r - ret_val > 0.5) ret_val++;
  return(ret_val);
}

#define TRUE 1
#define FALSE 0

/****************************************************************************/

/* default location of the SIR standard region definition */
/* char rname[] = "/auto/share/ref/regiondef1.dat";*/  /* file defining region codes */
char rname[] = "regiondef1.dat";  /* file defining region codes */

/* these file names used only when creating .job files */
char setup_name[] = "meas_meta_setup";            /* setup program path/name */
char sirf_name[] = "meas_meta_sir";               /* SIR program path/name */

/********************************************************************/

/* function prototypes */


int get_region_parms(FILE *mout, FILE *jout, int argc, int *argn, char *argv[], 
		     char *mname, int F_num, char *s2g);

int get_file_names(FILE *mout, int argc, int *argn, char *argv[]);


/* declare specific map projection prototypes needed */

extern void polster( float alon, float alat, float *x1, float *y1, float xlam, float slat);

extern void lambert1(float lat, float lon, float *x, float *y, 
		     float orglat, float orglon, int iopt);

extern void ease2_map_info(int iopt, int isc, int ind, 
			   double *map_equatorial_radius_m, 
			   double *map_eccentricity, double *e2,
			   double *map_reference_latitude, 
			   double *map_reference_longitude, 
			   double *map_second_reference_latitude,
			   double *sin_phi1, double *cos_phi1, double *kz,
			   double *map_scale, int *bcols, int *brows, 
			   double *r0, double *s0, double *epsilon);

/****************************************************************************/
/* note: program is designed to be run from command line OR interactively 
*/


int main(int argc,char *argv[])
{
 
  char mname[256], line[1024], s2g[256];  
  time_t tod;
  char ltime[29];
  int argn=1;
  char platform[28];
  int F_num=0;
  FILE *mout, *jout;

  
  printf("MEaSures Meta_Make Program\nProgram: %s  Version: %f\n\n",prog_name,prog_version);

  if (argc < 2) {
    printf("\nusage: %s meta_name platform start_day stop_day year def in_list\n\n",argv[0]);
    printf(" input parameters:\n");
    printf("   meta_name   = meta file output name\n");
    printf("   platform    = name of the platform (e.g. F13)\n");
    printf("   start_day   = start day\n");
    printf("   end_day     = end day\n");
    printf("   year        = year input\n");
    printf("   def         = region def file \n");
    printf("   in_list     = name of input file containing list of swath files\n\n");
  }

  /* get local time/date */
  (void) time(&tod);
  (void) strftime(ltime,28,"%X %x",localtime(&tod));

  /* get meta file name */
  if (argn > argc || argv[argn] == NULL) {
    printf("Enter output meta file name:");
    fgets(line,sizeof(line),stdin);
    sscanf(line,"%s",mname);
  } else
    sscanf(argv[argn],"%s",mname);
  printf("\nMetafile name: %s \n",mname);
  argn++;  

  /* get satellite number */
  if (argn > argc || argv[argn] == NULL) {
    printf("Enter platform name or number: (e.g. SSMI F13) ");
    fgets(line,sizeof(line),stdin);
    sscanf(line,"%s",platform);
  } else
    sscanf(argv[argn],"%s",platform);
  argn++;
  /* decode platform number */
  sscanf(platform,"SSMI F%2d",&F_num);
  sscanf(platform,"%d",&F_num);
  printf("\nPlatform number: %s %d \n",platform,F_num);
  if (F_num<1 || F_num>20) {
    fprintf(stderr,"*** error decoding platform number: %s %d\n",platform,F_num);
    exit(-1);
  }

  /* open output meta file and write header */
  printf("Opening meta output file %s\n",mname);
  mout=fopen(mname,"w");
  if (mout==NULL) {
    fprintf(stderr,"*** could not open output meta file: %s\n",mname);
    exit(-1);
  }
  fprintf(mout,"File=%s\n",mname);
  fprintf(mout,"Generated by=%s\n",prog_name);
  fprintf(mout,"Version=B%f\n",prog_version);
  fprintf(mout,"Meta_file_date=%s\n",ltime);
  fprintf(mout,"Sensor=%s\n",platform);

  if (MAKEJOB) {
    /* open output job file and write header */
    sprintf(line,"%s.job",mname);
    printf("Opening job output file %s\n",line);
    jout=fopen(line,"w");
    if (jout==NULL) {
      fprintf(stderr,"*** could not open output job file: %s\n",line);
      exit(-1);
    }
    fprintf(jout,"#!/bin/sh\n");
    fprintf(jout,"# Job File=%s\n",line);
    fprintf(jout,"# Generated by=%s\n",prog_name);
    fprintf(jout,"# Version=%s\n",prog_version);
    fprintf(jout,"# Meta_file_date=%s\n",ltime);
    fprintf(jout,"# Meta_file=%s\n",mname);
  } else {
    printf("* No job script created *\n");
    jout=NULL;
  }
  
  /* get s2g argument */
  if (argn > argc || argv[argn] == NULL) {
    printf("Enter s2g file name: (blank=NONE)");
    fgets(line,sizeof(line),stdin);
    sscanf(line,"%s",s2g);
  } else
    sscanf(argv[argn],"%s",s2g);
  printf("S2G name: %s \n",s2g);
  argn++;
  if (strncmp(s2g,"NONE",4)==0) s2g[0]='\0';
  if (strncmp(s2g,"none",4)==0) s2g[0]='\0';

  /* get rest of input region parameters and write to file */
  get_region_parms(mout,jout,argc,&argn,argv,mname,F_num,s2g);

  /* get list of input files and save to file */
  get_file_names(mout,argc,&argn,argv);

  /* close output files */
  fclose(mout);
  printf("Finished writing meta file %s\n",mname);

  if (MAKEJOB) {
    fclose(jout);
    printf("Finished writing job file %s.job\n",mname);

    /* make job file executable */
    if (ENABLE_SYSTEM_CALL) {
      sprintf(line,"chmod 755 %s.job",mname);
      system(line);
    }

    printf("\nJob command: nice %s > %s.out 2>&1\n",mname,mname);
  }
  printf("\nAll done\n");

  exit( 0 );

}

  

/****************************************************************************/

int get_file_names(FILE *mout, int argc, int *argn, char *argv[])
{  /* read swath data files from list file and write to meta file */

  char lname[1000], line[1000];
  FILE *Lfile;
  int last=1, count=0;

  fprintf(mout,"Begin_input_file_list\n");

  /* get list file name */
  if (*argn > argc || argv[*argn] == NULL) {
    printf("Enter input swath list file name: (blank or NON for manual input) ");
    fgets(lname,sizeof(lname),stdin);
  } else
    sscanf(argv[*argn],"%s",lname);
  (*argn)++;
  if (strncmp(lname,"NONE",4)==0) lname[0]='\0';
  if (strncmp(lname,"none",4)==0) lname[0]='\0';

  if (lname[0]!='\0') { /* read file names from input list file */
    /* open input list file */
    printf("Opening input list file %s\n",lname);
    Lfile=fopen(lname,"r");
    if (Lfile==NULL) {
      fprintf(stderr,"*** could not open input list file %s\n",lname);
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
	fprintf(stdout,"Input_file=%s",line);
	count++;
      } else
	last=0;

    } while (last);
    fclose(Lfile);

  } else { /* manually input file names (or piped in) */

    last=1;
    do {
      printf("Enter file %d name: ",count);
      if (fgets(line,sizeof(line),stdin)==NULL || line[0] == '\0' || line[0] == (char) 10 || line[0] == '\n') 
	last=0;
      else {
	fprintf(mout," Input_file=%s",line);
	fprintf(stdout,"Input_file=%s",line);
	count++;
      } 
    } while (last);
    
  }
  fprintf(mout,"End_input_file_list\n");

  printf("Total number of input files: %d\n",count);
  return(0);
}
  


/* *********************************************************************** */

void getregdata(int regnum, int *iproj, int *dateline, float *latl, float *lonl,
		float *lath, float *lonh, char *regname)
{
  char line[180], *s, *p;  
  int regnum1=1, last;

  /* try to get environment variable */
  p=getenv("SIR_region");
  if (p==NULL) {
    printf("*** standard regions environment variable 'SIR_region' not defined!\n");    
    p=rname; /* use default if environment variable not available */
  }  

  FILE *rid=fopen(p,"r");
  if (rid==NULL) {
    fprintf(stderr,"*** could not open standard regions file %s\n",p);
    exit(-1);
  }

  /* skip first two input file header lines*/
  fgets(line,180,rid);
  fgets(line,180,rid);  

  last=1;
  do {
    /* read line of input file and check */
    if (fgets(line,180,rid) == NULL || regnum1==9999 || regnum1==0) {
      fprintf(stderr,"*** region %d not found in %s\n",regnum,p);
      exit(-1);
      last=0;
    }	
    sscanf(line,"%d %d %d %f %f %f %f %s\n",
	   &regnum1, iproj, dateline, latl, lonl, lath,  lonh, regname);
    s=strstr(line,regname);
    strncpy(regname,s,10);
    regname[10]='\0';
    /* printf("%s : %4d  %2d %2d %6.1f      %8.1f   %8.1f    %8.1f    '%s'\n",
       line,regnum1, *iproj, *dateline, *latl, *lonl, *lath,  *lonh, regname); */
    if (regnum1 == 9999 || regnum1 == 0) {
      fprintf(stderr,"*** region %d not found in %s\n",regnum,p);
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


void section_pixels(int isection, int *ix, int *iy, int nsect, int nt, float alpha, 
		    int nsx, int nsy, int non_size, int *nsx2,int *nsy2,
		    int *ix1, int *iy1, int *ix2, int *iy2, 
		    int *jx1, int *jy1, int *jx2, int *jy2)
{
  /*
       computes the pixel locations of sectioned images

       inputs:
        isection: section number (1...nsect)
        nsect:    number of sections (valid values: 1,2,3,4,6,8,9,12,15,16,18,20)
        nt:       sectioning code (0=vertical orientation,1=horizontal)
        alpha:    overlap ratio
        nsx,nsy:  image size in pixels
        non_size: number of grid pixels/image pixel

       outputs:
        ix,iy:    lower-left corner of the isection'th image
        nsx2,nsy2: image size for isection'th image
        ix1..iy2: lower-left,upper right corner of useful pixels of section
        jx1..jy2: lower-left,upper right corner of section in final image
  */
  int nx,ny,n;
  int inx,iny,nx1,ny1,nay,nax;

  *ix=1;
  *iy=1;
  *ix1=1;			/* section pixel range */
  *iy1=1;
  *ix2=nsx;
  *iy2=nsy;
  *jx1=1;			/* destination pixel range */
  *jy1=1;
  *jx2=nsx;
  *jy2=nsy;
  *nsx2=nsx;
  *nsy2=nsy;
  if (nsect == 1) return;


  /* based on sectioning code, determine the dimensions of the image sectioning */
  if (nsect == 2) {
    nx=1;
    ny=2;
  } else if (nsect == 3) {
    nx=1;
    ny=3;
  } else if (nsect == 4) {
    nx=2;
    ny=2;
  } else if (nsect == 6) {
    nx=2;
    ny=3;
  } else if (nsect == 8) {
    nx=2;
    ny=4;
  } else if (nsect == 9) {
    nx=3;
    ny=3;
  } else if (nsect == 12) {
    nx=3;
    ny=4;
  } else if (nsect == 15) {
    nx=3;
    ny=5;
  } else if (nsect == 16) {
    nx=4;
    ny=4;
  } else if (nsect == 18) {
    nx=6;
    ny=3;
  } else if (nsect == 20) {
    nx=4;
    ny=5;
  } else {
    printf("*** ERROR: invalid number of sections %d\n",nsect);
    nsect=1;
    return;
  }

  if (isection > nsect) {
    printf("*** ERROR: invalid sectioning %d\n",isection,nsect);
    return;
  }

  if (nt==1) { /* horizontal orientaion rather than vertical */
    n=nx;
    nx=ny;
    ny=n;
  }

  inx=((isection-1) % nx)+1;
  iny=((isection-1) / nx)+1;
  
  ny1=nsy/ny;
  if (non_size*(ny1/non_size) != ny1) 
    ny1=non_size*(1+ny1/non_size);
  nay=alpha*nsy;
  nay=non_size*(nay/non_size);

  if (nx >1) {
    nx1=nsx/nx;
    if (non_size*(nx1/non_size) < nx1) 
      nx1=non_size*(1+nx1/non_size);
    if (nx*nx1 < nsx) 
      nx1=nx1+non_size;
    n=alpha*nsx;
    nax=non_size*(n/non_size);
    if (nax < n)
      nax=nax+non_size;
    *nsx2=nx1+nax;
    if (inx != 1 && inx != nx) 
      *nsx2=*nsx2+nax;
    *jx1=(inx-1)*nx1+1;
    *jx2=min(inx*nx1,nsx);
    *ix1=nax+1;
    if (inx==1) *ix1=1;
    *ix=*jx1-*ix1+1;
    *ix2=*ix1+nx1-1;
  }

  if (ny > 1) {
    ny1=nsy/ny;
    if (non_size*(ny1/non_size) < ny1)
      ny1=non_size*(1+ny1/non_size);
    if (ny*ny1 < nsy)
      ny1=ny1+non_size;
    n=alpha*nsy;
    nay=non_size*(n/non_size);
   if (nay < n)
     nay=nay+non_size;
   *nsy2=ny1+nay;
   if (iny != 1 && iny != ny) 
     *nsy2=*nsy2+nay;
   *jy1=(iny-1)*ny1+1;
   *jy2=min(iny*ny1,nsy);
   *iy1=nay+1;
   if (iny == 1) *iy1=1;
   *iy=*jy1-*iy1+1;
   *iy2=*iy1+ny1-1;
  }
  return;
}

/* utility routines */

int get_prompt_iarg(int argc, int *argn, char *argv[], char *prompt, int deflt)
{
  static char line[120];
  int val=deflt;  

  if (*argn > argc || argv[*argn] == NULL) {
    printf("%s ",prompt);
    fgets(line,sizeof(line),stdin);
    sscanf(line,"%d",&val);
  } else
    sscanf(argv[*argn],"%d",&val);
  (*argn)++;

  return(val);
}

float get_prompt_farg(int argc, int *argn, char *argv[], char *prompt, float deflt)
{
  static char line[120];
  float val=deflt;  

  if (*argn > argc || argv[*argn] == NULL) {
    printf("%s ",prompt);
    fgets(line,sizeof(line),stdin);
    sscanf(line,"%d",&val);
  } else
    sscanf(argv[*argn],"%f",&val);
  (*argn)++;

  return(val);
}

char *get_prompt_sarg(int argc, int *argn, char *argv[], char *prompt, char *deflt, char *buff, int blen)
{
  if (*argn > argc || argv[*argn] == NULL) {
    printf("%s ",prompt);
    fgets(buff,blen,stdin);
  } else
    strncpy(buff,argv[*argn],blen);
  (*argn)++;
  return(buff);
}


int get_prompt_larg(int argc, int *argn, char *argv[], char *prompt, int deflt)
{
  static char line[120], *s;
  int val=deflt;  

  if (*argn > argc || argv[*argn] == NULL) {
    printf("%s ",prompt);
    fgets(line,sizeof(line),stdin);
    s=line;
  } else
    s=argv[*argn];
  (*argn)++;

  if (s[0]=='T' || s[0]=='t' || s[0]=='Y' || s[0]=='y' || s[0]=='1')
    val=TRUE;
  if (s[0]=='F' || s[0]=='f' || s[0]=='N' || s[0]=='n' || s[0]=='0')
    val=FALSE;

  return(val);
}


/* routine that reads the input args and generates region definitions */

int get_region_parms(FILE *mout, FILE *jout, int argc, int *argn, char *argv[], 
		      char *mname, int F_num, char *s2g)
{
  /* define meta regions and file sections  and write to meta and job files */

  int err=0;  
  int negg=2; /* only do eggs */
  int sections=FALSE, nsection;
  char fnamel[1000], regname[11], reg[4], cpol, sen, cegg, chan;
  char TF[]={'F', 'T'};
  int dstart, dend, year, mstart, mend;
  float a_init, a_offset, b_init, b_weight, angle_ref, response_threshold;
  int nits, flatten, median_flag;
  char rfile[250];  
  int pfile;
  FILE *pid;
  int nregions, poleflag, dateline, iproj, regnum, iregion, ircnt;
  float latl, lonl, lath, lonh, deglat, deglon;
  int projt, nsx, nsy, xdim, ydim, nease;
  float ascale, bscale, xdeg, ydeg, a0, b0, aorglon, aorglat;
  float maplxlon, maprxlon, maplxlat, maprxlat, mapuylat, mapuylon, maplylat, maplylon;
  float x0, y0, lmostx, rmostx, umosty, lmosty;
  int toil, iasc, ibeam, ipolar;
  int non_size;
  float ascale_s, bscale_s, a0_s, b0_s, xdeg_s, ydeg_s;
  int nsx_s, nsy_s, nsect, nt, nsx2, nsy2;
  float a02, b02, xdeg2, ydeg2, ascale2, bscale2;
  int isection, ix, iy, ix1, iy1, ix2, iy2, jx1, jy1, jx2, jy2;
  int ix1g, iy1g, ix2g, iy2g, jx1g, jy1g, jx2g, jy2g;
  float temp, alpha, tsplit1, tsplit2;

  char a_name[120], b_name[120], i_name[120], j_name[120], c_name[120], p_name[120],
    v_name[120], e_name[120], lisname[120], aa_name[120], bb_name[120], non_aname[120], 
    non_bname[120], non_vname[120], grd_aname[120], grd_bname[120],grd_vname[120], 
    grd_iname[120], grd_jname[120], grd_pname[120], grd_cname[120], setname[120], *fname;

  double map_equatorial_radius_m,map_eccentricity, e2,
    map_reference_latitude, map_reference_longitude, 
    map_second_reference_latitude, sin_phi1, cos_phi1, kz,
    map_scale, r0, s0, epsilon;
  int bcols, brows, ind;
  
  fprintf(mout,"Egg_or_slice=%d\n",negg);

  if (MAKEJOB) {    
    /* write out setup file command to job file */
    fprintf(jout," echo ""Running setup on %s""\n",mname);
    sprintf(fnamel,"%s.out",mname);
    fprintf(jout,"if %s %s > %s\n",setup_name,mname,fnamel);
    fprintf(jout,"then\n");
    fprintf(jout," echo ""setup successfully completed""\n");
    fprintf(jout," SIRFsOK=0\n");
  }
  
  /* read time period information */
  dstart=get_prompt_iarg(argc,argn,argv,"Enter starting day: ",1);
  dend=get_prompt_iarg(argc,argn,argv,"Enter ending day: ",1);
  year=get_prompt_iarg(argc,argn,argv,"Enter year (XXXX): ",1997);
  mstart=0;  /* start minute of day */
  mend=1440; /* end minute of day */

  printf("Input day range %d to %d year %d\n",dstart,dend,year);
  
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
  response_threshold=-8.0;  /* response pattern minimum threshold in dB */
  flatten=FALSE;     /* flatten antenna response to 1,0 if TRUE */
  median_flag=FALSE; /* include median filter in SIR processing if TRUE */

  /* write to meta file */
  fprintf(mout," A_initialization=%10.5f\n", a_init);  
  fprintf(mout," A_offset_value=%10.5f\n", a_offset);
  fprintf(mout," B_initialization=%10.5f\n", b_init);
  fprintf(mout," B_weight=%10.5f\n", b_weight);
  fprintf(mout," Max_iterations=%3d\n", nits);
  fprintf(mout," Reference_incidence_angle=%10.5f\n", angle_ref);
  fprintf(mout," Response_threshold=%10.5f\n", response_threshold);
  fprintf(mout," Flat_response=%c\n", TF[flatten]);
  fprintf(mout," Median_filter=%c\n", TF[median_flag]);

  /* read region parameters definition file name */
  (void) get_prompt_sarg(argc,argn,argv,"Enter region parameters file name: (NONE for manual input)","NONE",rfile, sizeof(rfile));
  printf("rfile=%s  %d\n",rfile,strncmp(rfile,"NONE",4));  
 
  if (strncmp(rfile,"NONE",4)==0 || strncmp(rfile,"none",4)==0) { /* use parameters file as input */
    pfile=FALSE;
    nregions=get_prompt_iarg(argc,argn,argv,"How many regions to output? ",1);
  } else {  /* manual or command line input */
    pfile=TRUE;
    fprintf(mout," Region_parameters_file=%s\n",rfile);
    pid=fopen(rfile,"r");
    if (pid==NULL) {
      fprintf(stderr,"*** could not open region parameters file: %s\n",rfile);
      exit(-1);      
    }
    fscanf(pid,"%d",&nregions);
    printf("Number of regions: %d\n",nregions);
  }
  sections=FALSE;
  fprintf(mout," Num_Regions=%2d\n",nregions);
  
  /* for each region, read in the parameters that define the region 
     size and projection and data selection criteria */

  ircnt=0;  /* count total regions */
  for (iregion=0; iregion<nregions; iregion++) {
    dateline=FALSE;
    
    /* region ID number */ 
    if (pfile) {
      fscanf(pid,"%d",&regnum);
      printf("Region %d number: %d\n",iregion,regnum);
    } else {
      printf("Region definition input for Region %d of %d\n",iregion,nregions);
      printf("Enter region ID number (or a negative value for manual definition)\n");      
      regnum=get_prompt_iarg(argc,argn,argv,"Region ID number: ",101);
    }
    
    /* define region, using auto definition if possible */
    strncpy(regname,"Custom",10);
    if (regnum > 0) { /* use region definition from standard region definition file */
      getregdata(regnum,&iproj,&dateline,&latl,&lonl,&lath,&lonh,regname);
      if (regnum >= 100 && regnum < 110) poleflag=2; /* south pole regions */
      if (regnum >= 110 && regnum < 120) poleflag=1; /* north pole regions */
      if (((regnum >= 0) && (regnum < 100)) || (regnum >= 120)) poleflag=0; /* non-polar area */
      printf("Region name: '%s'  Def Proj %d  Dateline %d\n",regname,iproj,dateline);
    } else {
      if (get_prompt_larg(argc,argn,argv,"Is this a polar region? (Y/N)",1)) {
	printf("Polar region:\n");
	deglat=get_prompt_farg(argc,argn,argv,"  Degrees in latitude?",0.);
	if (get_prompt_larg(argc,argn,argv," North pole? (Y/N)",1)) {
	  lath = 90.;	/* north pole */
	  latl = 90. - deglat;
	  lonh = 180.;
	  lonl = -180.;
	  poleflag = 1;
	} else {
	  latl = -90.;	/* south pole */
	  lath = -90. + deglat;
	  lonl = -180.;
	  lonh = 180;
	  poleflag = 2;
	}
      } else {
	printf("Specify coordinates: (-180 to 180 lon, -90 to 90 lat)\n");
	poleflag=0;
	dateline=0;
	latl=get_prompt_farg(argc,argn,argv," Lower-left corner Latitude:  ",0.);
	lonl=get_prompt_farg(argc,argn,argv," Lower-left corner Longitude: ",0.);
      	lath=get_prompt_farg(argc,argn,argv," Upper-right corner Latitude: ",0.);
	lonh=get_prompt_farg(argc,argn,argv," Upper-right corner Longitude:",0.);
	dateline=get_prompt_larg(argc,argn,argv," Region crosses daeline (Y/N) ",0);	
      }
      deglat=lath-latl;
      deglon=lonh-lonl;
    }
    
    /* print region ID number and bounding box info */
    printf("\nRegion definition information\n");
    printf("  Latitude range:  %f %f\n",latl,lath);
    printf("  Longitude range: %f %f\n",lonl,lonh);
    printf("  Region polar code (0=arbitrary, 1=N pol, 2=S pole: %d\n",poleflag);
    if (dateline) {
      printf("  Region crosses dateline\n");
      maplxlon=min(lonh,lonl);
      maprxlon=max(lonh,lonl);
      lonl=maplxlon;
      lonh=maprxlon;
      printf("  Corrected longitude range: %f %f\n",lonl,lonh);
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
    /* projection codes 
       0 = Rectalinear lat/lon
       1 = Lambertian Equal-Area (fixed radius)
       2 = Lambertian Equal-Area (local radius)
       5 = SAR Polar Stereographic
       8 = EASE2 N
       9 = EASE2 S
      10 = EASE2 T
      11 = EASE north
      12 = EASE south
      13 = EASE globe */

    printf("Projection code (0=lat/lon,1=Lambert(fixed),2=Lambert(local),5=Polar stereo,\n      8=EASE2N,9=EASE2S,10=EASE2T,11=EASE1N,12=EASE1S,13=EASE1G): %d\n",projt);

    /* for each particular projection, define the orgin, size, scale, offset */
    switch (projt) {
    case 0:   /* rectangular lat/lon */
      a0=lonl;
      b0=latl;
      aorglon=lonh-lonl;
      aorglat=lath-latl;
      ascale=24.0;	/* pixels/deg (SASS/NSCAT/Seawinds eggs) */
      bscale=ascale;
      printf("Lat/Lon grid pixel spacing: %f %f pix/deg\n",ascale,bscale);      
      xdeg=aorglon;
      ydeg=aorglat;
      nsx=xdeg*ascale;
      nsy=ydeg*bscale;
      printf("Corner: %f %f deg  Span: %f %f\n",a0,b0,xdeg,ydeg);
      break;
      
    case 1:   /* lambertian equal-area */
    case 2:
      ascale = 1./8.9;    /* 8.9 km/pixel (ers-1) */
      ascale = 2./8.9;	  /* 4.45 km/pixel (SASS/NSCAT/Seawinds eggs) */
      if (negg==1) 
	ascale = 4./8.9;  /* 2.225 km/pixel (Seawinds slices) */
      bscale = ascale;
      printf("Lambert pixel spacing: %f %f km/pixel\n",1./ascale,1./bscale);
      /* figure out what kind of area (polar or otherwise) and set up origin 
	 and mapping transformation variables for lambertian */
      if (poleflag==0) { /* not a polar region, origin in center */
	aorglat = (lath + latl)/2.;
	aorglon = (lonh + lonl)/2.;
	mapuylat = lath;
	maplylat = latl;
	maplxlon = lonl;
	maprxlon = lonh;
	if (lath*latl < 0.0) { /* region contains equator */
	  maplxlat = 0.;
	  maprxlat = 0.;
	  if (lath > abs(latl)) { /* mostly in northern hemisphere */
	    mapuylon = lonl;
	    maplylon = aorglon;
	  } else {	          /* mostly in southern hemisphere */
	    mapuylon = aorglon;
	    maplylon = lonl;
	  } 
	} else {
	  if (aorglat>=0.0) { /* region in northern hemisphere */
	    maplxlat = latl;
	    maprxlat = latl;
	    mapuylon = lonl;
	    maplylon = aorglon;
	  } else {
	    if (aorglat <= 0.0) { /* region in southern hemisphere */
	      maplxlat = lath;
	      maprxlat = lath;
	      mapuylon = aorglon;
	      maplylon = lonl;
	    } else {
	      fprintf(stderr,"*** invalid region definition\n");
	      exit(-1);
	    }
	  }
	}
      } else {		/* polar region */
	mapuylon = 0.;
	maplylon = 180.;
	if (poleflag==1) { /* north pole */
	  aorglat = lath;
	  aorglon = 180.;
	  maplxlon = 90.;
	  maplxlat = latl;
	  maprxlon = -90.;
	  maprxlat = latl;
	  mapuylat = latl;
	  maplylat = latl;
	} else {
	  if (poleflag==2) { /* south pole */
	    aorglat = latl;
	    aorglon = 0.;
	    maplxlon = -90.;
	    maplxlat = lath;
	    maprxlon = 90.;
	    maprxlat = lath;
	    mapuylat = lath;
	    maplylat = lath;
	  } else {
	    fprintf(stderr,"*** invalid region definition\n");
	    exit(-1);
	  }
	}
      }    
      lambert1(maplxlat,maplxlon,&x0,&y0,aorglat,aorglon,projt);
      lmostx = x0; /* left most x value */
      lambert1(maprxlat,maprxlon,&x0,&y0,aorglat,aorglon,projt);
      rmostx = x0; /* right most x value */
      lambert1(mapuylat,mapuylon,&x0,&y0,aorglat,aorglon,projt);
      umosty = y0; /* highest y value */
      lambert1(maplylat,maplylon,&x0,&y0,aorglat,aorglon,projt);
      lmosty = y0; /* lowest y value */
      break;

    case 5:   /* polar stereographic */
      if (regnum >= 200) { /* not a polar region */
	fprintf(stderr,"*** polar stereographic projection selected for non-polar region %d\n",regnum);
	return;
      }
      ascale = 8.9;          /* 8.9 km/pixel (ers-1) */
      ascale = 8.9/2.0;	     /* 4.45 km/pixel (SASS/NSCAT/Seawinds eggs) */
      if (negg==1) 
	ascale = 8.9/4.0;    /* 2.225 km/pixel (Seawinds slices) */
      bscale = ascale;
      printf("Polar Stereographic pixel spacing: %f, %f km/pixel\n",ascale,bscale);      
      if (((regnum >= 100) && (regnum<110)) || (poleflag==2)) { /* south hemi */
	xdeg = 0.0;
	ydeg = -70.0;
	maplxlat = lath;
	maplxlon = -90.;
	maprxlat = lath;
	maprxlon = 90.;
	mapuylat = lath;
	mapuylon = xdeg;
	maplylat = lath;
	maplylon = 180.;
      } else if (((regnum>=110) && (regnum<120))|| (poleflag==1)) { /* north hemi */
	xdeg = -45.0;
	ydeg = 70.0;
	maplxlat = latl;
	maplxlon = -135.;
	maprxlat = latl;
	maprxlon = 45.;
	mapuylat = latl;	
	mapuylon = 135.;
	maplylat = latl;
	maplylon = xdeg;
      }
      aorglon = xdeg;
      aorglat = ydeg;
      printf("Polster 1 %f %f %f %f  %f %f %f %f  %f %f %f %f\n",xdeg,ydeg,aorglon,aorglat,maplxlat,maplxlon,maprxlat,maprxlon,mapuylat,mapuylon,maplylat,maplylon); 
      
      polster(maplxlon,maplxlat,&x0,&y0,aorglon,aorglat);
      lmostx = x0; /* left most x value */
      printf("Polster 0 %f %f %f %f %f %f\n",maplxlon,maplxlat,x0,y0,aorglon,aorglat);
      polster(maprxlon,maprxlat,&x0,&y0,aorglon,aorglat);
      rmostx = x0; /* right most x value */
      polster(mapuylon,mapuylat,&x0,&y0,aorglon,aorglat);
      umosty = y0; /* highest y value */
      polster(maplylon,maplylat,&x0,&y0,aorglon,aorglat); 
      lmosty = y0; /* lowest y */
      printf("Polster 2 %f %f %f %f\n",lmostx,rmostx,umosty,lmosty);      

      break;      

    case  8:  /* ease2 grid N */
    case  9:  /* ease2 grid S */
    case 10:  /* ease2 grid T */
      if (pfile) {
	fscanf(pid,"%d",&nease);
      } else {
	nease=get_prompt_iarg(argc,argn,argv,"Enter EASE grid resolution factor: (0..5) ",4);
      }
      /* projt=regnum-300; */  /* for standard projection coding in putsir */

      /* define projection parameters for particular EASE2 case */
      ind=0;  /* standard base resolution */
      printf("EASE2 parameters: proj=%d  nease=%d  ind=%d\n",projt,nease,ind);      
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

    case 11:  /* ease grid north */
    case 12:  /* ease grid south */
    case 13:  /* ease grid cylindrical */
      if (pfile) {
	fscanf(pid,"%d",&nease);
      } else {
	nease=get_prompt_iarg(argc,argn,argv,"Enter EASE grid resolution factor: (2**n * 1.5 km) ",4);
      }
      projt=regnum-300;  /* for standard projection coding in putsir */

      switch(projt) {
      case 11:  /* ease grid north */
	nsx=720*powf(2., (4-nease))+1;
	nsy=720*powf(2., (4-nease))+1;
	ascale=2.0 * (double) 6371.228/((double) 25.067525*pow((double)2.0,(nease-4)));
	bscale=(double) 25.067525*pow((double) 2.0, (nease-4));
	xdeg=360.0*powf(2., (4-nease));
	ydeg=360.0*powf(2., (4-nease));
	a0=-xdeg;
	b0=-ydeg;
	xdim = 10*nint((nsx/ascale+10.)/10.);
	ydim = 10*nint((nsy/bscale+10.)/10.);
	aorglon=xdeg;
	aorglat=ydeg;
	break;

      case 12:  /* ease grid south */
	nsx=720*powf(2., (4-nease))+1;
	nsy=720*powf(2., (4-nease))+1;
	ascale=2.0*(double)6371.228/((double)25.067525*pow((double)2.0,(nease-4)));
	bscale=(double)25.067525*pow((double)2.0,(nease-4));
	xdeg=360.0*powf(2., (4-nease));
	ydeg=360.0*powf(2., (4-nease));
	a0=-xdeg;
	b0=-ydeg;
	xdim = 10*nint((nsx/ascale+10.)/10.);
	ydim = 10*nint((nsy/bscale+10.)/10.);
	aorglon=xdeg;
	aorglat=ydeg;
	break;

      case 13:  /* ease grid cylindrical */
	nsx=1382*powf(2.,(4-nease))+1;
	nsy= 586*powf(2.,(4-nease));
	ascale=(double)6371.228/((double)25.067525*pow((double)2.0,(nease-4)));
	bscale=(double) 25.067525*pow((double) 2.0, (nease-4));
	xdeg=691.0*powf(2.,(4-nease));
	ydeg=292.5*powf(2.,(4-nease));
	a0=-xdeg;
	b0=-ydeg;
	xdim = 10*nint((nsx/ascale+10.)/10.);
	ydim = 10*nint((nsy/bscale+10.)/10.);
	aorglon=xdeg;
	aorglat=ydeg;
	break;

      default: /* should not occur! */
	break;	
      }
      break;
      
    default:
      fprintf(stderr,"*** Error selecting projection type %d ***\n", projt);
      exit(-1);      
      break;
    }
  
    /* make map dimensions an even integer divisible by both the
       grid size and sectioning write */
  
    if (projt != 0 && projt < 8) {      
      printf("Region geometry (in km): (raw computations)  proj=%d\n",projt);
      printf("  upper-most y %f\n",umosty);
      printf("  lower-most y %f\n",lmosty);
      printf("  right-most x %f\n",rmostx);
      printf("  left-most  x %f\n",lmostx);
    }
    
    if (projt < 8) {  /* don't need this code section for EASE1/2 */
      temp = (lmostx-100.)/100.;
      lmostx = nint(temp)*100.;
      temp = (rmostx+100.)/100.;
      rmostx = nint(temp)*100.;
      temp = (lmosty-100.)/100.;
      lmosty = nint(temp)*100.;
      temp = (umosty+100.)/100.;
      umosty = nint(temp)*100.;
      if (projt != 0) {      
	printf("Region geometry (in km): (after smoothed dimensioning)\n");
	printf("  upper-most y %f\n",umosty);
	printf("  lower-most y %f\n",lmosty);
	printf("  right-most x %f\n",rmostx);
	printf("  left-most  x %f\n",lmostx);
	/* lower left corner */
	a0 = lmostx;
	b0 = lmosty;
	/* image dimensions */
	xdim = abs(lmostx)+abs(rmostx);
	ydim = abs(lmosty)+abs(umosty);
	/* pixel size */
	if ((projt==1)||(projt==2)) {
	  nsy = 10*nint((ydim*bscale+10.)/10.);
	  nsx = 10*nint((xdim*ascale+10.)/10.);
	} else if (projt == 5) {
	  nsy = 10*nint((ydim/bscale+10.)/10.);
	  nsx = 10*nint((ydim/ascale+10.)/10.);
	}
	/* origin */
	xdeg=aorglon;
	ydeg=aorglat;
      }
    }
      
    /* land/sea flag */
    toil=3;    /* flags not used */
    fprintf(mout,"  Toil_flag=%2d\n",toil);
  
    /* select ascending/descending data */
    iasc=0;
    if (pfile) {
      fscanf(pid,"%d",&iasc);
    } else {
      iasc=get_prompt_iarg(argc,argn,argv,"Enter AscDesc flag (0=all,1=asc,2=dsc,3=morn,4=eve)",0);
    }
    printf("Asc/Desc flag: (0=both,1=asc,2=dsc,3=morn,4=eve) %d\n",iasc);
    fprintf(mout,"  AscDesc_flag=%2d\n",iasc);

    /* SSM/I SPECIAL: select beam [channel] (selects frequency and polarization for SSMI) */
    if (pfile) {
      fscanf(pid,"%d",&ibeam);
    } else {
      ibeam=get_prompt_iarg(argc,argn,argv,"Select beam (1=19V,2=19H,3=22V,4=37V,5=37H,6=85V,7=85H) ",4);
    }
    printf("Beam index (1=19V,2=19H,3=22V,4=37V,5=37H,6=85V,7=85H): %d\n",ibeam);
    ipolar=0;   /* h pol */
    if (ibeam==1 || ibeam==3 || ibeam==4 || ibeam==6) ipolar=1; /* v pol */
    fprintf(mout,"  Polarization=%d\n",ipolar);
    fprintf(mout,"  Beam_index=%d\n", ibeam);

    /* SSM/I SPECIAL:set number of iterations based on channel number */
    nits=20;
    if (ibeam>=4) nits=30;
    fprintf(mout," Max_iterations=%d\n", nits);

    /* SSM/I SPECIAL: reset projection size to coarser for lower resolution beams */
    if (ibeam < 6) {
      if (projt==1 || projt==2) {  /* lambert */
	ascale = 1./8.9;    /* 8.9 km/pixel (ers-1) */
	bscale=ascale;
	nsy = 10*nint((ydim*bscale+10.)/10.);
	nsx = 10*nint((xdim*ascale+10.)/10.);
      }
      if (projt==5) {              /* polar stereographic */
	ascale=8.9;
	bscale=ascale;
	nsy = 10*nint((ydim/bscale+10.)/10.);
	nsx = 10*nint((ydim/ascale+10.)/10.);
      }
      if (projt==8 || projt==9 || projt==10) { /* EASE2 */
	printf("Resizing for low channel number %d\n",ibeam);	
	ascale=ascale-1;
	nsy = nsy/2;
	nsx = nsx/2;
	xdeg = xdeg/2;
	ydeg = ydeg/2;
	xdim=xdeg*2;
	ydim=ydeg*2;
	aorglon=xdeg;
	aorglat=ydeg;
      }
    }

    /* summarize region results */
    if (projt != 0) {
      printf("Projection %d   size %d %d\n",projt,nsx,nsy);
      printf("Scales:           %f %f\n",ascale, bscale);
      printf("Origin (lat,lon)  %f %f\n",aorglat,aorglon);
      printf("Offset x,y in km: %f %f\n",a0,b0);
      printf("Dimensions in km: %f %f\n",xdim, ydim);
    }
    printf("size in pixels (x,y)(lon,lat): %f %f\n\n",nsx,nsy);

    /* set grid image size parameters */
    non_size=5;  /* number of enhanced resolution pixels/non-enhanced pixels */
    if (projt>7) /* handle EASE differently */
      non_size=4;
 
    if (non_size*(nsx/non_size) != nsx || non_size*(nsy/non_size) != nsy) {
      fprintf(stderr,"*** warning: non grid size parameter %d does not evenly divide image size: %d %d\n",non_size,nsx,nsy);
      fprintf(stdout,"*** warning: non grid size parameter %d does not evenly divide image size: %d %d\n",non_size,nsx,nsy);
    }

    /* optionally section region in to smaller images for processing.  They will be recombined later */
    nsection=0;
    if (pfile) {
      fscanf(pid,"%d",&nsection);
    } else {
      nsection=get_prompt_iarg(argc,argn,argv,"Enter number of sections/pattern parameter (0=no sectioning) ",1);
    }
    if (nsection==1) nsection=0;
    printf("Sectioning code: %d\n",nsection);
    fprintf(mout,"  Sectioning_code=%d\n",nsection);
    if (nsection > 0) sections=TRUE;

  /* save unsectioned projection info */
    ascale_s=ascale;
    bscale_s=bscale;
    a0_s=a0;
    b0_s=b0;
    xdeg_s=xdeg;
    ydeg_s=ydeg;
    nsx_s=nsx;
    nsy_s=nsy;

    nsect=(nsection % 100);    /* number of sections */
    nt=nsection/100;           /* section type code */

    /* for each output section (0=unsection region) */
    for (isection=0; isection<=nsect; isection++) {
      fprintf(mout,"  Begin_section_description\n");
      fprintf(mout,"  Section_id=%d\n",isection);

      if (isection > 0) {
	/* recompute projection information based on sectioning */
	ascale=ascale_s;
	bscale=bscale_s;
	a0=a0_s;
	b0=b0_s;
	xdeg=xdeg_s;
	ydeg=ydeg_s;
	nsx=nsx_s;
	nsy=nsy_s;

	/* compute pixel locations for sectioning */
	section_pixels(isection, &ix, &iy, nsect, nt, 0.1, nsx, nsy, non_size,
		       &nsx2, &nsy2, &ix1, &iy1,  &ix2,  &iy2, &jx1, &jy1,  &jx2,  &jy2);

	/* compute section projection info */
	switch(projt) {
	case 0: /* lat/lon */
	  a02=(ix-1)*xdim/(float)nsx+a0;
	  b02=(iy-1)*ydim/(float)nsy+b0;
	  xdeg2=(float)nsx2*xdeg/(float)nsx;
	  ydeg2=(float)nsy2*ydeg/(float)nsy;
	  ascale2=xdeg2/(float)nsx2;
	  bscale2=ydeg2/(float)nsy2;
	  break;

	case 1: /* lambert */
	case 2:
	  a02=(ix-1)/ascale+a0;
	  b02=(iy-1)/bscale+b0;
	  xdeg2=xdeg;
	  ydeg2=ydeg;
	  ascale2=ascale;
	  bscale2=bscale;
	  break;

	case 5: /* polar ster */
	  a02=(ix-1)*ascale+a0;
	  b02=(iy-1)*bscale+b0;
	  xdeg2=xdeg;
	  ydeg2=ydeg;
	  ascale2=ascale;
	  bscale2=bscale;
	  break;

	case  8: /* ease2 grid */
	case  9:
	case 10:
	  a02=a0+(ix-1.0);
	  b02=b0+(iy-1.0);
	  xdeg2=xdeg;
	  ydeg2=ydeg;
	  ascale2=ascale;
	  bscale2=bscale;
	  break;

	case 11: /* ease1 grid */
	case 12:
	case 13:
	  a02=a0+(ix-1.0);
	  b02=b0+(iy-1.0);
	  xdeg2=xdeg;
	  ydeg2=ydeg;
	  ascale2=ascale;
	  bscale2=bscale;
	  break;
	}

	aorglon=xdeg2;
	aorglat=ydeg2;
	nsx=nsx2;
	nsy=nsy2;
	a0=a02;
	b0=b02;
	xdim=nsx/ascale;
	ydim=nsy/bscale;
	if (projt==5) {
	  xdim=nsx*ascale;
	  ydim=nsy*bscale;
	}
      }

      printf("\nSectioning: (x,y) %d %d\n",isection,nsect);

      /* write map projection info to output file */
      fprintf(mout,"   Project_type=%d\n",projt);
      fprintf(mout,"   Projection_origin_x=%16.9f\n", aorglat);
      fprintf(mout,"   Projection_origin_y=%16.9f\n", aorglon);
      fprintf(mout,"   Projection_offset_x=%16.9f\n", a0);
      fprintf(mout,"   Projection_offset_y=%16.9f\n", b0);
      fprintf(mout,"   Projection_scale_x=%16.9f\n", ascale);
      fprintf(mout,"   Projection_scale_y=%16.9f\n", bscale);
      fprintf(mout,"   Projection_dim_x=%d\n",xdim);
      fprintf(mout,"   Projection_dim_y=%d\n",ydim);
      fprintf(mout,"   Image_size_x=%d\n", nsx);
      fprintf(mout,"   Image_size_y=%d\n", nsy);
      if (isection > 0)
	fprintf(mout,"   Section_loc_pixels=%d %d %d %d %d %d %d %d\n",ix1,iy1,ix2,iy2,jx1,jy1,jx2,jy2);

      /* generate parameters for non-enhanced gridded images */
      nsx2=nsx/non_size;
      nsy2=nsy/non_size;
      if (non_size*nsx2 != nsx || non_size*nsy2 != nsy)
	fprintf(stderr,"*** WARNING: non grid size %d ' does not evenly divide image size %d %d\n",non_size,nsx,nsy);
      ascale2=ascale;
      bscale2=bscale;
      if (projt==0||projt==1|| projt==2) {/* rect, lambert */
	ascale2=ascale/non_size;
	bscale2=bscale/non_size;
      } else if (projt==5) { /* polar stereographic */
	ascale2=ascale*non_size;
	bscale2=bscale*non_size;
      } else if (projt==8 || projt==9 || projt==10) { /* EASE2 */
	ascale2=ascale-2;
      } else if (projt==11 || projt==12 || projt==13) { /* EASE1 */
	ascale2=ascale/4;
	bscale2=bscale/4;
      } else {
	nsx2=nsx;
	nsy2=nsy;
	fprintf(stderr,"*** WARNING: Projection type can not generate Non-enhanced parameters ***\n");
      }

      /* compute grid pixel locations */
      ix1g=(ix1-1)/non_size+1;
      iy1g=(iy1-1)/non_size+1;
      ix2g=(ix2-1)/non_size+1;
      iy2g=(iy2-1)/non_size+1;
      jx1g=(jx1-1)/non_size+1;
      jy1g=(jy1-1)/non_size+1;
      jx2g=(jx2-1)/non_size+1;
      jy2g=(jy2-1)/non_size+1;
    
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
      if (isection>0)
	fprintf(mout,"   Section_grd_pixels=%d %d %d %d %d %d %d %d\n", ix1g,iy1g,ix2g,iy2g,jx1g,jy1g,jx2g,jy2g);
	      
      /* for this region, generate standard product data file names abreviation */
      switch(regnum) {
      case 100:
	strncpy(reg,"Ant",4);
	break;
      case 110:
	strncpy(reg,"Arc",4);
	break;
      case 112:
	strncpy(reg,"NHe",4);
	break;
      case 202:
	strncpy(reg,"Grn",4);
	break;
      case 203:
	strncpy(reg,"Ala",4);
	break;
      case 204:
	strncpy(reg,"CAm",4);
	break;
      case 205:
	strncpy(reg,"NAm",4);
	break;
      case 206:
	strncpy(reg,"SAm",4);
	break;
      case 207:
	strncpy(reg,"NAf",4);
	break;
      case 208:
	strncpy(reg,"SAf",4);
	break;
      case 209:
	strncpy(reg,"Sib",4);
	break;
      case 210:
	strncpy(reg,"Eur",4);
	break;
      case 211:
	strncpy(reg,"SAs",4);
	break;
      case 212:
	strncpy(reg,"ChJ",4);
	break;
      case 213:
	strncpy(reg,"Ind",4);
	break;
      case 214:
	strncpy(reg,"Aus",4);
	break;
      case 256:
	strncpy(reg,"Ber",4);
	break;
      case 307:
	strncpy(reg,"E2M",4);
	break;
      case 308:
	strncpy(reg,"E2N",4);
	break;
      case 309:
	strncpy(reg,"E2S",4);
	break;
      case 310:
	strncpy(reg,"E2T",4);
	break;
      case 311:
	strncpy(reg,"EaN",4);
	break;
      case 312:
	strncpy(reg,"EaS",4);
	break;
      case 313:
	strncpy(reg,"EsG",4);
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
	if (regnum==100 || regnum == 206 || /* Ant, SAm */
	    regnum==208 || regnum == 214 || /* SAf, Aus */
	    regnum==213 || regnum == 309)  { /* Ind, E2S */
	  tsplit1=0.0;
	  tsplit2=12.0;
	}
	fprintf(mout,"   Local_time_split1=%16.9f\n", tsplit1);
	fprintf(mout,"   Local_time_split2=%16.9f\n", tsplit2);
      }
  
      /* create the full set of BYU naming standard file names -- whether used or not */
      iy=(year % 100);
      sen='F';      /* F=ssmi, A=AMSRE, R=SMMR, I=SSMIS */
      if (F_num < 10.0) /* code the sensor number for SSMI and SSMIS */
	cegg=(char) (F_num+48);  /* 0...9 */
      else
	cegg=(char) (F_num-10+65); /* A...Z */
      chan=(char) (ibeam+48);
      cpol='b';  /* both asc and desc (all data) used */

      /* modify file name if LTOD ascending/descending or morn/even */

      if (iasc != 0) {
	cpol='a';  /* asc */
	if (iasc==2) cpol='d'; /* desc */
	if (iasc==3) cpol='m'; /* morning/midnight */
	if (iasc==4) cpol='e'; /* evening */
	if (iasc==5) cpol='n'; /* noon/night */
      }

      if (isection > 0){ /* subsection names */
	sprintf(setname,"%c%c%c%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.setup",sen,cegg,chan,cpol,reg,iy,dstart,dend,nsection,isection);
	sprintf(lisname,"%c%c%c%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.lis",sen,cegg,chan,cpol,reg,iy,dstart,dend,nsection,isection);
	sprintf(a_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'a',reg,iy,dstart,dend,nsection,isection,"sir");
	sprintf(b_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'b',reg,iy,dstart,dend,nsection,isection,"sir");
	sprintf(i_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'I',reg,iy,dstart,dend,nsection,isection,"sir");
	sprintf(j_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'J',reg,iy,dstart,dend,nsection,isection,"sir");
	sprintf(c_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'C',reg,iy,dstart,dend,nsection,isection,"sir");
	sprintf(p_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'p',reg,iy,dstart,dend,nsection,isection,"sir");
	sprintf(v_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'V',reg,iy,dstart,dend,nsection,isection,"sir");
	sprintf(e_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'E',reg,iy,dstart,dend,nsection,isection,"sir");
	sprintf(aa_name,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'a',reg,iy,dstart,dend,nsection,isection,"ave");
	sprintf(bb_name,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'b',reg,iy,dstart,dend,nsection,isection,"ave");
	sprintf(non_aname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'a',reg,iy,dstart,dend,nsection,isection,"non");
	sprintf(non_bname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'b',reg,iy,dstart,dend,nsection,isection,"non");
	sprintf(non_vname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'V',reg,iy,dstart,dend,nsection,isection,"non");
	sprintf(grd_aname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'a',reg,iy,dstart,dend,nsection,isection,"grd");
	sprintf(grd_bname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'b',reg,iy,dstart,dend,nsection,isection,"grd");
	sprintf(grd_vname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'V',reg,iy,dstart,dend,nsection,isection,"grd");
	sprintf(grd_iname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'I',reg,iy,dstart,dend,nsection,isection,"grd");
	sprintf(grd_jname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'J',reg,iy,dstart,dend,nsection,isection,"grd");
	sprintf(grd_cname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'C',reg,iy,dstart,dend,nsection,isection,"grd");
	sprintf(grd_pname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d-s%0.3d-%0.2d.%3s",sen,cegg,chan,cpol,'p',reg,iy,dstart,dend,nsection,isection,"grd");
      } else { /* section names */
	sprintf(setname,"%c%c%c%c-%3s%0.2d-%0.3d-%0.3d.setup",sen,cegg,chan,cpol,reg,iy,dstart,dend);
	sprintf(lisname,"%c%c%c%c-%3s%0.2d-%0.3d-%0.3d.lis",sen,cegg,chan,cpol,reg,iy,dstart,dend);
	sprintf(a_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'a',reg,iy,dstart,dend,"sir");
	sprintf(b_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'b',reg,iy,dstart,dend,"sir");
	sprintf(i_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'I',reg,iy,dstart,dend,"sir");
	sprintf(j_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'J',reg,iy,dstart,dend,"sir");
	sprintf(c_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'C',reg,iy,dstart,dend,"sir");
	sprintf(p_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'p',reg,iy,dstart,dend,"sir");
	sprintf(v_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'V',reg,iy,dstart,dend,"sir");
	sprintf(e_name, "%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'E',reg,iy,dstart,dend,"sir");
	sprintf(aa_name,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'a',reg,iy,dstart,dend,"ave");
	sprintf(bb_name,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'b',reg,iy,dstart,dend,"ave");
	sprintf(non_aname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'a',reg,iy,dstart,dend,"non");
	sprintf(non_bname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'b',reg,iy,dstart,dend,"non");
	sprintf(non_vname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'V',reg,iy,dstart,dend,"non");
	sprintf(grd_aname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'a',reg,iy,dstart,dend,"grd");
	sprintf(grd_bname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'b',reg,iy,dstart,dend,"grd");
	sprintf(grd_vname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'V',reg,iy,dstart,dend,"grd");
	sprintf(grd_iname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'I',reg,iy,dstart,dend,"grd");
	sprintf(grd_jname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'J',reg,iy,dstart,dend,"grd");
	sprintf(grd_cname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'C',reg,iy,dstart,dend,"grd");
	sprintf(grd_pname,"%c%c%c%c-%c-%3s%0.2d-%0.3d-%0.3d.%3s",sen,cegg,chan,cpol,'p',reg,iy,dstart,dend,"grd");
      }

      /* save *-a-*.sir, .ave names for job script */
      //name_store(iregion)=a_name;      
      //name_store2(iregion)=aa_name;
      
      /* write out product names to meta file */
      printf("Output setup file:%s\n",setname);
      printf("Output A file:    %s\n",a_name);
      //printf("Output B file:    %s\n",b_name(1:length(b_name)));
      printf("Output C file:    %s\n",c_name);
      printf("Output I file:    %s\n",i_name);
      printf("Output J file:    %s\n",j_name);
      printf("Output P file:    %s\n",p_name);
      printf("Output E file:    %s\n",e_name);
      printf("Output V file:    %s\n",v_name);
      printf("Output A ave file:%s\n",aa_name);
      //printf("Output B ave file:%s\n",bb_name);
      printf("Output A non file:%s\n",non_aname);
      //printf("Output B non file:%s\n",non_bname);
      printf("Output V non file:%s\n",non_vname);
      printf("Output A grd file:%s\n",grd_aname);
      //printf("Output B grd file:%s\n",grd_bname);
      printf("Output V grd file:%s\n",grd_vname);
      printf("Output I grd file:%s\n",grd_iname);
      printf("Output J grd file:%s\n",grd_jname);
      printf("Output C grd file:%s\n",grd_cname);
      printf("Output P grd file:%s\n",grd_pname);
      printf("Output lis file:  %s\n",lisname);

      fprintf(mout,"  Setup_file=%s\n",setname);
      fprintf(mout,"  Begin_product_file_names\n");
      fprintf(mout,"   SIRF_A_file=%s\n",a_name);
      //fprintf(mout,"   SIRF_B_file=%s\n",b_name);
      fprintf(mout,"   SIRF_C_file=%s\n",c_name);
      fprintf(mout,"   SIRF_I_file=%s\n",i_name);
      fprintf(mout,"   SIRF_J_file=%s\n",j_name);
      fprintf(mout,"   SIRF_E_file=%s\n",e_name);
      fprintf(mout,"   SIRF_V_file=%s\n",v_name);
      fprintf(mout,"   SIRF_P_file=%s\n",p_name);
      fprintf(mout,"   AVE_A_file=%s\n",aa_name);
      //fprintf(mout,"   AVE_B_file=%s\n",bb_name);
      fprintf(mout,"   GRD_A_file=%s\n",grd_aname);
      //fprintf(mout,"   GRD_B_file=%s\n",grd_bname);
      fprintf(mout,"   GRD_V_file=%s\n",grd_vname);
      fprintf(mout,"   GRD_I_file=%s\n",grd_iname);
      fprintf(mout,"   GRD_J_file=%s\n",grd_jname);
      fprintf(mout,"   GRD_C_file=%s\n",grd_cname);
      fprintf(mout,"   GRD_P_file=%s\n",grd_pname);
      fprintf(mout,"   NON_A_file=%s\n",non_aname);
      //fprintf(mout,"   NON_B_file=%s\n",non_bname);
      fprintf(mout,"   NON_V_file=%s\n",non_vname);
      fprintf(mout,"   Info_file=%s\n",lisname);
      fprintf(mout,"  End_product_file_names\n");

      /* add SIRF commands to job script */
      if (MAKEJOB) {
	if (nsection==0 || nsection>0 && isection > 0) {
	  fprintf(jout," echo ""SIRFing %s""\n", setname);
	  fprintf(jout," if %s %s  > %s.out\n", sirf_name,setname,setname);
	  fprintf(jout," then\n");        /* SIRF successfull */
	  fprintf(jout,"  echo ""SIRF successfully completed""\n");
	  fprintf(jout," else\n");
	  fprintf(jout,"  SIRFsOK=1\n");  /* SIRF not successfull */
	  fprintf(jout	," fi\n");
	}	
      }
	      
      fprintf(mout," End_section_description\n");
      ircnt++;
    }   /* end section loop */

    fprintf(mout," End_region_description\n");
  }      /* end region loop */
  fprintf(mout,"End_description\n");
  
  if (pfile) fclose(pid);
	      
  printf("\nTotal regional images: %d Regions: %d\n", ircnt,nregions);
  return(err);  

  /* write conclusion to job script */
  if (MAKEJOB) {
    fname=setname;    
    if (sections) {
      fprintf(jout," echo ""Running combine_sect on %s""\n", fname);
      fprintf(jout," if combine_sect %s > %s.comb_out\n",fname,fname);
      fprintf(jout," then\n");
      fprintf(jout,"  echo ""combine_sect successfully completed""\n");
      fprintf(jout,"  echo ""Running clean_sect on %s""\n", fname);      
      fprintf(jout,"  clean_sect %s > %s.clean_out\n", fname,fname);
      fprintf(jout,"  echo ""clean_sect completed""\n");
      fprintf(jout," fi\n");
    }
    fprintf(jout," if [ $SIRFsOK = 0 ]\n");
    fprintf(jout," then\n");    
    fprintf(jout,"  echo ""Running clean_setup on %s""\n", fname);   
    fprintf(jout,"  clean_setup %s >%s.clean.out\n",fname,fname);
    fprintf(jout,"  echo ""clean_setup completed""\n");

    for (iregion=1; iregion<=nregions; iregion++) {
      //fprintf(jout,"  echo ""Running sir2gif on %s""\n", name_store2(iregion));
      //fprintf(jout,"  sir2gif -f%s %s > %s.gif_out2\n", S2G,name_store2(iregion),name_store2(iregion));
      //fprintf(jout,"  echo ""Running sir2gif on %s""\n", name_store(iregion));
      //fprintf(jout,"  sir2gif -f%s %s > %s.gif_out\n", S2G,name_store(iregion),name_store(iregion));
    }
    fprintf(jout," else\n");
    fprintf(jout,"  echo ""*** job failed due to failure of one or more SIRFs""\n");
    fprintf(jout," fi\n");
    
    fprintf(jout,"else\n");
    fprintf(jout," echo ""*** job failed during setup phase""\n");
    fprintf(jout," echo ""job execution completed""\n");
    fprintf(jout,"fi\n");
    
  }

  return(err);
}


