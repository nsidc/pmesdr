#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/****************************************************************************/

/* declarations for reading sensor swath TB files */

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
  char   ASTART_TIME[25];         /* datetimehr : scan date/time */
  double SCAN_TIME[HSCANS];       /* xtime : Time in seconds from 1/1/87    */
  double ORBIT[HSCANS];           /* rev : orbit number (hi-res)     */
  float  SC_LAT[HSCANS];          /* sclat : Spacecraft latitude            */
  float  SC_LON[HSCANS];          /* sclon : Spacecraft longitude            */
  float  SC_ALT[HSCANS];          /* sclalt : Spacecraft altitude            */
  char   ASTART_TIME_lo[25];         /* datetimehr : scan date/time */
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

/****************************************************************************/


int read_ssmiCSU_TB(char *fname, ssmiCSU *d, int verbose);  /* Read NetCDF FCDR File */


/****************************************************************************/

#define Calloc(n,type) (type *)calloc(n,sizeof(type))



int read_fcdr1(char * inpfile);

int main(int argc, char *argv[])
{
  FILE  *fopen();           /* Open file function                           */
  FILE  *inf;               /* Input file pointer                           */
  FILE  *opf;               /* Output file pointer                          */
  int    nscan;             /* Number of scans in input file                */
  int    i,j,k,n;           /* Loop counters                                */
  char  *pgm;               /* Name of program                              */
  char  *inpfile;           /* Input data file                              */
  char  *outfile;           /* Output data file                             */
  int   file_read_error;  

  int    year,month,day;
  int    hour,min,sec,fsec;
  int verbose=1;
  
  ssmiCSU *d;

  /* allocate space for record */
  d=(ssmiCSU *) malloc(sizeof(ssmiCSU));  

  /* check/load input arguments */

  pgm     = argv[0];
  inpfile = argv[1];
  if (argc == 3)
     outfile = argv[2];
  else {
     if (argc != 2) {
       fprintf(stderr, "Usage: %s <inpfile> <outfile>\n", pgm);
       exit(1);
     }
  }

  /* open and read input TB file */
  printf("Opening input TB file %s\n",inpfile);
  
  /* read input NetCDF Base file */
  printf("reading %s\n",inpfile);
  file_read_error=read_ssmiCSU_TB(inpfile, d, verbose);
  if (file_read_error < 0) {
    fprintf(stderr,"*** error reading %s\n",inpfile);
  }
  int    NUMSCAN;                  /* nhscan : Number of high res scans    */
  int    nlscan;                      /* Number of low res scans        */
  printf("done reading %d %d\n",d->nlscan,d->NUMSCAN);

  /* open output data file */

  if ((opf=fopen(outfile, "w")) == NULL) {
     fprintf(stderr, "%s:  can't open output file %s\n", pgm, outfile);
     exit(1);
  }

  /* Write output FCDR data */
  for (n=0; n<d->nlscan; n++) {
    sscanf(d->ASTART_TIME_lo,"%4d-%2d-%2dT%2d:%2d:%2d.%2d",&year,&month,&day,&hour,&min,&sec,&fsec);
    fprintf(opf,"%4d) %4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%2.2dZ %8.2f%8.2f%8.3f\n",
	    n+1,year,month,day,hour,min,sec,fsec,d->SC_LAT_lo[n],d->SC_LON_lo[n],d->SC_ALT[n]);
  }
  fclose(opf);

} 

/****************************************************************************/

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
