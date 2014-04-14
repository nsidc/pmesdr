/********************************************************************************

 code to do plotting to a byte image - used in conjunction w/ ll_nr_fp2.c

 written by D.G. Long Feb. 2000

 ********************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define TRUE_ (1)
#define FALSE_ (0)


/* pixel plotting stuff */

struct {
    int mt, nt, ist, ixwide, iywide, lx, ly, icol;
    float xl[2], yl[2];
    int is, ibnt;
} fpio_1;

struct {
  char  *a;
  int bytesperpixel;
  int color;
  int tab;
} fpimage_1;

static int c__1 = 1;

/**************************************************************************/

/* FLOATING ARRAY PLOTTING ROUTINES */
/* THIS CODE IS BASED ON THE REFLIB (RAMTEK EMULATION) PORTION OF LONGLIB */

/**************************************************************************/

/* Subroutine */ int fpinit_(nx, ny) /* INIT FLOATING ARRAY PLOTTING ROUTINES */
int *nx, *ny;
{
/* 	SET ARRAY SIZE */

    fpio_1.ixwide = *nx;  /* ARRAY X DIMENSION SIZE */
    fpio_1.iywide = *ny;  /* ARRAY Y DIMENSION SIZE */

/* 	INITIALIZE LINE TYPES */

    fpio_1.mt = -1; /* LINE BIT PATTERN SOLID */
    fpio_1.nt = 1; /* 1; */  /* LINE WIDTH */
    fpio_1.ist = 1; /* BIT PATTERN SCALE FACTOR */
    fpio_1.icol = 1; /* LINE COLOR */

/* 	INITIALIZE IMAGE MODE INFO */

    fpio_1.lx = 0; /* IMAGE MODE START X */
    fpio_1.ly = 0; /* IMAGE MODE START Y */
    fpio_1.xl[0] = (float)0.; /* LEFT DEFAULT WINDOW */
    fpio_1.yl[0] = (float)0.; /* UPPER DEFAULT WINDOW */
    fpio_1.xl[1] = (float) fpio_1.ixwide; /* RIGHT DEFAULT WINDOW */
    fpio_1.yl[1] = (float) (fpio_1.iywide - 1); /* LOWER DEFAULT WINDOW */
    fpio_1.is = 0; /* PIX SEQ: L-R, T-B */
    return 0;
} /* fpinit_ */



/* Subroutine */ int fpplot_(n, ip, col)
int *n, *ip, *col;
{
    /* Initialized data */

    static int mwp[7] = { 1,5,12,21,26,37,44 };
    static int mwx[44] = { 0,-1,1,1,-1,-1,0,2,1,0,0,-1,0,-1,-1,-1,0,0,1,1,
	    1,1,1,0,0,-1,-1,-1,-1,-1,-1,0,0,1,1,1,1,1,1,1,0,0,-1,-1 };
    static int mwy[44] = { 0,0,1,-1,-1,0,2,0,0,-1,-1,0,-1,0,0,1,1,1,1,0,0,
	    0,-1,-1,-1,-1,-1,0,0,1,1,1,1,1,1,0,0,0,-1,-1,-1,-1,-1,-1 };

    /* System generated locals */
    int i__1, i__2;

    /* Local variables */
    static int i;
    extern /* Subroutine */ int fpout_();
    static int iw, lw, ix1, iy1, ix2, iy2;


/* 	PLOT A LINE OF CONNECTED VECTORS */
/* 	INPUTS: */
/* 	N	(I)	NUMBER OF VECTOR PAIRS */
/* 	IP	(I)	ARRAY OF X,Y LOCATIONS OF POINTS */
/* 	COL	(R)	PLOTTING VALUE [COLOR] */

    /* Parameter adjustments */
    --ip;

    fpio_1.ibnt = 0;
    if (*n <= 0) {
	return 0;
    }

/* CONVERT A STRING OF VECTORS INTO INDIVIDUAL VECTORS */

    if (*n == 1) {
	ix1 = ip[1];
	iy1 = ip[2];
	lw = mwp[fpio_1.nt - 1];
	i__1 = lw;
	for (iw = 1; iw <= i__1; ++iw) {
	    ix1 += mwx[iw - 1];
	    iy1 += mwy[iw - 1];
	    ix2 = ix1;
	    iy2 = iy1;
	    fpout_(&ix1, &iy1, &ix2, &iy2, col);
/* L10: */
	}
    } else {
	i__1 = *n << 1;
	for (i = 3; i <= i__1; i += 2) {
	    ix1 = ip[i - 2];
	    iy1 = ip[i - 1];
	    ix2 = ip[i];
	    iy2 = ip[i + 1];
	    lw = mwp[fpio_1.nt - 1];
	    i__2 = lw;
	    for (iw = 1; iw <= i__2; ++iw) {
		ix1 += mwx[iw - 1];
		iy1 += mwy[iw - 1];
		ix2 += mwx[iw - 1];
		iy2 += mwy[iw - 1];
		fpout_(&ix1, &iy1, &ix2, &iy2, col);
/* L50: */
	    }
/* L100: */
	}
    }
    return 0;
} /* fpplot_ */



/* Subroutine */ int fpout_(ix1, iy1, ix2, iy2, col)
int *ix1, *iy1, *ix2, *iy2, *col;
{
    int i__1;

    static int isst, b[16], d, i, x, y;
    extern int ibits_();
    static int i1, i2, x1, y1, x2, y2, dx, dy, dif, ibt;
    static int inv;
    static int ixw, iyy;

    /* printf("fpout_ %d %d %d %d %d %d\n",*ix1,*iy1,*ix2,*iy2,*col,fpimage_1.bytesperpixel); */
    

/* 	CONVERT A VECTOR PLOT INTO RASTER SCAN */

/* 	IX1,IY1	(I)	START POINT OF LINE SEGMENT */
/* 	IX2,IY2	(I)	START POINT OF LINE SEGMENT */
/* 	COL	(I)	VALUE (COLOR) */

    for (i = 1; i <= 16; ++i) { /* 	BIT MAP FOR LINE TYPE */
	i__1 = i - 1;
	b[i - 1] = ibits_(&fpio_1.mt, &i__1, &c__1);
    }

    /*    if (*col > 255) *col=255; */
    if (*col < 0) *col=0;

    dx = (i__1 = *ix1 - *ix2, abs(i__1));
    dy = (i__1 = *iy1 - *iy2, abs(i__1));
    x1 = *ix1;
    x2 = *ix2;
    y1 = *iy1;
    y2 = *iy2;
    if (x1 < 0) {
	x1 = 0;
    }
    if (x2 < 0) {
	x2 = 0;
    }
    if (y1 < 0) {
	y1 = 0;
    }
    if (y2 < 0) {
	y2 = 0;
    }
    if (x1 > fpio_1.ixwide) {
	x1 = fpio_1.ixwide;
    }
    if (x2 > fpio_1.ixwide) {
	x2 = fpio_1.ixwide;
    }
    if (y1 >= fpio_1.iywide) {
	y1 = fpio_1.iywide - 1;
    }
    if (y2 >= fpio_1.iywide) {
	y2 = fpio_1.iywide - 1;
    }
    inv = FALSE_;
    if (dx < dy) {
	x1 = *iy1;
	y1 = *ix1;
	x2 = *iy2;
	y2 = *ix2;
	x = dy;
	dy = dx;
	dx = x;
	inv = TRUE_;
    }
    if (x2 < x1) {
	x = x1;
	x1 = x2;
	x2 = x;
	y = y1;
	y1 = y2;
	y2 = y;
    }
    dif = 1;
    if (y2 < y1) {
	dif = -1;
    }
    d = (dy << 1) - dx;
    i1 = dy << 1;
    i2 = dy - dx << 1;
    y = y1;
    iyy = 0;

    ixw = fpio_1.ixwide;
    isst = fpio_1.ist << 4;
    i__1 = x2;
    for (x = x1; x <= i__1; ++x) {
	++fpio_1.ibnt;
	ibt = fpio_1.ibnt / fpio_1.ist % 16 + 1;
	fpio_1.ibnt %= isst;
	if (inv) {
	    if (b[ibt - 1] != 0 || 1==1) {
	      switch (fpimage_1.bytesperpixel) {
	      case 1:
		fpimage_1.a[(x + iyy) * ixw + y] = *col;
		break;
	      case 2:
		*((short int *) (fpimage_1.a+fpimage_1.bytesperpixel*((x + iyy) * ixw + y))) = *col;
		break;
	      case 4:
		*((int *) (fpimage_1.a+fpimage_1.bytesperpixel*((x + iyy) * ixw + y))) = fpimage_1.color;
		break;
	      }	
	    }
	} else {
	    if (b[ibt - 1] != 0 || 1==1) {
	      switch (fpimage_1.bytesperpixel) {
	      case 1:
		fpimage_1.a[x + (y + iyy) * ixw] = *col;
		break;
	      case 2:
		*((short int *) (fpimage_1.a+fpimage_1.bytesperpixel*(x + (y + iyy) * ixw))) = *col;
		break;
	      case 4:
		*((int *) (fpimage_1.a+fpimage_1.bytesperpixel*(x + (y + iyy) * ixw))) = fpimage_1.color;
		break;
	      }
	    }
	}
	if (d < 0) {
	    d += i1;
	} else {
	    d += i2;
	    y += dif;
	}
/* L10: */
    }
    return 0;
} /* fpout_ */



/* Subroutine */ int fptexture_(itext, iwide, isize)
int *itext, *iwide, *isize;
{
    static int ilt[16] = { -1,21845,13311,16191,8191,13119,16359,255,
	    23485,13107,7295,15567,22015,3855,24383,23485 };
/* 	LINE TYPE BIT MAPS (16 BITS) */

    static int it;

/* 	CHANGE PLOT LINE TEXTURE AND SCALING */

/* 	INPUTS: */

/* 	ITEXT	(I)	LINE TYPE (0-15) */
/* 	IWIDE	(I)	LINE WIDTH (FOR RMPLOT) */
/* 	ISIZE	(I)	PIXEL SCALING */

    it = *itext + 1;
    if (it < 1) {
	it = 1;
    }
    if (it > 16) {
	it = it % 16 + 1;
    }
    fpio_1.mt = ilt[it - 1];
    fpio_1.nt = *iwide;
    if (fpio_1.nt < 1) {
	fpio_1.nt = 1;
    }
    if (fpio_1.nt > 7) {
	fpio_1.nt = 7;
    }
    fpio_1.ist = *isize;
    if (fpio_1.ist < 1) {
	fpio_1.ist = 1;
    }
    if (fpio_1.ist > 16) {
	fpio_1.ist = 16;
    }
    return 0;
} /* fptexture_ */



/* Subroutine */ int fpoutin_(cmd, nc, ird)
float *cmd;
int *nc, *ird;
{
    int i__1;

    static int index, ib, ix, iy, jx, jy, nbb;

/* 	READ/WRITE DATA IN IMAGE MODE */

/* 	INPUTS: */

/* 	CMD	(R)	ARRAY OF DATA TO READ/WRITE */
/* 	NC	(I)	NUMBER OF VALUES */
/* 	IRD	(I)	READ/WRITE CODE */
/* 			 0=READ */
/* 			 1=WRITE */

    /* Parameter adjustments */
    --cmd;

    /* Function Body */
    if (*nc <= 0) {
	return 0;
    }

    ix = 1;
    iy = 0;
    jx = 0;
    jy = 1;
    switch ((int)(fpio_1.is + 1)) {
	case 1:  goto L10;
	case 2:  goto L11;
	case 3:  goto L12;
	case 4:  goto L13;
	case 5:  goto L14;
	case 6:  goto L15;
	case 7:  goto L16;
	case 8:  goto L17;
    }
L10:
    goto L20;
/* "NORMAL" L-R,T-B */
L11:
    ix = -1;
/* R-L, T-B */
    goto L20;
L12:
    jy = -1;
/* L-R, B-T */
    goto L20;
L13:
    ix = -1;
/* R-L, B-T */
    iy = -1;
    goto L20;
L14:
    ix = 0;
/* T-B, L-R */
    iy = 1;
    jx = 1;
    jy = 0;
    goto L20;
L15:
    ix = 0;
/* B-T, L-R */
    iy = -1;
    jx = 1;
    jy = 0;
    goto L20;
L16:
    ix = 0;
/* T-B, R-L */
    iy = 1;
    jx = -1;
    jy = 0;
    goto L20;
L17:
    ix = 0;
/* B-T, R-L */
    iy = -1;
    jx = -1;
    jy = 0;
    goto L20;

L20:
    nbb = *nc;
    i__1 = nbb;
    if (fpimage_1.bytesperpixel > 1) printf("*** fpoutin only supports byte images ***\n");
    for (ib = 1; ib <= i__1; ++ib) {
	index = fpio_1.lx + fpio_1.ly * (fpio_1.ixwide + 1);
	if (*ird == 1) {
	    fpimage_1.a[index] = cmd[ib];
	} else {
	    cmd[ib] = fpimage_1.a[index];
	}
	fpio_1.lx += ix;
	fpio_1.ly += iy;
L65:
	if ((float) fpio_1.lx < fpio_1.xl[0]) {
	    fpio_1.lx = fpio_1.xl[1];
	    fpio_1.ly += jy;
	}
	if ((float) fpio_1.lx > fpio_1.xl[1]) {
	    fpio_1.lx = fpio_1.xl[0];
	    fpio_1.ly += jy;
	}
	if ((float) fpio_1.ly < fpio_1.yl[0]) {
	    fpio_1.ly = fpio_1.yl[1];
	    fpio_1.lx += jx;
	    goto L65;
	}
	if ((float) fpio_1.ly > fpio_1.yl[1]) {
	    fpio_1.ly = fpio_1.yl[0];
	    fpio_1.lx += jx;
	    goto L65;
	}
/* L100: */
    }

    return 0;
} /* fpoutin_ */


/* Subroutine */ int writefp_(v, n, ierr)
float *v;
int *n, *ierr;
{
    extern /* Subroutine */ int fpoutin_();

/* 	WRITE FLOAT WORDS IN IMAGE MODE */

/* 	INPUTS: */

/* 	V	(R)	ARRAY OF IMAGE DATA TO WRITE */
/* 	N	(I)	NUMBER OF WORDS TO WRITE */

    /* Parameter adjustments */
    --v;

    /* Function Body */
    fpoutin_(&v[1], n, &c__1);
    return 0;
} /* writefp_ */


/* Subroutine */ int fpstart_(ix, iy)
int *ix, *iy;
{
/* 	SET START OF IMAGE WRITE (COP) LOCATION TO (IX,IY) */
/* 	INPUTS: */
/* 	IX,IY	(I)	PIXEL LOCATION */


    fpio_1.lx = *ix;
    if ((float) fpio_1.lx < fpio_1.xl[0]) {
	fpio_1.lx = fpio_1.xl[0];
    }
    if ((float) fpio_1.lx > fpio_1.xl[1]) {
	fpio_1.lx = fpio_1.xl[1];
    }
    fpio_1.ly = *iy;
    if ((float) fpio_1.ly < fpio_1.yl[0]) {
	fpio_1.ly = fpio_1.yl[0];
    }
    if ((float) fpio_1.ly > fpio_1.yl[1]) {
	fpio_1.ly = fpio_1.yl[1];
    }

    return 0;
} /* fpstart_ */


/* Subroutine */ int fpwind_(ix, iy, ixm, iym)
int *ix, *iy, *ixm, *iym;
{

/* 	SET IMAGE MODE WINDOW */

/* 	INPUTS: */

/* 	IX,IY	(I)	UPPER-LEFT (MINIMUM) CORNER */
/* 	IXM,IYM	(I)	LOWER-RIGHT (MAXIMUM) CORNER */

/* 	NOTE: MINIMUM WINDOW SIZE ALLOWED IS 2X2 PIXELS */

    fpio_1.xl[0] = (float) (*ix);
    if (fpio_1.xl[0] < (float)0.) {
	fpio_1.xl[0] = (float)0.;
    }
    if (fpio_1.xl[0] > (float) fpio_1.ixwide) {
	fpio_1.xl[0] = (float) fpio_1.ixwide;
    }
    fpio_1.yl[0] = (float) (*iy);
    if (fpio_1.yl[0] < (float)0.) {
	fpio_1.yl[0] = (float)0.;
    }
    if (fpio_1.yl[0] > (float) fpio_1.iywide) {
	fpio_1.yl[0] = (float) fpio_1.iywide;
    }
    fpio_1.xl[1] = (float) (*ixm);
    if (fpio_1.xl[1] < (float)0.) {
	fpio_1.xl[1] = (float)0.;
    }
    if (fpio_1.xl[1] > (float) fpio_1.ixwide) {
	fpio_1.xl[1] = (float) fpio_1.ixwide;
    }
    fpio_1.yl[1] = (float) (*iym);
    if (fpio_1.yl[1] < (float)0.) {
	fpio_1.yl[1] = (float)0.;
    }
    if (fpio_1.yl[1] > (float) fpio_1.iywide) {
	fpio_1.yl[1] = (float) fpio_1.iywide;
    }
    if (fpio_1.xl[1] <= fpio_1.xl[0]) {
	fpio_1.xl[1] = fpio_1.xl[0] + 1;
    }
    if (fpio_1.yl[1] <= fpio_1.yl[0]) {
	fpio_1.yl[1] = fpio_1.yl[0] + 1;
    }
    fpio_1.lx = fpio_1.xl[0];
    fpio_1.ly = fpio_1.yl[0];

    return 0;
} /* fpwind_ */


/* Subroutine */ int fpdir_(ichan, iseq, ierr)
int *ichan, *iseq, *ierr;
{

/* 	SET SCAN SEQUENCE FOR IMAGE WRITING */

/* 	INPUTS: */

/* 	ICHAN	(I)	CHANNEL */
/* 	ISEQ	(I)	SCAN CODE */
/* 		    PIX-TO-PIX  LINE-TO-LINE */
/* 	ISEQ	0	L-R	   T-B */
/* 		1	R-L	   T-B */
/* 		2	L-R	   B-T */
/* 		3	R-L	   B-T */
/* 		4	T-B	   L-R */
/* 		5	B-T	   L-R */
/* 		6	T-B	   R-L */
/* 		7	B-T	   R-L */

/* 	OUTPUTS: */

/* 	IERR	(I)	STATUS ERROR */


    if (*iseq >= 0 && *iseq < 8) {
	fpio_1.is = *iseq;
    } else {
	*ierr = -1;
    }

    return 0;
} /* fpdir_ */



/* Subroutine */ int fppix_(ix, iy, val, irdwr)
int *ix, *iy;
int *val;
int *irdwr;
{
    static int index;


/* 	READ/WRITE A SINGLE PIXEL VALUE */

/* 	INPUTS: */

/* 	IX,IY	(I)	LOCATION */
/* 	VAL	(I)	VALUE (IN/OUT) */
/* 	IRDWR	(I)	=0 READS PIXEL VALUE */
/* 			=1 WRITE PIXEL VALUE */
/* 	IERR	(I)	NOT USED */


    if (*ix < 1 || *ix > fpio_1.ixwide) {
	return 0;
    }
    if (*iy < 1 || *iy > fpio_1.iywide) {
	return 0;
    }
    index = *ix + (*iy - 1) * fpio_1.ixwide;
    if (*irdwr == 1) {
      switch(fpimage_1.bytesperpixel) {
      case 1:
	fpimage_1.a[index] = *val;
	break;
      case 2:
	*((short int *) (fpimage_1.a + index * fpimage_1.bytesperpixel)) = *val;
	break;
      case 4:
	*((int *) (fpimage_1.a + index * fpimage_1.bytesperpixel)) = *val;
	break;
      }
      
    } else {
      switch(fpimage_1.bytesperpixel) {
      case 1:
	*val = fpimage_1.a[index];
	break;
      case 2:
	*val = *((short int *) (fpimage_1.a + index * fpimage_1.bytesperpixel));
	break;
      case 4:
	*val = *((int *) (fpimage_1.a + index * fpimage_1.bytesperpixel));
	break;
      }
    }
    
    return 0;
} /* fppix_ */


int fpimage_color(int c_opt, char *red, char *blue, char *green)
{
  switch(fpimage_1.bytesperpixel) {  
  case 1:  /* 8 bits */
    fpimage_1.color = c_opt;
    break;
  case 2:  /* 16 bits */
    fpimage_1.color = c_opt;
    break;
  case 4:  /* 24 or 32 bits */
    fpimage_1.color = c_opt;
    if (fpimage_1.tab == 0) 
      fpimage_1.color = 
	blue[c_opt]*256*256 +
	green[c_opt]*256 +
	red[c_opt]; 
    break;
  }
  return(fpimage_1.color);
}
