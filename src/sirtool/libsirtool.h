/**************************************************
*  libsir.h - Header file for libsir.c
*  SIR image file format utility library
*  Author: Vaughn C. Clayton
*  (C) August 2000 Brigham Young University
***************************************************/

#ifndef LIBSIR_H
#define LIBSIR_H

#define SHORT short
#define FLOAT float

#include "sir_cpp.h"


extern void print_sir_header(struct sirheader h);
extern int read_sir_header(FILE *imf, struct sirheader *h);
extern int read_sir_data(FILE *imf, float *data, struct sirheader *h);
extern float *LoadSIR(char *filename, struct sirheader *h);
void pixtolatlon(struct sirheader *h, int px, int py, float *lon, float *lat);
void latlon2pix(struct sirheader *h, float lon, float lat, int *px, int *py);
extern int sir_datablock(FILE *imf, float *stval, struct sirheader *sh,
			 int x1, int y1, int x2, int y2);



#define MAX_CFG 1024  /* maximum number of config file lines */

extern "C" {

extern void sir_plot(struct sirheader *, unsigned char *img, 
		     unsigned char *palette,
		     char *cfg_args[], int ncfg_args, char *fname,
		     int s_opt, int e_opt);
  
  
}


#endif
