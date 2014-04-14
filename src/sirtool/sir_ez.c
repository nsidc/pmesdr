/* easy BYU SIR file format C interface file

This file defines an easy programing C interface for the BYU SIR
file format.  See sir_ez.h for various definitions.

written  4 Nov 2000 by DGL at BYU
revised 30 Apr 2001 by DGL at BYU + added isir_lex, fixed sir_lex
revised 14 Feb 2002 by DGL at BYU + added sir_update_sir_head

see sir_ez.h for function documentation

(c) 2000,2001,2002 by BYU MERS 

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sir3.h"
#include "sir_ez.h"

void sir_init_head(sir_head *head)
{
  head->descrip_flag=0;
  head->iaopt_flag=0;
}


int get_sir_head_file(FILE *imf, sir_head *head)
{
  int ierr;

  /* check to see how optional storage parameters need to be set */

  if (head->descrip_flag != 1) {
    head->descrip = head->descrip_string;
    head->maxdes = MAXDES;
  }
  if (head->iaopt_flag != 1) {
    head->iaopt = head->iaopt_array;
    head->maxi = MAXI;
  }
  
  ierr = read_sir_header3(imf, &head->nhead, &head->ndes, &head->nhtype,
			  &head->idatatype, &head->nsx, &head->nsy, 
			  &head->xdeg, &head->ydeg, 
			  &head->ascale, &head->bscale, 
			  &head->a0, &head->b0, 
			  &head->ixdeg_off, &head->iydeg_off, &head->ideg_sc, 
			  &head->iscale_sc,
			  &head->ia0_off, &head->ib0_off, &head->i0_sc,
			  &head->ioff, &head->iscale, 
			  &head->iyear, &head->isday, &head->ismin, 
			  &head->ieday, &head->iemin, 
			  &head->iregion, &head->itype, &head->iopt, 
			  &head->ipol, 
			  &head->ifreqhm, &head->ispare1,
			  &head->anodata, &head->v_min, &head->v_max,
			  head->sensor, head->title, head->type, head->tag, 
			  head->crproc, head->crtime, 
			  head->maxdes, head->descrip, &head->ldes,
			  head->maxi, head->iaopt, &head->nia);  

  if (ierr < 0) {
     fprintf(stderr,"*** ERROR: get_sir_head_file reading SIR header\n");
     return(-2);
  }
  return(0);
}

int get_sir_head_name(char *fname, sir_head *head, FILE **imf)
{

  *imf = fopen(fname,"rb"); 
  if (*imf == NULL) {
     fprintf(stderr,"*** ERROR: get_sir_head_hame cannot open input file: %s\n",fname); 
     return(-1);
  }

  return(get_sir_head_file(*imf, head));
}


float *sir_data_alloc(sir_head *head)
{
  return( (float *) malloc(sizeof(float) * head->nsx * head->nsy) );
}


int get_sir_data(FILE *imf, float *stval, sir_head *head)
{
  return(read_sir_data(imf, head->nhead, head->idatatype, head->nsx, head->nsy,
		       head->ioff, head->iscale, stval));
}

int get_sir_data_block(FILE *imf, float *stval, sir_head *head,
		       int x1, int y1, int x2, int y2)
{
  return(read_sir_data_block(imf, head->nhead, head->idatatype, head->nsx, head->nsy,
			     head->ioff, head->iscale, x1, y1, x2, y2,
			     stval));
}


int get_sir_data_byte(FILE *imf, char *stval, sir_head *head, float smin,
		      float smax, int bmin, int bmax)
{
  return(read_sir_data_byte(imf, head->nhead, head->idatatype, 
			    head->nsx, head->nsy,
			    head->ioff, head->iscale, 
			    smin, smax, bmin, bmax,
			    stval));
}

int get_sir(char *fname, sir_head *head, float **stval)
{
  FILE *imf;
  int ierr;
  
  ierr = get_sir_head_name(fname, head, &imf);
  if (ierr < 0) return(ierr);

  *stval=sir_data_alloc(head);
  if (*stval == NULL) return(-4);
  ierr = get_sir_data(imf, *stval, head);
  return(ierr);
}


void print_sir_head(FILE *omf, sir_head *head)
{
  print_head(omf, head->nhead, head->ndes, head->nhtype, head->idatatype, 
	     head->nsx, head->nsy, 
	     head->xdeg, head->ydeg, head->ascale, head->bscale, head->a0, head->b0, 
	     head->ioff, head->iscale, head->iyear, 
	     head->isday, head->ismin, head->ieday, head->iemin, 
	     head->iregion, head->itype, head->iopt, head->ipol, 
	     head->ifreqhm, head->ispare1,
	     head->anodata, head->v_min, head->v_max,
	     head->sensor, head->title, head->type, head->tag, 
	     head->crproc, head->crtime, 
	     head->descrip, head->ldes, head->iaopt, head->nia);  
}


int put_sir(char *fname, sir_head *head, float *stval)
{
  int ierr;
  
  ierr = write_sir3(fname, stval, &head->nhead, head->nhtype,
		    head->idatatype, head->nsx, head->nsy, 
		    head->xdeg, head->ydeg, head->ascale, head->bscale, 
		    head->a0, head->b0, 
		    head->ixdeg_off, head->iydeg_off, head->ideg_sc, 
		    head->iscale_sc,
		    head->ia0_off, head->ib0_off, head->i0_sc,
		    head->ioff, head->iscale, 
		    head->iyear, head->isday, head->ismin, head->ieday,
		    head->iemin, 
		    head->iregion, head->itype, head->iopt, head->ipol, 
		    head->ifreqhm, head->ispare1,
		    head->anodata, head->v_min, head->v_max,
		    head->sensor, head->title, head->type, head->tag, 
		    head->crproc, head->crtime, 
		    head->descrip, head->ldes,
		    head->iaopt, head->nia);  
  return(ierr);
}

void sir_pix2latlon(float x, float y, float *alon, float *alat, sir_head *head)
{
  pixtolatlon(x, y, alon, alat, head->iopt, head->xdeg, head->ydeg, head->ascale,
	     head->bscale, head->a0, head->b0);
}

int sir_latlon2pix(float alon, float alat, float *x, float *y, sir_head *head)
{
  int ix,iy;
  
  latlon2pix(alon, alat, x, y, head->iopt, head->xdeg, head->ydeg, head->ascale,
	     head->bscale, head->a0, head->b0);  
  return(sir_pix(*x, *y, &ix, &iy, head));
}


int sir_pix(float x, float y, int *ix, int *iy, sir_head *head)
{
  f2ipix(x, y, ix, iy, head->nsx, head->nsy);
  if (*ix == 0 || *iy == 0)
    return(-1);
  else
    return((*iy-1) * head->nsx + (*ix-1));
}

int sir_lex(int ix, int iy, sir_head *head)
{
  if (ix < 1 || ix > head->nsx || iy < 1 || iy > head->nsy)
    return(-1);
  else
    return((iy-1) * head->nsx + (ix-1));
}

int isir_lex(int *ix, int *iy, int i, sir_head *head)
{
  if (i < 0 || i >= head->nsx * head->nsy) {
    *ix = 0;
    *iy = 0;
    return(-1);
  } else {
    *ix = 1 + (i % head->nsx);
    *iy = 1 + (i / head->nsx);
    return(0);
  }
}

void sir_update_head(sir_head *head, float *stval)
{
  update_sir_header(stval, head->nhtype, head->nsx, head->nsy, &head->anodata,
		    &head->v_min, &head->v_max);
}
