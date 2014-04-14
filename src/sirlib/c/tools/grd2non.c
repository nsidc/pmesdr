/* (c) 2002, 2014 BYU MERS Laboratory

   this program read .grd file (made with the low resolution "drop in the 
   bucket" image formation algorithm) in BYU SIR image file format and 
   generates the corresponding .non file (a non-enhanced image which matches
   the pixel size and projection of .ave and .sir files) in BYU SIR image
   file format.

   Written by DGL 18 Feb. 2002
   Revised by DGL  7 Mar. 2014 + include EASE2

   Written in ANSI C.

   should be linked with 

   sir_ez.c
   sir_io.c

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sir_ez.h"  /* get easy sir routine interface */


int main(int argc, char **argv)
{
  int   ierr, i, j, nsx2, nsy2, ix, iy, ix1, iy1;
  char  outfname[180], temp[180], non_size=5;

  float *stval;        /* pointer to input image storage */
  float *stval2;       /* pointer to output image storage */

  sir_head head;       /* sir_ez defined structure to store SIR header info */
	
  fprintf(stdout,"BYU .grd to .non program\n") ;
  if(argc < 1) {
    fprintf(stdout,"\nusage: %s file <out>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   file    =  input .grd file\n");
    fprintf(stdout,"   out     =  (optional) output .non file.  Default=file.non\n");
    return(0);
  }

  /* initialize sir_ez structure and read in input SIR file */

  sir_init_head(&head);
  ierr = get_sir(argv[1], &head, &stval);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR header from file '%s'\n",argv[1]);
     exit(-1);
  }
  sir_update_head(&head, stval);

  /*
  fprintf(stdout,"\nSIR file header: '%s'\n",argv[1]);
  print_sir_head(stdout, &head);
  */

  if (head.iopt==8 || head.iopt==9 || head.opt==10)
    non_size=4;  /* special case for EASE2 */  

  /* allocate output image array */

  nsx2 = head.nsx * non_size;
  nsy2 = head.nsy * non_size;

  stval2 = (float *) malloc(sizeof(float) * nsx2 * nsy2);
  if (stval2 == NULL) {
     fprintf(stderr,"*** ERROR: image memory allocation failure...\n");
     exit(-1);
  }

  /* copy data into new array (duplicate pixel values) */

  for (iy = 1; iy <= head.nsy; iy++)
    for (ix = 1; ix <= head.nsx; ix++) {

       i = (iy-1) * head.nsx + ix - 1;  /* word number (1..nsx*nsy) of 
					   pixel (ix,iy) */
       for (iy1 = 0; iy1 < 5; iy1++)
	 for (ix1 = 0; ix1 < 5; ix1++) {
	   j = (non_size*(iy-1)+iy1) * nsx2 + non_size*(ix-1)+ix1;
	   *(stval2 + j) = *(stval + i);

	 }
     }
  
  /* generate new header values, reusing original header */
  
  switch (head.iopt) {

  case 1:   /* Lambert */
  case 2:
    head.ascale = 5 * head.ascale;
    head.bscale = 5 * head.bscale;
    break;

  case 8:   /* EASE2 grid */
  case 9:   /* note: EASE2 grids use grd/non size ratios of 4 rather than 5 */
  case 10:
    head.ascale = head.ascale + 4;
    break;

  case 11:  /* EASE1 grid */
  case 12:
  case 13:
    head.ascale = head.ascale * 5;
    head.bscale = head.bscale / 5;
    break;
    
  case 5:   /* polar stereographic */
  case -1:  /* image only */
  case 0:   /* lat/lon */
  default:  /* unknown */
    head.ascale = head.ascale / 5;
    head.bscale = head.bscale / 5;
    break;
  }

  head.nsx = nsx2;
  head.nsy = nsy2;

  sprintf(temp,"NON: %s",head.title);
  strncpy(head.title,temp,80);

  sprintf(temp,"BYU grd2non - %s",head.crproc);
  strncpy(head.crproc,temp,100);

/* write a sir format file */

  if (argc > 2)
    strncpy(outfname,argv[2],180);
  else
    sprintf(outfname,"%s.non",argv[1]);

  fprintf(stdout,"\nWriting output SIR file '%s'\n", outfname);
  ierr = put_sir(outfname, &head, stval2);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output .non file ***\n");
     fflush(stderr);
  }

  return(0);
}


