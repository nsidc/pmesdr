/* (c) copyright 2014 David G. Long, Brigham Young University */
/*****************************************************************
  Filename:  meas_undump.c    MEaSURES project

  program to generate standard SIR or NETCDF products from 
  intermediate dump file

  Written by DGL at BYU 05/16/2014 

******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "sir3.h"

#define VERSION 0.0

#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define rnd(a) ((a) >= 0 ? floor((a)+0.5L) : ceil((a)-0.5L))

/****************************************************************************/

/* utility functions */

char *addpath(char *outpath, char *name, char *temp)
{ /* append path to name, return pointer to temp */
  sprintf(temp,"%s/%s",outpath,name);
  return(temp);  
}

void image_minmax(float *val, int nsx, int nsy, float anodata, float *amin, float *amax)
{ 
  int i;    
  *amin=1.e25;
  *amax=-1.e25;  
  for (i==0;i<nsx*nsy;i++)
    if (val[i] > anodata) {	  
      *amin = min(*amin, *(val+i));
      *amax = max(*amax, *(val+i));
    }
  return;    
}


/* function prototypes */

int nc_open_file_read_head(char *outfile, int *ncid_in, int *nsx, int *nsy, 
			   int *iopt, float *ascale, float *bscale, 
			   float* a0, float *b0, float *xdeg, float *ydeg, 
			   int *isday, int *ieday, int *ismin, int *iemin,
			   int *iyear, int *iregion, int *ipol, 
			   int *nsx2, int *nsy2, int *non_size_x, 
			   int *non_size_y, float *ascale2, float *bscale2, 
			   float *a02, float *b02, float *xdeg2, float *ydeg2,
			   float *a_init, int *ibeam, int *nits, 
			   int *median_flag, int *nout);

int get_string_nc(int ncid, char *name, char *str, int maxc);

int get_float_array_nc(int ncid, char *name, float *val, int *nsx, int *nsy,
		       float *anodata_A); 

int nc_close_file(int ncid);

int netcdf_write_single(char *outpath, char *a_name_ave, float *val, char *varname,
			int idatatype, int nsx, int nsy, float xdeg, float ydeg, 
			float ascale, float bscale, float a0, float b0, 
			int ioff, int iscale, int iyear, int isday, int ismin, int ieday, int iemin, 
			int iregion, int itype, int iopt, int ipol, int ifreqhm, 
			float anodata, char *sensor, char *title, char *type_A, char *tag,
			char *crproc, char *crtime); 

/****************************************************************************/

/* main program */

int main(int argc, char **argv)
{
  int ncerr, ncid;
  int nits, ierr;
  float a_init, *val;
  char polch;

  char *file_in;
  char outpath[150], tstr[350], sensor_in[41];
  int storage = 0;  /* def SIR */  
  int verbose = 1;  /* def verbose */
  int non_out = 0;  

  char regname[11], *s;
  int ncnt, i, j, n, ii, nsize, nout;
  float tbval, ang, azang=0.0;
  int count, ktime, iadd, end_flag;
  char *x;
  int non_size_x, non_size_y, nsx2, nsy2, ix, iy, nsize2;
  float xdeg2, ydeg2, ascale2, bscale2, a02, b02;

  /* SIR file header information */

  float v_min_A, v_max_A, anodata_A, anodata_B, v_min_B, v_max_B, 
    anodata_C, v_min_C, v_max_C, anodata_I, v_min_I, v_max_I, 
    anodata_Ia, v_min_Ia, v_max_Ia;
  int nsx, nsy, ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin;
  int ioff_B, iscale_B, itype_B, ioff_I, iscale_I, itype_I, 
    ioff_Ia, iscale_Ia, itype_Ia, ioff_C, iscale_C, itype_C;
  int ioff_P, iscale_P, itype_P, ioff_V, iscale_V, itype_V, ioff_E, iscale_E, itype_E;
  float anodata_P, v_min_P, v_max_P, anodata_V, v_min_V, v_max_V,
    anodata_E, v_min_E, v_max_E;
  int iregion, itype_A, nhead, ndes, nhtype, idatatype, ldes, nia;
  int ipol, ifreqhm, ispare1;
  char title[101], sensor[41], crproc[101], type_A[139], tag[101], crtime[29];
  char type_B[139], type_I[139], type_Ia[139], type_C[139], type_P[139],
    type_V[139], type_E[139];
  float xdeg, ydeg, ascale, bscale, a0, b0;
  int iopt;
  int ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc;
#define MAXDES 1024
  char descrip[MAXDES+1];
#define MAXI 128
  short iaopt[MAXI];

  float total;
  char pol;
  float amin, amax;

  time_t tod;

  char a_name[100], b_name[100], 
    c_name[100], p_name[100], v_name[100], e_name[100],
    a_name_ave[100], b_name_ave[100], non_aname[100], 
    i_name[100], j_name[100],
    non_vname[100], grd_aname[100], grd_bname[100], grd_vname[100], 
    grd_pname[100], grd_cname[100], bgi_name[100], 
    grd_iname[100], grd_jname[100],
    info_name[100], line[100];

  int errors = 0;
  int median_flag, ibeam;

  /* begin program */

  printf("BYU SSM/I meta undump program: version %f\n",VERSION);

  if (argc < 2) {
    printf("\nusage: %s dump_in outpath storage_option verbose\n\n",argv[0]);
    printf(" input parameters:\n");
    printf("   dump_in         = input dump file name\n");
    printf("   outpath         = output path\n");
    printf("   storage_option  = (0=SIR [def], 1=NetCDF)\n");
    printf("   verbose         = (0=quiet [def], 1=verbose)\n");
    return(0);
  }
  file_in=argv[1];

  strncpy(outpath,"./",250); /* default output path */
  if (argc > 2) 
    sscanf(argv[2],"%s",outpath);
  printf("Output path %s: ",outpath);

  if (argc > 3) 
    sscanf(argv[3],"%d",&storage);
  printf("Storage option %d: ",storage);
  if (storage == 0)
    printf(" BYU .SIR file output format\n");
  else if (storage == 1)
    printf(" NetCDF file output format\n");
  else
    exit(-1);

  if (argc > 4) 
    sscanf(argv[4],"%d",&verbose);

  
  /* read input file header */

  printf("Open input dump file %s\n",file_in);  
  ncerr=nc_open_file_read_head(file_in, &ncid, &nsx, &nsy, &iopt, 
			       &ascale, &bscale, &a0, &b0, &xdeg, &ydeg, 
			       &isday, &ieday, &ismin, &iemin, &iyear, 
			       &iregion, &ipol, &nsx2, &nsy2, 
			       &non_size_x, &non_size_y, 
			       &ascale2, &bscale2, &a02, &b02, &xdeg2, &ydeg2,
			       &a_init, &ibeam, &nits, &median_flag,
			       &nout); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr != 0) {
     printf("ERROR opening or reading input dump file: %s\n",argv[1]); 
     exit(-1);
  }
  printf("Finished with file read\n");

  /* read other strings */
  ncerr=get_string_nc(ncid,"Region_name",regname,10); check_err(ncerr, __LINE__,__FILE__);
  ncerr=get_string_nc(ncid,"Sensor_name",sensor_in,40); check_err(ncerr, __LINE__,__FILE__);
  ncerr=get_string_nc(ncid,"Creator",crproc,101); check_err(ncerr, __LINE__,__FILE__);
  ncerr=get_string_nc(ncid,"Creation_time",crtime,29); check_err(ncerr, __LINE__,__FILE__);

  if (verbose) {
    /* file header read completed, summarize */
    printf("\nInput file header info: '%s'\n",file_in);
    printf("  Year, day range: %d  %d - %d\n",iyear,isday,ieday);
    printf("  Image size: %d x %d  Projection: %d\n",nsx,nsy,iopt);
    printf("  Origin: %f,%f  Span: %f,%f\n",a0,b0,xdeg,ydeg);
    printf("  Scales: %f,%f  Pol: %d  Reg: %d\n",ascale,bscale,ipol,iregion);
    printf("  Grid size: %d x %d = %d  Scales %d %d\n",nsx2,nsy2,nsx2*nsy2,non_size_x,non_size_y);
    printf("  Grid Origin: %f,%f  Grid Span: %f,%f\n",a02,b02,xdeg2,ydeg2);
    printf("  Grid Scales: %f,%f\n",ascale2,bscale2);
    printf("  Region name: %s  Sensor name: %s\n",regname,sensor_in);    
    printf("\n");
  }
 
  /* get output file names */
  if (nout > 1) { /* create full suite of images */
    ncerr=get_string_nc(ncid,"a_name",a_name,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"c_name",c_name,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"e_name",e_name,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"i_name",i_name,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"j_name",j_name,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"v_name",v_name,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"p_name",p_name,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"a_name_ave",a_name_ave,100); check_err(ncerr, __LINE__,__FILE__);
    if (nout > 14) {	
      ncerr=get_string_nc(ncid,"non_aname",non_aname,100); check_err(ncerr, __LINE__,__FILE__);   
      ncerr=get_string_nc(ncid,"non_vname",non_vname,100); check_err(ncerr, __LINE__,__FILE__);
      non_out=1;  /* create NON files */      
    } else
      non_out=0;  /* no NON files */      
    ncerr=get_string_nc(ncid,"grd_aname",grd_aname,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"grd_vname",grd_vname,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"grd_iname",grd_iname,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"grd_jname",grd_jname,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"grd_pname",grd_pname,100); check_err(ncerr, __LINE__,__FILE__);
    ncerr=get_string_nc(ncid,"grd_cname",grd_cname,100); check_err(ncerr, __LINE__,__FILE__);
  } else {  /* create only BGI file */
    printf("Generating only BGI file *\n");    
    ncerr=get_string_nc(ncid,"bgi_name",bgi_name,100); check_err(ncerr, __LINE__,__FILE__);
  }
  
  /* allocate storage space for image array */
  nsize = nsx * nsy;  
  val  = (float *) malloc(sizeof(float)*nsize);
  if (val == NULL) {
     printf("*** ERROR: inadequate memory for image storage\n");
     exit(-1);
  }

   /* generate output SIR file header info */
  nhtype=31;		/* set header type */
  idatatype=2;		/* output image is in standard i*2 form */
  ipol=ipol+1;
  ifreqhm=1;
  polch='V';
  if (ipol==1) polch='H';

  /* get channel frequency in hundreds of MHz -- SSM/I specific */
  if (ibeam == 1 || ibeam == 2) {
    ifreqhm=194;
  } else if (ibeam == 3) {
    ifreqhm=222;
  } else if (ibeam == 4 || ibeam == 5) {
    ifreqhm=370;
  } else if (ibeam == 6 || ibeam == 7) {
    ifreqhm=855;
  }

  nia=0;                /* no extra integers */
  ldes=0;               /* no extra text */
  ndes=0;
  ispare1=0;
  strncpy(tag,"(c) 2014 BYU MERS Laboratory",40);

  /* expand sensor description */
  sprintf(sensor,"%s %d%c %d",sensor_in,ifreqhm/10,polch,ibeam);  
  regname[9]='\0';

  if (median_flag == 1) 
    sprintf(title,"SIRF image of %s",regname);
  else
    sprintf(title,"SIR image of %s",regname);

  (void) time(&tod);
  (void) strftime(crtime,28,"%X %x",localtime(&tod));
  if (verbose)
    printf("Current time: '%s'\n",crtime);

  /* set projection scale factors */
  switch (iopt){
  case -1: /* image only */
    ideg_sc=10;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 0: /* rectalinear lat/lon */
    ideg_sc=100;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=-100;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 1: /* lambert */
  case 2:
    ideg_sc=100;
    iscale_sc=1000; /* original = 100 */
    i0_sc=1;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 5: /* polar stereographic */
    ideg_sc=100;
    iscale_sc=1000;  /* original = 100 */
    i0_sc=1;
    ixdeg_off=-100;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case  8: /* EASE2 grid */
  case  9:
  case 10:
    ideg_sc=10;
    iscale_sc=100;
    i0_sc=10;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  case 11: /* EASE1 grid */
  case 12:
  case 13:
    ideg_sc=10;
    iscale_sc=1000;
    i0_sc=10;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
    break;
  default: /* unknown */
    ideg_sc=100;
    iscale_sc=1000;
    i0_sc=100;
    ixdeg_off=0;
    iydeg_off=0;
    ia0_off=0;
    ib0_off=0;
  }

  /* image specific header info */

  ioff_A=100;
  iscale_A=200;
  itype_A=3;
  anodata_A=100.00;
  v_min_A=180.0;
  v_max_A=295.0;
  sprintf(type_A,"A Tb image  (%s)",a_name);
  
  ioff_I=-1;
  iscale_I=100;
  itype_I=7;
  anodata_I=-1.00;
  v_min_I=-1.0;
  v_max_I=1.0;
  sprintf(type_I,"Incidence Angle std  (%s)",a_name);
  
  ioff_Ia=0;
  iscale_Ia=100;
  itype_Ia=9;
  anodata_Ia=0.0;
  v_min_Ia=40.0;
  v_max_Ia=60.0;
  sprintf(type_Ia,"Incidence Angle ave  (%s)",a_name);
  
  ioff_C=-1;
  iscale_C=9;
  itype_C=8;
  anodata_C=-1.00;
  v_min_C=-1.0;
  v_max_C=500.0;
  sprintf(type_C,"Count image  (%s)",a_name);

  ioff_P=-1;
  iscale_P=1;
  itype_P=11;
  anodata_P=-1.00;
  v_min_P=0.0;
  v_max_P=(ieday-isday)*24*60+iemin-ismin;
  if (v_max_P > 65400.0) v_max_P=65400.0;
  sprintf(type_P,"Pixel Time image  (%s)",a_name);

  ioff_V=-1;
  iscale_V=100;
  itype_V=23;
  anodata_V=-1.00;
  v_min_V=0.0;
  v_max_V=15.0;
  sprintf(type_V,"Tb STD  (%s)",a_name);

  ioff_E=-16;
  iscale_E=100;
  itype_E=21;
  anodata_E=-16.00;
  v_min_E=-15.0;
  v_max_E=15.0;
  sprintf(type_E,"Tb mean error  (%s)",a_name);

  if (nout < 5) {  /* this must have been a bgi file */

    /* get bgi A image array */
    ncerr=get_float_array_nc(ncid,"bgi_image",val,&nsx,&nsy,&anodata_A); check_err(ncerr, __LINE__,__FILE__);      
    if (ncerr == 0) {
      image_minmax(val,nsx,nsy,anodata_A,&amin,&amax);     
      sprintf(title, "BGI Tb image of %s",regname);
      sprintf(type_A,"A Tb image  (%s)",bgi_name);
      printf(" Writing A output BGI file '%s'\n", bgi_name);
      printf("   image min/max: %f %f  %f\n",amin,amax,anodata_A);
      if (storage == 0) { /* SIR file format */
	ierr = write_sir3(addpath(outpath,bgi_name,tstr), val, &nhead, nhtype, 
			  idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			  ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			  ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
			  iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
			  anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
			  crproc, crtime, descrip, ldes, iaopt, nia);
      } else {  /* netcdf file format */
	ierr = netcdf_write_single(outpath, bgi_name, val, "Tb_bg",
				   idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
				   ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
				   iregion, itype_A, iopt, ipol, ifreqhm, 
				   anodata_A, sensor, title, type_A, tag,
				   crproc, crtime); check_err(ierr, __LINE__,__FILE__);
      }      
      if (ierr < 0) {
	printf("*** ERROR writing A BGI output file ***\n");
	errors++;
      }
    } else {
      printf("*** ERROR reading dump file ***\n");
      errors++;
    }
    
    return(errors);

  }  /* otherwise write the full set out */
      

  /* Tb AVE image */
  ncerr=get_float_array_nc(ncid,"ave_image",val,&nsx,&nsy,&anodata_A); check_err(ncerr, __LINE__,__FILE__);      
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_A,&amin,&amax);     
    sprintf(title, "AVE Tb image of %s",regname);
    printf(" Writing Tb (A) output AVE file '%s'\n", a_name_ave);
    printf("   image min/max: %f %f\n",amin,amax);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,a_name_ave,tstr), val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
			anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, a_name_ave, val, "Tb_ave",
				 idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
				 ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_A, iopt, ipol, ifreqhm, 
				 anodata_A, sensor, title, type_A, tag,
				 crproc, crtime); check_err(ncerr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing A AVE output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* Tb SIR image */
  ncerr=get_float_array_nc(ncid,"a_image",val,&nsx,&nsy,&anodata_A); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {  
    image_minmax(val,nsx,nsy,anodata_A,&amin,&amax);   
    if (median_flag == 1)
      sprintf(title,"SIRF Tb image of %s",regname);
    else
      sprintf(title,"SIR Tb image of %s",regname); 
    printf(" Writing Tb (A) output SIR file '%s'\n", a_name);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_A);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,a_name,tstr), val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
			anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, a_name, val, "Tb_sir",
				 idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
				 ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_A, iopt, ipol, ifreqhm, 
				 anodata_A, sensor, title, type_A, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing A SIR output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* output other auxilary product images */

  /* Istd SIR image */
  ncerr=get_float_array_nc(ncid,"i_image",val,&nsx,&nsy,&anodata_I); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_I,&amin,&amax);     
    sprintf(title,"Istd image of %s",regname); 
    printf(" Writing Istd (I) output SIR file '%s'\n", i_name);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_I);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,i_name,tstr), val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_I, iscale_I, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_I, iopt, ipol, ifreqhm, ispare1,
			anodata_I, v_min_I, v_max_I, sensor, title, type_I, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, i_name, val, "Istd",
				 idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
				 ioff_I, iscale_I, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_I, iopt, ipol, ifreqhm, 
				 anodata_I, sensor, title, type_I, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing Istd output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* Iave SIR image */
  ncerr=get_float_array_nc(ncid,"j_image",val,&nsx,&nsy,&anodata_Ia); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_Ia,&amin,&amax);     
    sprintf(title,"Iave image of %s",regname); 
    printf(" Writing Iave (J) output SIR file '%s'\n", j_name);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_Ia);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,j_name,tstr), val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_Ia, iscale_Ia, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_Ia, iopt, ipol, ifreqhm, ispare1,
			anodata_Ia, v_min_Ia, v_max_Ia, sensor, title, type_Ia, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, j_name, val, "Iave",
				 idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
				 ioff_Ia, iscale_Ia, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_Ia, iopt, ipol, ifreqhm, 
				 anodata_Ia, sensor, title, type_Ia, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing Istd output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* this product is not produced for weighted SIR/SIRF */
  /* printf(" Writing Cnt output SIR file '%s' %d\n", c_name, tmax);
  v_max_C = (float) (10 * (tmax/10+1));
  ierr = write_sir3(addpath(outpath,c_name,tstr), val, &nhead, nhtype, 
		   idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
		   ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
		   ioff_C, iscale_C, iyear, isday, ismin, ieday, iemin, 
		   iregion, itype_C, iopt, ipol, ifreqhm, ispare1,
		   anodata_C, v_min_C, v_max_C, sensor, title, type_C, tag,
		   crproc, crtime, descrip, ldes, iaopt, nia);
  if (ierr < 0) {
    printf("*** ERROR writing Istd output file ***\n");
    errors++;
  }*/

  /* V SIR image */
  ncerr=get_float_array_nc(ncid,"v_image",val,&nsx,&nsy,&anodata_V); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_V,&amin,&amax);     
    sprintf(title,"Tb STD image of %s",regname);
    printf(" Writing Tb STD (V) output SIR file '%s'\n", v_name);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_V);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,v_name,tstr), val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_V, iscale_V, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_V, iopt, ipol, ifreqhm, ispare1,
			anodata_V, v_min_V, v_max_V, sensor, title, type_V, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, v_name, val, "Tbstd",
				 idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
				 ioff_V, iscale_V, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_V, iopt, ipol, ifreqhm, 
				 anodata_V, sensor, title, type_V, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing Tb STD (V) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* E SIR image */
  ncerr=get_float_array_nc(ncid,"e_image",val,&nsx,&nsy,&anodata_E); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_E,&amin,&amax);     
    sprintf(title,"Tb err image of %s",regname);
    printf(" Writing Tb err (E) output SIR file '%s'\n", e_name);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_E);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,e_name,tstr), val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_E, iscale_E, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_E, iopt, ipol, ifreqhm, ispare1,
			anodata_E, v_min_E, v_max_E, sensor, title, type_E, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, e_name, val, "Tberr",
				 idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
				 ioff_E, iscale_E, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_E, iopt, ipol, ifreqhm, 
				 anodata_E, sensor, title, type_E, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing Tb err (E) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* time P SIR image */
  ncerr=get_float_array_nc(ncid,"p_image",val,&nsx,&nsy,&anodata_P); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) { 
    image_minmax(val,nsx,nsy,anodata_P,&amin,&amax);    
    sprintf(title,"Pixel time image of %s",regname);
    printf(" Writing pixel time (P) output SIR file '%s'\n", p_name);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_P);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,p_name,tstr), val, &nhead, nhtype, 
			idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_P, iscale_P, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_P, iopt, ipol, ifreqhm, ispare1,
			anodata_P, v_min_P, v_max_P, sensor, title, type_P, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, p_name, val, "Ptime",
				 idatatype, nsx, nsy, xdeg, ydeg, ascale, bscale, a0, b0, 
				 ioff_E, iscale_E, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_P, iopt, ipol, ifreqhm, 
				 anodata_P, sensor, title, type_P, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing pixel time (P) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* Grid A grd image */
  ncerr=get_float_array_nc(ncid,"grd_a_image",val,&nsx,&nsy,&anodata_A); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_A,&amin,&amax);     
    sprintf(title,"Grid Tb image of %s",regname);
    printf(" Writing Tb (A) output GRD file '%s'\n", grd_aname);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_A);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,grd_aname,tstr), val, &nhead, nhtype, 
			idatatype, nsx2, nsy2, xdeg2, ydeg2, 
			ascale2, bscale2, a02, b02, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
			anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, grd_aname, val, "Tb_grd",
				 idatatype, nsx2, nsy2, xdeg2, ydeg2, 
				 ascale2, bscale2, a02, b02, 
				 ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_A, iopt, ipol, ifreqhm, 
				 anodata_A, sensor, title, type_A, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing GRD Tb (A) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* Grid Tb STD V grd image */
  ncerr=get_float_array_nc(ncid,"grd_v_image",val,&nsx,&nsy,&anodata_V); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_V,&amin,&amax);     
    printf(" Writing Tb std (V) output GRD file '%s'\n", grd_vname);
    sprintf(title,"Grid Tb STD image of %s",regname);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_V);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,grd_vname,tstr), val, &nhead, nhtype, 
			idatatype, nsx2, nsy2, xdeg2, ydeg2, 
			ascale2, bscale2, a02, b02, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_V, iscale_V, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_V, iopt, ipol, ifreqhm, ispare1,
			anodata_V, v_min_V, v_max_V, sensor, title, type_V, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, grd_vname,  val, "Tbstd_grd",
				 idatatype, nsx2, nsy2, xdeg2, ydeg2, 
				 ascale2, bscale2, a02, b02, 
				 ioff_V, iscale_V, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_V, iopt, ipol, ifreqhm, 
				 anodata_V, sensor, title, type_V, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing GRD Tb std (V) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* Grid Inc STD I grd image */
  ncerr=get_float_array_nc(ncid,"grd_i_image",val,&nsx,&nsy,&anodata_I); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_I,&amin,&amax);
    sprintf(title,"Grid Inc STD image of %s",regname);
    printf(" Writing Inc std (I) output GRD file '%s'\n", grd_iname);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_I);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,grd_iname,tstr), val, &nhead, nhtype, 
			idatatype, nsx2, nsy2, xdeg2, ydeg2,
			ascale2, bscale2, a02, b02, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_I, iscale_I, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_I, iopt, ipol, ifreqhm, ispare1,
			anodata_I, v_min_I, v_max_I, sensor, title, type_I, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, grd_iname, val, "Istd_grd",
				 idatatype, nsx2, nsy2, xdeg2, ydeg2,
				 ascale2, bscale2, a02, b02, 
				 ioff_I, iscale_I, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_I, iopt, ipol, ifreqhm, 
				 anodata_I, sensor, title, type_I, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing GRD Inc std (I) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }


  /* Grid Inc ave J grd image */
  ncerr=get_float_array_nc(ncid,"grd_j_image",val,&nsx,&nsy,&anodata_Ia); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {  
    image_minmax(val,nsx,nsy,anodata_Ia,&amin,&amax);        
    sprintf(title,"Grid Inc ave image of %s",regname);
    printf(" Writing Inc ave (J) output GRD file '%s'\n", grd_jname);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_Ia);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,grd_jname,tstr), val, &nhead, nhtype, 
			idatatype, nsx2, nsy2, xdeg2, ydeg2,
			ascale2, bscale2, a02, b02, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_Ia, iscale_Ia, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_Ia, iopt, ipol, ifreqhm, ispare1,
			anodata_Ia, v_min_Ia, v_max_Ia, sensor, title, type_Ia, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, grd_jname, val, "Iave_grd",
				 idatatype, nsx2, nsy2, xdeg2, ydeg2,
				 ascale2, bscale2, a02, b02, 
				 ioff_Ia, iscale_Ia, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_Ia, iopt, ipol, ifreqhm, 
				 anodata_Ia, sensor, title, type_Ia, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing GRD Inc ave (J) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }


  /* Grid Count C grd image */
  ncerr=get_float_array_nc(ncid,"grd_c_image",val,&nsx,&nsy,&anodata_C); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_C,&amin,&amax);          
    sprintf(title,"Grid Cnt image of %s",regname);
    printf(" Writing Count (C) output GRD file '%s'\n", grd_cname);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_C);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,grd_cname,tstr), val, &nhead, nhtype, 
			idatatype, nsx2, nsy2, xdeg2, ydeg2,
			ascale2, bscale2, a02, b02, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_C, iscale_C, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_C, iopt, ipol, ifreqhm, ispare1,
			anodata_C, v_min_C, v_max_C, sensor, title, type_C, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, grd_cname, val, "Cnt_grd",
				 idatatype, nsx2, nsy2, xdeg2, ydeg2,
				 ascale2, bscale2, a02, b02, 
				 ioff_C, iscale_C, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_C, iopt, ipol, ifreqhm, 
				 anodata_C, sensor, title, type_C, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing GRD Count (C) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  /* Grid pixel time P grd image */
  ncerr=get_float_array_nc(ncid,"grd_p_image",val,&nsx,&nsy,&anodata_P); check_err(ncerr, __LINE__,__FILE__);
  if (ncerr == 0) {
    image_minmax(val,nsx,nsy,anodata_P,&amin,&amax);          
    sprintf(title,"Grid Pixel time image of %s",regname);
    printf(" Writing Pixel time (P) output GRD file '%s'\n", grd_pname);
    printf("   image min/max: %f %f  %f\n",amin,amax,anodata_P);
    if (storage == 0) { /* SIR file format */
      ierr = write_sir3(addpath(outpath,grd_pname,tstr), val, &nhead, nhtype, 
			idatatype, nsx2, nsy2, xdeg2, ydeg2,
			ascale2, bscale2, a02, b02, 
			ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			ioff_P, iscale_P, iyear, isday, ismin, ieday, iemin, 
			iregion, itype_P, iopt, ipol, ifreqhm, ispare1,
			anodata_P, v_min_P, v_max_P, sensor, title, type_P, tag,
			crproc, crtime, descrip, ldes, iaopt, nia);
    } else {  /* netcdf file format */
      ierr = netcdf_write_single(outpath, grd_pname, val, "Ptime_grd",
				 idatatype, nsx2, nsy2, xdeg2, ydeg2,
				 ascale2, bscale2, a02, b02, 
				 ioff_P, iscale_P, iyear, isday, ismin, ieday, iemin, 
				 iregion, itype_P, iopt, ipol, ifreqhm, 
				 anodata_P, sensor, title, type_P, tag,
				 crproc, crtime); check_err(ierr, __LINE__,__FILE__);
    }
    if (ierr < 0) {
      printf("*** ERROR writing GRD Pixel time (P) output file ***\n");
      errors++;
    }
  } else {
    printf("*** ERROR reading dump file ***\n");
    errors++;
  }

  if (non_out) {
    /* non-enhanced images. These are pixel replicated grd images that exactly
       overlay the enhanced resolution images */
    
    /* Non-enhanced TB A non image */
    ncerr=get_float_array_nc(ncid,"non_a_image",val,&nsx,&nsy,&anodata_A); check_err(ncerr, __LINE__,__FILE__);
    if (ncerr == 0) {
      image_minmax(val,nsx,nsy,anodata_A,&amin,&amax);          
      sprintf(title,"Non-enhanced image of %s",regname);
      printf(" Writing Tb (A) output NON file '%s'\n", non_aname);
      printf("   image min/max: %f %f  %f\n",amin,amax,anodata_A);
      if (storage == 0) { /* SIR file format */
	ierr = write_sir3(addpath(outpath,non_aname,tstr), val, &nhead, nhtype, 
			  idatatype, nsx, nsy, xdeg, ydeg, 
			  ascale, bscale, a0, b0, 
			  ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			  ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
			  iregion, itype_A, iopt, ipol, ifreqhm, ispare1,
			  anodata_A, v_min_A, v_max_A, sensor, title, type_A, tag,
			  crproc, crtime, descrip, ldes, iaopt, nia);
      } else {  /* netcdf file format */
	ierr = netcdf_write_single(outpath, non_aname, val, "Tb_non",
				   idatatype, nsx, nsy, xdeg, ydeg, 
				   ascale, bscale, a0, b0, 
				   ioff_A, iscale_A, iyear, isday, ismin, ieday, iemin, 
				   iregion, itype_A, iopt, ipol, ifreqhm, 
				   anodata_A, sensor, title, type_A, tag,
				   crproc, crtime); check_err(ierr, __LINE__,__FILE__);
      }
      if (ierr < 0) {
	printf("*** ERROR writing Non-enhanced Tb (A) output file ***\n");
	errors++;
      }
    } else {
      printf("*** ERROR reading dump file ***\n");
      errors++;
    }

    /* Non-enhanced Tb STD V non image */
    ncerr=get_float_array_nc(ncid,"non_v_image",val,&nsx,&nsy,&anodata_V); check_err(ncerr, __LINE__,__FILE__);
    if (ncerr == 0) {
      image_minmax(val,nsx,nsy,anodata_V,&amin,&amax);          
      sprintf(title,"Non Tb STD image of %s",regname);
      printf(" Writing Tb std (V) output NON file '%s'\n", non_vname);
      printf("   image min/max: %f %f  %f\n",amin,amax,anodata_V);
      if (storage == 0) { /* SIR file format */
	ierr = write_sir3(addpath(outpath,grd_vname,tstr), val, &nhead, nhtype, 
			  idatatype, nsx, nsy, xdeg, ydeg, 
			  ascale, bscale, a0, b0, 
			  ixdeg_off, iydeg_off, ideg_sc, iscale_sc, ia0_off, ib0_off, i0_sc,
			  ioff_V, iscale_V, iyear, isday, ismin, ieday, iemin, 
			  iregion, itype_V, iopt, ipol, ifreqhm, ispare1,
			  anodata_V, v_min_V, v_max_V, sensor, title, type_V, tag,
			  crproc, crtime, descrip, ldes, iaopt, nia);
      } else {  /* netcdf file format */
	ierr = netcdf_write_single(outpath, grd_vname, val, "Tbstd_non",
				   idatatype, nsx, nsy, xdeg, ydeg, 
				   ascale, bscale, a0, b0, 
				   ioff_V, iscale_V, iyear, isday, ismin, ieday, iemin, 
				   iregion, itype_V, iopt, ipol, ifreqhm, 
				   anodata_V, sensor, title, type_V, tag,
				   crproc, crtime); check_err(ierr, __LINE__,__FILE__);
      }
      if (ierr < 0) {
	printf("*** ERROR writing Non-enhanced Tb std (V) output file ***\n");
	errors++;
      }
    } else {
      printf("*** ERROR reading dump file ***\n");
      errors++;
    }
  }
  
/* end of program */
  free(val);

  return(errors);
}


void no_trailing_blanks(char *s)
{  /* remove trailing blanks (spaces) from string */
  int n=strlen(s);
  
  while (n > 0) {
    if (s[n] != ' ' && s[n] != '\0') return;
    if (s[n] == ' ') s[n] = '\0';
    n--;
  }
  return;
}


  
