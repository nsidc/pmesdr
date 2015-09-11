/* ezdump include file

This include file defines an easy programing C interface for the 
measures intermediate dump file format.  A data type which stores 
the key header information is defined to permit easy transfer of
dump file header information 

written  9 Jun 2014 by DGL at BYU

*/
#include <stdio.h>

typedef struct
{
  char fname[FILENAME_MAX];
  int ncid;
  int nsx, nsy, iopt;
  float ascale, bscale, a0, b0, xdeg, ydeg;
  int isday, ieday, ismin, iemin, iyear, iregion, ipol;
  int nsx2, nsy2, non_size_x, non_size_y;
  float ascale2, bscale2;
  float a02, b02, xdeg2, ydeg2;
  float a_init;
  int ibeam, nits, median_flag, nout;  
} eznc_head;


/* function prototypes for netcdf_dump.c */

void check_err(const int stat, const int line, const char *file);

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
   
int add_string_nc(int ncid, char *name, char *str, int maxc);

int add_float_array_nc(int ncid, char *name, float *val, int nsx, int nsy, float anodata);

int nc_close_file(int ncid);

int nc_open_file_write_head(int ncid, int nsx, int nsy, int iopt, 
			    float ascale, float bscale, float a0, float b0, float xdeg, float ydeg, 
			    int isday, int ieday, int ismin, int iemin, int iyear, int iregion, int ipol, 
			    int nsx2, int nsy2, int non_size_x, int non_size_y, float ascale2, float bscale2, 
			    float a02, float b02, float xdeg2, float ydeg2,
			    float a_init, int ibeam, int nits, int median_flag, int nout);

int ez_nc_open_file_read_head(char *filename, int *ncid, eznc_head *dhead);

int ez_copy_head(eznc_head *in, eznc_head *out);

int copy_string_nc(int ncid_in, int ncid_out, char *name, char *str, int maxc);

int copy_float_array_nc(int ncid_in, int ncid_out, char *name, float *val, int *nsx, int *nsy, float *anodata);

