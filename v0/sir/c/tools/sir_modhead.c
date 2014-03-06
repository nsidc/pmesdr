/*
   This program modifies the header BYU SIR images.  It DOES not remap the image
   nor change the projection.

   Written by DGL 19 Dec 2001

   This program should be linked with 
    sir_ez.c
    sir_geom.c
    sir_io.c
    sir_ez.h
    sir3.h

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "sir_ez.h"   /* get easy sir routine interface */

#define MAXp 22       /* maximum input parameters */
#define MAXLINE 1024  /* maximum line length of input */

#define DEBUG 0       /* set to 1 to print extra debugging lines, 0 to suppress */


/* function prototypes */

void define_sir_head(sir_head *out_head, FILE *, 
		     int nSarg, char **Sarg_param, char **Sarg_value);

void Sarg_process(sir_head *out_head, int nSarg, char **Sarg_param, char **Sarg_value);

int getline1 (FILE *imf, char *line, int max_length);

void list_header_variables(FILE *imf);

void process_standard_region(sir_head *head, char *std_region_num, char *pix_size);


/* main program */

int main(int argc, char **argv)
{
  FILE  *imf;
  int   i, j, k, n, nsx, nsy, ix, iy, x, y;
  float x1, y1, alon, alat;
  char  *out = NULL, fname[120];
  char  *in, *Sarg_param[MAXp], *Sarg_value[MAXp];
  char  *std_region_num, *pix_size;
  int   ierr, Nfiles = 0, nSarg = 0;
  int   processing_option = 0, print_headers = 0, revise_head_flag = 0;
  float *stval;  /* pointers to image storage */

  sir_head head;


  fprintf(stdout,"BYU SIR header modification program\n");

  /* count input files and check for optional args */

  for (i=1; i < argc; i++) {
    if (*argv[i] == '-') {
      if (*(argv[i]+1) == 'h' || *(argv[i]+1) == 'H')
	list_header_variables(stdout);
      if (*(argv[i]+1) == 'p' || *(argv[i]+1) == 'P')
	print_headers = 1;
      if (*(argv[i]+1) == 'o' || *(argv[i]+1) == 'O')
	if (i+1 < argc)
	  out = argv[++i];
      if (*(argv[i]+1) == 'i' || *(argv[i]+1) == 'I')
	revise_head_flag = 1;
      if (*(argv[i]+1) == 's' || *(argv[i]+1) == 'S')
	if (i+2 < argc) {
	  Sarg_param[nSarg] = argv[++i];
	  Sarg_value[nSarg] = argv[++i];
	  nSarg++;
	}
    } else
      if (Nfiles++ == 0)
	in = argv[i];
  }
  if (DEBUG) fprintf(stdout,"Nfiles=%d  nSarg=%d\n",Nfiles,nSarg);

  if (argc < 2 || Nfiles == 0) {
    fprintf(stdout,"\nusage: %s <options> in_file\n\n",argv[0]);
    fprintf(stdout," input parameters:\n");
    fprintf(stdout,"   <options> (upper or lower case accepted for option letter code)\n");
    fprintf(stdout,"     -h = list file header variables\n");
    fprintf(stdout,"     -i = interactively modify output SIR header\n");
    fprintf(stdout,"     -o out = name of output SIR file [default=out.sir]\n");
    fprintf(stdout,"     -p = print out file headers\n");
    fprintf(stdout,"     -s <name> <value> = set header parameter <name> to <value>\n");
    fprintf(stdout,"   in_file  =  input SIR file\n");
    return(0);
  }

  /* determine output file name if not specified */

  if (out == NULL) {  
    (void) strncpy(fname,"out.sir",120);
    /*
    printf("Enter output file name: ");
    scanf("%s",&fname);
    */
    out = fname;
  }
  if (DEBUG) fprintf(stdout,"Output file: %s\n",out);


  /* read input file */

  fprintf(stdout,"Reading input SIR template file '%s'\n",in);
  ierr = get_sir(in, &head, &stval);
  if (ierr < 0) {
    fprintf(stderr,"*** ERROR: can not read SIR file... %d\n",ierr);
    exit(-1);
  }
  if (print_headers == 1) {      /* print input SIR file header */
    fprintf(stdout,"\nInput SIR file header: %s\n",in);
    print_sir_head(stdout, &head);
  }

  /* define output file header */

  if (nSarg > 0)
    Sarg_process(&head, nSarg, Sarg_param, Sarg_value);

  if (revise_head_flag == 1)
    define_sir_head(&head, stdin, nSarg, Sarg_param, Sarg_value);

  if (print_headers == 1) {     /* print output SIR file header */
    fprintf(stdout,"\nOutput SIR file header: %s\n",out);
    print_sir_head(stdout, &head);
  }

  /* write output sir file */

  fprintf(stdout,"Output SIR file '%s'\n", out);

  ierr = put_sir(out, &head, stval);
  if (ierr < 0) {
     fprintf(stderr,"*** ERROR writing output SIR file ***\n");
     fflush(stderr);
     exit(-1);
  }

  return(0);
}



/* support routines */


void dupdate(char *prompt, int *d, FILE *imf)
{
  char line[MAXLINE];

  fprintf(stdout,prompt, *d);
  (void) getline1(imf, line, MAXLINE);
  if (strlen(line) > 0) sscanf(line,"%d",d);
}

void fupdate(char *prompt, float *f, FILE *imf)
{
  char line[MAXLINE];

  fprintf(stdout,prompt, *f);
  (void) getline1(imf, line, MAXLINE);
  if (strlen(line) > 0) sscanf(line,"%f",f);
}

void supdate(char *prompt, char *s, int l, FILE *imf)
{
  char line[MAXLINE];

  fprintf(stdout,prompt, s);
  (void) getline1(imf, line, MAXLINE);
  if (strlen(line) > 0) strncpy(s,line,l);
}


void get_projection_name(char *ptype, int iopt, int n);
void set_header_projection_scale_factors(sir_head *head);


#define PSIZE 22

void define_sir_head(sir_head *head, FILE *imf, int nSarg, char **param, char **value)
{
  int last_iopt=-100;
  
  char line[MAXLINE], ptype[PSIZE+1];
  
  fprintf(stdout,"\nThis program does NOT change projections. \n");
  fprintf(stdout,"CAUTION: error checking is very limited.   It is very easy\n");
  fprintf(stdout,"to create headers with errors which cause problems with the file\n");
  fprintf(stdout,"storage accuracy and/or file processing programs.\n\n");

  head->nhtype = 30;
  /*  head->nhead = 1; */
  /*  head->nia = 0; */      /* no extra header shorts */
  /*  head->ldes = 0; */     /* no extra header descriptions */

  if (nSarg > 0)
    Sarg_process(head, nSarg, param, value);


  /* interactively modify SIR header parameters */
    
 LOOP:

  fprintf(stdout,"The following SIR header parameters can not be reset in this program:\n");
  
  fprintf(stdout,"  NSX x NSY Size: %d x %d\n",head->nsx,head->nsy);
  switch(head->iopt) {
   case -1:
     fprintf(stdout,"  Rectangular image-only projection: \n");
     fprintf(stdout,"   Xspan,  Yspan:  %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"   Xscale, Yscale: %f , %f\n",head->ascale,head->bscale);
     fprintf(stdout,"   Xorg,   Yorg:   %f , %f\n",head->a0,head->b0);
     break;

   case 0:
     fprintf(stdout,"  Rectangular Lat/Long projection: \n");
     fprintf(stdout,"   Size (deg):     %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"   Lon, Lat scale: %f , %f (pix/deg)\n",head->ascale,head->bscale);
     fprintf(stdout,"   Offsets:        %f , %f\n",head->a0,head->b0);
     break;

   case 2:
     fprintf(stdout,"  Lambert form: (local radius)\n");
   case 1:
     if (head->iopt==1) fprintf(stdout,"  Lambert projection: (fixed radius)\n");
     fprintf(stdout,"   Center point:      %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"   Lon, Lat scale:    %f , %f (km/pix)\n",1./head->ascale,1./head->bscale);
     fprintf(stdout,"   Lower-Left Corner: %f , %f\n",head->a0,head->b0);
     break;

   case 5:
     fprintf(stdout,"  Polar sterographic form: \n");
     fprintf(stdout,"   Center Lon,Lat:    %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"   X,Y scales:        %f , %f (km/pix)\n",head->ascale,head->bscale);
     fprintf(stdout,"   Lower-Left Corner: %f , %f\n",head->a0,head->b0);
     break;

   case 11:
   case 12:
     fprintf(stdout,"  EASE polar azimuthal form: \n");
     fprintf(stdout,"   Map center (col,row): %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"   A,B scales:           %f , %f\n",head->ascale,head->bscale);
     fprintf(stdout,"   Map origin (col,row): %f , %f\n",head->a0,head->b0);
     break;

   case 13:
     fprintf(stdout,"  EASE cylindrical form: \n");
     fprintf(stdout,"   Map center (col,row): %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"   A,B scales:           %f , %f\n",head->ascale,head->bscale);
     fprintf(stdout,"   Map origin (col,row): %f , %f\n",head->a0,head->b0);
     break;

   default:
     fprintf(stdout,"  Unrecognized SIR file option: \n");
     fprintf(stdout,"   Xspan,  Yspan:  %f , %f\n",head->xdeg,head->ydeg);
     fprintf(stdout,"   Xscale, Yscale: %f , %f\n",head->ascale,head->bscale);
     fprintf(stdout,"   Xorg,   Yorg:   %f , %f\n",head->a0,head->b0);
     break;
  }
  fprintf(stdout,"  Transform scale factors: \n");
  fprintf(stdout,"   IXdeg_off, IYdeg_off, Ideg_sc: %d %d\n",head->ixdeg_off, head->iydeg_off);
  fprintf(stdout,"   Iscale_sc, Ideg_sc:            %d %d\n",head->iscale_sc,head->ideg_sc);
  fprintf(stdout,"   Ia0_off, Ib0_off, I0_sc:       %d %d %d\n",head->ia0_off,head->ib0_off,head-> i0_sc);

  /*
  dupdate("Enter NSX X dimension image size in pixels: [def=%d] ",&head->nsx,imf);
  dupdate("Enter NSY Y dimension image size in pixels: [def=%d] ",&head->nsy,imf);

  get_projection_name(ptype,head->iopt,PSIZE);
  
  fprintf(stdout,"Enter IOPT projection type (-1 = image only [remapping not possible],\n");
  fprintf(stdout,"                             0 = rect lat/lon,\n");
  fprintf(stdout,"                             1 = Lambert fixed radius,\n");
  fprintf(stdout,"                             2 = Lambert local radius,\n");
  fprintf(stdout,"                             5 = Polar sterographic,\n");
  fprintf(stdout,"                            11 = EASE North Hem,\n");
  fprintf(stdout,"                            12 = EASE South Hem,\n");
  fprintf(stdout,"                            13 = EASE Global\n");
  fprintf(stdout,"    IOPT: [Def=%d (%s)] ",head->iopt,ptype);
  (void) getline1(stdin, line, MAXLINE);  
  if (strlen(line) > 0) sscanf(line,"%d",&head->iopt);
  switch(head->idatatype){
  case -1:
  case 0:
  case 1:
  case 2:
  case 5:
  case 11:
  case 12:
  case 13:
    break;
  default:
    head->iopt = 0;
    fprintf(stdout,"*** Invalid input.  IOPT set to %d\n",head->iopt);
    goto LOOP;
  }


  fupdate("Enter XDEG (x axis length) projection parameter: [def=%f]  ",&head->xdeg,imf);
  fupdate("Enter YDEG (y axis length) projection parameter: [def=%f]  ",&head->ydeg,imf);
  fupdate("Enter ASCALE (x axis scale) projection parameter: [def=%f] ",&head->ascale,imf);
  fupdate("Enter BSCALE (y axis scale) projection parameter: [def=%f] ",&head->bscale,imf);
  fupdate("Enter A0 (x axis origin) projection parameter: [def=%f]    ",&head->a0,imf);
  fupdate("Enter B0 (y axis origin) projection parameter: [def=%f]    ",&head->b0,imf);

  if (head->iopt != last_iopt) {
    last_iopt = head->iopt;
    set_header_projection_scale_factors(head);
    fprintf(stdout,"Projection parameter scaling reset\n");
    
  }
  */

  /* Data storage scaling parameters */
  fprintf(stdout,"\nThe following SIR header parameters can be modified in this program:\n");

  dupdate("Enter data form (1=one byte, 2=two byte integer, 4=float)/pixel: [def=%d] ",&head->idatatype,imf);
  switch(head->idatatype){
  case 1:
  case 2:
  case 4:
    break;
  default:
    head->idatatype = 2;
    fprintf(stdout,"*** Input overridden.  IDATATYPE set to %d\n",head->idatatype);
  }
  dupdate("Enter IOFF data storage offset factor: [def=%d]  ",&head->ioff,imf);
  dupdate("Enter ISCALE data storage scale factor: [def=%d] ",&head->iscale,imf);
  fupdate("Enter ANODATA no-data value: [def=%f] ",&head->anodata,imf);
  fupdate("Enter V_MIN default minimum viewable data value: [def=%f] ",&head->v_min,imf);
  fupdate("Enter V_MAX default maximum viewable data value: [def=%f] ",&head->v_max,imf);

  /* Various descriptive header values */

  dupdate("Enter IYEAR data start year (4 digits): [def=%d] ",&head->iyear,imf);
  dupdate("Enter ISDAY data start day-of-year: [def=%d] ",&head->isday,imf);
  dupdate("Enter ISMIN data start min-of-day: [def=%d]  ",&head->ismin,imf);
  dupdate("Enter IEDAY data end day-of-year: [def=%d] ",&head->ieday,imf);
  dupdate("Enter IEMIN data end min-of-day: [def=%d]  ",&head->iemin,imf);
  dupdate("Enter IREGION region number code: [def=%d] ",&head->iregion,imf);
  dupdate("Enter ITYPE image type code: [def=%d] ",&head->itype,imf);
  dupdate("Enter IPOL polarization code (0=n/a, 1=h, 2=v, 3=both): [def=%d] ",&head->ipol,imf);
  dupdate("Enter IFREQHM sensor frequency in hundreds of MHz (e.g. 13GHz->130): [def=%d] ",&head->ifreqhm,imf);

  /* Various header strings */
  
  supdate("Enter TITLE image title: [def='%s'] ",head->title,100,imf);
  supdate("Enter SENSOR name/description: [def='%s'] ",head->sensor,40,imf);
  supdate("Enter TYPE: type description [def='%s'] ",head->type,138,imf);
  supdate("Enter TAG description: [def='%s'] ",head->tag,100,imf);
  supdate("Enter CRPROC creation process: [def='%s'] ",head->crproc,100,imf);
  supdate("Enter CRTIME creation date: [def='%s'] ",head->crtime,29,imf);

  /* review results */

  fprintf(stdout,"\nOutput header variable preview:\n");
  print_sir_head(stdout, head);    

  fprintf(stdout,"Accept as is or Revise (A/R) [default=A]:");
  (void) getline1(stdin, line, MAXLINE);
  if (*line == 'R' || *line == 'r') goto LOOP;

}


int getline1(FILE *imf, char *line, int max_length)
{  /* read a newline-terminated character string from a file stream */
  char *s, ch;
  int n=0;
  
  s=line;
  while ((ch = getc(imf)) != '\n' && ch != -1 && n++ < max_length)
    *(s++) = ch;

  *s = '\0';    /* ensure string is null terminated */
  if (ch == -1) 
    return(-1);
  else
    return(n);
}


void get_projection_name(char *ptype, int iopt, int n)  /* returns short text description */
{                                                       /* corresponding to projection code */
  switch(iopt) {
  case -1:   /* Image Only */
    strncpy(ptype,"Image Only",n);
    break;

  case 0:   /* Rectangular Lat/Lon */
    strncpy(ptype,"Rectangular Lat/Lon",n);
    break;

  case 1:   /* Lambert Fixed Radius */
    strncpy(ptype,"Lambert Fixed Radius",n);
    break;
    
  case 2:   /* Lambert Local Radius */
    strncpy(ptype,"Lambert Local Radius",n);
    break;

  case 5:   /* Polar Stereographic */
    strncpy(ptype,"Polar Stereographic",n);
    break;

  case 11:   /* EASE North Hem */
    strncpy(ptype,"EASE North Hem",n);
    break;

  case 12:   /* EASE South Hem */
    strncpy(ptype,"EASE South Hem",n);
    break;

  case 13:   /* EASE Global */
    strncpy(ptype,"EASE Global",n);
    break;

  default:   /* unknown & therefore invalid */
    strncpy(ptype,"Invalid",n);
    break;
  }
}



char *strupr(char *s)
{ char *out = s;
 
  for (;*s != '\0' ; s++)    
    if (*s > 96 && *s < 123)    
      *s = *s - 32;
  return(out);
  
}

void Sarg_process(sir_head *head, int nSarg, char **param, char **value)
{
  int i;
  char *s, line[MAXLINE];

  if (nSarg <= 0) return;

  for (i=0; i < nSarg; i++) {
    if (DEBUG) fprintf(stdout, "set header parameter %d <%s> = <%s>\n",i+1,param[i],value[i]);

    s = strupr(param[i]);
    /*  
    if (strncmp(s,"NSX",3) == 0)
      sscanf(value[i],"%d",&head->nsx);
    if (strncmp(s,"NSY",3) == 0)
      sscanf(value[i],"%d",&head->nsy);

    if (strncmp(s,"IOPT",3) == 0)
      sscanf(value[i],"%d",&head->iopt);
    switch(head->idatatype){
    case -1:
    case 0:
    case 1:
    case 2:
    case 5:
    case 11:
    case 12:
    case 13:
      break;
    default:
      head->iopt = 0;
      fprintf(stderr,"*** Invalid value.  IOPT reset to %d\n",head->iopt);
    }

    if (strncmp(s,"A0",3) == 0)
      sscanf(value[i],"%f",&head->a0);
    if (strncmp(s,"B0",3) == 0)
      sscanf(value[i],"%f",&head->b0);
    if (strncmp(s,"ASCALE",3) == 0)
      sscanf(value[i],"%f",&head->ascale);
    if (strncmp(s,"BSCALE",3) == 0)
      sscanf(value[i],"%f",&head->bscale);
    if (strncmp(s,"XDEG",3) == 0)
      sscanf(value[i],"%f",&head->xdeg);
    if (strncmp(s,"YDEG",3) == 0)
      sscanf(value[i],"%f",&head->ydeg);
    if (strncmp(s,"IXDEG_OFF",3) == 0)
      sscanf(value[i],"%d",&head->ixdeg_off);
    if (strncmp(s,"IYDEG_OFF",3) == 0)
      sscanf(value[i],"%d",&head->iydeg_off);
    if (strncmp(s,"IDEG_SC",3) == 0)
      sscanf(value[i],"%d",&head->ideg_sc);
    if (strncmp(s,"ISCALE_SC",3) == 0)
      sscanf(value[i],"%d",&head->iscale_sc);
    if (strncmp(s,"IA0_OFF",3) == 0)
      sscanf(value[i],"%d",&head->ia0_off);
    if (strncmp(s,"IB0_OFF",3) == 0)
      sscanf(value[i],"%d",&head->ib0_off);
    if (strncmp(s,"I0_SC",3) == 0)
      sscanf(value[i],"%d",&head->i0_sc);
*/
    if (strncmp(s,"IDATATYPE",3) == 0)
      sscanf(value[i],"%d",&head->idatatype);
    switch(head->idatatype){
    case 1:
    case 2:
    case 4:
      break;
    default:
      head->idatatype = 2;
      fprintf(stdout,"*** Input overridden.  IDATATYPE set to %d\n",head->idatatype);
    }

    if (strncmp(s,"IOFF",3) == 0)
      sscanf(value[i],"%d",&head->ioff);
    if (strncmp(s,"ISCALE",3) == 0)
      sscanf(value[i],"%d",&head->iscale);
    if (strncmp(s,"ANODATA",3) == 0)
      sscanf(value[i],"%f",&head->anodata);
    if (strncmp(s,"V_MIN",3) == 0)
      sscanf(value[i],"%f",&head->v_min);
    if (strncmp(s,"V_MAX",3) == 0)
      sscanf(value[i],"%f",&head->v_max);
    if (strncmp(s,"IYEAR",3) == 0)
      sscanf(value[i],"%d",&head->iyear);
    if (strncmp(s,"ISDAY",3) == 0)
      sscanf(value[i],"%d",&head->isday);
    if (strncmp(s,"IEDAY",3) == 0)
      sscanf(value[i],"%d",&head->ieday);
    if (strncmp(s,"ISMIN",3) == 0)
      sscanf(value[i],"%d",&head->ismin);
    if (strncmp(s,"IEMIN",3) == 0)
      sscanf(value[i],"%d",&head->iemin);
    if (strncmp(s,"IREGION",3) == 0)
      sscanf(value[i],"%d",&head->iregion);
    if (strncmp(s,"ITYPE",3) == 0)
      sscanf(value[i],"%d",&head->itype);
    if (strncmp(s,"IPOL",3) == 0)
      sscanf(value[i],"%d",&head->ipol);
    if (strncmp(s,"IFREQHM",3) == 0)
      sscanf(value[i],"%d",&head->ifreqhm);
    if (strncmp(s,"TITLE",3) == 0)
      strncpy(head->title,value[i],100);
    if (strncmp(s,"SENSOR",3) == 0)
      strncpy(head->sensor,value[i],40);
    if (strncmp(s,"TYPE",3) == 0)
      strncpy(head->type,value[i],138);
    if (strncmp(s,"TAG",3) == 0)
      strncpy(head->tag,value[i],100);
    if (strncmp(s,"CRPROC",3) == 0)
      strncpy(head->crproc,value[i],100);
    if (strncmp(s,"CRTIME",3) == 0)
      strncpy(head->crtime,value[i],29);
  }
  
}


void list_header_variables(FILE *imf)
{
  /*
  fprintf(stdout,"\nSetable SIR file header variable names:\n");
  fprintf(stdout," NSX NSY IOPT A0 B0 ASCALE BSCALE XDEG YDEG\n");
  fprintf(stdout," IXDEG_OFF IYDEG_OFF IDEG_SC ISCALE_SC IA0_OFF IB0_OFF I0_SC\n");
  */
  fprintf(stdout," IDATATYPE IOFF ISCALE ANODATA V_MIN V_MAX\n");
  fprintf(stdout," IYEAR ISDAY ISMIN IEDAY IEMIN IREGION ITYPE IPOL IFREQHM\n");
  fprintf(stdout," TITLE SENSOR TYPE TAG CRPROC CRTIME\n");
}

