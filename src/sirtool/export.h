// export.h include file for exporting BMP and GIF file types
// 
// written 18 May 2002 by DGL at BYU + based on sir_ez
// 
// (c) 2002 by BYU MERS 


extern "C" {

int writegif(char *name_in, int *len_in, char *pic, int *pw, int *ph,
	     char *rmap, char *gmap, char *bmap, 
	     int *pnumcols, int *pcolorstyle);

int writebmp(char *name_in, char *image, int *nw, int *nh,
	     char *rmap, char *gmap, char *bmap, int *numcols);
}
