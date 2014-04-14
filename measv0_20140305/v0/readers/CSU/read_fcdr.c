#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <string.h>
#include <time.h>
#include "struct_fcdr.h"

#define nscan_lores 10000
#define nscan_hires 20000
#define ERRCODE        2 /* Handle errors by printing an error message */
                         /* and exiting with a non-zero status.        */
#define Calloc(n,type) (type *)calloc(n,sizeof(type))
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}
#define slen 100
#define EIA_NOMINAL 53.1
#define numchar       23

extern File_Hdr    fhdr;
extern Scan_LoRes *lscan;
extern Scan_HiRes *hscan;

int read_fcdr(inpfile)

char *inpfile;

{
  int    verbose=1;
  char  *p;
  int    i,n,na,nb;
  int    ncid;
  int    retval;
  float  cal;
  char   space[100];
  int    year,month,day;
  int    hour,minute,sec,fsec;

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
  int    nlscan;
  int    nhscan;
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

  /* Declare scan variables */

  double scantimelr[nscan_lores];
  int    scandatelr[nscan_lores][7];
  char   datetimelr[nscan_lores][numchar];
  double revlr[nscan_lores];
  float  sclat_lr[nscan_lores];
  float  sclon_lr[nscan_lores];
  float  scalt_lr[nscan_lores];
  float  lat_lr[nscan_lores][NL];
  float  lon_lr[nscan_lores][NL];
  float  eia_lr[nscan_lores][NL];
  signed char sun_lr[nscan_lores][NL];
  signed char qual_lr[nscan_lores][NL];

  double scantimehr[nscan_lores];
  int    scandatehr[nscan_lores][7];
  char   datetimehr[nscan_lores][numchar];
  double revhr[nscan_hires];
  float  sclat_hr[nscan_hires];
  float  sclon_hr[nscan_hires];
  float  scalt_hr[nscan_hires];
  float  lat_hr[nscan_hires][NH];
  float  lon_hr[nscan_hires][NH];
  float  eia_hr[nscan_hires][NH];
  signed char sun_hr[nscan_hires][NH];
  signed char qual_hr[nscan_hires][NH];

  float  tb19v[nscan_lores][NL];
  float  tb19h[nscan_lores][NL];
  float  tb22v[nscan_lores][NL];
  float  tb37v[nscan_lores][NL];
  float  tb37h[nscan_lores][NL];
  float  tb85v[nscan_hires][NH];
  float  tb85h[nscan_hires][NH];

  /* Open NetCDF File and get variable information */

  if (retval = nc_open(inpfile, NC_NOWRITE, &ncid))              ERR(retval);
  if (retval = nc_inq_unlimdim(ncid, &recid))                    ERR(retval);
  if (retval = nc_inq_dimid(ncid, "nscan_lores", &nlscan_dimid)) ERR(retval);
  if (retval = nc_inq_dimid(ncid, "nscan_hires", &nhscan_dimid)) ERR(retval);
  if (retval = nc_inq_dimlen(ncid, nlscan_dimid, &nscan))        ERR(retval);
  nlscan = nscan;
  if (retval = nc_inq_dimlen(ncid, nhscan_dimid, &nscan))        ERR(retval);
  nhscan = nscan;
  if (verbose) {
    printf("nscan_lores  = %d\n",nlscan);
    printf("nscan_hires  = %d\n\n",nhscan);
  }

  /* Read in Global attribute metadata fields */

  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "title",               title))         ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "creator_name",        author))        ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "creator_email",       email))         ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "creator_url",         url))           ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "institution",         institution))   ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "product_version",     version))       ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "revision_date",       revision_date)) ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "platform",            platform))      ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "sensor",              sensor))        ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "orbit_number",        orbit_number))  ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "source",              source))        ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "id",                  filename))      ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "time_coverage_start", startdate))     ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "time_coverage_end",   enddate))       ERR(retval);
  if (retval = nc_get_att_text(ncid, NC_GLOBAL, "date_created",        created))       ERR(retval);

  if (verbose) {
    printf("Title         = %s\n",title);
    printf("Author        = %s\n",author);
    printf("Contact       = %s\n",email);
    printf("URL           = %s\n",url);
    printf("Institution   = %s\n",institution);
    printf("Version       = %s\n",version);
    printf("Revision Date = %s\n",revision_date);
    printf("Satellite     = %s\n",platform);
    printf("Sensor        = %s\n",sensor);
    printf("Orbit Number  = %s\n",orbit_number);
    printf("Source File   = %s\n",source);
    printf("Filename      = %s\n",filename);
    printf("Granule Start = %s\n",startdate);
    printf("Granule End   = %s\n",enddate);
    printf("Creation Date = %s\n\n",created);
  }

  if (nlscan > 0) {

    /* Allocate data scan structure */

    lscan = Calloc(nlscan, Scan_LoRes);
    hscan = Calloc(nhscan, Scan_HiRes);

    /* Read data */

    if (retval = nc_inq_varid(ncid, "orbit_lores",          &revlr_varid))      ERR(retval);
    if (retval = nc_inq_varid(ncid, "scan_time_lores",      &scantimelr_varid)) ERR(retval);
    if (retval = nc_inq_varid(ncid, "scan_datetime_lores",  &scandatelr_varid)) ERR(retval);
    if (retval = nc_inq_varid(ncid, "spacecraft_lat_lores", &sclatlr_varid))    ERR(retval);
    if (retval = nc_inq_varid(ncid, "spacecraft_lon_lores", &sclonlr_varid))    ERR(retval);
    if (retval = nc_inq_varid(ncid, "spacecraft_alt_lores", &scaltlr_varid))    ERR(retval);
    if (retval = nc_inq_varid(ncid, "lat_lores",            &lat_lr_varid))     ERR(retval);
    if (retval = nc_inq_varid(ncid, "lon_lores",            &lon_lr_varid))     ERR(retval);
    if (retval = nc_inq_varid(ncid, "eia_lores",            &eia_lr_varid))     ERR(retval);
    if (retval = nc_inq_varid(ncid, "sun_glint_lores",      &sun_lr_varid))     ERR(retval);
    if (retval = nc_inq_varid(ncid, "quality_lores",        &qual_lr_varid))    ERR(retval);
    if (retval = nc_inq_varid(ncid, "fcdr_tb19v",           &tb19v_varid))      ERR(retval);
    if (retval = nc_inq_varid(ncid, "fcdr_tb19h",           &tb19h_varid))      ERR(retval);
    if (retval = nc_inq_varid(ncid, "fcdr_tb22v",           &tb22v_varid))      ERR(retval);
    if (retval = nc_inq_varid(ncid, "fcdr_tb37v",           &tb37v_varid))      ERR(retval);
    if (retval = nc_inq_varid(ncid, "fcdr_tb37h",           &tb37h_varid))      ERR(retval);
    if (retval = nc_inq_varid(ncid, "orbit_hires",          &revhr_varid))      ERR(retval);
    if (retval = nc_inq_varid(ncid, "scan_time_hires",      &scantimehr_varid)) ERR(retval);
    if (retval = nc_inq_varid(ncid, "scan_datetime_hires",  &scandatehr_varid)) ERR(retval);
    if (retval = nc_inq_varid(ncid, "spacecraft_lat_hires", &sclathr_varid))    ERR(retval);
    if (retval = nc_inq_varid(ncid, "spacecraft_lon_hires", &sclonhr_varid))    ERR(retval);
    if (retval = nc_inq_varid(ncid, "spacecraft_alt_hires", &scalthr_varid))    ERR(retval);
    if (retval = nc_inq_varid(ncid, "lat_hires",            &lat_hr_varid))     ERR(retval);
    if (retval = nc_inq_varid(ncid, "lon_hires",            &lon_hr_varid))     ERR(retval);
    if (retval = nc_inq_varid(ncid, "eia_hires",            &eia_hr_varid))     ERR(retval);
    if (retval = nc_inq_varid(ncid, "sun_glint_hires",      &sun_hr_varid))     ERR(retval);
    if (retval = nc_inq_varid(ncid, "quality_hires",        &qual_hr_varid))    ERR(retval);
    if (retval = nc_inq_varid(ncid, "fcdr_tb85v",           &tb85v_varid))      ERR(retval);
    if (retval = nc_inq_varid(ncid, "fcdr_tb85h",           &tb85h_varid))      ERR(retval);

    if (retval = nc_get_var_double(ncid,scantimelr_varid, &scantimelr[0]))      ERR(retval);
    if (retval = nc_get_var_text(ncid,  scandatelr_varid, &datetimelr[0][0]))   ERR(retval);
    if (retval = nc_get_var_double(ncid,revlr_varid,      &revlr[0]))           ERR(retval);
    if (retval = nc_get_var_float(ncid, sclatlr_varid,    &sclat_lr[0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, sclonlr_varid,    &sclon_lr[0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, scaltlr_varid,    &scalt_lr[0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, lat_lr_varid,     &lat_lr[0][0]))       ERR(retval);
    if (retval = nc_get_var_float(ncid, lon_lr_varid,     &lon_lr[0][0]))       ERR(retval);
    if (retval = nc_get_var_float(ncid, eia_lr_varid,     &eia_lr[0][0]))       ERR(retval);
    if (retval = nc_get_var_schar(ncid, sun_lr_varid,     &sun_lr[0][0]))       ERR(retval);
    if (retval = nc_get_var_schar(ncid, qual_lr_varid,    &qual_lr[0][0]))      ERR(retval);
    if (retval = nc_get_var_float(ncid, tb19v_varid,      &tb19v[0][0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, tb19h_varid,      &tb19h[0][0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, tb22v_varid,      &tb22v[0][0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, tb37v_varid,      &tb37v[0][0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, tb37h_varid,      &tb37h[0][0]))        ERR(retval);
    if (retval = nc_get_var_double(ncid,scantimehr_varid, &scantimehr[0]))      ERR(retval);
    if (retval = nc_get_var_text(ncid,  scandatehr_varid, &datetimehr[0][0]))   ERR(retval);
    if (retval = nc_get_var_double(ncid,revhr_varid,      &revhr[0]))           ERR(retval);
    if (retval = nc_get_var_float(ncid, sclathr_varid,    &sclat_hr[0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, sclonhr_varid,    &sclon_hr[0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, scalthr_varid,    &scalt_hr[0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, lat_hr_varid,     &lat_hr[0][0]))       ERR(retval);
    if (retval = nc_get_var_float(ncid, lon_hr_varid,     &lon_hr[0][0]))       ERR(retval);
    if (retval = nc_get_var_float(ncid, eia_hr_varid,     &eia_hr[0][0]))       ERR(retval);
    if (retval = nc_get_var_schar(ncid, sun_hr_varid,     &sun_hr[0][0]))       ERR(retval);
    if (retval = nc_get_var_schar(ncid, qual_hr_varid,    &qual_hr[0][0]))      ERR(retval);
    if (retval = nc_get_var_float(ncid, tb85v_varid,      &tb85v[0][0]))        ERR(retval);
    if (retval = nc_get_var_float(ncid, tb85h_varid,      &tb85h[0][0]))        ERR(retval);
  }

  /* Close the output data file, freeing all resources. */

  if (retval = nc_close(ncid)) ERR(retval);

  /* Load header variables into output header data structure */

  fhdr.nlscan       = nlscan;
  fhdr.nhscan       = nhscan;
  p=rindex(platform,'F');
  sscanf(p+1,"%2d",&scid);
  sscanf(orbit_number,"%5d",&orbitnumber);
  fhdr.scid         = scid;
  fhdr.orbit_number = orbitnumber;
  if (verbose) {
    printf("Spacecraft ID = %2d\n",scid);
    printf("Orbit Number  = %5d\n",orbitnumber);
  }

  strncpy(fhdr.title,title,100);
  strncpy(fhdr.author,author,100);
  strncpy(fhdr.email,email,100);
  strncpy(fhdr.url,url,100);
  strncpy(fhdr.institution,institution,100);
  strncpy(fhdr.version,version,100);
  strncpy(fhdr.revision_date,revision_date,100);
  strncpy(fhdr.platform,platform,100);
  strncpy(fhdr.sensor,sensor,100);
  strncpy(fhdr.startdate,startdate,100);
  strncpy(fhdr.enddate,enddate,100);
  strncpy(fhdr.created,created,100);
  strncpy(fhdr.inpfile,source,100);
  strncpy(fhdr.outfile,filename,100);

  /* Load scan variables into output scan data structure */

  for (n=0; n<nlscan; n++) {
    sscanf(datetimelr[n],"%4d-%2d-%2dT%2d:%2d:%2d.%2d",&year,&month,&day,&hour,
           &minute,&sec,&fsec);
    lscan[n].scantime[0] = year;
    lscan[n].scantime[1] = month;
    lscan[n].scantime[2] = day;
    lscan[n].scantime[3] = hour;
    lscan[n].scantime[4] = minute;
    lscan[n].scantime[5] = sec;
    lscan[n].scantime[6] = fsec;
    lscan[n].xtime = scantimelr[n];
    lscan[n].rev   = revlr[n];
    lscan[n].sclat = sclat_lr[n];
    lscan[n].sclon = sclon_lr[n];
    lscan[n].scalt = scalt_lr[n];
    for (i=0; i<NL; i++) { /* Low-res scan values */
      lscan[n].lat[i]    = lat_lr[n][i];
      lscan[n].lon[i]    = lon_lr[n][i];
      lscan[n].eia[i]    = eia_lr[n][i];
      lscan[n].glint[i]  = sun_lr[n][i];
      lscan[n].tb19v[i]  = tb19v[n][i];
      lscan[n].tb19h[i]  = tb19h[n][i];
      lscan[n].tb22v[i]  = tb22v[n][i];
      lscan[n].tb37v[i]  = tb37v[n][i];
      lscan[n].tb37h[i]  = tb37h[n][i];
      lscan[n].qual[i]   = qual_lr[n][i];
    }
  }

  for (n=0; n<nhscan; n++) {
    sscanf(datetimehr[n],"%4d-%2d-%2dT%2d:%2d:%2d.%2d",&year,&month,&day,&hour,
           &minute,&sec,&fsec);
    hscan[n].scantime[0] = year;
    hscan[n].scantime[1] = month;
    hscan[n].scantime[2] = day;
    hscan[n].scantime[3] = hour;
    hscan[n].scantime[4] = minute;
    hscan[n].scantime[5] = sec;
    hscan[n].scantime[6] = fsec;
    hscan[n].xtime = scantimehr[n];
    hscan[n].rev   = revhr[n];
    hscan[n].sclat = sclat_hr[n];
    hscan[n].sclon = sclon_hr[n];
    hscan[n].scalt = scalt_hr[n];
    for (i=0; i<NH; i++) {  /* High-res scan values */
      hscan[n].lat[i]    = lat_hr[n][i];
      hscan[n].lon[i]    = lon_hr[n][i];
      hscan[n].eia[i]    = eia_hr[n][i];
      hscan[n].glint[i]  = sun_hr[n][i];
      hscan[n].tb85v[i]  = tb85v[n][i];
      hscan[n].tb85h[i]  = tb85h[n][i];
      hscan[n].qual[i]   = qual_hr[n][i];
    }
  }

  /* Return to main program */

  return 0;
}
