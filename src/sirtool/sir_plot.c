/*
   routines to enable annontaion of a SIR image within sirtool

   Written by DGL June 3, 2002

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "sir_ez.h"


/* global pixel plotting variables */

float x1_opt,y1_opt,x2_opt,y2_opt,d_opt,a_opt,t_opt;
int s_opt,e_opt,i_opt,c_opt,w_opt,latlon_opt,Ii_opt,D_opt,n_opt,M_opt;
char m_opt[200]=" ";
char *f_opt;

int debug = 0;

int nsx, nsy;
sir_head *head;

extern struct { /* pixel plotting stuff */
  unsigned char  *a;
  int bytesperpixel;
  int color;
  int tab;
  unsigned char *pal;  
} fpimage_1;

void vinit_(int *, int*);
void process_option(char *);
void plotvt_(float *, float *, int *);
void newvpen_(int *, int *);
void fpimage_color(int);
float syms_(float *, float *, float *, char *, float *, int *, int *);

void sir_plot(sir_head *sh, unsigned char *img, unsigned char *palette,
	      char *cfg_args[], int ncfg_args, char *fname, int s_opt_in, int e_opt_in)
{
  int i,j;
  int one=1, four=4, c_999=9999;
  float x1, y1;
  float zero=0.0;

  /* initialize */

  x1_opt=1.0;
  y1_opt=1.0;
  x2_opt=2.0;
  y2_opt=2.0;
  d_opt=1.0;
  a_opt=0.0;
  t_opt=10.0;
  i_opt=255;
  c_opt=255;
  w_opt=1;
  latlon_opt=0;
  Ii_opt=0;
  D_opt=5;
  n_opt=0;
  M_opt=0;
  debug = 0;
  s_opt = s_opt_in;
  e_opt = e_opt_in;

  fpimage_1.a = img;
  fpimage_1.bytesperpixel = 3;
  fpimage_1.color = c_opt;
  fpimage_1.pal = palette;
  f_opt = fname;

  head = sh;
  nsx = sh->nsx;
  nsy = sh->nsy;

  /* initialize pixel plotting into byte array*/

  vinit_(&nsx,&nsy);

  x1=nsx;
  y1=nsy;
  plotvt_(&x1,&y1,&four);  /* set viewport */
  i=0;
  newvpen_(&i,&w_opt); /* set default line width */
  x1=c_opt;
  plotvt_(&x1,&zero,&i);   /* set default color */
  j=4;
  (void) syms_(&x1_opt,&y1_opt,&t_opt,"\\@\\[",&a_opt,&j,&one);

  fpimage_color(c_opt); /* override plotting array color*/
  
  /* process input options to (possibly) plot onto output image 
     options previously processed will be ignored */  

  x1_opt=1.0; y1_opt=1.0; x2_opt=1.0; y2_opt=1.0;  /* set default locations */

  /* printf("sir_plot %d\n",ncfg_args); */
  for (i=0; i < ncfg_args; i++) {
    /* printf("sir_plot %d: '%s'  %d\n",i,cfg_args[i],ncfg_args); */
    j=0;
    if (*(cfg_args[i]) == '-') j=1;  /* remove leading dash if necessary */
    if (!(*(cfg_args[i]+j) == '%' || *(cfg_args[i]+j) == '#'))
      process_option((cfg_args[i]+j));
  }
  plotvt_(&zero,&zero,&c_999);  /* turn off pixel plotting into array */
}


void line_plot_from_file(char *filename, int q, int Ii_opt, int latlon_opt, int c_opt, int w_opt);
void fppix_(int *, int *, int *, int *);
void plot_line(int Ii_opt, int latlon_opt, float *x1_opt, float *y1_opt, float *x2_opt, float *y2_opt);
void latlon_trans(float *, float *);
void circle_(float *, float *, float *, float *, float *, float *, float *);


void latlon_trans(float *x, float *y)
{
  float lon=*x,lat=*y;
  sir_latlon2pix(lon, lat, x, y, head);
}


void process_option(char line[])
{  /* process command line: flag=0 for first pass, flag=1 for second pass */
  char *c, *c2, *name, fname[256];
  int i, j, k, jj,q;
  float x,y,x0,y0;

  int zeroi=0, one=1, two=2, three=3, five=5;
  float zerof=0.0, f360=360.0;
 
  // printf("process_option '%s'\n",line);  
 
  /* general parameter extraction (assumes unique parameter names) */
  
  if ((c = strstr(line+1,"c=")) != NULL) { /* plotting color */
    sscanf(c+2,"%d",&c_opt);
    if (c_opt > 255) c_opt=255;
    if (c_opt < 0) c_opt=0;
  }
  if ((c = strstr(line+1,"w=")) != NULL) { /* plotting width */
    sscanf(c+2,"%d",&w_opt);
    if (w_opt > 7) w_opt=7;
    if (w_opt < 1) w_opt=1;
  }
  if ((c = strstr(line+1,"I=")) != NULL) { /* plotting increment */
    sscanf(c+2,"%d",&Ii_opt);
    if (Ii_opt < 0) Ii_opt=0;
  }
  
  latlon_opt=0;
  if ((c = strstr(line+1,"x=")) != NULL) { /* coordinates */
    sscanf(c+2,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"y=")) != NULL) {
    sscanf(c+2,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"n=")) != NULL) {
    latlon_opt=1;
    sscanf(c+2,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"e=")) != NULL) {
    latlon_opt=1;
    sscanf(c+2,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"N=")) != NULL) {
    latlon_opt=2;
    sscanf(c+2,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"E=")) != NULL) {
    latlon_opt=2;
    sscanf(c+2,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"x1=")) != NULL) {
    sscanf(c+3,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"y1=")) != NULL) {
    sscanf(c+3,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"n1=")) != NULL) {
    latlon_opt=1;
    sscanf(c+3,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"e1=")) != NULL) {
    latlon_opt=1;
    sscanf(c+3,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"N1=")) != NULL) {
    latlon_opt=2;
    sscanf(c+3,"%f",&y1_opt);
  }
  if ((c = strstr(line+1,"E1=")) != NULL) {
    latlon_opt=2;
    sscanf(c+3,"%f",&x1_opt);
  }
  if ((c = strstr(line+1,"x2=")) != NULL) {
    sscanf(c+3,"%f",&x2_opt);
  }
  if ((c = strstr(line+1,"y2=")) != NULL) {
    sscanf(c+3,"%f",&y2_opt);
  }
  if ((c = strstr(line+1,"n2=")) != NULL) {
    latlon_opt=1;
    sscanf(c+3,"%f",&y2_opt);
  }
  if ((c = strstr(line+1,"e2=")) != NULL) {
    latlon_opt=1;
    sscanf(c+3,"%f",&x2_opt);
  }
  if ((c = strstr(line+1,"N2=")) != NULL) {
    latlon_opt=2;
    sscanf(c+3,"%f",&y2_opt);
  }
  if ((c = strstr(line+1,"E2=")) != NULL) {
    latlon_opt=2;
    sscanf(c+3,"%f",&x2_opt);
  }
   
  /* specific command options */

  switch(line[0]) {
    case 'b':   /* box */
      if (debug) printf(" debug: plot box f=%d (%f,%f)-(%f,%f) col=%d wide=%d\n",latlon_opt,x1_opt,y1_opt,x2_opt,y2_opt,c_opt,w_opt);
	
      newvpen_(&zeroi,&w_opt);
      x=c_opt;
      plotvt_(&x,&zerof,&zeroi);
      fpimage_color(c_opt); /* override plotting array color*/

      x0 = x1_opt;
      y0 = y1_opt;
      x = x1_opt;
      y = y2_opt;
      plot_line(Ii_opt,latlon_opt,&x0,&y0,&x,&y);
      x0 = x1_opt;
      y0 = y2_opt;
      x = x2_opt;
      y = y2_opt;
      plot_line(Ii_opt,latlon_opt,&x0,&y0,&x,&y);
      x0 = x2_opt;
      y0 = y2_opt;
      x = x2_opt;
      y = y1_opt;
      plot_line(Ii_opt,latlon_opt,&x0,&y0,&x,&y);
      x0 = x2_opt;
      y0 = y1_opt;
      x = x1_opt;
      y = y1_opt;
      plot_line(Ii_opt,latlon_opt,&x0,&y0,&x,&y);

      if (latlon_opt != 0) {
	latlon_trans(&x1_opt,&y1_opt);
	latlon_trans(&x2_opt,&y2_opt);
      }

      break;

    case 'c':   /* circle */
      if ((c = strstr(line+1,"d=")) != NULL) { /* circle radius */
	sscanf(c+2,"%f",&d_opt);
	if (d_opt < 0) d_opt=0.0;
      }

      if (debug) printf(" debug: plot circle f=%d (%f,%f) radius=%f color=%d width=%d\n",latlon_opt,x1_opt,y1_opt,d_opt,c_opt,w_opt);
	
      if (latlon_opt != 0) {
	latlon_trans(&x1_opt,&y1_opt);
      }
      newvpen_(&zeroi,&w_opt);
      x=c_opt;
      plotvt_(&x,&zerof,&zeroi);
      fpimage_color(c_opt); /* override plotting array color*/
      circle_(&x1_opt,&y1_opt,&zerof,&f360,&d_opt,&d_opt,&zerof);
      plotvt_(&zerof,&zerof,&five);
      break;

    case 'd':   /* debug */
      debug = 1;
      break;

    case 'G':   /* add color bar */
      if (debug) printf(" debug: add color bar f=%d (%f,%f)-(%f,%f) M=%d\n",latlon_opt,x1_opt,y1_opt,x2_opt,y2_opt,M_opt);
      if (latlon_opt != 0) {
	latlon_trans(&x1_opt,&y1_opt);
	latlon_trans(&x2_opt,&y2_opt);
	if (debug) printf(" debug: transformed location       (%f,%f)-(%f,%f)\n",x1_opt,y1_opt,x2_opt,y2_opt);
      }

      if ((c = strstr(line+1,"M=")) != NULL) {  /* index */
	sscanf(c+2,"%d",&M_opt);
	if (M_opt > 3) M_opt=3;
	if (M_opt < 0) M_opt=0;
      }

      switch(M_opt) {   /* direction option */
      case 0: /* l-r */
	for (k=x1_opt;k<=x2_opt;k++) {
	  i = (e_opt-s_opt)*(k - x1_opt)/(x2_opt - x1_opt) + s_opt;
	  if (i < s_opt) i=s_opt;
	  if (i > e_opt) i=e_opt;
	  fpimage_color(i);
	  for (j=y1_opt;j<y2_opt;j++)
	    if (k > 0 && k <= nsx && j >=0 && j <  nsy) {
	      jj = nsy - j;
	      fppix_(&k, &jj, &i, &one);	    
	      /* *(fpimage_1.a+(nsy-j-1)*nsx+k-1) = i; */
	    }
	}
	break;
	
      case 1: /* r-l */
	for (k=x1_opt;k<=x2_opt;k++) {
	  i = (s_opt-e_opt)*(k - x1_opt)/(x2_opt - x1_opt) + e_opt;
	  if (i < s_opt) i=s_opt;
	  if (i > e_opt) i=e_opt;
	  fpimage_color(i);
	  for (j=y1_opt;j<y2_opt;j++)
	    if (k > 0 && k <= nsx && j >=0 && j <  nsy) {
	      jj = nsy - j;		
	      fppix_(&k, &jj, &i, &one);	    
	      /* *(fpimage	_1.a+(nsy-j-1)*nsx+k-1) = i; */
	    }
	}
	break;
	
      case 2: /* b-t */
	for (j=y1_opt;j<y2_opt;j++) {
	  i = (e_opt-s_opt)*(j - y1_opt)/(y2_opt - y1_opt) + s_opt;
	  if (i < s_opt) i=s_opt;
	  if (i > e_opt) i=e_opt;
	  fpimage_color(i);
	  for (k=x1_opt;k<x2_opt;k++) 
	    if (k >= 0 && k < nsx && j >=0 && j <  nsy) {
	      jj = nsy - j;		
	      fppix_(&k, &jj, &i, &one);
	      /* *(fpimage_1.a+(nsy-j-1)*nsx+k-1) = i; */
	    }
	}
	break;
	
      case 3:  /* t-b */
	for (j=y1_opt;j<y2_opt;j++) {
	  i = (s_opt-e_opt-1)*(j - y1_opt)/(y2_opt - y1_opt) + e_opt;
	  if (i < s_opt) i=s_opt;
	  if (i > e_opt) i=e_opt;
	  fpimage_color(i);
	  for (k=x1_opt;k<=x2_opt;k++) 
	    if (k >= 0 && k < nsx && j >=0 && j <  nsy) {
	      jj = nsy - j;		
	      fppix_(&k, &jj, &i, &one);
	      /* *(fpimage_1.a+(nsy-j-1)*nsx+k-1) = i; */
	    }
	}
	break;
	
      }
      break;
      

    case 'l':   /* line */
      if (debug) printf(" debug: plot line f=%d (%f,%f)-(%f,%f) col=%d wide=%d\n",latlon_opt,x1_opt,y1_opt,x2_opt,y2_opt,c_opt,w_opt);

      newvpen_(&zeroi,&w_opt);
      x=c_opt;
      plotvt_(&x,&zerof,&zeroi);
      fpimage_color(c_opt); /* override plotting array color*/
      
      plot_line(Ii_opt,latlon_opt,&x1_opt,&y1_opt,&x2_opt,&y2_opt);
      
      break;

    case 'S':   /* symbol marker */

      if ((c = strstr(line+1,"D=")) != NULL) {  /* symbol dimension */
	sscanf(c+2,"%d",&D_opt);
	if (D_opt < 1) D_opt=1;
      }
      if ((c = strstr(line+1,"n=")) != NULL) {  /* symbol id number */
	sscanf(c+2,"%d",&n_opt);
	if (n_opt < 0) n_opt=0;
	if (n_opt > 2) n_opt=2;
      }

      if (debug) printf(" debug: plot symbol f=%d (%f,%f) n=%d D=%d col=%d wide=%d\n",latlon_opt,x1_opt,y1_opt,n_opt,D_opt,c_opt,w_opt);
	
      if (latlon_opt != 0)
	latlon_trans(&x1_opt,&y1_opt);
      newvpen_(&zeroi,&w_opt);
      x=c_opt;
      plotvt_(&x,&zerof,&zeroi);
      fpimage_color(c_opt); /* override plotting array color*/
      switch (n_opt) {  /* make symbol */
      case 2:  /* asterick */
      case 1:  /* X */
	x=x1_opt - D_opt;
	y=y1_opt - D_opt;
	plotvt_(&x,&y,&three);
	x=x1_opt + D_opt;
	y=y1_opt + D_opt;
	plotvt_(&x,&y,&two);
	x=x1_opt + D_opt;
	y=y1_opt - D_opt;
	plotvt_(&x,&y,&three);
	x=x1_opt - D_opt;
	y=y1_opt + D_opt;
	plotvt_(&x,&y,&two);
	if (n_opt == 1) break;
      case 0:  /* + */
      default:
	x=x1_opt - D_opt;
	y=y1_opt;
	plotvt_(&x,&y,&three);
	x=x1_opt + D_opt;
	plotvt_(&x,&y,&two);
	x=x1_opt;
	y=y1_opt - D_opt;
	plotvt_(&x,&y,&three);
	y=y1_opt + D_opt;
	plotvt_(&x,&y,&two);
      }
      plotvt_(&zerof,&zerof,&five);

      break;

    case 't':   /* add text */
      if ((c = strstr(line+1,"t=")) != NULL) { /* text height */
	sscanf(c+2,"%f",&t_opt);
	if (t_opt < 0) t_opt=0.0;
      }
      if ((c = strstr(line+1,"a=")) != NULL) { /* text angle */
	sscanf(c+2,"%f",&a_opt);
      }
      if ((c = strstr(line+1,"m=")) != NULL) { /* text message */
	strcpy(m_opt,(c+2));
	while ((c = strstr(m_opt,"`")) != NULL)
	  *c = ' ';   /* replace back quotes with spaces */
      }
      
      if (debug) printf(" debug: plot text f=%d (%f,%f) text='%s' color=%d width=%d\n",latlon_opt,x1_opt,y1_opt,m_opt,c_opt,w_opt);
	
      if (latlon_opt != 0) {
	latlon_trans(&x1_opt,&y1_opt);
      }
      newvpen_(&zeroi,&w_opt);
      x=c_opt;
      plotvt_(&x,&zerof,&zeroi);
      fpimage_color(c_opt); /* override plotting array color*/
      j=strlen(m_opt);
      if ((c2=strchr(m_opt,',')) != NULL) j=c2-m_opt;
      if (debug) printf(" debug: character count in text line: %d\n",j);
      if (j > 0)
	(void) syms_(&x1_opt,&y1_opt,&t_opt,m_opt,&a_opt,&j,&one);
      plotvt_(&zerof,&zerof,&five);

      break;

    case 'T':   /* add text extracted from path-stripped input file name*/
      j=1; i=1;
      if ((c = strstr(line+1,"t=")) != NULL) { /* text height */
	sscanf(c+2,"%f",&t_opt);
	if (t_opt < 0) t_opt=0.0;
      }
      if ((c = strstr(line+1,"a=")) != NULL) { /* text angle */
	sscanf(c+2,"%f",&a_opt);
      }
      if ((c = strstr(line+1,"S=")) != NULL) { /* start char */
	sscanf(c+2,"%d",&i);
      }
      if ((c = strstr(line+1,"C=")) != NULL) { /* number of chars */
	sscanf(c+2,"%d",&j);
      }
      if (strlen(f_opt) >= i) {
	if (j==0) j=strlen((f_opt+i-1));
	strncpy(m_opt,(f_opt+i-1),j);
	*(m_opt+j)='\0';
      }
      
      if (debug) printf(" debug: plot fname text f=%d (%f,%f) Ext-text='%s' color=%d width=%d start=%d count=%d '%s'\n",latlon_opt,x1_opt,y1_opt,m_opt,c_opt,w_opt,i,j,f_opt);
      
      if (latlon_opt != 0) {
	latlon_trans(&x1_opt,&y1_opt);
      }
      newvpen_(&zeroi,&w_opt);
      x=c_opt;
      plotvt_(&x,&zerof,&zeroi);
      fpimage_color(c_opt); /* override plotting array color*/
      j=strlen(m_opt);
      if (debug) printf(" debug: character count in text line: %d\n",j);
      if (j > 0)
	(void) syms_(&x1_opt,&y1_opt,&t_opt,m_opt,&a_opt,&j,&one);
      plotvt_(&zerof,&zerof,&five);
      break;

    case 'z':   /* plot lines specified in external ascii file */
      if (debug) printf(" debug: plot lines from file col=%d wide=%d\n",c_opt,w_opt);
      
      if ((c = strstr(line+1,"F=")) != NULL) { /* file name */
	name=c+2;
	strcpy(fname,name);
	c=fname;
	while (*c != ',' && *c != '\0') c++;
	*c='\0';
      } else
	printf("*** No input file specified, plot lines from file command ignored ***\n");
      if (debug) printf(" debug: input plot line file name '%s'\n",fname);
      
      latlon_opt=0;	
      if ((c = strstr(line+1,"Z=")) != NULL) { /* lat/lon vs x,y plotting */
	sscanf(c+2,"%d",&latlon_opt);
      }
      
      q=0;	
      if ((c = strstr(line+1,"o=")) != NULL) { /* line plot flag option */
	sscanf(c+2,"%d",&q);
      }
      if (debug) printf(" debug: plot lines from file latlon/xy: %d  flag option: %d\n",latlon_opt,q);
      
      line_plot_from_file(fname,q,Ii_opt,latlon_opt,c_opt,w_opt);

      break;
      
    default:
      break;
    }

}



void line_plot_from_file(char filename[], int q, int Ii_opt, int latlon_opt, int c_opt, int w_opt)
{     
  FILE *fid;
  int ip,opt;
  float x,y,x1,y1,x2=0.,y2=0.,dx,dy;
  int zeroi=0, five=5;
  float zerof=0.0;
  
  fid=fopen(filename,"r");
  if (fid == NULL) {
    printf("*** error opening line plot file %s ***\n",filename);
    return;
  }
  if (debug) printf(" debug: reading from line plot file: %s\n",filename);

  newvpen_(&zeroi,&w_opt);
  x=c_opt;
  plotvt_(&x,&zerof,&zeroi);
  fpimage_color(c_opt); /* override plotting array color*/
  if (debug) printf(" debug: color %d width %d  type %d Ii %d latlon_opt\n",c_opt,w_opt,Ii_opt,latlon_opt); 

  while (1==1) {
    if (fscanf(fid,"%f",&y1) < 0) return;
    if (fscanf(fid,"%f",&x1) < 0) return;
    if (fscanf(fid,"%d",&ip) < 0) return;
    if (q >= 0) 
      if (fscanf(fid,"%d",&opt) < 0) return;

    /*    printf("%f %f %d %d\n",x1,y1,ip,opt); */
    
    if (q <= 0 || q == opt) {

	if (ip == 3) {
	  x2 = x1;
	  y2 = y1;
	} else if (ip ==2) {
	  dx = x1;
	  dy = y1;
	  x = x2;
	  y = y2;
	  /* printf("plot_line %f %f %f %f\n",x,y,dx,dy); */
	  plot_line(Ii_opt,latlon_opt,&x,&y,&dx,&dy);
	  x2 = x1;
	  y2 = y1;
	} 
    }
  }
  plotvt_(&zerof,&zerof,&five);

}

	
void plot_line(int Ii_opt, int latlon_opt, float *x1_opt, float *y1_opt, float *x2_opt, float *y2_opt)
{
  float x, y, x0, y0, dx, dy, zerof = 0.0;
  int k;
  int two=2, three=3, five=5;
  
  if (Ii_opt > 1) {
    if (latlon_opt != 0) {
      x0 = *x1_opt;
      y0 = *y1_opt;
      dx = (*x2_opt - *x1_opt)/Ii_opt;
      dy = (*y2_opt - *y1_opt)/Ii_opt;

      for (k=0;k<Ii_opt+1;k++) {
	x=x0+dx*k;
	y=y0+dy*k;
	latlon_trans(&x,&y);
	if (k == 0)
	  plotvt_(&x,&y,&three);
	else {
	  plotvt_(&x,&y,&two);
	}
      }
      plotvt_(&zerof,&zerof,&five);
      
      latlon_trans(x1_opt,y1_opt);
      latlon_trans(x2_opt,y2_opt);
    } else {
      plotvt_(x1_opt,y1_opt,&three);
      plotvt_(x2_opt,y2_opt,&two);
      plotvt_(&zerof,&zerof,&five);
    }
  } else {
    if (latlon_opt != 0) {
      latlon_trans(x1_opt,y1_opt);
      latlon_trans(x2_opt,y2_opt);
    } 
    plotvt_(x1_opt,y1_opt,&three);
    plotvt_(x2_opt,y2_opt,&two);
    plotvt_(&zerof,&zerof,&five);
  }
}
