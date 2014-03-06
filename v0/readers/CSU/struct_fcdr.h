#define NL           64  /* Number of pixels in low-res scan            */
#define NH          128  /* Number of pixels in high-res scan           */

/************************************************************************/
/* Define data Structures for SSM/I FCDR                                */
/************************************************************************/

/* File header information */

typedef struct fhdr {
  int    scid;                        /* Spacecraft ID                  */
  int    nlscan;                      /* Number of low res scans        */
  int    nhscan;                      /* Number of low res scans        */
  int    orbit_number;                /* Orbit (i.e. granule) number    */
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
} File_Hdr;

/* Low resolution scan data */

typedef struct tlores {
  double xtime;                       /* Time in seconds from 1/1/87    */
  double rev;                         /* orbit number                   */
  int    scantime[7];                 /* Scan date/time                 */
  float  sclat;                       /* Spacecraft latitude            */
  float  sclon;                       /* Spacecraft longitude           */
  float  scalt;                       /* Spacecraft altitude            */
  float  lat[NL];                     /* Latitude (low-res)             */
  float  lon[NL];                     /* Longitude (low-res)            */
  float  eia[NL];                     /* Earth incidence angle (low-res)*/
  float  glint[NL];                   /* Sun glint angle (low-res)      */
  float  tb19v[NL];                   /* 19 GHz V-Pol TBs               */
  float  tb19h[NL];                   /* 19 GHz H-Pol TBs               */
  float  tb22v[NL];                   /* 22 GHz V-Pol TBs               */
  float  tb37v[NL];                   /* 37 GHz V-Pol TBs               */
  float  tb37h[NL];                   /* 37 GHz H-Pol TBs               */
  char   qual[NL];                    /* Quality Flag (low-res)         */
} Scan_LoRes;

/* High resolution scan data */

typedef struct thires {
  double xtime;                       /* Time in seconds from 1/1/87    */
  double rev;                         /* orbit number                   */
  int    scantime[7];                 /* Scan date/time                 */
  float  sclat;                       /* Spacecraft latitude            */
  float  sclon;                       /* Spacecraft longitude           */
  float  scalt;                       /* Spacecraft altitude            */
  float  lat[NH];                     /* Latitude (hi-res)              */
  float  lon[NH];                     /* Longitude (hi-res)             */
  float  eia[NH];                     /* Earth incidence angle (hi-res) */
  float  glint[NH];                   /* Sun glint angle (hi-res)       */
  float  tb85v[NH];                   /* 85 GHz V-Pol TBs (hi-res)      */
  float  tb85h[NH];                   /* 85 GHz V-Pol TBs (hi-res)      */
  char   qual[NH];                    /* Quality Flag (hi-res)          */
} Scan_HiRes;
