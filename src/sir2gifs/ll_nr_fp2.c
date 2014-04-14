
/* *** LAST REVISED ON 13-OCT-1994 06:59:25.96 */
/* *** SOURCE FILE: [LONGLIB93.SOURCES.C.LONGLIB]LL_NR.C */
/* updated to include stdlib to define exit dgl  2 Apr 2011 */

#include <stdio.h>
#include <stdlib.h>

typedef int integer;  /* long int */
typedef float real;
typedef int logical;  /* long int */
typedef short int shortint;
typedef double doublereal;

#define abs(x) ((x) >= 0 ? (x) : -(x))
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define dabs(x) (doublereal)abs(x)

#define TRUE_ (1)
#define FALSE_ (0)


/* Common Block Declarations */

typedef struct {		/* meta file */
    integer lu, mp;
    real re[2], ox, oy, sf, px, py;
    integer mx, mw, mcol, msc;
    real pang, pvp[4];
} pxpcom_common;

typedef struct {        /*This structure is used by the ramxlib routines*/
    integer ichan, imr[128];
    real rre[2], rox, roy, rsf;
    integer nimr;
    real rang;
    integer mm, ipw, ipsc, icol;
    real rvp[4];
    integer irid;
    real rlim[2];
    integer irxlim, irylim, iddev;
} rmtek_common;

typedef struct {			/* terminal graphics */
    integer ivt100, imv[32];
    real vre[2], vox, voy, vsf;
    integer nimv;
    real vang;
    integer iterm;
    real vvp[4];
    integer mv, mvw, ivcol, ixlim, iylim, iyrev;
    real vlim[2];
    integer imode;
} vt100_common;


static struct {			/* last plot location */
    real xlast, ylast;
    integer i0last;
} lstplt_;

#define lstplt_1 lstplt_

static struct {			/* asynchronous interupts */
    integer iastfl, iastpl;
} astc_;

#define astc_1 astc_


/* allocate key common blocks */

pxpcom_common pxpcom_;
#define pxpcom_1 pxpcom_

rmtek_common rmtek_;
#define rmtek_1 rmtek_

vt100_common vt100_;
#define vt100_1 vt100_

int nb = 1024;  /* This variable is used by the ramxlib routines */
FILE *met_file;			/* meta file storage */
int met_file_open = 0;


#define MAXSTACK 6
static pxpcom_common pxpcom_store[MAXSTACK];
static rmtek_common rmtek_store[MAXSTACK];
static vt100_common vt100_store[MAXSTACK];
static integer pxpcom_ns = -1;
static integer rmtek_ns = -1;
static integer vt100_ns = -1;


/* Table of constant values */

static integer c__0 = 0;
static integer c__1 = 1;
static integer c__2 = 2;
static integer c__3 = 3;
static integer c__4 = 4;
static integer c__5 = 5;
static integer c__8 = 8;
static integer c__10 = 10;
static integer c__999 = 999;
static integer c__1000 = 1000;
static integer c__1001 = 1001;
static integer c__1003 = 1003;
static integer c__1002 = 1002;
static integer c__2000 = 2000;
static integer c__2001 = 2001;
static integer c__2002 = 2002;
static integer c__2010 = 2010;
static integer c__2011 = 2011;
static integer c__2012 = 2012;
static integer c_n1 = -1;
static integer c_n4 = -4;
static real c_b19 = (float)0.;
static real c_b21 = (float)1.;
static real c_b24 = (float)999.;
static real c_b27 = (float)-1.;
static real c_b391 = (float)-999.;


/***************************************************************************/
/* 	LONGLIB.FOR SOURCE CODE ROUTINES FOR THE PLOTTING LIBRARY */
/* 	ONLY VECTOR PLOTTING IS SUPPORTED. */

/* 	THIS FILE CONSITS OF THE FIRST SECTION OF THE MAIN PLOT ROUTINES. */
/* 	THIS FILE CONSISTS OF THE DEVICE DEPENDENT ROUTINES. */
/* 	THE RAMTEK COMMUNICATIONS ROUTINES ARE IN A SEPARATE FILE. */
/* 	DEVICE DEPENDENT ROUTINES ARE ALSO CONTAINED IN THE CURSORLIB FILE. */

/* 	THIS FILE REFLECTS VERSION 7 OF THE LONGLIB GRAPHICS LIBRARY */
/*	TRANSLATED FROM FORTRAN BY DGL, DEC 1993 */

/***************************************************************************/

/* Subroutine */ int frame_(ipl, id, vpx, vpy, zom)
integer *ipl, *id;
real *vpx, *vpy, *zom;
{
    /* System generated locals */
    integer i__1;
    real r__1;

    /* Local variables */
    static integer iddn, ians;
    static real z;
    extern /* Subroutine */ int enableast_(), pplots_(), rplots_(), vplots_();

    static integer idd;
    static char ans;
    static integer kpl;

/* 	ROUTINE TO INTIALIZE THE LONGLIB GRAPHICS PLOT PACKAGE */

/* 	IPL	LOGICAL DEVICE NUMBER FOR PRINTER DATA FILE (NORMALLY 3) */
/* 		> 0 FORTRAN FILE TYPE NUMBER */
/* 		= 0 ASK IF PRINT FILE DESIRED (DEFAULT FILE 3) */
/* 		< 0 DO NOT CREATE PRINTER FILE */
/* 	ID	DEVICE TYPE */
/* 		< 0 DO NOT CLEAR SCREEN OF SCREEN DEVICE */
/* 		= 0 ASK WHICH SCREEN DEVICE TO USE (? FOR HELP) */
/* 		= 1 USE VT100 TERMINAL EQUIPPED WITH SELNAR GR100+ */
/* 		= 2 USE RAMTEK (LARGE 1280x1024) ONLY */
/* 		= 3 USE BOTH LARGE RAMTEK AND VT100 WITH SELNAR GR100 OR GR100+ */
/* 		= 4 USE NO SCREEN DEVICE */
/* 		= 5 USE VT125 TERMINAL IN TEKTRONICS 4010 MODE */
/* 		= 6 USE VT100 TERMINAL WITH SELNAR GR100 IN TEKTRONICS 4010 MODE */
/* 		= 7 USE VT100 TERMINAL WITH SELNAR GR480 */
/* 		= 8 USE VT240 TERMINAL (TEK 4010 MODE, VT100 MODE) */
/* 		= 9 USE VT220 TERMINAL WITH SELNAR SG220 (TEK 4014) */
/* 		= 10 USE TEKTRONICS 4010 TERMINAL */
/* 		= 11 USE TEKTRONICS 4107 COLOR TERMINAL */
/* 		= 12 USE GRAPHON GO-235 TERMINAL */
/* 		> 50 RAMTEK AND TERMINAL.  TERMINAL CODE=MOD(ID,50) */
/* 	VPX,VPY COORDINATES OF BOTTOM LEFT ORIGIN */
/* 	ZOM	ZOOM FACTOR */
/* 		< 0 USE SMALL RAMTEK SCREEN */

    enableast_();			/* ENABLE CONTROL-C INTERRUPT CODE */

    kpl = *ipl;
    z = *zom;
    if (kpl == 0) {
L16:
	printf("Create Longlib Meta File? (Y/N) [N] ");
	ans = getch();
	if (ans == EOF) {
	    goto L99;
	}
	ians = ans;
	kpl = -3;
	if (ians == 89 || ians == 121) {
	    kpl = 3;
	}
	if (ians == 63) {
	    goto L16;
	}
    }
    pxpcom_1.lu = -1;
    if (kpl > 0) {			/* INTIALIZE PRINTER */
	r__1 = dabs(z);
	pplots_(&kpl, vpx, vpy, &r__1);
    }
    idd = abs(*id);			
    iddn = - (*id >= 0 ? 1 : -1);	/* SET CLEAR SCREEN FLAG */
    if (idd == 0) {
	iddn = -1;			/* CLEAR SCREEN WHEN PROMPTING */
L5:
	printf("Longlib Screen Graphics Device? [?=Help] ");
	ans = getch();
	if (ans == EOF)
	    goto L99;

/* 	CONVERT INPUT CHARACTER INTO UPPER CASE ASCII VALUE */
	ians = ans;
	if (ians > 96)
	    ians += -32;

	if (ians == 63) {	/* 	INPUT WAS A QUESTION MARK? */
	    printf("Available Screen Devices:\n    R : 1024 Ramtek\n");
	    printf("    r : 512 Ramtek\n    V : VT100 w/Selnar Gr100\n");
	    printf("    F : VT240\n    D : VT220 w/Selnar Sg220\n");
	    printf("    E : Tektronics 4010/14\n    A : Tektronics 4107/109\n");
	    printf("    G : Graphon G)-235\n    X : Xterm\n");
	    printf("    T : VT125\n");
	    printf("    B : Big Ramtek\n    L : Little Ramtek\n");
	    printf("    M : Minature Ramtek\n    P : Pico Ramtek\n");
	    printf("    Z : Exit\n    N : None [default]\n");
	    goto L5;
	}
	idd = 4;			/* DEFAULT = NO SCREEN DEVICE */
/*	idd = 13;			DEFAULT = xterm */

/* 	DECODE TERMINAL SCREEN OPTION */

	if (ians == 90) 		/* END PROGRAM (Z) */
	    goto L99;
	if (ians == 82 || ians == 50 || ians == 51) {
	    idd = 2;			/* LARGE RAMTEK */
	    if (ans == 114) 	        /* SMALL RAMTEK */
		z = -(doublereal)dabs(z);
	}
	if (ians == 84) {
	    idd = 5;
	}				/* VT125 4010 (T) */
	if (ians == 83) {
	    idd = 6;
	}				/* SELNAR GR100 (S) */
	if (ians == 86) {
	    idd = 6;
	}				/* SELNAR GR100 (V) */
	if (ians == 75) {
	    idd = 6;
	}				/* SELNAR GR100 (K) */
	if (ians == 70) {
	    idd = 8;
	}				/* VT240 (4010) (F) */
	if (ians == 68) {
	    idd = 9;
	}				/* VT220 (4010) (D) */
	if (ians == 69) {
	    idd = 10;
	}				/* Tektronics 4010/4014 (E) */
	if (ians == 65) {
	    idd = 11;
	}				/* Tektronics 4107/4109 (A) */
	if (ians == 71) {
	    idd = 12;
	}				/* GRAPHON GO-235 (ENHANCED) (G) */
	if (ians == 88) {
	    idd = 13;
	}				/* XTERM PROGRAM WITH TEKTRONICS (X) */
	if (ians == 72) {
	    idd = 51;
	}				/* HUGE RAMTEK (H) */
	if (ians == 76) {
	    idd = 52;
	}				/* LITTLE RAMTEK (L) */
	if (ians == 66) {
	    idd = 54;
	}				/* BIG RAMTEK (B) */
	if (ians == 77) {
	    idd = 53;
	}				/* MINATURE RAMTEK (M) */
	if (ians == 80) {
	    idd = 55;
	}				/* PICO RAMTEK (P) */
    }

    rmtek_1.ichan = -1;			/* DISABLE RAMTEK BY DEFAULT */
    vt100_1.ivt100 = -1;		/* DISABLE TERMINAL BY DEFAULT */

    if (idd != 2 && idd != 4 && idd < 50) {
	i__1 = idd * iddn;
	vplots_(&i__1, vpx, vpy, &z);	/* INITALIZE VT100 */
    }

    if (idd == 2 || idd == 3 || idd > 50) {
	if (idd == 2) {
	    idd = 1;
	}
	if (idd == 3) {
	    idd = 1;
	}
	if (idd > 50) {
	    idd += -50;
	}
	i__1 = idd * iddn;
	rplots_(&i__1, vpx, vpy, &z);		/* INTIALIZE RAMTEK */
    }
    return 0;
L99:

/* 	DO IMMEDIATE EXIT FROM PROGRAM */
    exit(0);
    return 0;
} /* frame_ */



/* Subroutine */ int plots_(i, j)
integer *i, *j;
{
    extern /* Subroutine */ int frame_();


/* 	THIS SUBROUTINE PROVIDES PLOTS-10 COMPATIBILITY */
/* 	ARGUMENTS ARE IGNORED.  CALL FRAME WITH PROMPT FOR */
/* 	HARDCOPY AND GRAPHICS SCREEN OUTPUTS. */

    frame_(&c__0, &c__0, &c_b19, &c_b19, &c_b21);
    return 0;
} /* plots_ */



/* Subroutine */ int vplots_(id, vpx, vpy, zom)
integer *id;
real *vpx, *vpy, *zom;
{
      vt100_1.ivt100 = -1; /* DISABLE TERMINAL PACKAGE */
      return 0;
}

/* Subroutine */ int vinit_(nsx0, nsy0)
integer *nsx0, *nsy0;
{
    extern /* Subroutine */ int fpinit_(), plotvt_(), newvcol_(), newvpen_();

    fpinit_(nsx0, nsy0);

    vt100_1.nimv = 1; /* RESET VECTOR BUFFTER */
    vt100_1.imv[0] = 0;     /* ORIGIN VECTOR BUFFER */
    vt100_1.imv[1] = 0; /* ORIGIN VECTOR BUFFER */
    vt100_1.vox = (float)0.; /* ORIGIN */
    vt100_1.voy = (float)0.; /* ORIGIN */
    vt100_1.vsf = (float)1.; /* SCALE FACTOR */
    vt100_1.vang = (float)0.;
    vt100_1.iterm = 44;
    vt100_1.vre[0] = (float)1.;
    vt100_1.vre[1] = (float)1.;
    vt100_1.vlim[0] = (real) (*nsx0 - 1);
    vt100_1.vlim[1] = (real) (*nsy0 - 1);
    vt100_1.ixlim = vt100_1.vlim[0];
    vt100_1.iylim = vt100_1.vlim[1];
    vt100_1.iyrev = 1; /* DO NOT INVERT Y DIMENSION COORDINATE IF =0 */
    vt100_1.ivt100 = 2; /* ENABLE TERMINAL GR OUTPUT */
    vt100_1.vvp[0] = (float)0.; /* VIEW PORT PARAMETERS LOWER-LEFT */
    vt100_1.vvp[1] = (float)0.;
    plotvt_(&c_b24, &c_b24, &c__4); /* SET RIGHT VIEWPORT PARAMETERS UPPER-RIGHT */
    newvpen_(&c__0, &c__1); /* SET DEFAULT PEN TYPE,WIDTH */
    newvcol_(&c_n1); /* INIT. COLOR GRAPHICS */
    newvcol_(&c__1); /* SET DEFAULT PEN COLOR */
    return 0;
} /* vinit_ */


/* Subroutine */ int vtplot_(n, m, ie, lwide)
integer *n, *m, *ie, *lwide;
{
    extern /* Subroutine */ int fptexture_(), fpplot_();

/* 	PLOTS PIXEL MAPPED VECTOR TO FP ARRAY */

/* N	NUMBER OF POINTS */
/* M	ARRAY OF POINTS TO BE OUTPUT */
/* 		M(1) = X1 */
/* 		M(2) = Y1 */
/* 		M(3) = X2 */
/* 		M(4) = Y2,... ETC */
/* IE    ERASE FLAG */
/* 		0=NORMAL (DRAW VISIBLE) */
/* 		1=XOR    (IF NOT SUPPORTED, DRAW VISIBLE) */
/* 		2=ERASE  (IF NOT SUPPORTED, DRAW VISIBLE) */
/* 		3=XOR    (IF NOT SUPPORTED, DRAW ERASE) */

    /* Parameter adjustments */
    --m;

    /* Function Body */
    if (*n < 1 || vt100_1.ivt100 < 0) {
	return 0;
    }

    fptexture_(&vt100_1.mv, &vt100_1.mvw, &c__1);
    fpplot_(n, &m[1], &vt100_1.ivcol);
    return 0;
} /* vtplot_ */



/* Subroutine */ int newvpen_(it, iw)
integer *it, *iw;
{

/* 	THIS ROUTINE CHANGES THE TERMINAL LINE TYPE ON TERMINAL */
/* 	TERMINAL MUST BE IN GRAPHICS MODE */

/* 	IT = LINE TYPE (IF TERMINAL HARDWARE SUPPORTS) */
/* 	IW = LINE WIDTH (IF SUPPORTED) */

    if (vt100_1.ivt100 < 0)
	return 0;

    vt100_1.mv = *it;
    vt100_1.mvw = *iw;

    return 0;
} /* newvpen_ */



/* Subroutine */ int newvcol_(ic)
integer *ic;
{

/* 	THIS ROUTINE CHANGES THE TERMINAL LINE COLOR */
/* 	IC = LINE COLOR (IF SUPPORTED BY TERMINAL HARDWARE) */

    if (vt100_1.ivt100 < 0)
	return 0;

    vt100_1.ivcol = *ic;

    return 0;
} /* newvcol_ */



/* Subroutine */ int cterm_(ix)
integer *ix;
{
    return 0;
} /* cterm_ */


/* Subroutine */ int plotvt_(xa, ya, ia)
real *xa, *ya;
integer *ia;
{
    /* Initialized data */

    static integer nc0 = 13;
    static integer ic0[13] = { 2,3,0,-2,-3,5,11,9,-4,4,999,-9,6 };
    static logical pen = FALSE_;
    static integer ie = 0;

    /* System generated locals */
    integer i__1;
    static real equiv_1[2], equiv_3[2];
    static integer equiv_5[2];

    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    extern integer iand_();
    static real tang;
    static integer ivta, ivtb, nimv1;
    static real t;
    extern /* Subroutine */ int clpit_();
    static integer i0, i1;
    static real x0, y0;
#define av (equiv_1)
#define iv (equiv_5)
    static real xm, ym, xv, yv, xx, yx;
    extern integer ipclip_();
#define av1 (equiv_1)
#define av2 (equiv_1 + 1)
#define iv1 (equiv_5)
#define iv2 (equiv_5 + 1)
    extern /* Subroutine */ int vtplot_();
#define xv2 (equiv_3)
#define yv2 (equiv_3 + 1)
    extern integer ior_();
#define xvs (equiv_3)
    extern /* Subroutine */ int newvcol_();

/* 	PLOT TO TERMINAL GRAPHICS SCREEN */
/* 	USING VTPLOT VECTOR STRING LINE PLOTTING SUBROUTINE */
/* 	C VERSION:	DGL JULY 1987 */
/* 	MODIFIED:		DGL JUNE 1990 */

/* XA	X COORDINATE */
/* YA	Y COORDINATE */
/* IA	PEN CONTROL */
/*     +999   END PLOTTING */
/*      +11   END PLOTTING */
/* 	+9   ERASE TO (XA,YA) PEN DOWN (DRAW IN BACKGROUND INDEX COLOR) */
/* 	+6   SET RELATIVE ROTATION ANGLE TO XA. PEN UNCHANGED. */
/* 	+5   PEN UP AT CURRENT POINT */
/* 	+4   SET UPPER RIGHT CORNER OF VIEW PORT */
/* 	+3   MOVE TO (XA,YA) PEN UP */
/* 	+2   MOVE TO (XA,YA) PEN DOWN */
/* 	 0   CLEAR SCREEN WHEN XA < 0 */
/* 	     SET PEN COLOR WHEN XA >= 0 AND XA < 999 */
/* 	     SET TO XOR (NOT AVAILABLE ON ALL DEVICES) WHEN XA=999 */
/* 	-2   MOVE TO (XA,YA) PEN DOWN. SET ORIGIN TO (XA,YA) */
/* 	-3   MOVE TO (XA,YA) PEN UP.   SET ORIGIN TO (XA,YA) */
/* 	-4   SET LOWER LEFT CORNER OF VIEW PORT */
/* 	-9   ERASE TO (XA,YA) PEN DOWN. SET ORIGIN TO (XA,YA) */

/* 	ANY OTHER VALUE OF IA IS TREATED AS A NOP */



/* 	EQUIVALENCES ARE USED ONLY FOR CONVENIENCE */

/* START PEN UP */

/* PEN TYPE (XOR,ERASE,WRITE) DEF=WRITE */
    if (vt100_1.ivt100 <= 0)
	return 0;

    /* note the -1.0 with xa and ya to make 1..nsx -> 0..nsx etc. */

    i0 = *ia;
    tang = vt100_1.vang * (float).017453294;
    *av1 = (*xa -1.0) * cos(tang) - (*ya - 1.0) * sin(tang);
    *av2 = (*xa -1.0) * sin(tang) + (*ya - 1.0) * cos(tang);
    *av1 = vt100_1.vsf * *av1 + vt100_1.vox;
    *av2 = vt100_1.vsf * *av2 + vt100_1.voy;

/* 	DECODE INPUT COMMAND */

    i__1 = nc0;
    for (i1 = 1; i1 <= i__1; ++i1) {
	if (i0 == ic0[i1 - 1]) {
	    goto L40;
	}
/* L30: */
    }
    goto L800;
L40:
    switch ((int)i1) {
	case 1:  goto L100;
	case 2:  goto L100;
	case 3:  goto L75;
	case 4:  goto L100;
	case 5:  goto L100;
	case 6:  goto L50;
	case 7:  goto L50;
	case 8:  goto L100;
	case 9:  goto L70;
	case 10:  goto L70;
	case 11:  goto L50;
	case 12:  goto L100;
	case 13:  goto L85;
    }

/* 	PEN UP IN PLACE AND TERMINATE PLOTTING COMMANDS */

L50:
/* FINISH UP LAST BUFFER */
    if (! pen || vt100_1.nimv == 0)
	goto L60;

/* EMPTY BUFFER? */
    vtplot_(&vt100_1.nimv, vt100_1.imv, &ie, &vt100_1.mvw);
/* PLOT IT */
L60:
    if (i0 == 11 || i0 == 999) {
	vt100_1.ivt100 = -1; /* DISABLE TERMINAL GRAPHICS */
    }
    if (vt100_1.nimv > 1) {
	nimv1 = (vt100_1.nimv << 1) - 1;
	vt100_1.imv[0] = vt100_1.imv[nimv1 - 1];
	vt100_1.imv[1] = vt100_1.imv[nimv1];
	vt100_1.nimv = 1;
    }
    pen = FALSE_;
    return 0;

/* 	CHANGE VIEWPORT VALUES */

L70:
    *av1 /= vt100_1.vre[0];
    *av2 /= vt100_1.vre[1];

/* 	INSURE VIEWPORT VALUES DO NOT EXCEED HARDWARE WINDOW LIMITS */

    if (*av1 < (float)0.)
	*av1 = (float)0.;

    if (*av1 > vt100_1.vlim[0])
	*av1 = vt100_1.vlim[0];

    if (*av2 < (float)0.)
	*av1 = (float)0.;

    if (*av2 > vt100_1.vlim[1])
	*av2 = vt100_1.vlim[1];

    if (i0 > 0) {
	vt100_1.vvp[2] = *av1;
	vt100_1.vvp[3] = *av2;
    } else {
	vt100_1.vvp[0] = *av1;
	vt100_1.vvp[1] = *av2;
    }
    return 0;

/* 	CHANGE COLOR/CLEAR SCREEN */

L75:
    if (*xa >= (float)0.) 
	goto L80;

/* 	CLEAR SCREEN AND PEN UP */

    pen = FALSE_;
    vt100_1.nimv = 1;
    vt100_1.imv[0] = 0;
    vt100_1.imv[1] = 0;
    return 0;
L80:
    if (pen) /* PLOT LAST BUFFER */
	vtplot_(&vt100_1.nimv, vt100_1.imv, &ie, &vt100_1.mvw);

    ie = 0;
    if (*xa != (float)999.) {
	vt100_1.ivcol = (integer) (*xa);
	newvcol_(&vt100_1.ivcol);  /* CHANGE LINE COLOR */
	if (vt100_1.ivcol == 0) /* SET ERASE FLAG */
	    ie = 2;
    } else
	ie = 1;   /* SET XOR LINE MODE */

    nimv1 = (vt100_1.nimv << 1) + 1;
    vt100_1.imv[0] = vt100_1.imv[nimv1 - 1];
    vt100_1.imv[1] = vt100_1.imv[nimv1];
    vt100_1.nimv = 1;
    pen = FALSE_;
    return 0;
L85:
    vt100_1.vang += *xa;			/* PLOT ANGLE */
    return 0;

/* 	PEN MOTION COMMAND */

L100:						/* MOVE PEN */
    if (*ia > 0)
	goto L200;

    vt100_1.vox = *av1;
    vt100_1.voy = *av2;
L200:
    i0 = abs(i0);
    if (i0 == 9) {
	if (pen) {
	    vtplot_(&vt100_1.nimv, vt100_1.imv, &ie, &vt100_1.mvw);
	}	/* PLOT LAST BUFFER */
	nimv1 = (vt100_1.nimv << 1) + 1;
	vt100_1.imv[0] = vt100_1.imv[nimv1 - 1];
	vt100_1.imv[1] = vt100_1.imv[nimv1];
	vt100_1.nimv = 1;
	pen = FALSE_;
    }
    xm = vt100_1.vvp[0] * vt100_1.vre[0];
    ym = vt100_1.vvp[1] * vt100_1.vre[1];
    xx = vt100_1.vvp[2] * vt100_1.vre[0];
    yx = vt100_1.vvp[3] * vt100_1.vre[1];

/* 	GET CLIP FLAGS */

    ivta = ipclip_(av1, av2, &xm, &ym, &xx, &yx);
    ivtb = ipclip_(&x0, &y0, &xm, &ym, &xx, &yx);
    *xv2 = *av1;
    *yv2 = *av2;
    if (ior_(&ivta, &ivtb) == 0) {	/* LINE VISIBLE */
	goto L220;
    }
    if (iand_(&ivta, &ivtb) != 0) {	/* LINE INVISIBLE */
	i0 = 3;
	goto L250;
    }

/* CLIP LINE */

/* L210: */
    if (ivtb != 0) {		/* OLD POINT IS OUTSIDE WINDOW */
	*xv2 = x0;
	*yv2 = y0;
	if (pen) {
	    vtplot_(&vt100_1.nimv, vt100_1.imv, &ie, &vt100_1.mvw);
	    vt100_1.nimv = 0;
	    pen = FALSE_;
	}
	clpit_(&ivtb, xv2, yv2, av1, av2, &xm, &ym, &xx, &yx);
	if (ivtb != 0) {
	    i0 = 3;
	    vt100_1.nimv = 0;
	    x0 = *av1;
	    y0 = *av2;
	    pen = FALSE_;
	    return 0;
	}
	if (i0 == 2 || i0 == 9) {
	    for (i1 = 1; i1 <= 2; ++i1) {
		t = xvs[i1 - 1] / vt100_1.vre[i1 - 1];
		if (t < (float)0.) {
		    t = (float)0.;
		}
		if (t > vt100_1.vlim[i1 - 1]) {
		    t = vt100_1.vlim[i1 - 1];
		}
		iv[i1 - 1] = t;
/* L205: */
	    }
	    if (vt100_1.iyrev == 1) {
		iv[1] = vt100_1.iylim - iv[1];
	    }
	    vt100_1.nimv = 1;
	    vt100_1.imv[0] = iv[0];
	    vt100_1.imv[1] = iv[1];
	    pen = TRUE_;
	}
    }
    xv = *av1;
    yv = *av2;
    if (ivta != 0) {
	clpit_(&ivta, &xv, &yv, &x0, &y0, &xm, &ym, &xx, &yx);
    }
    if (ivta != 0) {
	if (pen) {
	    vtplot_(&vt100_1.nimv, vt100_1.imv, &ie, &vt100_1.mvw);
	    pen = FALSE_;
	}
	i0 = 3;
	vt100_1.nimv = 0;
	x0 = *av1;
	y0 = *av2;
	return 0;
    }
    *xv2 = xv;
    *yv2 = yv;

L220:
    for (i1 = 1; i1 <= 2; ++i1) {
	t = xvs[i1 - 1] / vt100_1.vre[i1 - 1];
	if (t < (float)0.) {
	    t = (float)0.;
	}
	if (t > vt100_1.vlim[i1 - 1]) {
	    t = vt100_1.vlim[i1 - 1];
	}
	iv[i1 - 1] = t;
/* L710: */
    }
    if (vt100_1.iyrev == 1) {
	iv[1] = vt100_1.iylim - iv[1];
    }
/* MOVE ORIGIN */
L250:
    x0 = *av1;
    y0 = *av2;
    if (i0 == 2) {
	goto L500;
    }
    if (! pen) {
	goto L400;
    }
    if (i0 == 9) {
	vtplot_(&vt100_1.nimv, vt100_1.imv, &c__2, &vt100_1.mvw);
    } else {
	vtplot_(&vt100_1.nimv, vt100_1.imv, &ie, &vt100_1.mvw);
    }

L400:
    vt100_1.imv[0] = iv[0];
    vt100_1.imv[1] = iv[1];
    vt100_1.nimv = 1;
    pen = FALSE_;
    return 0;

L450:
    vt100_1.imv[2] = iv[0];
    vt100_1.imv[3] = iv[1];
    vt100_1.nimv = 2;
    pen = TRUE_;
    return 0;

L500:				/* PEN DOWN MOVEMENT */
    if (! pen) {
	goto L450;
    }
    pen = TRUE_;
    nimv1 = (vt100_1.nimv << 1) + 1;
    if (nimv1 < 33) {
	goto L550;
    }
    vtplot_(&vt100_1.nimv, vt100_1.imv, &ie, &vt100_1.mvw);
    vt100_1.imv[0] = vt100_1.imv[30];
    vt100_1.imv[1] = vt100_1.imv[31];
    vt100_1.nimv = 1;
    nimv1 = 3;

L550:
    vt100_1.imv[nimv1 - 1] = iv[0];
    vt100_1.imv[nimv1] = iv[1];
    ++vt100_1.nimv;

L800:
    return 0;
} /* plotvt_ */

#undef xvs
#undef yv2
#undef xv2
#undef iv2
#undef iv1
#undef av2
#undef av1
#undef iv
#undef av



/* Subroutine */ int clpit_(ivtb, xv2, yv2, av1, av2, xm, ym, xx, yx)
integer *ivtb;
real *xv2, *yv2, *av1, *av2, *xm, *ym, *xx, *yx;
{
    extern integer iand_(), ipclip_();

/* 	CLIPS A LINE SEGMENT PARTIALY VISIBLE */

    if (iand_(ivtb, &c__1) != 0) {	/* LEFT EDGE */
	*yv2 += (*av2 - *yv2) * (*xm - *xv2) / (*av1 - *xv2);
	*xv2 = *xm;
	*ivtb = ipclip_(xv2, yv2, xm, ym, xx, yx);
    }
    if (iand_(ivtb, &c__2) != 0) {	/* RIGHT EDGE */
	*yv2 += (*av2 - *yv2) * (*xx - *xv2) / (*av1 - *xv2);
	*xv2 = *xx;
	*ivtb = ipclip_(xv2, yv2, xm, ym, xx, yx);
    }
    if (iand_(ivtb, &c__4) != 0) {	/* BOTTOM EDGE */
	*xv2 += (*av1 - *xv2) * (*ym - *yv2) / (*av2 - *yv2);
	*yv2 = *ym;
	*ivtb = ipclip_(xv2, yv2, xm, ym, xx, yx);
    }
    if (iand_(ivtb, &c__8) != 0) {	/* TOP EDGE */
	*xv2 += (*av1 - *xv2) * (*yx - *yv2) / (*av2 - *yv2);
	*yv2 = *yx;
	*ivtb = ipclip_(xv2, yv2, xm, ym, xx, yx);
    }
    return 0;
} /* clpit_ */


/* Subroutine */ int pplot_(xa, ya, ia)
real *xa, *ya;
integer *ia;
{
    /* Initialized data */

    static integer nc0 = 14;
    static integer ic0[14] = { 2,3,0,-2,-3,5,9,-9,10,11,999,4,-4,6 };
    static struct {
	real e_1[2];
	} equiv_7 = { (float)32.7, (float)32.7 };


    /* System generated locals */
    integer i__1;
    static real equiv_1[2], equiv_3[2];
    static integer equiv_5[2];

    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    extern integer iand_();
#define alim ((real *)&equiv_7)
    static real tang;
    static integer ivta, ivtb;
#define alim1 ((real *)&equiv_7)
#define alim2 ((real *)&equiv_7 + 1)
    static real t;
    extern /* Subroutine */ int clpit_();
    static integer i0, i1;
#define av (equiv_1)
#define iv (equiv_5)
    static real xm, ym, xv, yv, xx, yx;
    extern integer ipclip_();
#define av1 (equiv_1)
#define av2 (equiv_1 + 1)
    extern /* Subroutine */ int pplotp_();
#define iv1 (equiv_5)
#define iv2 (equiv_5 + 1)
#define xv2 (equiv_3)
#define yv2 (equiv_3 + 1)
    extern integer ior_();
#define xvs (equiv_3)


/* 	C VERSION:   	DGL JULY 1987 */
/* 	MODIFIED:		DGL JUNE 1990 */

/* 	PLOT TO PRINTER GRAPHICS FILE */

/* XA	X COORDINATE */
/* YA	Y COORDINATE */
/* IA	PEN CONTROL */
/* 	+999 END OF PLOTS */
/* 	+11  END OF PLOTS */
/* 	+10  EJECT PAGE */
/* 	+9   MOVE TO (XA,YA) ERASE */
/* 	+3   MOVE TO (XA,YA) PEN UP */
/* 	+6   CHANGE RELATIVE ROTATION ANGLE TO XA. PEN UNCHANGED */
/* 	+5   PEN UP AT CURRENT LOCATION */
/* 	+4   SET UPPER RIGHT CORNER OF VIEW PORT */
/* 	+2   MOVE TO (XA,YA) PEN DOWN */
/* 	 0   CHANGE PLOTTING COLOR TO XA IF XA >= 0 */
/* 	-2   MOVE TO (XA,YA) PEN DOWN SET ORIGIN TO (XA,YA) */
/* 	-3   MOVE TO (XA,YA) PEN UP   SET ORIGIN TO (XA,YA) */
/* 	-4   SET LOWER LEFT CORNER OF VIEW PORT */
/* 	-9   MOVE TO (XA,YA) ERASE    SET ORIGIN TO (XA,YA) */

/* 	ANY OTHER VALUE OF IA IS TREATED AS A NOP */
/* 	EQUIVALENCES ARE USED ONLY FOR CONVENIENCE */
/* 	META FILE LIMITS ON X AND Y IN INCHES */

    if (pxpcom_1.lu <= 0)
	return 0;
    i0 = *ia;
    tang = pxpcom_1.pang * (float).0174532;
    *av1 = *xa * cos(tang) - *ya * sin(tang);
    *av2 = *xa * sin(tang) + *ya * cos(tang);
    *av1 = pxpcom_1.sf * *av1 + pxpcom_1.ox;
    *av2 = pxpcom_1.sf * *av2 + pxpcom_1.oy;

/* 	DECODE COMMAND */

    i__1 = nc0;
    for (i1 = 1; i1 <= i__1; ++i1) {
	if (i0 == ic0[i1 - 1]) {
	    goto L40;
	}
/* L30: */
    }
    return 0;
L40:
    switch ((int)i1) {
	case 1:  goto L100;
	case 2:  goto L100;
	case 3:  goto L50;
	case 4:  goto L100;
	case 5:  goto L100;
	case 6:  goto L910;
	case 7:  goto L100;
	case 8:  goto L100;
	case 9:  goto L715;
	case 10:  goto L900;
	case 11:  goto L900;
	case 12:  goto L70;
	case 13:  goto L70;
	case 14:  goto L85;
    }
L50:					/* 	CHANGE COLOR TO XA */

    if (*xa >= (float)0.) {		/* 	CHANGE PLOT COLOR */
	pxpcom_1.mcol = *xa;
	pplotp_(&pxpcom_1.mw, &pxpcom_1.mcol, &c__1002);
    }
    return 0;

L70:
    if (*av1 < (float)0.)
	*av1 = (float)0.;
    if (*av1 > *alim1)
	*av1 = *alim1;
    if (*av2 < (float)0.)
	*av2 = (float)0.;
    if (*av2 > *alim2)
	*av2 = *alim2;

    if (i0 > 0) {
	pxpcom_1.pvp[2] = *av1;
	pxpcom_1.pvp[3] = *av2;
    } else {
	pxpcom_1.pvp[0] = *av1;
	pxpcom_1.pvp[1] = *av2;
    }
    return 0;

/* 	CHANGE PLOTTING ANGLE BY XA */
L85:
    pxpcom_1.pang += *xa;
    return 0;

/* 	DRAW LINE SEGMENT */
L100:
    if (*ia <= 0) {
	pxpcom_1.ox = *av1;
	pxpcom_1.oy = *av2;
    }
    i0 = abs(i0);
    xm = pxpcom_1.pvp[0];
    ym = pxpcom_1.pvp[1];
    xx = pxpcom_1.pvp[2];
    yx = pxpcom_1.pvp[3];
    ivta = ipclip_(av1, av2, &xm, &ym, &xx, &yx);
    ivtb = ipclip_(&pxpcom_1.px, &pxpcom_1.py, &xm, &ym, &xx, &yx);
    if (ior_(&ivta, &ivtb) == 0) {
	goto L705;
    }					/* LINE ENTIRELY VISIBLE */
    if (iand_(&ivta, &ivtb) != 0) {	/* LINE ENTIRELY INVISIBLE */
	pxpcom_1.px = *av1;
	pxpcom_1.py = *av2;
	return 0;
    }
    if (ivtb != 0) {			/* OLD POINT IS OUTSIDE WINDOW */
	*xv2 = pxpcom_1.px;
	*yv2 = pxpcom_1.py;
	clpit_(&ivtb, xv2, yv2, av1, av2, &xm, &ym, &xx, &yx);
	if (ivtb != 0) {		/* VECTOR DOES NOT INTERSECT */
	    pxpcom_1.px = *av1;
	    pxpcom_1.py = *av2;
	    return 0;
	}
	for (i1 = 1; i1 <= 2; ++i1) {
	    if (xvs[i1 - 1] < (float)0.) {
		xvs[i1 - 1] = (float)0.;
	    }
	    if (xvs[i1 - 1] > alim[i1 - 1]) {
		xvs[i1 - 1] = alim[i1 - 1];
	    }
	    t = xvs[i1 - 1] / pxpcom_1.re[i1 - 1];
	    iv[i1 - 1] = t;
/* L701: */
	}
	pplotp_(iv2, iv1, &c__3);
    }
    xv = *av1;
    yv = *av2;
    if (ivta != 0) {
	clpit_(&ivta, &xv, &yv, &pxpcom_1.px, &pxpcom_1.py, &xm, &ym, &xx, &
		yx);
    }
    if (ivta != 0) {
	pxpcom_1.px = *av1;
	pxpcom_1.py = *av2;
	return 0;
    }
    pxpcom_1.px = *av1;
    pxpcom_1.py = *av2;
    *av1 = xv;
    *av2 = yv;
    goto L715;

L705:
    pxpcom_1.px = *av1;
    pxpcom_1.py = *av2;
L715:
    for (i1 = 1; i1 <= 2; ++i1) {
	if (av[i1 - 1] < (float)0.) {
	    av[i1 - 1] = (float)0.;
	}
	if (av[i1 - 1] > alim[i1 - 1]) {
	    av[i1 - 1] = alim[i1 - 1];
	}
	t = av[i1 - 1] / pxpcom_1.re[i1 - 1];
	iv[i1 - 1] = t;
/* L720: */
    }
    pplotp_(iv2, iv1, &i0);
    return 0;
L900:				/* I0=999 OR I0=11 END PLOTS */
    i0 = 11;
    goto L715;
L910:
    i0 = 3;
    *av1 = pxpcom_1.px;
    *av2 = pxpcom_1.py;
    goto L715;
} /* pplot_ */

#undef xvs
#undef yv2
#undef xv2
#undef iv2
#undef iv1
#undef av2
#undef av1
#undef iv
#undef av
#undef alim2
#undef alim1
#undef alim



/* Subroutine */ int plotrm_(xa, ya, ia)
real *xa, *ya;
integer *ia;
{
    /* Initialized data */

    static integer nc0 = 13;
    static integer ic0[13] = { 2,3,0,-2,-3,5,11,9,-9,4,-4,999,6 };
    static integer icol0 = 0;
    static logical pen = FALSE_;

    /* System generated locals */
    integer i__1;
    static real equiv_1[2], equiv_3[2];
    static integer equiv_5[2];

    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    extern integer iand_();
    static real tang;
    static integer ierr, ivta, ivtb;
    extern /* Subroutine */ int ramclose_();
    static integer nimr1;
    static real t;
    extern /* Subroutine */ int clpit_();
    static integer i0, i1;
    static real x0, y0;
#define av (equiv_1)
#define iv (equiv_5)
    static real xm, ym, xv, yv, xx, yx;
    extern integer ipclip_();
#define av1 (equiv_1)
#define av2 (equiv_1 + 1)
    extern /* Subroutine */ int rmplot_();
#define iv1 (equiv_5)
#define iv2 (equiv_5 + 1)
#define xv2 (equiv_3)
#define yv2 (equiv_3 + 1)
    extern integer ior_();
    extern /* Subroutine */ int rmclear_();
#define xvs (equiv_3)


/* 	C VERSION:	DGL JULY 1987 */
/* 	MODIFIED:		DGL JUNE 1990 */

/* 	PLOT TO RAMTEK USING RMPLOT VECTOR STRING LINE PLOTTING ROUTINE */

/* XA	X COORDINATE */
/* YA	Y COORDINATE */
/* IA	PEN CONTROL */
/*     +999   END PLOTTING */
/*      +11   END PLOTTING */
/* 	+9   ERASE TO (XA,YA) PEN DOWN (ERASE COLOR IS 0) */
/* 	+6   RELATIVE ROTATION ANGLE SET TO XA. PEN UNCHANGED */
/* 	+5   PEN UP AT CURRENT POSITION */
/* 	+4   SET UPPER RIGHT CORNER OF VIEW PORT */
/* 	+3   MOVE TO (XA,YA) PEN UP */
/* 	+2   MOVE TO (XA,YA) PEN DOWN */
/* 	 0   CLEAR SCREEN WHEN XA < 0 */
/* 	     XA >= CHANGE COLOR TABLE VALUE USED FOR PLOTTING TO XA */
/* 	-2   MOVE TO (XA,YA) PEN DOWN SET ORIGIN TO (XA,YA) */
/* 	-3   MOVE TO (XA,YA) PEN UP   SET ORIGIN TO (XA,YA) */
/* 	-4   SET LOWER RIGHT CORNER OF VIEW PORT */
/* 	-9   ERASE TO (XA,YA) PEN DOWN (ERASE COLOR IS 0) SET ORIGIN */
/* 		TO (XA,YA) */

/* 	ANY OTHER VALUE OF IA IS TREATED AS A NOP */

/* 	EQUIVALENCES ARE USED ONLY FOR CONVENIENCE */

    if (rmtek_1.ichan <= 0)
	return 0;

    i0 = *ia;
    tang = rmtek_1.rang * (float).0174532;
    *av1 = *xa * cos(tang) - *ya * sin(tang);
    *av2 = *xa * sin(tang) + *ya * cos(tang);
    *av1 = rmtek_1.rsf * *av1 + rmtek_1.rox;
    *av2 = rmtek_1.rsf * *av2 + rmtek_1.roy;

/* 	DECODE COMMAND */

    i__1 = nc0;
    for (i1 = 1; i1 <= i__1; ++i1) {
	if (i0 == ic0[i1 - 1]) {
	    goto L40;
	}
    }
    goto L800;
L40:
    switch ((int)i1) {
	case 1:  goto L100;
	case 2:  goto L100;
	case 3:  goto L75;
	case 4:  goto L100;
	case 5:  goto L100;
	case 6:  goto L50;
	case 7:  goto L50;
	case 8:  goto L80;
	case 9:  goto L80;
	case 10:  goto L70;
	case 11:  goto L70;
	case 12:  goto L50;
	case 13:  goto L95;
    }
L50:					/* FINISH UP LAST BUFFER */
    if (! pen || rmtek_1.nimr == 0) {
	goto L60;
    }					/* EMPTY BUFFER */
    rmplot_(&rmtek_1.ichan, &rmtek_1.nimr, rmtek_1.imr, &rmtek_1.icol, &ierr);
/* PLOT IT */

L60:
    if (i0 != 5) {
	ramclose_(&rmtek_1.ichan);
    }	/* CLOSE RAM TEK CHANNEL */
    if (rmtek_1.nimr > 1) {
	nimr1 = (rmtek_1.nimr << 1) - 1;
	rmtek_1.imr[0] = rmtek_1.imr[nimr1 - 1];
	rmtek_1.imr[1] = rmtek_1.imr[nimr1];
	rmtek_1.nimr = 1;
    }
    pen = FALSE_;
    return 0;

L70:
    *av1 /= rmtek_1.rre[0];
    *av2 /= rmtek_1.rre[1];
    if (*av1 < (float)0.) {
	*av1 = (float)0.;
    }
    if (*av1 > rmtek_1.rlim[0]) {
	*av1 = rmtek_1.rlim[0];
    }
    if (*av2 < (float)0.) {
	*av1 = (float)0.;
    }
    if (*av2 > rmtek_1.rlim[1]) {
	*av2 = rmtek_1.rlim[1];
    }
    if (i0 > 0) {
	rmtek_1.rvp[2] = *av1;
	rmtek_1.rvp[3] = *av2;
    } else {
	rmtek_1.rvp[0] = *av1;
	rmtek_1.rvp[1] = *av2;
    }
    return 0;

L75:					/* CHANGE COLOR TABLE VALUE */
    if (*xa >= (float)0.) {
	goto L80;
    }
    rmclear_(&rmtek_1.ichan, &ierr);	/* CLEAR RAM TEK SCREEN */
    pen = FALSE_;
    rmtek_1.nimr = 1;
    rmtek_1.imr[0] = 0;
    rmtek_1.imr[1] = 0;
    return 0;

L80:
    if (! pen) {
	goto L90;
    }
    rmplot_(&rmtek_1.ichan, &rmtek_1.nimr, rmtek_1.imr, &rmtek_1.icol, &ierr);

/* PLOT LAST BUFFER */
    nimr1 = (rmtek_1.nimr << 1) + 1;
    rmtek_1.imr[0] = rmtek_1.imr[nimr1 - 1];
    rmtek_1.imr[1] = rmtek_1.imr[nimr1];
    rmtek_1.nimr = 1;

L90:
    if (i0 == 9) {			/* ERASE LINE */
	pen = TRUE_;
	goto L100;
    }
    if (*xa < (float)1024.) {
	rmtek_1.icol = *xa;
    }
    if (rmtek_1.icol < 0) {
	rmtek_1.icol = 0;
    }
    pen = FALSE_;			/* PEN UP */
    return 0;
L95:					/* ROTATION ANGLE */
    rmtek_1.rang += *xa;
    return 0;

L100:					/* MOVE PEN */
    if (*ia > 0) {
	goto L200;
    }
    rmtek_1.rox = *av1;
    rmtek_1.roy = *av2;
L200:
    i0 = abs(i0);
    xm = rmtek_1.rvp[0] * rmtek_1.rre[0];
    ym = rmtek_1.rvp[1] * rmtek_1.rre[1];
    xx = rmtek_1.rvp[2] * rmtek_1.rre[0];
    yx = rmtek_1.rvp[3] * rmtek_1.rre[1];
    ivta = ipclip_(av1, av2, &xm, &ym, &xx, &yx);
    ivtb = ipclip_(&x0, &y0, &xm, &ym, &xx, &yx);
    *xv2 = *av1;
    *yv2 = *av2;
    if (ior_(&ivta, &ivtb) == 0) {
	goto L220;
    }						/* LINE VISIBLE */
    if (iand_(&ivta, &ivtb) != 0) {		/* LINE INVISIBLE */
	i0 = 3;
	goto L250;
    }

/* CLIP LINE */

    if (ivtb != 0) {				/* OLD POINT IS OUTSIDE WINDOW */
	*xv2 = x0;
	*yv2 = y0;
	if (pen) {
	    rmplot_(&rmtek_1.ichan, &rmtek_1.nimr, rmtek_1.imr, &rmtek_1.icol,
		     &ierr);			/* PLOT LAST BUFFER */
	    rmtek_1.nimr = 0;
	    pen = FALSE_;
	}
	clpit_(&ivtb, xv2, yv2, av1, av2, &xm, &ym, &xx, &yx);
	if (ivtb != 0) {
	    i0 = 3;
	    rmtek_1.nimr = 0;
	    x0 = *av1;
	    y0 = *av2;
	    pen = FALSE_;
	    return 0;
	}
	if (i0 == 2 || i0 == 9) {
	    for (i1 = 1; i1 <= 2; ++i1) {
		t = xvs[i1 - 1] / rmtek_1.rre[i1 - 1];
		if (t < (float)0.) {
		    t = (float)0.;
		}
		if (t > rmtek_1.rlim[i1 - 1]) {
		    t = rmtek_1.rlim[i1 - 1];
		}
		iv[i1 - 1] = t;
	    }
	    iv[1] = rmtek_1.irylim - iv[1];
	    rmtek_1.nimr = 1;
	    rmtek_1.imr[0] = iv[0];
	    rmtek_1.imr[1] = iv[1];
	    pen = TRUE_;
	}
    }
    xv = *av1;
    yv = *av2;
    if (ivta != 0) {
	clpit_(&ivta, &xv, &yv, &x0, &y0, &xm, &ym, &xx, &yx);
    }
    if (ivta != 0) {
	if (pen) {
	    rmplot_(&rmtek_1.ichan, &rmtek_1.nimr, rmtek_1.imr, &rmtek_1.icol,
		     &ierr);			/* PLOT LAST BUFFER */
	    pen = FALSE_;
	}
	i0 = 3;
	rmtek_1.nimr = 0;
	x0 = *av1;
	y0 = *av2;
	return 0;
    }
    *xv2 = xv;
    *yv2 = yv;

L220:
    for (i1 = 1; i1 <= 2; ++i1) {
	t = xvs[i1 - 1] / rmtek_1.rre[i1 - 1];
	if (t < (float)0.) {
	    t = (float)0.;
	}
	if (t > rmtek_1.rlim[i1 - 1]) {
	    t = rmtek_1.rlim[i1 - 1];
	}
	iv[i1 - 1] = t;
    }
    iv[1] = rmtek_1.irylim - iv[1];		/* MOVE ORIGIN */
L250:
    x0 = *av1;
    y0 = *av2;
    if (i0 == 2) {
	goto L500;
    }
    if (! pen) {
	goto L400;
    }
    if (i0 != 9) {
	rmplot_(&rmtek_1.ichan, &rmtek_1.nimr, rmtek_1.imr, &rmtek_1.icol, &
		ierr);		/* PLOT LAST BUFFER */
    } else {
	rmtek_1.imr[2] = iv[0];
	rmtek_1.imr[3] = iv[1];
	rmplot_(&rmtek_1.ichan, &c__2, rmtek_1.imr, &icol0, &ierr);
/* PLOT ERASE LINE */
    }

L400:
    rmtek_1.imr[0] = iv[0];
    rmtek_1.imr[1] = iv[1];
    rmtek_1.nimr = 1;
    pen = FALSE_;
    return 0;

L450:
    rmtek_1.imr[2] = iv[0];
    rmtek_1.imr[3] = iv[1];
    rmtek_1.nimr = 2;
    pen = TRUE_;
    return 0;
L500:				/* PEN DOWN MOVEMENT */
    if (! pen) {
	goto L450;
    }
    pen = TRUE_;
    nimr1 = (rmtek_1.nimr << 1) + 1;
    if (nimr1 < 129) {
	goto L550;
    }
    rmplot_(&rmtek_1.ichan, &rmtek_1.nimr, rmtek_1.imr, &rmtek_1.icol, &ierr);

    rmtek_1.imr[0] = rmtek_1.imr[126];
    rmtek_1.imr[1] = rmtek_1.imr[127];
    rmtek_1.nimr = 1;
    nimr1 = 3;
L550:
    rmtek_1.imr[nimr1 - 1] = iv[0];
    rmtek_1.imr[nimr1] = iv[1];
    ++rmtek_1.nimr;
L800:
    return 0;
} /* plotrm_ */

#undef xvs
#undef yv2
#undef xv2
#undef iv2
#undef iv1
#undef av2
#undef av1
#undef iv
#undef av


integer ipclip_(x, y, xm, ym, xx, yx)
real *x, *y, *xm, *ym, *xx, *yx;
{
    /* System generated locals */
    integer ret_val;

    /* Local variables */
    static integer cd;

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CHECKS TO SEE IF POINT XY IS IN RECTANGLE (XM,YM)-(XX,YX) */
/* 	RETURNS ZERO IF IT IS */

    cd = 0;
    if (*x < *xm) {
	cd = 1;
    } else {
	if (*x > *xx) {
	    cd = 2;
	}
    }
    if (*y < *ym) {
	cd += 4;
    } else {
	if (*y > *yx) {
	    cd += 8;
	}
    }
    ret_val = cd;
    return ret_val;
} /* ipclip_ */



/* Subroutine */ int rplots_(id, vpx, vpy, zom)
integer *id;
real *vpx, *vpy, *zom;
{
    /* Local variables */
    static real wide;
    static integer ierr;
    extern /* Subroutine */ int rmtexture_(), plotrm_();
    static integer ich, iid;
    extern /* Subroutine */ int ramopen_();

/* 	C VERSION:	DGL DEC. 1993 */
/* 	SIZE SPECIFIED VERSION:	DGL MAY, 1993 */

/* 	THIS ROUTINE INTIALIZES THE RAMTEK GRAPHICS DEVICE */

/* 	ID	SCREEN CONTROL */
/* 		<= 0 CLEAR SCREEN ON OPEN */
/* 		= 1 NORMAL (1280X1024) RAMTEK [DEFAULT] WIDE=11" */
/* 		= 2 SMALL RAMTEK (512X512) WIDE=11" */
/* 		= 3 384X256 WIDE=7" */
/* 		= 4 768X512 WIDE=7" */
/* 		= 5 256X172 WIDE=7" */
/* 		= 101 CUSTOM SIZE.  THEN ID(2)=X SIZE, ID(3)=Y SIZE, ID(4)=WIDE */
/* 	VPX,VPY	ORIGIN IN INCHES */
/* 	ZOM	SCALE FACTOR */
/* 		< 0 SMALL RAMTEK (512X512) WIDE=11" */

    /* parameter adjustments */
    --id;

    /* Function Body */
    iid = id[1]; /*parameter adjust */
    rmtek_1.irid = abs(iid) % 100;	/* NORMAL RAMTEK (1280X1024) */
    if (*zom < (float)0.) {
	rmtek_1.irid = 2;
    }					/* SMALL RAMTEK (512X512) */
    rmtek_1.mm = 0;			/* LINE TYPE */
    rmtek_1.ipw = 1;			/* LINE WIDTH */
    rmtek_1.ipsc = 0;			/* LINE TYPE SCALE FACTOR */
    rmtek_1.nimr = 1;			/* RESET VECTOR BUFFTER */
    rmtek_1.imr[0] = 0;			/* ORIGIN VECTOR BUFFER */
    rmtek_1.imr[1] = 0;			/* ORIGIN VECTOR BUFFER */
    rmtek_1.icol = 255;			/* DEFAULT COLOR TABLE VALUE */
    rmtek_1.rox = *vpx;			/* X ORIGIN */
    rmtek_1.roy = *vpy;			/* Y ORIGIN */
    rmtek_1.rsf = dabs(*zom);		/* SCALE RACTOR */
    rmtek_1.rang = (float)0.;		/* ROTATION */
    if (rmtek_1.rsf <= (float)0.) {
	rmtek_1.rsf = (float)1.;
    }
    rmtek_1.rre[0] = (float)1024.;
    rmtek_1.rlim[0] = (float)1279.;
    wide = (float)11.;
    if (rmtek_1.irid == 2) {
	rmtek_1.rre[0] = (float)512.;
	rmtek_1.rlim[0] = (float)511.;
    } else if (rmtek_1.irid == 3) {
	wide = (float)7.;
	rmtek_1.rre[0] = (float)256.;
	rmtek_1.rlim[0] = (float)384.;
    } else if (rmtek_1.irid == 4) {
	rmtek_1.irid = 3;
	wide = (float)7.;
	rmtek_1.rre[0] = (float)512.;
	rmtek_1.rlim[0] = (float)768.;
    } else if (rmtek_1.irid == 5) {
	rmtek_1.irid = 3;
	wide = (float)7.;
	rmtek_1.rre[0] = (float)172.;
	rmtek_1.rlim[0] = (float)256.;
    }

    if (abs(iid) > 100) {	/* SET CUSTOM SIZE (ONLY USEFUL ON SOME DEVICES) */
	rmtek_1.irid = 4;
	wide = (real) id[4];
	rmtek_1.rre[0] = (real) id[3];
	rmtek_1.rlim[0] = (real) id[2];
    }

    ierr = rmtek_1.rre[0];
    rmtek_1.iddev = rmtek_1.rlim[0] + (float)1.05;
    rmtek_1.rlim[1] = rmtek_1.rre[0] - (float)1.;
    rmtek_1.irxlim = rmtek_1.rlim[0];
    rmtek_1.irylim = rmtek_1.rlim[1];
    rmtek_1.rre[0] = wide / rmtek_1.rre[0];
/* BIG RAM TEK RESOLUTION X (1024 PIXELS IN 11 INCHES) */
    rmtek_1.rre[1] = rmtek_1.rre[0];	/* RAMTEK RESOLUTION Y */
    rmtek_1.rvp[0] = (float)0.;		/* VIEW PORT PARAMETERS */
    rmtek_1.rvp[1] = (float)0.;		/* LOWER LEFT */

/* 	OPEN UP RAMTEK DISPLAY */

    ich = abs(iid) / 100;		/* WINDOW NUMBER FOR RAMXLIB */
    if (ich == 0)
	ich = 1;

    ramopen_(&ich, &rmtek_1.irid, &rmtek_1.iddev, &ierr); /* OPEN RAMTEK CHANNEL */
    if (ich < 0 || ierr != 0) {		/* RAMTEK NOT AVAILABLE */
	printf("\n *** RAMTEK not available ***\n");
	exit(0);
    }
    rmtek_1.ichan = ich;

/* 	TEST FOR CUSTOM DISPLAY SIZE */

    if (abs(iid) > 100) {
	if (rmtek_1.iddev != id[2] || ierr != id[3]) {
/* C			RRE(1)=IERR */
/* C			RLIM(1)=IDDEV */
/* C			RLIM(2)=RRE(1)-1. */
/* C			IRXLIM=RLIM(1) */
/* C			IRYLIM=RLIM(2) */
/* C			RRE(1)=WIDE/RRE(1) */
/* C			RRE(2)=RRE(1) */
	}
    }

    rmtexture_(&rmtek_1.ichan, &rmtek_1.mm, &rmtek_1.ipw, &rmtek_1.ipsc, &
	    ierr);			/* SET LINE TEXTURE */
    plotrm_(&c_b24, &c_b24, &c__4);	/* UPPER RIGHT */
    if (iid <= 0) {
	plotrm_(&c_b27, &c_b19, &c__0);
    }					/* CLEAR SCREEN */
    return 0;
} /* rplots_ */



integer irmchan_(idev)
integer *idev;
{
    /* System generated locals */
    integer ret_val;

/* 	C VERSION:   DGL DEC. 1993 */
/* 	RETURNS RAMTEK CHANNEL NUMBER IN CURRENT USE */
/* 	A VALUE LESS THAN 1 INDICATES THAT CHANNEL IS NOT OPEN */

/* 	OUTPUTS: */
/* 	IDEV	(I)	RAMTEK DEVICE NUMBER */

    ret_val = rmtek_1.ichan;
    *idev = rmtek_1.iddev;
    return ret_val;
} /* irmchan_ */


/* Subroutine */ int whererm_(orx, ory, zom, ang, rx, ry, nt, ns, icolor, 
	nchan)
real *orx, *ory, *zom, *ang, *rx, *ry;
integer *nt, *ns, *icolor, *nchan;
{

/* 	C VERSION:   DGL DEC. 1993 */
/* 	RETURNS PLOT INFO FROM RAMTEK */


    *orx = rmtek_1.rox;			/* X ORIGIN */
    *ory = rmtek_1.roy;			/* Y ORIGIN */
    *zom = rmtek_1.rsf;			/* ZOOM SCALE FACTOR */
    *ang = rmtek_1.rang;		/* PLOTTING ANGLE */
    if (rmtek_1.rsf != (float)0.) {
	*rx = rmtek_1.rre[0] / rmtek_1.rsf;
    }					/* X DIRECTION RESOLUTION */
    if (rmtek_1.rsf != (float)0.) {
	*ry = rmtek_1.rre[1] / rmtek_1.rsf;
    }					/* Y DIRECTION RESOLUTION */
    *nt = rmtek_1.mm;			/* LINE TYPE */
    *ns = rmtek_1.ipw;			/* PIXEL SCALING OF LINE TYPE */
    *nchan = rmtek_1.ichan;		/* CHANNEL NUMBER IF NCHAN > 0 */
    *icolor = rmtek_1.icol;		/* CURRENT COLOR TABLE VALUE */
    return 0;
} /* whererm_ */


/* Subroutine */ int wherevt_(orx, ory, zom, ang, rx, ry, nvt, isel, nt, nw, 
	ncol)
real *orx, *ory, *zom, *ang, *rx, *ry;
integer *nvt, *isel, *nt, *nw, *ncol;
{

/* 	C VERSION:   DGL DEC. 1993 */
/* 	RETURNS PLOT INFO FOR TERMINAL */

    *orx = vt100_1.vox;			/* X ORIGIN */
    *ory = vt100_1.voy;			/* Y ORIGIN */
    *zom = vt100_1.vsf;			/* ZOOM SCALE FACTOR */
    *ang = vt100_1.vang;		/* PLOTTING ANGLE */
    if (vt100_1.vsf != (float)0.) {
	*rx = vt100_1.vre[0] / vt100_1.vsf;
    }					/* X DIRECTION RESOLUTION */
    if (vt100_1.vsf != (float)0.) {
	*ry = vt100_1.vre[1] / vt100_1.vsf;
    }					/* Y DIRECTION RESOLUTION */
    *nvt = vt100_1.ivt100;		/* NVT > 0 MEANS VT ENABLED */
    *isel = vt100_1.iterm;		/* 0=SELNAR GR1000+ 1=GR100 */
    *nt = vt100_1.mv;			/* LINE TYPE */
    *nw = vt100_1.mvw;			/* LINE WIDTH */
    *ncol = vt100_1.ivcol;		/* LINE COLOR */
    return 0;
} /* wherevt_ */


/* Subroutine */ int wherepr_(orx, ory, ax, ay, zom, ang, rx, ry, nvt, mxx, 
	mww, mcoll)
real *orx, *ory, *ax, *ay, *zom, *ang, *rx, *ry;
integer *nvt, *mxx, *mww, *mcoll;
{
/* 	C VERSION:   DGL DEC. 1993 */
/* 	RETURNS PLOT INFO FROM PRINTER */
    *orx = pxpcom_1.ox;			/* X ORIGIN */
    *ory = pxpcom_1.oy;			/* Y ORIGIN */
    *ax = pxpcom_1.px;			/* LAST SCALED,SHIFTED X */
    *ay = pxpcom_1.py;			/* LAST SCALED,SHIFTED Y */
    *zom = pxpcom_1.sf;			/* ZOOM SCALE FACTOR */
    *ang = pxpcom_1.pang;		/* PLOTTING ANGLE */
    if (pxpcom_1.sf != (float)0.) {
	*rx = pxpcom_1.re[0] / pxpcom_1.sf;
    }					/* X DIRECTION RESOLUTION */
    if (pxpcom_1.sf != (float)0.) {
	*ry = pxpcom_1.re[1] / pxpcom_1.sf;
    }					/* Y DIRECTION RESOLUTION */
    *nvt = pxpcom_1.lu;			/* NVT > 0 MEANS PR ENABLED */
    *mxx = pxpcom_1.mx;			/* LINE TYPE */
    *mww = pxpcom_1.mw;			/* LINE WIDTH */
    *mcoll = pxpcom_1.mcol;		/* LINE COLOR */
    return 0;
} /* wherepr_ */


/* Subroutine */ int plot_(xa, ya, i0)
real *xa, *ya;
integer *i0;
{
    extern /* Subroutine */ int astinter_(), pplot_(), plotrm_(), plotvt_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	+ ADD CONTROL-C INTERRUPT */
/* 	  NOTE: IF CONTROL-C CODE IS NOT USED, LINES MARKED WITH CONTROL-C */
/* 	  CAN BE COMMENTED OUT. */

/* 	BASIC SUBROUTINE FOR PLOTTING TO ALL DEVICES */

    astc_1.iastpl = 1;				/* CONTROL-C */
    pplot_(xa, ya, i0);
    plotrm_(xa, ya, i0);
    plotvt_(xa, ya, i0);
    lstplt_1.xlast = *xa;
    lstplt_1.ylast = *ya;
    lstplt_1.i0last = *i0;
    astc_1.iastpl = 0;				/* CONTROL-C */
    if (astc_1.iastfl != 0) {
	astinter_(&astc_1.iastfl);
    }						/* CONTROL-C */
    return 0;
} /* plot_ */


/* Subroutine */ int pfactor_(sc)
real *sc;
{
/* 	C VERSION:   DGL DEC. 1993 */
/* 	CHANGE SCALE FACTOR ON PRINTER */

/* 	SC	(R)	SCALE FACTOR */

    if (*sc > (float)0.)
	pxpcom_1.sf = *sc * pxpcom_1.sf;
    if (*sc <= (float)0.)
	pxpcom_1.sf = (float)1.;
    return 0;
} /* pfactor_ */


/* Subroutine */ int rfactor_(sc)
real *sc;
{

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CHANGE SCALE FACTOR ON RAMTEK DISPLAY */

    if (*sc > (float)0.)
	rmtek_1.rsf *= *sc;
    if (*sc <= (float)0.)
	rmtek_1.rsf = (float)1.;
    return 0;
} /* rfactor_ */


/* Subroutine */ int vfactor_(sc)
real *sc;
{

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CHANGE SCALE FACTOR FOR TERMINAL PLOTTING */

    if (*sc > (float)0.)
	vt100_1.vsf *= *sc;
    if (*sc <= (float)0.)
	vt100_1.vsf = (float)1.;
    return 0;
} /* vfactor_ */



/* Subroutine */ int metmap_(iarray, na, ma, n, m, xll, yll, xur, yur)
integer *iarray, *na, *ma, *n, *m;
real *xll, *yll, *xur, *yur;
{
    /* Initialized data */

    static real alim[2] = { (float)32.7,(float)32.7 };

    /* System generated locals */
    integer iarray_dim1, i__1, i__2, i__3;

    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    static real tang;
    static integer i1, i2;
    extern /* Subroutine */ int pplot_();
    static real av[2], xa, ya, ax, ay;
    static integer iv[2], ix, iy, nx, ny;
    extern /* Subroutine */ int pplotp_();

/* 	MAKE IMAGE MAP ON META FILE */

/* 	AUTHOR: DG LONG, JPL */
/* 	DATE: 25 JAN 1991 */

/* 	INPUTS: */
/* 	  IARRAY  : COLOR CODES */
/* 	  NA,MA	  : DIMENSIONS OF IARRAY */
/* 	  N,M	  : PLOT N BY M PORTION OF IARRAY (0 < N,M < 2049) */
/* 	  XLL,YLL : POSITION OF LOWER-LEFT CORNER OF MAP (IN PLOT UNITS) */
/* 	  XLL,YLL : POSITION OF UPPER-RIGHT CORNER OF MAP (IN PLOT UNITS) */

/* 	CALLS: */
/* 	  PPLOT */
/* 	  PPLOTP */
/* 	META FILE LIMITS ON X AND Y IN INCHES */

    /* Parameter adjustments */
    iarray_dim1 = *na;

    /* Function Body */

    if (pxpcom_1.lu <= 0)
	return 0;

    pplot_(xll, yll, &c__3);	/* 	ENTER ORIGIN POINT IN METAFILE */

/* 	ENTER X,Y AXES LENGTHS */
    xa = *xll;
    ya = *yll;
    tang = pxpcom_1.pang * (float).0174532;
    av[0] = xa * cos(tang) - ya * sin(tang);
    av[1] = xa * sin(tang) + ya * cos(tang);
    ax = pxpcom_1.sf * av[0] + pxpcom_1.ox;
    ay = pxpcom_1.sf * av[1] + pxpcom_1.oy;
    xa = *xur;
    ya = *yur;
    tang = pxpcom_1.pang * (float).0174532;
    av[0] = xa * cos(tang) - ya * sin(tang);
    av[1] = xa * sin(tang) + ya * cos(tang);
    av[0] = pxpcom_1.sf * av[0] + pxpcom_1.ox - ax;
    av[1] = pxpcom_1.sf * av[1] + pxpcom_1.oy - ay;

    for (i1 = 1; i1 <= 2; ++i1) {
	if (av[i1 - 1] < (float)0.) {
	    av[i1 - 1] = (float)0.;
	}
	if (av[i1 - 1] > alim[i1 - 1]) {
	    av[i1 - 1] = alim[i1 - 1];
	}
	iv[i1 - 1] = av[i1 - 1] / pxpcom_1.re[i1 - 1];
/* L720: */
    }
    pplotp_(iv, &iv[1], &c__2000);

    nx = *n;
    if (nx < 1)
	nx = 1;
    if (nx > 2048)
	nx = 2048;
    ny = *m;
    if (ny < 1)
	ny = 1;
    if (ny > 2048)
	ny = 2048;

    pplotp_(&nx, &ny, &c__2001);

/* 	OUTPUT MAP DATA IN Y-MAJOR ORDER, I.E., (1,1) (1,2) (1,3)... */

    i2 = 1;
    i__1 = nx;
    for (ix = 1; ix <= i__1; ++ix) {
	i__2 = ny;
	for (iy = 1; iy <= i__2; ++iy) {
/* 		I1=IX+(IY-1)*NA */
	    iv[i2 - 1] = iarray[ix + (iy - 1) * iarray_dim1];
	    if ((i__3 = iv[i2 - 1], abs(i__3)) > 32760) {
		iv[i2 - 1] = (iv[i2-1] >= 0 ? 32760 : -32760);
	    }
	    ++i2;
	    if (i2 == 3) {
		pplotp_(iv, &iv[1], &c__2002);
		i2 = 1;
	    }
/* L100: */
	}
    }
    if (i2 == 2)
	pplotp_(iv, &iv[1], &c__2002);
    return 0;
} /* metmap_ */



/* Subroutine */ int metmap2_(iarray, na, ma, ns, ms, ne, me, xll, yll, xur, 
	yur)
integer *iarray, *na, *ma, *ns, *ms, *ne, *me;
real *xll, *yll, *xur, *yur;
{
    /* Initialized data */

    static real alim[2] = { (float)32.7,(float)32.7 };

    /* System generated locals */
    integer iarray_dim1, i__1, i__2, i__3;

    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    static real tang;
    static integer i1, i2;
    extern /* Subroutine */ int pplot_();
    static real av[2], xa, ya, ax, ay;
    static integer iv[2], nx, ny, ix, iy;
    extern /* Subroutine */ int pplotp_();
    static integer nx1, nx2, ny1, ny2;


/* 	MAKE IMAGE MAP ON META FILE */

/* 	AUTHOR: DG LONG, JPL */
/* 	DATE: 25 JAN 1991 */

/* 	INPUTS: */
/* 	  IARRAY  : COLOR CODES */
/* 	  NA,MA	  : DIMENSIONS OF IARRAY */
/* 	  NS,MS	  : STARTING LOWER-LEFT CORNER ( 0 < NS < NA, 0 < MS < MA) */
/*	  NE,ME	  : ENDING UPPER-RIGHT CORNER  ( NS <= NS <= NA, MS <= MS <= MA
)*/
/* 	  	    OF RECTRANGLE TO PLOT (0 < NE-NS, ME-NS < 2048) */
/* 	  XLL,YLL : POSITION OF LOWER-LEFT CORNER OF MAP (IN PLOT UNITS) */
/* 	  XUR,YUR : POSITION OF UPPER-RIGHT CORNER OF MAP (IN PLOT UNITS) */

/* 	CALLS: */
/* 	  PPLOT */
/* 	  PPLOTP */

/* 	META FILE LIMITS ON X AND Y IN INCHES */

    /* Parameter adjustments */
    iarray_dim1 = *na;

    /* Function Body */

    if (pxpcom_1.lu <= 0)
	return 0;

/* 	ENTER ORIGIN POINT IN METAFILE */

    pplot_(xll, yll, &c__3);

/* 	ENTER X,Y AXES LENGTHS */

    xa = *xll;
    ya = *yll;
    tang = pxpcom_1.pang * (float).0174532;
    av[0] = xa * cos(tang) - ya * sin(tang);
    av[1] = xa * sin(tang) + ya * cos(tang);
    ax = pxpcom_1.sf * av[0] + pxpcom_1.ox;
    ay = pxpcom_1.sf * av[1] + pxpcom_1.oy;
    xa = *xur;
    ya = *yur;
    tang = pxpcom_1.pang * (float).0174532;
    av[0] = xa * cos(tang) - ya * sin(tang);
    av[1] = xa * sin(tang) + ya * cos(tang);
    av[0] = pxpcom_1.sf * av[0] + pxpcom_1.ox - ax;
    av[1] = pxpcom_1.sf * av[1] + pxpcom_1.oy - ay;

    for (i1 = 1; i1 <= 2; ++i1) {
	if (av[i1 - 1] < (float)0.) {
	    av[i1 - 1] = (float)0.;
	}
	if (av[i1 - 1] > alim[i1 - 1]) {
	    av[i1 - 1] = alim[i1 - 1];
	}
	iv[i1 - 1] = av[i1 - 1] / pxpcom_1.re[i1 - 1];
/* L720: */
    }
    pplotp_(iv, &iv[1], &c__2000);

    nx1 = *ns;
    if (nx1 < 1)
	nx1 = 1;
    if (nx1 > 2047)
	nx1 = 2047;
    nx2 = *ne;
    if (nx2 < nx1)
	nx2 = 1;
    if (nx2 > 2048)
	nx2 = 2048;
    nx = nx2 - nx1 + 1;
    ny1 = *ms;
    if (ny1 < 1)
	ny1 = 1;
    if (ny1 > 2047)
	ny1 = 2047;
    ny2 = *me;
    if (ny2 < ny1)
	ny2 = 1;
    if (ny2 > 2048)
	ny2 = 2048;
    ny = ny2 - ny1 + 1;
    pplotp_(&nx, &ny, &c__2001);

/* 	OUTPUT MAP DATA IN Y-MAJOR ORDER, I.E., (1,1) (1,2) (1,3)... */

    i2 = 1;
    i__1 = nx2;
    for (ix = nx1; ix <= i__1; ++ix) {
	i__2 = ny2;
	for (iy = ny1; iy <= i__2; ++iy) {
	    iv[i2 - 1] = iarray[ix + (iy - 1) * iarray_dim1];
	    if ((i__3 = iv[i2 - 1], abs(i__3)) > 32760) {
		iv[i2 - 1] = (iv[i2-1] >= 0 ? 32760 : -32760);
	    }
	    ++i2;
	    if (i2 == 3) {
		pplotp_(iv, &iv[1], &c__2002);
		i2 = 1;
	    }
/* L100: */
	}
    }
    if (i2 == 2)
	pplotp_(iv, &iv[1], &c__2002);

    return 0;
} /* metmap2_ */



/* Subroutine */ int metmapa_(iarray, na, ma, n, m, xll, yll, xur, yur)
shortint *iarray;
integer *na, *ma, *n, *m;
real *xll, *yll, *xur, *yur;
{
    /* Initialized data */

    static real alim[2] = { (float)32.7,(float)32.7 };

    /* System generated locals */
    integer iarray_dim1, i__1, i__2, i__3;

    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    static real tang;
    static integer i1, i2;
    extern /* Subroutine */ int pplot_();
    static real av[2], xa, ya, ax, ay;
    static integer iv[2], ix, iy, nx, ny;
    extern /* Subroutine */ int pplotp_();


/* 	MAKE IMAGE MAP ON META FILE */

/* 	AUTHOR: DG LONG, JPL */
/* 	DATE: 25 JAN 1991 */

/* 	INPUTS: */
/* 	  IARRAY  : COLOR CODES */
/* 	  NA,MA	  : DIMENSIONS OF IARRAY */
/* 	  N,M	  : PLOT N BY M PORTION OF IARRAY (0 < N,M < 2049) */
/* 	  XLL,YLL : POSITION OF LOWER-LEFT CORNER OF MAP (IN PLOT UNITS) */
/* 	  XLL,YLL : POSITION OF UPPER-RIGHT CORNER OF MAP (IN PLOT UNITS) */

/* 	CALLS: */
/* 	  PPLOT */
/* 	  PPLOTP */

/* 	META FILE LIMITS ON X AND Y IN INCHES */

    /* Parameter adjustments */
    iarray_dim1 = *na;

    /* Function Body */

    if (pxpcom_1.lu <= 0)
	return 0;

/* 	ENTER ORIGIN POINT IN METAFILE */

    pplot_(xll, yll, &c__3);

/* 	ENTER X,Y AXES LENGTHS */

    xa = *xll;
    ya = *yll;
    tang = pxpcom_1.pang * (float).0174532;
    av[0] = xa * cos(tang) - ya * sin(tang);
    av[1] = xa * sin(tang) + ya * cos(tang);
    ax = pxpcom_1.sf * av[0] + pxpcom_1.ox;
    ay = pxpcom_1.sf * av[1] + pxpcom_1.oy;
    xa = *xur;
    ya = *yur;
    tang = pxpcom_1.pang * (float).0174532;
    av[0] = xa * cos(tang) - ya * sin(tang);
    av[1] = xa * sin(tang) + ya * cos(tang);
    av[0] = pxpcom_1.sf * av[0] + pxpcom_1.ox - ax;
    av[1] = pxpcom_1.sf * av[1] + pxpcom_1.oy - ay;

    for (i1 = 1; i1 <= 2; ++i1) {
	if (av[i1 - 1] < (float)0.) {
	    av[i1 - 1] = (float)0.;
	}
	if (av[i1 - 1] > alim[i1 - 1]) {
	    av[i1 - 1] = alim[i1 - 1];
	}
	iv[i1 - 1] = av[i1 - 1] / pxpcom_1.re[i1 - 1];
/* L720: */
    }
    pplotp_(iv, &iv[1], &c__2000);

    nx = *n;
    if (nx < 1)
	nx = 1;
    if (nx > 2048)
	nx = 2048;
    ny = *m;
    if (ny < 1)
	ny = 1;
    if (ny > 2048)
	ny = 2048;
    pplotp_(&nx, &ny, &c__2001);

/* 	OUTPUT MAP DATA IN Y-MAJOR ORDER, I.E., (1,1) (1,2) (1,3)... */

    i2 = 1;
    i__1 = nx;
    for (ix = 1; ix <= i__1; ++ix) {
	i__2 = ny;
	for (iy = 1; iy <= i__2; ++iy) {
/* 		I1=IX+(IY-1)*NA */
	    iv[i2 - 1] = iarray[ix + (iy - 1) * iarray_dim1];
	    if ((i__3 = iv[i2 - 1], abs(i__3)) > 32760) {
		iv[i2 - 1] = (iv[i2-1] >= 0 ? 32760 : -32760);
	    }
	    ++i2;
	    if (i2 == 3) {
		pplotp_(iv, &iv[1], &c__2002);
		i2 = 1;
	    }
/* L100: */
	}
    }
    if (i2 == 2)
	pplotp_(iv, &iv[1], &c__2002);
    return 0;
} /* metmapa_ */



/* Subroutine */ int metmap2a_(iarray, na, ma, ns, ms, ne, me, xll, yll, xur, 
	yur)
shortint *iarray;
integer *na, *ma, *ns, *ms, *ne, *me;
real *xll, *yll, *xur, *yur;
{
    /* Initialized data */

    static real alim[2] = { (float)32.7,(float)32.7 };

    /* System generated locals */
    integer iarray_dim1, i__1, i__2, i__3;

    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    static real tang;
    static integer i1, i2;
    extern /* Subroutine */ int pplot_();
    static real av[2], xa, ya, ax, ay;
    static integer iv[2], nx, ny, ix, iy;
    extern /* Subroutine */ int pplotp_();
    static integer nx1, nx2, ny1, ny2;

/* 	MAKE IMAGE MAP ON META FILE */

/* 	AUTHOR: DG LONG, JPL */
/* 	DATE: 25 JAN 1991 */

/* 	INPUTS: */
/* 	  IARRAY  : COLOR CODES */
/* 	  NA,MA	  : DIMENSIONS OF IARRAY */
/* 	  NS,MS	  : STARTING LOWER-LEFT CORNER ( 0 < NS < NA, 0 < MS < MA) */
/*	  NE,ME	  : ENDING UPPER-RIGHT CORNER  ( NS <= NS <= NA, MS <= MS <= MA
)*/
/* 	  	    OF RECTRANGLE TO PLOT (0 < NE-NS, ME-NS < 2048) */
/* 	  XLL,YLL : POSITION OF LOWER-LEFT CORNER OF MAP (IN PLOT UNITS) */
/* 	  XUR,YUR : POSITION OF UPPER-RIGHT CORNER OF MAP (IN PLOT UNITS) */

/* 	CALLS: */
/* 	  PPLOT */
/* 	  PPLOTP */

/* 	META FILE LIMITS ON X AND Y IN INCHES */

    /* Parameter adjustments */
    iarray_dim1 = *na;

    /* Function Body */

    if (pxpcom_1.lu <= 0)
	return 0;

/* 	ENTER ORIGIN POINT IN METAFILE */

    pplot_(xll, yll, &c__3);

/* 	ENTER X,Y AXES LENGTHS */

    xa = *xll;
    ya = *yll;
    tang = pxpcom_1.pang * (float).0174532;
    av[0] = xa * cos(tang) - ya * sin(tang);
    av[1] = xa * sin(tang) + ya * cos(tang);
    ax = pxpcom_1.sf * av[0] + pxpcom_1.ox;
    ay = pxpcom_1.sf * av[1] + pxpcom_1.oy;
    xa = *xur;
    ya = *yur;
    tang = pxpcom_1.pang * (float).0174532;
    av[0] = xa * cos(tang) - ya * sin(tang);
    av[1] = xa * sin(tang) + ya * cos(tang);
    av[0] = pxpcom_1.sf * av[0] + pxpcom_1.ox - ax;
    av[1] = pxpcom_1.sf * av[1] + pxpcom_1.oy - ay;

    for (i1 = 1; i1 <= 2; ++i1) {
	if (av[i1 - 1] < (float)0.)
	    av[i1 - 1] = (float)0.;
	if (av[i1 - 1] > alim[i1 - 1])
	    av[i1 - 1] = alim[i1 - 1];
	iv[i1 - 1] = av[i1 - 1] / pxpcom_1.re[i1 - 1];
/* L720: */
    }
    pplotp_(iv, &iv[1], &c__2000);

    nx1 = *ns;
    if (nx1 < 1)
	nx1 = 1;
    if (nx1 > 2047)
	nx1 = 2047;
    nx2 = *ne;
    if (nx2 < nx1)
	nx2 = 1;
    if (nx2 > 2048)
	nx2 = 2048;
    nx = nx2 - nx1 + 1;
    ny1 = *ms;
    if (ny1 < 1)
	ny1 = 1;
    if (ny1 > 2047)
	ny1 = 2047;
    ny2 = *me;
    if (ny2 < ny1)
	ny2 = 1;
    if (ny2 > 2048)
	ny2 = 2048;
    ny = ny2 - ny1 + 1;
    pplotp_(&nx, &ny, &c__2001);

/* 	OUTPUT MAP DATA IN Y-MAJOR ORDER, I.E., (1,1) (1,2) (1,3)... */

    i2 = 1;
    i__1 = nx2;
    for (ix = nx1; ix <= i__1; ++ix) {
	i__2 = ny2;
	for (iy = ny1; iy <= i__2; ++iy) {
	    iv[i2 - 1] = iarray[ix + (iy - 1) * iarray_dim1];
	    if ((i__3 = iv[i2 - 1], abs(i__3)) > 32760) {
		iv[i2 - 1] = (iv[i2-1] >= 0 ? 32760 : -32760);
	    }
	    ++i2;
	    if (i2 == 3) {
		pplotp_(iv, &iv[1], &c__2002);
		i2 = 1;
	    }
/* L100: */
	}
    }
    if (i2 == 2)
	pplotp_(iv, &iv[1], &c__2002);

    return 0;
} /* metmap2a_ */


/* Subroutine */ int metcol_(ired, igrn, iblu, icod, n)
integer *ired, *igrn, *iblu, *icod, *n;
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i, n1;
    extern /* Subroutine */ int pplotp_();

/* 	DUMP COLOR TABLE DATA TO META FILE */

/* 	IRED,IGRN,IBLU	(I)	RGB VALUES FOR COLOR TABLE INDEXES */
/* 	ICOD		(I)	CODE VALUE */
/* 	N		(I)	NUMBER OF ENTRIES (0 < N < 1025) */

    /* parameter adjustments */

    /* Function Body */
    if (pxpcom_1.lu <= 0)
	return 0;
    n1 = *n;
    if (n1 < 1)
	return 0;
    if (n1 > 1024)
	n1 = 1024;
    pplotp_(&n1, &c__0, &c__2010);
    i__1 = n1;
    for (i = 0; i < i__1; ++i) {
	pplotp_(&ired[i], &igrn[i], &c__2011);
	pplotp_(&iblu[i], &icod[i], &c__2012);
    }
    return 0;
} /* metcol_ */



/* Subroutine */ int pplotp_(m0, m1, m2)
integer *m0, *m1, *m2;
{
    /* Local variables */
    static shortint mb[128];
    static char ch[20];

/* 	C VERSION:   DGL DEC. 1993 */
/* 	WRITE PLOTTING DATA TO META FILE */

/* M0	WORD 1 OF OUTPUT */
/* M1	WORD 2 OF OUTPUT */
/* M2	WORD 3 OF OUTPUT (COMMAND WORD) */

/* 	LONGLIB META FILE FORMAT */

/* 	M0	M1	M2	COMMAND INTERPRETATION */

/* 	X	Y	2	PEN DOWN MOVE */
/* 	X	Y	3	PEN UP MOVE */
/* 	X	Y	-2*	PEN DOWN MOVE WITH NEW ORIGIN */
/* 	X	Y	-3*	PEN UP MOVE WITH NEW ORIGIN */
/* 	X	Y	9	PEN ERASE MOVE */
/* 	X	Y	-9*	PEN ERASE WITH NEW ORIGIN */
/* 	X	Y	10	NEW PAGE */
/* 	-	-	11	END OF FILE */
/* 	-	-	999	END OF FILE */
/* 	RESX	RESY	1000	SET RESOLUTION */
/* 	TYPE	SCALE	1001	SET LINE TYPE */
/* 	-	COLOR	1002	SET PEN COLOR */
/* 	WIDTH	-	1003	SET LINE WIDTH */
/* 	XSC	YSC	2000	IMAGE SIZE SCALE FACTOR */
/* 	NX	NY	2001	IMAGE ARRAY SIZES */
/* 	V1	V2	2002	IMAGE DATA */
/* 	N	-	2010	COLOR TABLE ENTRY SIZE */
/* 	R	G	2011	COLOR TABLE DATA R AND G */
/* 	B	C	2012	COLOR TABLE DATA B AND CODES */
/* 	-	-	(ELSE*)	INVALID COMMAND */

/* 	* NORMALLY NOT SENT TO FILE */

/* 	FIRST COMMAND IN FILE SHOULD SET RESOLUTION */
/* 	NEXT COMMANDS SHOULD SET LINE TYPE, WIDTH AND COLOR */

/* OUTPUT RECORD BUFFER */

    if (pxpcom_1.lu <= 0) {	/* NO PRINT FLAG */
	return 0;
    }
    if (pxpcom_1.mp < 0) {	/* NO OUTPUT LU */
	goto L20;
    }				/* CHECK FOR FILE CLOSED */
    pxpcom_1.mp += 3;		/* INCREMENT POINTER */
    if (pxpcom_1.mp < 128) {	/* CHECK FOR FULL BUFFER */
	goto L10;
    }

    if (met_file_open == 0) {	/* NO OUTPUT FILE OPEN, CREATE AND OPEN IT */
	if (pxpcom_1.lu > 9) {
		sprintf(ch, "%2d.dat",pxpcom_1.lu);
	} else {
		sprintf(ch, "%1d.dat",pxpcom_1.lu);
	};
	met_file = fopen(ch,"w");
/*	if (met_file == EOF) { */
	if (feof(met_file)) {
		printf("\n*** ERROR OPENING METAFILE OUTPUT FILE ***\n");
	} else {
		met_file_open = 3;
	};
    }
    if (met_file_open != 0) {
	fwrite(&mb[0], sizeof(shortint), 128, met_file);
    }
    pxpcom_1.mp = 3;
L10:
    mb[pxpcom_1.mp - 1] = *m2	;	/* STORE DATA */
    mb[pxpcom_1.mp - 2] = *m1;
    mb[pxpcom_1.mp - 3] = *m0;
    if (*m2 != 11 && *m2 != 999) {	/* CHECK FOR END OF PLOTS */
	goto L20;
    }
    if (met_file_open != 0) {		/* WRITE RECORD LAST */
	fwrite(&mb[0], sizeof(shortint), 128, met_file);
    }
    fclose(met_file);			/* CLOSE FILE OUTPUT */
    met_file_open = 0;
    pxpcom_1.mp = -1;			/* FLAG END OF PLOTS */
L20:
    return 0;
} /* pplotp_ */


/* Subroutine */ int pplots_(ia, x0, y0, zom)
integer *ia;
real *x0, *y0, *zom;
{
    extern /* Subroutine */ int pplot_();
    static integer ir1, ir2;
    extern /* Subroutine */ int pplotp_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	INITIALIZE PRINTER HISTORY FILE GRAPHICS PACKAGE */

/* IA	LOGICAL UNIT FOR OUTPUT */
/* 	= 0   DEFAULTS TO 3 */
/* 	< 0  DOES NOT PRODUCE OUTPUT */

    pxpcom_1.lu = *ia;			/* FORTRAN FILE NUMBER */
    pxpcom_1.ox = *x0;			/* ORIGIN X */
    pxpcom_1.oy = *y0;			/* ORIGIN Y */
    pxpcom_1.sf = *zom;			/* SCALE FACTOR */
    if (pxpcom_1.sf <= (float)0.) {
	pxpcom_1.sf = (float)1.;
    }
    pxpcom_1.pang = (float)0.;		/* PLOTTING ANGLE ROTATION */
    pxpcom_1.px = (float)0.;		/* LAST POINT PLOTTED */
    pxpcom_1.py = (float)0.;
    ir1 = 1000;				/* "PIXELS" PER INCH RESOLUTION */
    ir2 = 1000;
    pxpcom_1.re[0] = (float)1. / (real) ir1; /* RESOLUTION FOR META FILE */
    pxpcom_1.re[1] = (float)1. / (real) ir2;
    pxpcom_1.pvp[0] = (float)0.;	/* VIEW PORT PARAMETERS LOWER LEFT */
    pxpcom_1.pvp[1] = (float)0.;
    pplot_(&c_b24, &c_b24, &c__4);	/* VIEW PORT UPPER RIGHT CORNER SET */
    pplot_(&c_b391, &c_b391, &c_n4);
    if (pxpcom_1.lu == 0) {
	pxpcom_1.lu = 3;
    }
    pxpcom_1.mp = 0;			/* PRINT BUFFER COUNTER */
    pxpcom_1.mx = 0;			/* DEFAULT LINE TYPE (SOLID) */
    pxpcom_1.msc = 1;			/* DEFAULT LINE SCALING (UNIT) */
    pxpcom_1.mw = 1;			/* DEFAULT LINE WIDTH (UNIT) */
    pxpcom_1.mcol = 1;			/* DEFAULT LINE COLOR */
    if (pxpcom_1.lu < 0) {
	return 0;
    }
    pplotp_(&ir2, &ir1, &c__1000);	/* INITIALIZE RESOLUTION */
    pplotp_(&pxpcom_1.mx, &pxpcom_1.msc, &c__1001);  /* INITIALIZE LINE TYPE, SCALING */
    pplotp_(&pxpcom_1.mw, &pxpcom_1.mcol, &c__1002); /* INITIALIZE LINE COLOR */
    pplotp_(&pxpcom_1.mw, &c__0, &c__1003);	/* INITIALIZE LINE WIDTH */
    return 0;
} /* pplots_ */


/* Subroutine */ int metaon_(ia)
integer *ia;
{
/* 	TURNS ON/OFF HISTORY FILE (META FILE) OUTPUT */
/* 	META FILE MUST HAVE BEEN PREVIOUSLY INITIALIZED */

/* 	IA	(I)	METAFILE OPTION FLAG */
/* 			= -1: TURN OFF META FILE PLOTTING */
/* 			= +1: TURN ON META FILE PLOTTING */


    if (pxpcom_1.lu == 0) {		/* NOT INITIALIZED */
	return 0;
    }
    if (*ia == -1) {			/* TURN META FILE OFF */
	if (pxpcom_1.lu > 0) {
	    pxpcom_1.lu = -10000 - pxpcom_1.lu;
	}
    } else if (*ia == 1) {		/* TURN META FILE ON */
	if (pxpcom_1.lu < -10000) {
	    pxpcom_1.lu = -pxpcom_1.lu - 10000;
	}
    }
    return 0;
} /* metaon_ */



/* Subroutine */ int ppen_(ip)
integer *ip;
{
    static integer m;
    extern /* Subroutine */ int pplotp_();
    static integer mw1, msc1;

/* 	SELECT PRINTER PLOTTING LINE TYPE */
/* 	C VERSION:   DGL DEC. 1993 */

/* IP	PEN CONTROL WORD (IP<0 RESETS LINE TYPE TO SOLID, 1 DOT WIDE) */

/* 	UNITS DIGIT=LINE TYPE */
/* 	  0 = NO CHANGE */
/* 	  1-9 LINE TYPE */

/* 	TENS  DIGIT=LINE WIDTH */
/* 	  0 = NO CHANGE */
/* 	  1-7 WIDTH IN DOTS */

/* 	HUNDREDS DIGIT=LINE TYPE SCALE FACTOR */
/* 	  0 = NO CHANGE */


    if (*ip <= 0) {
	m = 1;
	mw1 = 1;
	pxpcom_1.msc = 1;
	goto L10;
    }
    msc1 = *ip / 100;
    if (msc1 != 0) {
	pxpcom_1.msc = msc1;
    }
    mw1 = *ip % 100 / 10;
    m = *ip % 10;
    if (m == 0) {
	m = pxpcom_1.mx + 1;
    }
L10:
    pxpcom_1.mx = m - 1;
    if (pxpcom_1.lu > 0) {	/* 	CHANGE LINE TYPE AND SCALE */
	pplotp_(&pxpcom_1.mx, &pxpcom_1.msc, &c__1001);

	if (mw1 != 0) {		/* 	CHANGE PEN WIDTH */
	    pxpcom_1.mw = mw1;
	    pplotp_(&pxpcom_1.mw, &c__0, &c__1003);
	}
    }
    return 0;
} /* ppen_ */



/* Subroutine */ int rmpen_(ip)
integer *ip;
{
    static integer ierr, ipsc1, m;
    extern /* Subroutine */ int rmtexture_();
    static integer ipw1;

/* 	MODIFY RAMTEK PLOTTING LINE TYPE */
/* 	C VERSION:   DGL DEC. 1993 */

/* IP	PEN CONTROL WORD */
/* 		NOTE: IP<0 RESET TO SOLID LINE, WIDTH 1 */

/* 	UNITS DIGIT=LINE TYPE */
/* 	  0 = NO CHANGE */
/* 	  1-9 LINE TYPE */

/* 	TENS  DIGIT=LINE WIDTH */
/* 	  0 = NO CHANGE */
/* 	  1-7 LINE WIDTH */

/* 	HUNDREDS DIGIT=LINE TYPE SCALE FACTOR */
/* 	  0 = NO CHANGE */
/* 	  1-7 BIT PATTERN SCALE FACTOR */

    if (*ip <= 0) {
	m = 1;
	rmtek_1.ipw = 1;
	rmtek_1.ipsc = 0;
	goto L10;
    }
    ipsc1 = *ip / 100;
    if (ipsc1 != 0)
	rmtek_1.ipsc = ipsc1 - 1;

    ipw1 = *ip % 100 / 10;
    if (ipw1 > 7)
	ipw1 = 7;

    if (ipw1 != 0)
	rmtek_1.ipw = ipw1;

    m = *ip % 10;
    if (m == 0)
	m = rmtek_1.mm + 1;

L10:
    rmtek_1.mm = m - 1;
    rmtexture_(&rmtek_1.ichan, &rmtek_1.mm, &rmtek_1.ipw, &rmtek_1.ipsc, &
	    ierr);
    return 0;
} /* rmpen_ */



/* Subroutine */ int vpen_(ip)
integer *ip;
{
    static integer mv, mv1, mvw;
    extern /* Subroutine */ int newvpen_();
    static integer mvw1;

/* 	CHANGE TERMINAL PLOTTING LINE TYPE */
/* 	C VERSION:   DGL DEC. 1993 */

/* IP	PEN CONTROL WORD */
/* 		NOTE: IP<0 IS RESET TO SOLID LINE UNIT WIDTH */

/* 	UNITS DIGIT=LINE TYPE */
/* 	  0 = NO CHANGE */
/* 	  1-9 LINE TYPE */

/* 	TENS  DIGIT=LINE WIDTH */
/* 	  0 = NO CHANGE */
/* 	  1-7 LINE WIDTH */

/* 	HUNDREDS DIGIT=LINE TYPE SCALE FACTOR (NOT IMPLEMENTED) */

    if (*ip <= 0) {
	mv = 0;
	mvw = 1;
	goto L10;
    }
    mvw1 = *ip % 100 / 10;
    if (mvw1 != 0)
	mvw = mvw1;
    mv1 = *ip % 10;
    if (mv1 == 0)
	mv1 = mv + 1;
    mv = mv1 - 1;

L10:	/* 	CALL TERMINAL DEPENDENT ROUTINE TO CHANGE LINE TYPE */
    newvpen_(&mv, &mvw);
    return 0;
} /* vpen_ */



/* Subroutine */ int newpage_()
{
    extern /* Subroutine */ int pplot_();

/* 	C VERSION:   DGL DEC. 1993 */

/* 	THIS ROUTINE CAUSES A RESETING OF THE PRINTER PAGE WITHOUT */
/* 	AFFECTING SCREEN DEVICES */

    pplot_(&c_b19, &c_b19, &c__10);		/* EJECT PRINTER PAGE */
    return 0;
} /* newpage_ */


/* Subroutine */ int factor_(sc)
real *sc;
{
    extern /* Subroutine */ int pfactor_(), rfactor_(), vfactor_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CHANGE SCALE FACTORS FOR ALL PLOT PACKAGES */

/* 	SC  SCALE FACTOR */

    if (pxpcom_1.lu >= 0)
	pfactor_(sc);
    if (rmtek_1.ichan >= 0)
	rfactor_(sc);
    if (vt100_1.ivt100 >= 0)
	vfactor_(sc);
    return 0;
} /* factor_ */


/* Subroutine */ int newpen_(ip)
integer *ip;
{
    extern /* Subroutine */ int ppen_(), vpen_(), plot_(), rmpen_();

/* 	CHANGE PLOTTING LINE TYPE ON ALL DEVICES */
/* 	C VERSION:   DGL DEC. 1993 */

/* IP	PEN TYPE CONTROL WORD */
/* 		NOTE: IP<0 RESETS PEN WIDTH TO 1 AND LINE TYPE TO SOLID */

/* 	NOT ALL GRAPHICS DEVICES SUPPORT ALL HARWARE LINE TYPES.  IF THEY DO, 
*/
/* 	THE LINE TYPES SHOULD RESEMBLE THESE: */

/* 	UNITS DIGIT = LINE TYPE 0-9 */
/* 	  0 = NO CHANGE */
/* 	  1-9 LINE TYPE */
/* 		TYPE	PATTERN */
/* 		 1	SOLID */
/* 		 2	DOTTED */
/* 		 3	LONG-DOT-LONG */
/* 		 4	MEDDASH-MEDDASH */
/* 		 5	LONGDASH-LONGDASH */
/* 		 6	LONG-DOT-DOT-LONG */
/* 		 7	LONG-SHORT-LONG */
/* 		 8	SHORT-SHORT */
/* 		 9	LONG-DOT-DOT-DOT-LONG */

/* 	TENS  DIGIT = LINE WIDTH IN PIXELS OR DOTS */
/* 	  0 = NO CHANGE */
/* 	  1-7 LINE WIDTH */

/* 	HUNDREDS DIGIT = LINE TYPE SCALE FACTOR */
/* 	  0 = NO CHANGE */
/* 	  1-9 LINE TYPE SCALE FACTOR */

/* 	FIRST PICK UP PEN AT CURRENT LOCATION, THEN CHANGE LINE TYPE */

    plot_(&c_b19, &c_b19, &c__5);

    rmpen_(ip);
    ppen_(ip);
    vpen_(ip);
    return 0;
} /* newpen_ */



/* Subroutine */ int plotnd_()
{
    extern /* Subroutine */ int plot_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	TERMINATES PLOTS */

    plot_(&c_b19, &c_b19, &c__999);
    return 0;
} /* plotnd_ */



/* Subroutine */ int fixrm0_(orx, ory, zom, ang, rx, ry, nt, ns, icolor, 
	nchan)
real *orx, *ory, *zom, *ang, *rx, *ry;
integer *nt, *ns, *icolor, *nchan;
{
    static integer ierr;
    extern /* Subroutine */ int rmtexture_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	SET PLOT COMMON DATA TO ABSOLUTE KNOWN VALUES */
/* 	NO ERROR CHECKING PROVIDED */

    rmtek_1.rox = *orx;			/* X ORIGIN */
    rmtek_1.roy = *ory;			/* Y ORIGIN */
    rmtek_1.rsf = *zom;			/* ZOOM SCALE FACTOR */
    rmtek_1.rang = *ang;
/* 	RRE(1)=RX*RSF			! X DIRECTION RESOLUTION */
/* 	RRE(2)=RY*RSF			! Y DIRECTION RESOLUTION */
/* PLOTTING ANGLE */
    rmtek_1.mm = *nt;			/* LINE TYPE */
    rmtek_1.ipw = *ns;			/* PIXEL SCALING OF LINE TYPE */
    rmtek_1.ichan = *nchan;		/* CHANNEL NUMBER IF NCHAN > 0 */
    rmtek_1.icol = *icolor;		/* CURRENT COLOR TABLE VALUE */
    rmtexture_(&rmtek_1.ichan, &rmtek_1.mm, &rmtek_1.ipw, &rmtek_1.ipsc, &
	    ierr);
    return 0;
} /* fixrm0_ */


/* Subroutine */ int fixvt0_(orx, ory, zom, ang, rx, ry, nvt, isel, nt, nw, 
	ncol)
real *orx, *ory, *zom, *ang, *rx, *ry;
integer *nvt, *isel, *nt, *nw, *ncol;
{
    extern /* Subroutine */ int newvpen_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	SET PLOT COMMON DATA TO ABSOLUTE KNOWN VALUES */
/* 	NO ERROR CHECKING PROVIDED */

    vt100_1.vox = *orx;			/* X ORIGIN */
    vt100_1.voy = *ory;			/* Y ORIGIN */
    vt100_1.vsf = *zom;			/* ZOOM SCALE FACTOR */
    vt100_1.vang = *ang;
/* 	VRE(1)=RX*VSF			! X DIRECTION RESOLUTION */
/* 	VRE(2)=RY*VSF			! Y DIRECTION RESOLUTION */
/* PLOTTING ANGLE */
    vt100_1.ivt100 = *nvt;		/* NVT > 0 MEANS VT ENABLED */
    vt100_1.iterm = *isel;		/* INTERNAL TERMINAL CODE */
    vt100_1.mv = *nt;			/* LINE TYPE */
    vt100_1.mvw = *nw;			/* LINE WIDTH */
    vt100_1.ivcol = *ncol;		/* LINE COLOR */
    newvpen_(&vt100_1.mv, &vt100_1.mvw);/* CHANGE TERMINAL LINE TYPE */
    return 0;
} /* fixvt0_ */


/* Subroutine */ int fixpr0_(orx, ory, ax, ay, zom, ang, rx, ry, nvt, mxx, 
	mww, mcoll)
real *orx, *ory, *ax, *ay, *zom, *ang, *rx, *ry;
integer *nvt, *mxx, *mww, *mcoll;
{
    extern /* Subroutine */ int pplotp_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	SET PRINTER COMMON DATA TO ABSOLUTE KNOWN VALUES */
/* 	NO ERROR CHECKING PROVIDED */

    pxpcom_1.ox = *orx;			/* X ORIGIN */
    pxpcom_1.oy = *ory;			/* Y ORIGIN */
    pxpcom_1.px = *ax;			/* LAST SCALED, SHIFTED X */
    pxpcom_1.py = *ay;			/* LAST SCALED, SHIFTED Y */
    pxpcom_1.sf = *zom;			/* ZOOM SCALE FACTOR */
    pxpcom_1.pang = *ang;
/* 	RE(1)=RX*SF			! X DIRECTION RESOLUTION */
/* 	RE(2)=RY*SF			! Y DIRECTION RESOLUTION */
/* PLOTTING ANGLE */
    pxpcom_1.lu = *nvt;			/* NVT > 0 MEANS PR ENABLED */
    pxpcom_1.mx = *mxx;			/* LINE TYPE */
    pxpcom_1.mw = *mww;			/* LINE WIDTH */
    pxpcom_1.mcol = *mcoll;		/* LINE COLOR */
    if (pxpcom_1.lu > 0) {		/* WRITE DATA TO FILE */
	pplotp_(&pxpcom_1.mx, &pxpcom_1.msc, &c__1001); /* LINE TYPE, SCALE */
	pplotp_(&pxpcom_1.mw, &pxpcom_1.mcol, &c__1002);/* LINE COLOR */
	pplotp_(&pxpcom_1.mw, &c__0, &c__1003); 	/* LINE WIDTH */
    }
    return 0;
} /* fixpr0_ */



/* Subroutine */ int where_(x, y, z)
real *x, *y, *z;
{
/* 	C VERSION:   DGL DEC. 1993 */
/* 	RETURN LAST VALUES IN CALL TO PLOT. */

/* 	X,Y	(R): 	LAST POSITION */
/* 	Z	(R):	SCALE FACTOR--PRIORITIZED TERMINAL,RAMTEK,PRINTER */

    *x = lstplt_1.xlast;
    *y = lstplt_1.ylast;
    *z = pxpcom_1.sf;
    if (rmtek_1.ichan > 0)
	*z = rmtek_1.rsf;
    if (vt100_1.ivt100 > 0)
	*z = vt100_1.vsf;
    return 0;
} /* where_ */



/* Subroutine */ int absplt_(x, y, a, z)
real *x, *y, *a, *z;
{
    static integer ncol, isel, nchan, mcoll;
    extern /* Subroutine */ int fixrm0_(), fixpr0_(), fixvt0_();
    static real ax, ay;
    static integer ns, nt, nw;
    static real rx, ry;
    static integer icolor;
    static real ang;
    static integer nlu;
    static real zom;
    static integer nvt;
    static real orx, ory;
    static integer mww, mxx;
    extern /* Subroutine */ int whererm_(), wherepr_(), wherevt_();

/* 	C VERSION:   DGL DEC. 1993 */

/* 	SET PLOTTING ORIGIN, ANGLE, AND SCALE FACTOR TO ABSOLUTE UNITS */
/* 	NO ERROR CHECKING DONE */

/* 	X,Y	NEW ORIGIN */
/* 	A	NEW ANGLE */
/* 	Z	NEW SCALE FACTOR */

    wherevt_(&orx, &ory, &zom, &ang, &rx, &ry, &nvt, &isel, &nt, &nw, &ncol);
    fixvt0_(x, y, z, a, &rx, &ry, &nvt, &isel, &nt, &nw, &ncol);
    wherepr_(&orx, &ory, &ax, &ay, &zom, &ang, &rx, &ry, &nlu, &mxx, &mww, &
	    mcoll);
    fixpr0_(x, y, &ax, &ay, z, a, &rx, &ry, &nlu, &mxx, &mww, &mcoll);
    whererm_(&orx, &ory, &zom, &ang, &rx, &ry, &nt, &ns, &icolor, &nchan);
    fixrm0_(x, y, z, a, &rx, &ry, &nt, &ns, &icolor, &nchan);
    return 0;
} /* absplt_ */



/* Subroutine */ int savpl_()
{
    extern /* Subroutine */ int plot_();
    static real x, y, z;
    extern /* Subroutine */ int where_(), psavpl_(), rsavpl_(), vsavpl_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CENTRAL ROUTINE TO SAVE THE CURRENT PLOTTING STATE */

    where_(&x, &y, &z);		/* GET LAST PLOT COMMAND */
    plot_(&x, &y, &c__3);	/* DO PENUP */
    psavpl_();
    rsavpl_();
    vsavpl_();
    return 0;
} /* savpl_ */


/* Subroutine */ int respl_()
{
    extern /* Subroutine */ int plot_();
    static real x, y, z;
    extern /* Subroutine */ int where_(), prespl_(), rrespl_(), vrespl_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CENTRAL ROUTINE TO SAVE THE CURRENT PLOTTING STATE */

    where_(&x, &y, &z);		/* GET LAST PLOT COMMAND */
    plot_(&x, &y, &c__3);	/* DO PENUP */
    prespl_();
    rrespl_();
    vrespl_();
    return 0;
} /* respl_ */


/* Subroutine */ int psavpl_()
{ /* 	ROUTINE TO SAVE THE CURRENT PRINTER HISTORY FILE PLOTTING STATE */
  /* 	SAVES UP TO SIX LEVELS OF SOFTWARE STACK */
  /* 	REVISED C VERSION:   DGL DEC. 1996 */

    if (pxpcom_1.lu <= 0)
	return 0;
    if (pxpcom_ns > MAXSTACK-1)   /* STACK OVERFLOW */
	return 0;
    ++pxpcom_ns;
    pxpcom_store[pxpcom_ns] = pxpcom_1;
    return 0;
} /* psavpl_ */


/* Subroutine */ int prespl_()
{ /* 	RESTORES SAVED PRINTER HISTORY PLOTTING STATE */
    if (pxpcom_ns < 0) /* EMPTY STACK */
	return 0;
    pxpcom_1 = pxpcom_store[pxpcom_ns];
    pxpcom_ns--;
    pplotp_(&pxpcom_1.mx, &pxpcom_1.msc, &c__1001); /* LINE TYPE, SCALE */
    pplotp_(&pxpcom_1.mw, &pxpcom_1.mcol, &c__1002);/* LINE COLOR */
    pplotp_(&pxpcom_1.mw, &c__0, &c__1003);	    /* LINE WIDTH */
    return 0;
} /* prespl_ */


/* Subroutine */ int rsavpl_()
{ /* 	ROUTINE TO SAVE THE CURRENT RAMTEK PLOTTING STATE */
  /* 	SAVES UP TO SIX LEVELS OF SOFTWARE STACK */
  /* 	REVISED C VERSION:   DGL DEC. 1996 */
    if (rmtek_ns > MAXSTACK-1)		/* STACK OVERFLOW */
	return 0;
    ++rmtek_ns;
    rmtek_store[rmtek_ns] = rmtek_1;
    return 0;
} /* rsavpl_ */


/* Subroutine */ int rrespl_()
{ /* 	RESTORES SAVED RAMTEK PLOTTING STATE */
    integer ierr;
    if (rmtek_ns < 0)  /* EMPTY STACK */
	return 0;
    rmtek_1 = rmtek_store[rmtek_ns];
    rmtexture_(&rmtek_1.ichan, &rmtek_1.mm, &rmtek_1.ipw, &rmtek_1.ipsc,
	    &ierr);
    --rmtek_ns;
    return 0;
} /* rrespl_ */


/* Subroutine */ int vsavpl_()
{ /* 	ROUTINE TO SAVE THE CURRENT TERMINAL PLOTTING STATE */
  /* 	SAVES UP TO SIX LEVELS OF SOFTWARE STACK */
  /* 	REVISED C VERSION:   DGL DEC. 1996 */
    if (vt100_ns > MAXSTACK-1)		/* STACK OVERFLOW */
	return 0;
    ++vt100_ns;
    vt100_store[vt100_ns] = vt100_1;
    return 0;
} /* vsavpl_ */

/* Subroutine */ int vrespl_()
{ /* 	RESTORES SAVED TERMINAL PLOTTING STATE */
    real r__1;
    if (vt100_ns < 0) 	/* EMPTY STACK */
	return 0;
    vt100_1 = vt100_store[vt100_ns];
    newvpen_(&vt100_1.mv, &vt100_1.mvw);
    r__1 = (real) vt100_1.ivcol;
    plotvt_(&r__1, &c_b19, &c__0);
    --vt100_ns;
    return 0;
} /* vrespl_ */


/* Subroutine */ int terscl_(id, it, x, y, ix, iy)
integer *id, *it;
real *x, *y;
integer *ix, *iy;
{
    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    static real tang, av1, av2;

/* 	AUTHOR:   DGL AUG, 1990 */
/* 	COMPUTES THE TRANSFORMATION TO/FROM TERMINAL COORDINATES */

/* 	PARAMETERS: */
/* 	  ID :	TRANSFORMATION DIRECTION FLAG (INPUT) */
/* 		 > 0 FORWARD (STANDARD TO TERMINAL PIXEL COORDINATES) */
/* 		ELSE REVERSE (FROM TERMINAL PIXEL COORINATES TO STANDARD) */
/* 	  IT :  TERMINAL FLAG (RETURNED) */
/* 		 -1 : TERMINAL PACKAGE NOT ENABLED */
/* 		  0 : NON-TEKTRONICS 42XX COMPATIBLE TERMINAL */
/* 		  1 : TEKTRONICS 42XX COMPATIBLE TERMINAL */
/* 	  X,Y:  STANDARD COORDINATES */
/* 		  INPUT IF ID>0, OUTPUT OTHERWISE */
/* 	  IX,IY:TERMINAL PIXEL COORDINATES */
/* 		  OUTPUT IF ID>0, INPUT OTHERWISE */

    *it = -1;		/* ERROR RETURN */
    if (vt100_1.ivt100 <= 0)
	return 0;

/* 	RETURN TERMINAL TYPE */
    *it = 0;
/* NON-TEXTRONICS 42XX COMPATIBLE TERMINAL */
    if (vt100_1.iterm == 6)
	*it = 1;

/* 	COMPUTE TRANSFORMATION, ROTATION, AND SCALING */
/* Tektronics 4107/4109 */
    tang = vt100_1.vang * (float).017453294;
    if (*id > 0) {		/* FORWARD TRANSFORMATION */
	av1 = *x * cos(tang) - *y * sin(tang);
	av2 = *x * sin(tang) + *y * cos(tang);
	*ix = (vt100_1.vsf * av1 + vt100_1.vox) / vt100_1.vre[0];
	*iy = (vt100_1.vsf * av2 + vt100_1.voy) / vt100_1.vre[1];
	if (vt100_1.iyrev == 1) {
	    *iy = vt100_1.iylim - *iy;
	}
    } else {			/* INVERSE TRANSFORMATION */
	if (vt100_1.iyrev == 1) {
	    *iy = vt100_1.iylim - *iy;
	}
	av1 = (*ix * vt100_1.vre[0] - vt100_1.vox) / vt100_1.vsf;
	av2 = (*iy * vt100_1.vre[1] - vt100_1.voy) / vt100_1.vsf;
	*x = av1 * cos(tang) + av2 * sin(tang);
	*y = -(doublereal)av1 * sin(tang) + av2 * cos(tang);
    }

    return 0;
} /* terscl_ */

/***************************************************************************/

/* THE FOLLOWING ROUTINES ARE BASED ON THE DEC VAX ASYNCHRONOUS CONTROL-C */
/* INTERRUPT HANDLING PROTOCALL.  "ENAST" IS A MACHINE DEPENDENT ROUTINE */
/* WHICH ENABLES/DISABLES THE CONTROL-C INTERRUPT.  IT IS CALLED BY "ENABLEAST"*/
/* TO ENABLE THE INTERRUPT AND BY "ASTEXIT" (WHICH IS CALLED WHEN INTERRUPT*/
/* OCCURS).  IN THIS FILE, WE REPLACE THESE ROUTINES WITH DUMMY CALLS SO */
/* THAT THE CONTROL-C ROUTINES ARE NOT USED.  A SEPARATE FILE CONTAINS */
/* THE WORKING CODE.  THE OTHER FILE CAN BE SUBSTITUTED TO ALLOW CONTROL-C */
/* INTERRUPTS.  WHEN AN INTERRUPT OCCURS, WE SET A FLAG IN THE "ASTC" */
/* COMMON BLOCK AND CONTINUE IF THE TERMINAL PACKAGE IN LONGLIB IS IN USE */
/* OTHERWISE WE DIRECTLY CALL THE INTERRUPT HANDLER "ASTINTER".  IF THE */
/* TERMINAL PACKAGE IS IN USE, EACH TIME "PLOT" IS CALLED WE CHECK THE STATUS*/
/* OF THE FLAG.  IF THE FLAG IS SET WE CALL THE INTERRUPT HANDLER "ASTINTER"*/
/* WHICH CAN EXIT THE PROGRAM, RESET TERMINAL CONDITION, ETC.  IN THIS WAY */
/* WE CAN USE THE CONTROL-C STUFF WITH OR WITHOUT LONGLIB BE USED AT ALL. */
/* IF YOU WISH TO USE THE CONTROL-C STUFF WITHOUT THE REST OF LONGLIB, */
/* CALL "ENABLEAST" FROM YOUR CODE.  THE LONGLIB ROUTINE "FRAME" CALLS */
/* "ENABLAST" WHEN THE LONGLIB PACKAGE IS USED. */

/* YOU CAN ELIMINATE THESE ROUTINES IF DESIRED.  IF THIS IS DONE BE SURE */
/* TO COMMENT OUT THE CONTROL-C LINES IN THE ROUTINE 'PLOT' AND 'FRAME'. */

/* ********************************************************************** */

/* Subroutine */ int enableast_()
{

/* 	C VERSION:   DGL DEC. 1993 */
/* 	ENABLE CONTROL-C INTERRUPT */
/* 	DUMMY ROUTINE */

    astc_1.iastfl = 0;			/* CONTROL-C INTERRUPT FLAG */
    astc_1.iastpl = 0;			/* CURRENTLY PLOTTING FLAG */
    return 0;
} /* enableast_ */



/* Subroutine */ int enast_(iopt)
integer *iopt;
{
/* 	C VERSION:   DGL DEC. 1993 */
/* 	CONTROL-C INTERRUPT QIO SET UP */

/* 	DUMMY ROUTINE */

    return 0;
} /* enast_ */



/* Subroutine */ int astexit_(iopt)
integer *iopt;
{
    extern /* Subroutine */ int astinter_(), enast_();

/* 	C VERSION:   DGL DEC. 1993 */

/* 	ROUTINE CALLED IN THE EVENT OF A CONTROL-C FROM THE TERMINAL. */
/* 	A FLAG (IASTFL) IS SET AND IF WE ARE PLOTTING TO THE TERMINAL */
/* 	THE ROUTINE RETURNS.  IF NOT PLOTTING TO THE TERMINAL, THE */
/* 	CONTROL-C HANDLER IS CALLED IMMEDIATELY */

    enast_(iopt);			/* RESET CONTORL-C QIO */
    if (astc_1.iastfl < 0)
	return 0;
/* 					!  A CONTROL-C INTERRUPT */
/* NO CONTROL-C INTERRUPTS DURING */
    if (vt100_1.ivt100 < 0) {		/* TERMINAL PACKAGE NOT IN CURRENT USE */
	astc_1.iastfl = 0;		/* IMMEDIATELY CALL INTERRUPT HANDLER */
	astinter_(&astc_1.iastfl);
    } else {				/* TERMINAL PACKAGE IN USE */
	if (astc_1.iastpl == 1) {	/* CURRENTLY PLOTTING, INTERRUPT LATER */
	    astc_1.iastfl = 1;
	} else {			/* NOT PLOTTING, INTERRUPT NOW */
	    astc_1.iastfl = 0;
	    astinter_(&astc_1.iastfl);
	}
    }
    return 0;
} /* astexit_ */



/* Subroutine */ int astinter_(iv)
integer *iv;
{
    /* Local variables */
    static char ians;
    static integer ismode;

/* 	CONTROL-C INTERUPT HANDLING CODE */
/* 	USES THE CURRENT MODE OF THE GRAPHICS TERMINAL TO INSURE THAT */
/* 	TERMINAL IS IN TERMINAL MODE AND IS RETURNED TO GRAPHICS MODE */
/* 	IF IN THAT MODE.  IF LONGLIB IS NOT ENABLED, THE OPTION TO CONTINUE */
/* 	OR ABORT PROGRAM IS GIVEN. */

    astc_1.iastfl = -1;		/* ENTERING HANDLER SO FLAG NO INTERRUPT */
    ismode = vt100_1.imode;	/* SAVE CURRENT GRAPHICS/TERMINAL MODE */
    printf("\n *** CONTROL-C INTERRUPT ***\n");
L505:
    printf("Continue or Abort? ");
    ians = getch();
    if (ians == EOF)
	goto L99;

    if (ians == '?') {
	printf(" X,Z : Abort program\n   C : Clear Screen\n");
	printf("   Q : Quit (disable screen plotting)\n");
	printf("   S : Skip screen plotting until next 'cterm(2)'\n");
	printf("   R : Reset skip\n   U : Unquit\n");
	printf("   T : Terminal mode\n   G: Graphics mode\n");
	goto L505;
    }
    if (ians == 'Z' || ians == 'z') {		/* TERMINATE PROGRAM */
	goto L99;
    }
    if (ians == 'X' || ians == 'x') {		/* TERMINATE/CLEAR */
	goto L99;
    }
    if (ians == 'C' || ians == 'c') {		/* CLEAR SCREEN */
    }
    if (ians == 'Q' || ians == 'q') {		/* TURN OFF SCREEN PLOTTING */
	vt100_1.ivt100 = -98;
    }
    if (ians == 'S' || ians == 's') {		/* SKIP PLOTTING */
	vt100_1.ivt100 = -99;
    }
    if (ians == 'R' || ians == 'r' && vt100_1.ivt100 == -99) { /* RESET SKIP */
	vt100_1.ivt100 = 1;
	ismode = 1;
    }
    if (ians == 'U' || ians == 'u' && vt100_1.ivt100 == -98) { /* UNQUIT */
	vt100_1.ivt100 = 1;
	ismode = 1;
    }
    if (ians == 'T' || ians == 't') {		/* TERMINAL MODE */
	ismode = 0;
    }
    if (ians == 'G' || ians == 'g') {		/* GRAPHICS MODE */
	ismode = 1;
    }
    if (ismode == 1) {				/* RESTORE TERMINAL GRAPHICS MODE */
    }
    astc_1.iastfl = 0;				/* EXIT HANDLER */
    return 0;
L99:
    exit(0);				/* TERMINATE PROGRAM */
    return -1;
} /* astinter_ */



/**************************************************************************/

/* 	AUXILARY ROUTINES FOR USE WITH THE RAMTEK PACKAGE OF LONGLIB */
/* 	BOTH RAMTEK AND RAMTEK EMULATION FILE ROUTINES ARE SUPPORTED BY */
/* 	THESE ROUTINES */

/**************************************************************************/

/* Subroutine */ int rmpix_(x, y, ix0, iy0)
real *x, *y;
integer *ix0, *iy0;
{
    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    static real cosa, sina;

/* 	RETURNS RAMTEX PIXEL (IX0,IY0) CORRESPONDING TO LONGLIB POINT (X,Y) */
/* 	(NO CLIPPING) */

    if (rmtek_1.ichan < 0)
	return 0;

/* 	TRANSFORM INPUT POINT */
    cosa = cos(rmtek_1.rang * (float).0174532);
    sina = sin(rmtek_1.rang * (float).0174532);
    *ix0 = ((cosa * *x - sina * *y) * rmtek_1.rsf + rmtek_1.rox) / 
	    rmtek_1.rre[0];
    *iy0 = rmtek_1.irylim - (integer) (((sina * *x + cosa * *y) * rmtek_1.rsf 
	    + rmtek_1.roy) / rmtek_1.rre[1] + (float).499);

/* 	CODE TO INCLUDE CLIPPING */

/* 	IF (IX0.LT.0) IX0=0 */
/* 	IF (IX0.GT.IRXLIM) IX0=IRXLIM */
/* 	IF (IY0.LT.0) IY0=0 */
/* 	IF (IY0.GT.IRYLIM) IY0=IRYLIM */

    return 0;
} /* rmpix_ */



/* Subroutine */ int rmpixb_(x, y, ix0, iy0)
real *x, *y;
integer *ix0, *iy0;
{
    /* Builtin functions */
    double cos(), sin();

    /* Local variables */
    static real cosa, sina;
    static integer iy;

/* 	GIVEN A PIXEL LOCATION ON THE RAMTEK, RETURNS THE LONGLIB */
/* 	COORDINATE VALUE */

    if (rmtek_1.ichan < 0)
	return 0;

    cosa = cos(rmtek_1.rang * (float).0174532);
    sina = sin(rmtek_1.rang * (float).0174532);
    iy = rmtek_1.irylim - *iy0;
    *x = (((real) (*ix0) * rmtek_1.rre[0] - rmtek_1.rox) * cosa + ((real) iy *
	     rmtek_1.rre[1] - rmtek_1.roy) * sina) / rmtek_1.rsf;
    *y = (-(doublereal)((real) (*ix0) * rmtek_1.rre[0] - rmtek_1.rox) * sina 
	    + ((real) iy * rmtek_1.rre[1] - rmtek_1.roy) * cosa) / 
	    rmtek_1.rsf;

    return 0;
} /* rmpixb_ */



/* Subroutine */ int rmsize_(xlen, ylen, nxsize, nysize)
real *xlen, *ylen;
integer *nxsize, *nysize;
{

/* 	RETURNS RAMTEX SCREEN SIZE IN LONGLIB PACKAGE */
/* 	  IN CURRENT PLOT UNITS (XLEN,YLEN) */
/* 	  AND PIXELS (NXSIZE,NYSIZE) */

    if (rmtek_1.ichan < 0)
	return 0;

/* 	TRANSFORM INPUT POINT */
    *xlen = rmtek_1.irxlim * rmtek_1.rre[0] / rmtek_1.rsf;
    *ylen = rmtek_1.irylim * rmtek_1.rre[1] / rmtek_1.rsf;
    *nxsize = rmtek_1.irxlim;
    *nysize = rmtek_1.irylim;

    return 0;
} /* rmsize_ */



/* Subroutine */ int rmsetsize_(xlen, ylen, nxsize, nysize)
real *xlen, *ylen;
integer *nxsize, *nysize;
{
    extern /* Subroutine */ int plotrm_();

/* 	RESETS THE RAMTEK SCREEN SIZE IN THE LONGLIB RAMTEK PACKAGE */
/* 	  (XLEN,YLEN) IN PLOT UNITS  XLEN<1280, Y<1024 */
/* 	  (NXSIZE,NYSIZE) < (1280,1024) IN PIXELS */
/* 	ORIGIN RESET TO (0,0) AND SCALE FACTOR RESET TO ZERO. */
/* 	VIEWPORT RESET TO FULL SCREEN AND PLOT ANGLE RESET TO ZERO */
/* 	PEN SHOULD BE UP BEFORE CALLING THIS ROUTINE */

    if (rmtek_1.ichan < 0)
	return 0;

/* 	TRANSFORM INPUT POINT */

    rmtek_1.irxlim = *nxsize;
    rmtek_1.irylim = *nysize;
    rmtek_1.rsf = (float)1.;
    rmtek_1.rox = (float)0.;
    rmtek_1.roy = (float)0.;
    rmtek_1.rang = (float)0.;
    rmtek_1.rre[0] = *xlen / rmtek_1.irxlim;
    rmtek_1.rre[1] = *ylen / rmtek_1.irylim;
    rmtek_1.rlim[0] = (real) rmtek_1.irxlim;
    rmtek_1.rlim[1] = (real) rmtek_1.irylim;
    rmtek_1.rvp[0] = (float)0.;		/* VIEW PORT PARAMETERS */
    rmtek_1.rvp[1] = (float)0.;		/* LOWER LEFT */
    plotrm_(&c_b24, &c_b24, &c__4);	/* UPPER RIGHT */
    return 0;
} /* rmsetsize_ */



/* Subroutine */ int rammap_(iarray, na, ma, n, m, ixs, iys, bx, by)
integer *iarray, *na, *ma, *n, *m, *ixs, *iys;
real *bx, *by;
{
    /* System generated locals */
    integer iarray_dim1, i__1, i__2;
    real r__1;

    /* Builtin functions */
    integer i_nint();

    /* Local variables */
    extern /* Subroutine */ int astinter_();
    static integer mplot[4], ix, iy;
    extern /* Subroutine */ int rambox_();
    static integer iy1, iby, jnt;
    extern /* Function */ integer irmchan_();
    int ichan;

/* 	PROGRAM TO MAKE AN IMAGE MAP ON THE RAMTEK DEVICE */

/* 	AUTHOR: DG LONG, JPL */
/* 	DATE: 17 JAN 1991 */

/* 	INPUTS: */
/* 	  IARRAY  : COLOR CODE (RAMTEK COLOR CODE NUMBERS) */
/* 	  NA,MA	  : DIMENSIONS OF IARRAY */
/* 	  N,M	  : PLOT N BY M PORTION OF IARRAY */
/* 	  IXS,IYS : POSITION OF LOWER LEFT CORNER OF MAP (IN RAMTEK PIXELS) */

/* 	  BX,BY   : BOX SIZE (IN RAMTEK PIXELS: 0 < BX < 1024) */
/* 			IF BY < 0 REVERSE ARRAY */

/* 	CALLS: */
/* 	  RMPLOT */

/* CONTROL-C */

/* 	MAKE MAP IN ROW ORDER FOR EFFICIENCIES SAKE */

    /* Parameter adjustments */
    iarray_dim1 = *na;

    ichan = irmchan_(&i__1);

    /* Function Body */
    i__1 = *n;
    for (ix = 1; ix <= i__1; ++ix) {
	i__2 = *m;
	for (iy1 = 1; iy1 <= i__2; ++iy1) {
	   astc_1.iastpl = 1;/* CONTROL-C */
	    iy = iy1;
	    if (iby < 0)
		iy = *m - iy1 + 1;
	    jnt = iarray[ix + (iy - 1) * iarray_dim1];
	    r__1 = *bx * (ix - 1);
	    mplot[0] = *ixs + i_nint(&r__1);
	    r__1 = *by * (iy1 - 1);
	    mplot[1] = *iys + i_nint(&r__1);
	    r__1 = *bx * ix;
	    mplot[2] = *ixs + i_nint(&r__1);
	    r__1 = *by * iy1;
	    mplot[3] = *iys + i_nint(&r__1);
	    rambox_(&ichan, &mplot[0], &mplot[1], &mplot[2], &mplot[3], &jnt);
	    astc_1.iastpl = 0;			/* CONTROL-C */
	    if (astc_1.iastfl != 0)
		astinter_(&astc_1.iastfl);	/* CONTROL-C */
	}
    }

    return 0;
} /* rammap_ */



/* Subroutine */ int rammap2_(iarray, na, ma, n, m, ixs, iys, bx, by)
shortint *iarray;
integer *na, *ma, *n, *m, *ixs, *iys;
real *bx, *by;
{
    /* System generated locals */
    integer iarray_dim1, i__1, i__2;
    real r__1;

    /* Builtin functions */
    integer i_nint();

    /* Local variables */
    extern /* Subroutine */ int astinter_();
    static integer mplot[4], ix, iy;
    extern /* Subroutine */ int rambox_();
    extern /* Function */ integer irmchan_();
    int ichan;
    static integer iy1, iby, jnt;

/* 	PROGRAM TO MAKE AN IMAGE MAP ON THE RAMTEK DEVICE */

/* 	AUTHOR: DG LONG, JPL */
/* 	DATE: 17 JAN 1991 */

/* 	INPUTS: */
/* 	  IARRAY  : COLOR CODE (RAMTEK COLOR CODE NUMBERS) */
/* 	  NA,MA	  : DIMENSIONS OF IARRAY */
/* 	  N,M	  : PLOT N BY M PORTION OF IARRAY */
/* 	  IXS,IYS : POSITION OF LOWER LEFT CORNER OF MAP (IN RAMTEK PIXELS) */

/* 	  BX,BY   : BOX SIZE (IN RAMTEK PIXELS: 0 < BX < 1024) */
/* 			IF BY < 0 REVERSE ARRAY */

/* 	CALLS: */
/* 	  RMPLOT */

/* 	MAKE MAP IN ROW ORDER FOR EFFICIENCIES SAKE */

    /* Parameter adjustments */
    iarray_dim1 = *na;

    ichan = irmchan_(&i__1);

    /* Function Body */
    i__1 = *n;
    for (ix = 1; ix <= i__1; ++ix) {
	i__2 = *m;
	for (iy1 = 1; iy1 <= i__2; ++iy1) {
	    iy = iy1;
	    if (iby < 0) {
		iy = *m - iy1 + 1;
	    }
	    jnt = iarray[ix + (iy - 1) * iarray_dim1];
	    r__1 = *bx * (ix - 1);
	    mplot[0] = *ixs + i_nint(&r__1);
	    r__1 = *by * (iy1 - 1);
	    mplot[1] = *iys + i_nint(&r__1);
	    r__1 = *bx * ix;
	    mplot[2] = *ixs + i_nint(&r__1);
	    r__1 = *by * iy1;
	    mplot[3] = *iys + i_nint(&r__1);
	    rambox_(&ichan, &mplot[0], &mplot[1], &mplot[2], &mplot[3], &jnt);
	    astc_1.iastpl = 0;			/* CONTROL-C */
	    if (astc_1.iastfl != 0)
		astinter_(&astc_1.iastfl);	/* CONTROL-C */
/* L5: */
	}
/* L10: */
    }

    return 0;
} /* rammap2_ */


/* Subroutine */ int chchan_(ichan2)
integer *ichan2;
{
    rmtek_1.ichan = *ichan2;
    return 0;
} /* chchan_ */

