/*
	Wrappers for the functions contained in the sir_geom and sir_io libraries.

	written 18 May 2002 by DGL at BYU
 
	(c) 2002 by BYU MERS 


*/

//#define SWAP 1                /* swap bytes (little endian: dec, pc) */
//#undef SWAP                   /* no byte swapping (big endian: HP,SUN) */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libsirtool.h"


void print_sir_header(struct sirheader h)
{
  fprintf(stdout,"SIR file header: \n");
  print_sir_head(stdout, &h);
}



int read_sir_header(FILE *imf, struct sirheader *h)
{
  return get_sir_head_file(imf, h);
}



/*
	The user is responsible for allocating storage space before calling this
	function! The size required is header.nsx*header.nsy*sizeof(float).
*/
int read_sir_data(FILE *imf, float *data, struct sirheader *h)
{
  return get_sir_data(imf, data, h);
}



float *LoadSIR(char *filename, struct sirheader *h) 
{
  FILE *file;
  long size;                 /* size of the image in bytes. */
  float *data;
  
  /* make sure the file is there. */
  if ((file = fopen(filename, "rb"))==NULL) {
    fprintf(stderr, "File Not Found : %s\n",filename);
    return NULL;
  }

  sir_init_head(h);
  read_sir_header(file, h);

  size = (long)h->nsx * (long)h->nsy * (long)sizeof(float);
  data = (float*)malloc(size);
  if(data==NULL) {
    fprintf(stderr, "Error allocating SIR data memory\n");
    return NULL;
  }
  read_sir_data(file, data, h);
  
  return data;
}


void pixtolatlon(struct sirheader *sh, int px, int py, float *lon, float *lat)
{
  sir_pix2latlon(px, py, lon, lat, sh);
}


void latlon2pix(struct sirheader *sh, float lon, float lat, int *px, int *py)
{
  float fx, fy;
  sir_latlon2pix(lon, lat, &fx, &fy, sh);
  sir_pix(fx, fy, px, py, sh);
}


int sir_datablock(FILE *imf, float *stval, struct sirheader *sh,
		  int x1, int y1, int x2, int y2)
{
  return(get_sir_data_block(imf, stval, sh, x1, y1, x2, y2));
}






