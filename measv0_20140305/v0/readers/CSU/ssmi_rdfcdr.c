#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "struct_fcdr.h"

#define Calloc(n,type) (type *)calloc(n,sizeof(type))

/* Declare output data structures as global variables */

File_Hdr    fhdr;
Scan_LoRes *lscan;
Scan_HiRes *hscan;

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

  /* procedures called by main program */

  int    read_fcdr();       /* Read NetCDF FCDR File                       */

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

  /* read input NetCDF Base file */

  read_fcdr(inpfile);

  /* open output data file */

  if ((opf=fopen(outfile, "w")) == NULL) {
     fprintf(stderr, "%s:  can't open output file %s\n", pgm, outfile);
     exit(1);
  }

  /* Write output FCDR data file in NetCDF */

  for (n=0; n<fhdr.nlscan; n++) {
    fprintf(opf,"%4d) %4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%2.2dZ %8.2f%8.2f%8.3f\n",
           n+1,lscan[n].scantime[0],lscan[n].scantime[1],lscan[n].scantime[2],
           lscan[n].scantime[3],lscan[n].scantime[4],lscan[n].scantime[5],
           lscan[n].scantime[6],lscan[n].sclat,lscan[n].sclon,lscan[n].scalt);
  }
  fclose(opf);

} 
