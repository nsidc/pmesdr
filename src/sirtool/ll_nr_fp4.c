
/* *** LAST REVISED ON 13-OCT-1994 06:59:25.96 */
/* *** SOURCE FILE: [LONGLIB93.SOURCES.C.LONGLIB]LL_NR.C */

/* revised to eliminate printer and ramtek calls 
   terminal specific routines also eliminated */

#include <stdlib.h>
#include <stdio.h>

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


/* allocate key common blocks */

vt100_common vt100_;
#define vt100_1 vt100_


#define MAXSTACK 6
static vt100_common vt100_store[MAXSTACK];
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
    integer i__1;
    real r__1;

    /* Local variables */
    static integer iddn, ians;
    static real z;
    extern  vplots_();

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


    kpl = *ipl;
    z = *zom;
    idd = abs(*id);			
    iddn = - (*id >= 0 ? 1 : -1);	/* SET CLEAR SCREEN FLAG */
    if (idd == 0) {  /* ordinarily, prompt -- taken out for this version */
    }
    
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
    }
    return 0;
L99:

/* 	DO IMMEDIATE EXIT FROM PROGRAM */
    (void) exit(-1);
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

    plotvt_(xa, ya, i0);
    lstplt_1.xlast = *xa;
    lstplt_1.ylast = *ya;
    lstplt_1.i0last = *i0;
    return 0;
} /* plot_ */


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

/* 	C VERSION:   DGL DEC. 1993 */

/* 	THIS ROUTINE CAUSES A RESETING OF THE PRINTER PAGE WITHOUT */
/* 	AFFECTING SCREEN DEVICES */

    return 0;
} /* newpage_ */


/* Subroutine */ int factor_(sc)
real *sc;
{
    extern /* Subroutine */ int vfactor_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CHANGE SCALE FACTORS FOR ALL PLOT PACKAGES */

/* 	SC  SCALE FACTOR */

    if (vt100_1.ivt100 >= 0)
	vfactor_(sc);
    return 0;
} /* factor_ */


/* Subroutine */ int newpen_(ip)
integer *ip;
{
    extern /* Subroutine */ int vpen_(), plot_();

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



/* Subroutine */ int where_(x, y, z)
real *x, *y, *z;
{
/* 	C VERSION:   DGL DEC. 1993 */
/* 	RETURN LAST VALUES IN CALL TO PLOT. */

/* 	X,Y	(R): 	LAST POSITION */
/* 	Z	(R):	SCALE FACTOR--PRIORITIZED TERMINAL,RAMTEK,PRINTER */

    *x = lstplt_1.xlast;
    *y = lstplt_1.ylast;
    *z = vt100_1.vsf;
    return 0;
} /* where_ */



/* Subroutine */ int absplt_(x, y, a, z)
real *x, *y, *a, *z;
{
    static integer ncol, isel, nchan, mcoll;
    extern /* Subroutine */ int fixvt0_();
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
    extern /* Subroutine */ int wherevt_();

/* 	C VERSION:   DGL DEC. 1993 */

/* 	SET PLOTTING ORIGIN, ANGLE, AND SCALE FACTOR TO ABSOLUTE UNITS */
/* 	NO ERROR CHECKING DONE */

/* 	X,Y	NEW ORIGIN */
/* 	A	NEW ANGLE */
/* 	Z	NEW SCALE FACTOR */

    wherevt_(&orx, &ory, &zom, &ang, &rx, &ry, &nvt, &isel, &nt, &nw, &ncol);
    fixvt0_(x, y, z, a, &rx, &ry, &nvt, &isel, &nt, &nw, &ncol);
    return 0;
} /* absplt_ */



/* Subroutine */ int savpl_()
{
    extern /* Subroutine */ int plot_();
    static real x, y, z;
    extern /* Subroutine */ int where_(), vsavpl_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CENTRAL ROUTINE TO SAVE THE CURRENT PLOTTING STATE */

    where_(&x, &y, &z);		/* GET LAST PLOT COMMAND */
    plot_(&x, &y, &c__3);	/* DO PENUP */
    vsavpl_();
    return 0;
} /* savpl_ */


/* Subroutine */ int respl_()
{
    extern /* Subroutine */ int plot_();
    static real x, y, z;
    extern /* Subroutine */ int where_(), vrespl_();

/* 	C VERSION:   DGL DEC. 1993 */
/* 	CENTRAL ROUTINE TO SAVE THE CURRENT PLOTTING STATE */

    where_(&x, &y, &z);		/* GET LAST PLOT COMMAND */
    plot_(&x, &y, &c__3);	/* DO PENUP */
    vrespl_();
    return 0;
} /* respl_ */


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


