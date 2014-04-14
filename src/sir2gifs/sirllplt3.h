/* define templates for routines from sir image plotting library */
/* revised to support sirllplt3.c by DGL Feb. 28, 2012 */

#define NMAX 5  /* max number of input files */

int get_input_files(char **argv, int narg, char *base, char *names[]);
void print_options_info(FILE *omf);
void make_byte_image(char *data, float *stval[], float *smin, float *smax, 
		     int nsx, int nsy, int nchans,
		     int s_opt, int e_opt, int *nc_opt, int z_opt, 
		     float *anodata, int debug);
void make_byte_image2(char *data, float *stval[], float *smin, float *smax, 
		      int nsx, int nsy, int nchans,
		      int s_opt, int e_opt, int *nc_opt, int z_opt, 
		      float *anodata, int debug,
		      int SF, int dsize, char red[], char green[], char blue[]);
int plot_region_resize(float anodata[], float *stval[], int nchans, int npix, 
		       int *nsx, int *nsy, float *ascale, float *bscale, 
		       int iopt, float *xdeg, float *ydeg, float *a0, float *b0);
int plot_pass_one(char **argv);
int plot_pass_two(char **argv, char *data, float *stval, int nsx, int nsy);
void plot_initialize(char *rtab, char *gtab, char *btab, int *ncol, int nchans);
void rotate_image(int rot, char *data, int *nsx, int *nsy);


/*  global variables from SIR file header used in location transformations*/

int nsx, nsy;
float xdeg, ydeg, ascale, bscale, a0, b0;  /* SIR header location info */
int iopt;

extern char rtab[256], gtab[256], btab[256]; /* color table */
extern int debug;

float *in_stval[NMAX];    /* input image storage pointer */
float in_anodata[NMAX];   /* input image nodata values */


char *names[NMAX];


#include "preprocess.h"

float *stval[OMAX];        /* output image storage pointer */
int nchans;                /* number of output channels */
float out_anodata[OMAX];   /* output image nodata values */
char *command_lines[OMAX]; /* preprocess command lines */

rpn_entry rpn_list[OMAX][DEPTH];  /* preprocessing command list */
int rpn_depth[OMAX];



/* pixel plotting variables */

extern float x1_opt,y1_opt,x2_opt,y2_opt,d_opt,a_opt,t_opt;
extern int z_opt,s_opt,e_opt,i_opt,c_opt,w_opt,latlon_opt,
  subregion,Ii_opt,D_opt,n_opt,M_opt,nc_opt[OMAX];
extern char r_opt,g_opt,b_opt;
extern char m_opt[200], *f_opt;
extern int npix, ncol, rot_opt, chan_opt;

extern float smin[OMAX], smax[OMAX];    /* image scale range */


