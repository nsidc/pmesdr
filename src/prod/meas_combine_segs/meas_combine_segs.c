/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_combine_segs.c    MEaSURES project

  program to generate single dump fie from segemented
  files with the aid of the meta file

  Written by DGL at BYU 06/07/2014 

******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <unistd.h>  /* for access function */

#define VERSION 0.0

#define CREATE_NON 1  /* set to 1 to include NON images, 0 to not create */


#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define rnd(a) ((a) >= 0 ? floor((a)+0.5L) : ceil((a)-0.5L))

/****************************************************************************/

/* utility functions */

void no_trailing_blanks(char *s)
{  /* remove trailing blanks (spaces) from string */
  int n=strlen(s);
  
  while (n > 0) {
    if (s[n] == 10) s[n] = '\0';
    if (s[n] != ' ' && s[n] != '\0') return;
    if (s[n] == ' ') s[n] = '\0';
    n--;
  }
  return;
}

void eprintf(char *s)
{ /* print to both stdout and stderr to catch errors */
  fprintf(stdout,s);
  fflush(stdout);
  fprintf(stderr,s);
  fflush(stderr);
}

/****************************************************************************/

/* function prototypes */

void combine_dump(char *names, int *pix, int nsection, int ncnt, int ncnt0, 
		  int verbose, int option, char *in_path, char *out_path,
		  float ascale, float bscale, float a0, float b0, 
		  float xdeg, float ydeg, int nsx, int nsy, 
		  float ascale2, float bscale2, float a02, float b02, 
		  float xdeg2, float ydeg2, int nsx2, int nsy2, 
		  int *errors);

/****************************************************************************/

/* global variables */

int max_name_len=40;
int max_regions=100;
int max_sections=18;
int max_filetypes=36;

/****************************************************************************/

/* main program */

int main(int argc, char **argv)
{
  int ncerr, ncid;
  int nits, ierr;
  int nout;
  char polch;

  char *file_in;
  char inpath[150], outpath[150];
  
  int verbose = 3;  /* def verbosity level */
  int option = 0;   /* def option */
  
  int i, j, ncnt=0, ncnt0, kreg;
  int nsx, nsy, regnum, projt;
  float ascale, bscale, a0, b0, xdeg, ydeg;
  int non_size_x, non_size_y, nsx2, nsy2, ix, iy;
  float xdeg2, ydeg2, ascale2, bscale2, a02, b02;
  int ix1, iy1, ix2, iy2, jx1, jy1, jx2, jy2;
  int ix1g, iy1g, ix2g, iy2g, jx1g, jy1g, jx2g, jy2g;

  float ascale_s,bscale_s,a0_s,b0_s,xdeg_s,ydeg_s;
  int nsx_s,nsy_s, nsx_g, nsy_g;
  float ascale_g,bscale_g,a0_g,b0_g,xdeg_g,ydeg_g;

  FILE *file_id;  

  char line[100];
  int isect;
  int ireg=0;
  int flag, flag_out, flag_region, flag_section, flag_values;
  char *s, *x;
  int nsection=0, isection, cnt;
  int pind,nind;

  int errors = 0;

  int pix[max_sections*max_filetypes*16];
  char names[max_sections*max_filetypes*max_name_len];

  /* begin program */

  printf("BYU MEASuREs meta combine segments (dump) program: version %f\n",VERSION);

  if (argc < 2) {
    printf("\nusage: %s meta inpath outpath option verbose\n\n",argv[0]);
    printf(" input parameters:\n");
    printf("   meta    = input meta file name\n");
    printf("   inpath  = input path for SIR images\n");
    printf("   outpath = output path\n");
    printf("   option  = (0=SIR files [def], 1=BGI files)\n");
    printf("   verbose = (0=quiet, 1=verbose [def])\n");
    return(0);
  }
  file_in=argv[1];

  /* get verbose flag first */
  if (argc > 5) 
    sscanf(argv[5],"%d",&verbose);

  strncpy(inpath,"./",100); /* default input path */
  if (argc > 2) 
    sscanf(argv[2],"%s",inpath);
  if (verbose)
    printf("Input path: %s\n",inpath);

  strncpy(outpath,"./",100); /* default output path */
  if (argc > 3)
    sscanf(argv[3],"%s",outpath);
  if (verbose)
    printf("Output path: %s\n",outpath);

  if (argc > 4) 
    sscanf(argv[4],"%d",&option);

  if (verbose)
    if (option == 0)
      printf("Processing option: %d (SIR files)\n",option);
    else if (option == 1)
      printf("Processing option: %d (BGI files)\n",option);
    else
      exit(-1);

  /* open and digest contents of meta file */
  ireg=0;

  if (verbose>0)
    printf("Open meta file %s\n",file_in);
  file_id=fopen(file_in,"r");
  if (file_id == NULL) {
    printf("*** could not open input meta file %s\n",file_in);
    return(-1);
  }

  /* read key information from meta file */

  flag=1;
  while (flag) {
    fgets(line,sizeof(line),file_id);
    no_trailing_blanks(line);
    if (ferror(file_id)) {
      printf("*** error reading meta file\n");
      flag=0;
    } else {
      //printf("read '%s'\n",line);
      
      if (strstr(line,"End_description") != NULL)
	flag=0;

      if (strstr(line,"Begin_region_description") != NULL) {

	/* new region started, set default values */
	isect=0;      
	ireg=ireg+1;
	if (verbose>1)
	  printf(" Start region %d in meta file\n",ireg);


	/* initialize file names array */
	for (j=0; j< max_sections*max_name_len; j++) 
	  names[j]='\0';

	/* read region information */

	flag_region=1;
	while(flag_region) {
	  fgets(line,sizeof(line),file_id);
	  no_trailing_blanks(line);
	  if (ferror(file_id)) {
	    printf("*** error reading meta file at region \n");
	    flag_region=0;
	  } else {
	    //printf("region read '%s'\n",line);

	    if (strstr(line,"End_region_description") != NULL)
	      flag_region=0;
      
	    if (strstr(line,"Region_id") != NULL) {
	      x = strchr(line,'=');
	      regnum=atoi(++x);
	    }

	    if (strstr(line,"Sectioning_code") != NULL) {
	      x = strchr(line,'=');
	      nsection=atoi(++x);
	      nsection=nsection%100;
	      isect=0;
	    }

	    if (strstr(line,"Begin_section_description") != NULL) {
	      /* new section, read section information */

	      flag_section=1;
	      while(flag_section) {
		fgets(line,sizeof(line),file_id);
		no_trailing_blanks(line);
		if (ferror(file_id)) {
		  printf("*** error reading meta file at section \n");
		  flag_section=0;
		} else {
		  //printf("section read '%s'\n",line);
		  if (strstr(line,"End_section_description") != NULL)
		    flag_section=0;
      
		  if (strstr(line,"Section_id") != NULL) {
		    x = strchr(line,'=');
		    isection =atoi(++x);
		    isect++;		    
		    if (verbose>1)
		      printf("  Section %d region %d\n",isection,ireg); 
		  }

		  if (strstr(line,"Project_type") != NULL) {
		    x = strchr(line,'=');
		    projt=atoi(++x);
		  }
      
		  if (strstr(line,"Projection_origin_x") != NULL) {
		    x = strchr(line,'=');
		    ydeg=atof(++x);
		  }

		  if (strstr(line,"Projection_origin_y") != NULL) {
		    x = strchr(line,'=');
		    xdeg=atof(++x);
		  }

		  if (strstr(line,"Projection_offset_x") != NULL) {
		    x = strchr(line,'=');
		    a0=atof(++x);
		  }

		  if (strstr(line,"Projection_offset_y") != NULL) {
		    x = strchr(line,'=');
		    b0=atof(++x);
		  }

		  if (strstr(line,"Projection_scale_x") != NULL) {
		    x = strchr(line,'=');
		    ascale=atof(++x);
		  }

		  if (strstr(line,"Projection_scale_y") != NULL) {
		    x = strchr(line,'=');
		    bscale=atof(++x);
		  }

		  if (strstr(line,"Image_size_x") != NULL) {
		    x = strchr(line,'=');
		    nsx=atoi(++x);
		  }

		  if (strstr(line,"Image_size_y") != NULL) {
		    x = strchr(line,'=');
		    nsy=atoi(++x);
		  }

		  if (strstr(line,"Section_loc_pixels") != NULL) {
		    x = strchr(line,'=');
		    sscanf(x+1,"%d %d %d %d %d %d %d %d",&ix1,&iy1,&ix2,&iy2,&jx1,&jy1,&jx2,&jy2);
		    if (verbose>2)
		      printf("   section location pixels: %d %d %d %d %d %d %d %d\n",ix1,iy1,ix2,iy2,jx1,jy1,jx2,jy2);
		  }

		  if (strstr(line,"Grid_projection_origin_x") != NULL) {
		    x = strchr(line,'=');
		    ydeg2=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_origin_y") != NULL) {
		    x = strchr(line,'=');
		    xdeg2=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_offset_x") != NULL) {
		    x = strchr(line,'=');
		    a02=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_offset_y") != NULL) {
		    x = strchr(line,'=');
		    b02=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_scale_x") != NULL) {
		    x = strchr(line,'=');
		    ascale2=atof(++x);
		  }

		  if (strstr(line,"Grid_projection_scale_y") != NULL) {
		    x = strchr(line,'=');
		    bscale2=atof(++x);
		  }

		  if (strstr(line,"Grid_scale_x") != NULL) {
		    x = strchr(line,'=');
		    non_size_x=atof(++x);
		  }

		  if (strstr(line,"Grid_scale_y") != NULL) {
		    x = strchr(line,'=');
		    non_size_y =atof(++x);
		  }

		  if (strstr(line,"Grid_size_x") != NULL) {
		    x = strchr(line,'=');
		    nsx2=atoi(++x);
		  }

		  if (strstr(line,"Grid_size_y") != NULL) {
		    x = strchr(line,'=');
		    nsy2=atoi(++x);
		  }

		  if (strstr(line,"Section_grd_pixels") != NULL) {
		    x = strchr(line,'=');
		    sscanf(x+1,"%d %d %d %d %d %d %d %d",&ix1g,&iy1g,&ix2g,&iy2g,&jx1g,&jy1g,&jx2g,&jy2g);
		    if (verbose>2)
		      printf("   section grd location pixels: %d %d %d %d %d %d %d %d\n          %s\n",ix1g,iy1g,ix2g,iy2g,jx1g,jy1g,jx2g,jy2g,x+1);
		  }

		  if (strstr(line,"Begin_product_file_names") != NULL) {
		    if (isection == 0) {
		      ascale_s=ascale;
		      bscale_s=bscale;
		      a0_s=a0;
		      b0_s=b0;
		      xdeg_s=xdeg;
		      ydeg_s=ydeg;
		      nsx_s=nsx;
		      nsy_s=nsy;
		      ascale_g=ascale2;
		      bscale_g=bscale2;
		      a0_g=a02;
		      b0_g=b02;
		      xdeg_g=xdeg2;
		      ydeg_g=ydeg2;
		      nsx_g=nsx2;
		      nsy_g=nsy2;
		    }

		    /* read output file names and save location */
		    ncnt=0;
		    flag_values=1;
		    while(flag_values) {
		      fgets(line,sizeof(line),file_id);
		      no_trailing_blanks(line);
		      if (ferror(file_id)) {
			printf("*** error reading meta file at section \n");
			flag_values=0;
		      } else {
			//printf("section read '%s'\n",line);
			if (strstr(line,"End_product_file_names") != NULL)
			  flag_values=0;

			x = strchr(line,'=');
			if (x != NULL) {
			  if (isection > max_sections || ncnt > max_filetypes) {
			    printf("*** combine_section error %d %d %d %d %d\n",
				   isection,ncnt,kreg,max_sections,max_filetypes);
			    printf("*** program has failed!\n");
			    errors=-99;
			    return(errors);
			  } else {
			    if (verbose>2)
			      printf("   filename (%d,%2d,%d): %s\n",isection,ncnt,kreg,x+1);
			    pind=16*(isection*max_filetypes+ncnt);
			    nind=max_name_len*(isection*max_filetypes+ncnt);
			    strncpy(&names[nind],x+1,max_name_len); /* names(iseciont,ncnt) */
			    pix[pind+ 0]=ix1; /* pix(isection,ncnt,*) */
			    pix[pind+ 1]=iy1;
			    pix[pind+ 2]=ix2;
			    pix[pind+ 3]=iy2;
			    pix[pind+ 4]=jx1;
			    pix[pind+ 5]=jy1;
			    pix[pind+ 6]=jx2;
			    pix[pind+ 7]=jy2;
			    pix[pind+ 8]=ix1g;
			    pix[pind+ 9]=iy1g;
			    pix[pind+10]=ix2g;
			    pix[pind+11]=iy2g;
			    pix[pind+12]=jx1g;
			    pix[pind+13]=jy1g;
			    pix[pind+14]=jx2g;
			    pix[pind+15]=jy2g;
			    if (verbose>3)
			      printf("    indexes: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",ix1,iy1,ix2,iy2,jx1,jy1,jx2,jy2,ix1g,iy1g,ix2g,iy2g,jx1g,jy1g,jx2g,jy2g);
			    x = strstr(x+1,".lis");
			    if (x != NULL) 
			      ncnt0=ncnt;
			    ncnt++;
			  }
			}
		      }
		    }
		  }
		}
	      }
	    } /* end section */
	  }
	} /* end region */  
	if (nsection > 0) { /* combine sections */
	  if (verbose>1)
	    printf(" Combining sections: %d %d region %d\n",nsection, ncnt, ireg);

	  /* combine section dump files into a single file 
	     we only have to do this once for each region */

	  combine_dump(names, pix, nsection, ncnt, ncnt0, verbose, option, inpath, outpath,
		       ascale_s, bscale_s, a0_s, b0_s, xdeg_s, ydeg_s, nsx_s, nsy_s, 
		       ascale_g, bscale_g, a0_g, b0_g, xdeg_g, ydeg_g, nsx_g, nsy_g, 
		       &errors);

	  kreg++;  /* total region counter */ 

	}
      }
    } /* end description */
  }

  
/* end of program */
  printf("Finished program with %d errors\n",errors);
  return(errors);
}


/****************************************************************************/  

#include <ezdump.h>


int combine_segments(int nsection, int ncid0, int *ncid, char *var_name, int itype, int grd, 
		     int nsx, int nsy, int nsx2, int nsy2, float *stval, float *stval2, int *pix, 
		     eznc_head *dhead, int verbose)
{
  int i, j, ncerr, flag=1, nx, ny, nsx1, nsy1;
  int pind, nind;
  float anodata;
  int ix1,iy1,ix2,iy2,jx1,jy1,ix,iy,jx,jy,jx2,jy2,iadd1,iadd2;
  char err_txt[256];  

  if (grd==1) {
    nx=nsx2;
    ny=nsy2;
  } else {
    nx=nsx;
    ny=nsy;
  }

  for (j=1; j <= nsection; j++) 
    if (ncid[j]>0) { /* for each valid section */

      if (verbose>4)
	printf("      section %d %d %d,%d %s\n",j,ncid[j],nx,ny,var_name);  

      /* read array */
      ncerr=get_float_array_nc(ncid[j],var_name,stval2,&nsx1,&nsy1,&anodata); check_err(ncerr, __LINE__,__FILE__);  

      if (flag==1) { /* initialize array to anodata on first section */
	for (i=0; i<nx*ny; i++)
	  stval[i]=anodata;
	flag=0;
      }

      /* copy segment pixels to full image array */
      pind=16*(j*max_filetypes+itype)+grd*8;
      ix1=pix[pind+0];
      iy1=pix[pind+1];
      ix2=pix[pind+2];
      iy2=pix[pind+3];
      jx1=pix[pind+4];
      jy1=pix[pind+5];
      jx2=pix[pind+6];
      jy2=pix[pind+7];
      if (verbose>3)
	printf("      pixel %d,%d indexes %d %d %d %d %d %d %d %d\n",itype,j,ix1,iy1,ix2,iy2,jx1,jy1,jx2,jy2);

      /* copy pixel values */
      for (iy=iy1; iy<=iy2; iy++) {
	jy=iy-iy1+jy1;
	for (ix=ix1; ix<=ix2; ix++) {
	  jx=ix-ix1+jx1;
	  iadd2=dhead[j].nsx*(iy-1)+ix;
	  iadd1=nx*(jy-1)+jx;
	  stval[iadd1-1]=stval2[iadd2-1];
	}
      }
    }

  /* write output array */
  ncerr=add_float_array_nc(ncid0,var_name,stval,nx,ny,anodata); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr != 0) {
    sprintf(err_txt,"*** ERROR writing variable array %s ***\n",var_name);
    eprintf(err_txt);  
  }

  return(ncerr);  
}



void combine_dump(char *names, int *pix, int nsection, int ncnt, int ncnt0, 
		  int verbose, int option, char *in_path, char *out_path,
		  float ascale, float bscale, float a0, float b0, 
		  float xdeg, float ydeg, int nsx, int nsy, 
		  float ascale2, float bscale2, float a02, float b02, 
		  float xdeg2, float ydeg2, int nsx2, int nsy2, 
		  int *errors)
{
  int i, j, is, is0, flag, grd;
  int nind, nind0;
  int ierr, rerr=0, iok;
  float *stval=NULL, *stval2=NULL, anodata;
  char inname[500], outname[500];

  int ncerr, ncid0, ncid[max_sections];
  eznc_head dhead0, dhead[max_sections];  

  time_t tod;

  char regname[11];
  char sensor[41];  
  char crproc[101];
  char crtime[29];  

  char inter_name[1024];

  /* allocate a working array */
  stval=(float *) calloc(nsx*nsy,sizeof(float));
  if (stval == NULL) {
    fprintf(stderr,"*** failure to allocate working array in combine_sir %d %d\n",nsx,nsy);
    fprintf(stderr,"*** program failed\n");
    exit(-1);
  }

  /* open each input dump netcdf file */
  flag=1;  
  for (j=1; j <= nsection; j++) {  /* for each section */

    nind=max_name_len*(j*max_filetypes+ncnt0);

    if (option==1)  /* BG */
      sprintf(inname,"%s/%s_dump1.nc",in_path,&names[nind]); 
    else            /* everything else */
      sprintf(inname,"%s/%s_dump.nc",in_path,&names[nind]); 

    if (verbose>0)
      printf(" Dump segment filename (%d of %d): %s\n",j,nsection,inname);

    /* check to see if file exists 
       Note that file existence errors are not reported since we often have non-produced files */
    iok = access(inname, R_OK); 
    if (iok ==0 ) {  /* if file exists, process it */

      /* read dump file header */
      ncerr=ez_nc_open_file_read_head(inname, &ncid[j], &dhead[j]); check_err(ncerr, __LINE__,__FILE__);
      if (ncerr != 0) {
	printf("ERROR opening or reading input dump file: %s\n",inname); 
	exit(-1);
      }

      if (verbose>1)
	printf("  Opened file %d %s\n",j,inname);

      if (flag==1) {

	/* copy key header information */
	ez_copy_head(&dhead[j],&dhead0);
  	
	/* read some other strings */
	ncerr=get_string_nc(ncid[j],"Region_name",regname,10); check_err(ncerr, __LINE__,__FILE__);
	ncerr=get_string_nc(ncid[j],"Sensor_name",sensor,40); check_err(ncerr, __LINE__,__FILE__);
	ncerr=get_string_nc(ncid[j],"Creator",crproc,101); check_err(ncerr, __LINE__,__FILE__);
	ncerr=get_string_nc(ncid[j],"Creation_time",crtime,29); check_err(ncerr, __LINE__,__FILE__);

	if (verbose>2)
	  printf("  Copy over header %d info for region '%s'\n",j,regname);

	flag=0;      
      }
    } else {
      if (verbose>1)
	printf("  ** could not find file %s\n",inname);
    }
  }

  if (flag==0) { /* if there were no valid segments, do not attempt to produce output file */
  
    /* open combined dump file and copy information from header */
    nind=max_name_len*(0*max_filetypes+ncnt0);
    if (option==1)  /* BG */
      sprintf(outname,"%s/%s_dump1.nc",out_path,&names[nind0]); 
    else            /* everything else */
      sprintf(outname,"%s/%s_dump.nc",out_path,&names[nind0]); 
    if (verbose>0)
      printf(" Output dump file %s\n",outname);
  
    /* modify header information to match combined file */
    dhead0.nsx=nsx;
    dhead0.nsy=nsy;
    dhead0.ascale=ascale;
    dhead0.bscale=bscale;
    dhead0.a0=a0;
    dhead0.b0=b0;
    dhead0.xdeg=xdeg;
    dhead0.ydeg=ydeg;
    dhead0.nsx2=nsx2;
    dhead0.nsy2=nsy2;
    dhead0.ascale2=ascale2;
    dhead0.bscale2=bscale2;
    dhead0.a02=a02;
    dhead0.b02=b02;
    dhead0.xdeg2=xdeg2;
    dhead0.ydeg2=ydeg2;
  
    /* generate output intermediate dump file name, open file, and dump info */
    ncerr=ez_nc_open_file_write_head(outname, &ncid0, &dhead0); check_err(ncerr, __LINE__,__FILE__);
    if (ncerr < 0) {
      fprintf(stderr,"*** ERROR writing output dump file '%s'***\n",outname);
      (*errors)++;
    }

    /* add string information */
    ncerr=add_string_nc(ncid0,"Region_name",regname,10); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"Sensor_name",sensor,40); check_err(ncerr, __LINE__,__FILE__);
    //sprintf(crproc,"BYU MERS:meas_combine_segs v%f",VERSION);
    ncerr=add_string_nc(ncid0,"Creator",crproc,101); check_err(ncerr, __LINE__,__FILE__);
    (void) time(&tod);
    (void) strftime(crtime,28,"%X %x",localtime(&tod));
    ncerr=add_string_nc(ncid0,"Creation_time",crtime,29); check_err(ncerr, __LINE__,__FILE__); 

    /* add product file names */
    /* note that the order and names of the files names in the meta file must match this code */
    nind=max_name_len*(0*max_filetypes);
    ncerr=add_string_nc(ncid0,"a_name",&names[nind+max_name_len*0],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"c_name",&names[nind+max_name_len*1],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"i_name",&names[nind+max_name_len*2],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"j_name",&names[nind+max_name_len*3],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"e_name",&names[nind+max_name_len*4],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"v_name",&names[nind+max_name_len*5],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"p_name",&names[nind+max_name_len*6],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"a_name_ave",&names[nind+max_name_len*7],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"grd_aname",&names[nind+max_name_len*8],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"grd_vname",&names[nind+max_name_len*9],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"grd_iname",&names[nind+max_name_len*10],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"grd_jname",&names[nind+max_name_len*11],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"grd_cname",&names[nind+max_name_len*12],100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=add_string_nc(ncid0,"grd_pname",&names[nind+max_name_len*13],100); check_err(ncerr, __LINE__,__FILE__);

    if (CREATE_NON) {
      ncerr=add_string_nc(ncid0,"non_aname",&names[nind+max_name_len*14],100); check_err(ncerr, __LINE__,__FILE__);
      ncerr=add_string_nc(ncid0,"non_vname",&names[nind+max_name_len*15],100); check_err(ncerr, __LINE__,__FILE__);
    }

    /* allocate working arrays */
    stval=(float *) calloc(nsx*nsy,sizeof(float));
    stval2=(float *) calloc(nsx*nsy,sizeof(float));
    if (stval == NULL || stval2 == NULL) {
      fprintf(stderr,"*** failure to allocate working array in meas_combine_segs %d %d\n",nsx,nsy);
      fprintf(stderr,"*** program failed\n");
      exit(-1);
    }

    /* for each output array, combine segments and save */

    ncerr=combine_segments(nsection, ncid0, ncid, "ave_image", 7, 0, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "a_image", 0, 0, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "i_image", 2, 0, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "j_image", 3, 0, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    //ncerr=combine_segments(nsection, ncid0, ncid, "c_image", 1, 0, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "v_image", 5, 0, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "e_image", 4, 0, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "p_image", 6, 0, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);

    ncerr=combine_segments(nsection, ncid0, ncid, "grd_a_image", 8, 1, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "grd_v_image", 9, 1, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "grd_i_image", 10, 1, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "grd_j_image", 11, 1, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "grd_c_image", 12, 1, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    ncerr=combine_segments(nsection, ncid0, ncid, "grd_p_image", 13, 1, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);

    if (CREATE_NON) {
      ncerr=combine_segments(nsection, ncid0, ncid, "non_a_image", 14, 1, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
      ncerr=combine_segments(nsection, ncid0, ncid, "non_v_image", 15, 1, nsx, nsy, nsx2, nsy2, stval, stval2, pix, &dhead[0], verbose);
    }

    /* close input files */
    for (j=1; j <= nsection; j++)  /* for each section */
      if (ncid[j]>0)
	ncerr=nc_close_file(ncid[j]); check_err(ncerr, __LINE__,__FILE__);

    /* finsh and close combined intermediate dump file */
    ncerr=nc_close_file(ncid0); check_err(ncerr, __LINE__,__FILE__);
    if (verbose>0)
      printf("\nFinished writing dump file: %s\n",outname);

  } else {
    if (verbose>1)
      printf(" * no segments exist, not producing output\n");
  }

  if (stval != NULL)
    free(stval);
  return;
}
