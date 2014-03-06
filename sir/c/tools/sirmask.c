/*
   program to apply a mask to a SIR file

   Written by DGL 14 Feb. 2002

   This simple program reads two BYU sir-format input files.  The
   first file is masked using the second file with the result written
   to anout put sir file

   should be linked with 

   sir_ez.c
   sir_io.c

   (c) copyright 2002 by BYU MERS
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sir_ez.h"  /* get easy sir routine interface */


int main(int argc, char **argv)
{
  int   i, ierr;
  char  *in1 = NULL, *in2 = NULL, *out = NULL, line[255];
  float *stval, *mask;

  sir_head head1, head2;   /* header storage structures */

  fprintf(stdout,"BYU SIR mask file program\n") ;
  if(argc < 2) {
    fprintf(stdout,"\nusage: %s file in1 in2 <out>\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   in1  =  input SIR file\n");
    fprintf(stdout,"   in2  =  land mask SIR file\n");
    fprintf(stdout,"   out  =  optional output file name [default=in1.lmsk]\n");
    return(0);
  }
  in1 = argv[1];
  in2 = argv[2];
  if (argc > 3)
    out = argv[3];
  else {
    sprintf(line,"%s.lmsk",in1);
    out = line;
  }
  
  /* read input SIR file headers into memory */

  sir_init_head(&head1);
  ierr = get_sir(in1, &head1, &stval);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR file '%s'\n",in1);
     exit(-1);
  }
  sir_update_head(&head1, stval);  

  sir_init_head(&head2);
  ierr = get_sir(in2, &head2, &mask);
  if (ierr < 0) {
     fprintf(stdout,"*** Error reading SIR file '%s'\n",in2);
     exit(-1);
  }
  sir_update_head(&head2, mask);

  /* check compatibility of the images, i.e. same size and projection */

  if (!(head1.nsx    == head2.nsx    && head1.nsy    == head2.nsy    && 
	head1.iopt   == head2.iopt   && 
	head1.xdeg   == head2.xdeg   && head1.ydeg   == head2.ydeg   &&
	head1.ascale == head2.ascale && head1.bscale == head2.bscale &&
	head1.a0     == head2.a0     && head1.b0     == head2.b0) ) {
    fprintf(stdout,"*** Incompatible image size and/or projections\n");
    fprintf(stdout,"File:  %s\n",in1);
    print_sir_head(stdout, &head1);
    fprintf(stdout,"Mask: %s\n",in2);
    print_sir_head(stdout, &head2);
    exit(-1);
  }

  /* apply land mask */

  for (i = 0; i < head1.nsx * head1.nsy; i++) {
    if (mask[i] < 0.5)
      stval[i] = head1.anodata;
  }

/* write output difference SIR format file */

  fprintf(stdout,"Masked output %s\n", out);
  ierr = put_sir(out, &head1, stval);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output SIR file '%s'***\n",out);
     fflush(stderr);
  }

  return(0);
}









