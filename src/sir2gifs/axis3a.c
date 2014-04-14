
/* *** LAST REVISED ON 24-DEC-1993 07:39:06.19 */
/* *** SOURCE FILE: [LONGLIB93.SOURCES.C.AUXLIB]AXIS3.C */
/* updated to include stdio to define sprintf dgl  2 Apr 2011 */

#include <stdio.h>

#define abs(x) ((x) >= 0 ? (x) : -(x))
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define dabs(x) (doublereal)abs(x)
#define dmin(a,b) ((a) <= (b) ? (a) : (b))
#define dmax(a,b) ((a) >= (b) ? (a) : (b))

/* Table of constant values */

float c_b94 = (float)1e3;
float c_b8 = (float)0.;
int c__0 = 0;
int c__3 = 3;
int c__2 = 2;
double c_b117 = 10.;
int c_n1 = -1;
int c_n3 = -3;
int c__4 = 4;
int c__1 = 1;


int axis3a_(float *x0, float *y0, char *t, int *n0, float *s0,
	    float *a0, float *b0, float *c0, float *d0, float *e0, float *f0, 
	    int *icol)
{
    float space = 6.0f;

    int i__1, i__2, i__3;
    float r__1, r__2;
    double d__1;

    double cos(double), sin(double), r_lg10(float *), r_mod(float *, 
	    float *), r_int(float *), pow_dd(double *, double *);

    extern /* Subroutine */ int plot_(float *, float *, int *);
    int i, j, k;
    float e1, c1;
    int n1;
    float x1, y1, y2, x2, x3, fa;
    int nc;
    float co;
    int nd, ng;
    float cs, x01, y01, si, tl, xj, xn, xs;
    extern /* Subroutine */ int numbera_(float *, float *, float *, float *, float *
	    , float *, int *);
    extern float syms_(float *, float *, float *, char *, 
	    float *, int *, int *);
    float tl1, ang;
    int ndd;
    float hor;
    int njt;
    float dnx, dny;
    int nnt;
    float dty, dtx;
    int nst;


/* 	WRITTEN BY DGL  17-OCT-1983 */
/* 	REVISED BY DGL  28-AUG-1987 */

/* 	PLOTS A SINGLE COORDINATE AXIS USING SYMBOL AND NUMBER. */
/* 	THIS VERSION OF AXIS CAN HANDLE NON-INT AXIS LENGTH */
/* 	AND PERMITS FORMATTING OF THE NUMERIC LABELS. */

/* 	X0  (R)	X COORDINATE OF START OF AXIS */
/* 	Y0  (R)	Y COORDINATE OF START OF AXIS */
/* 	T   (C)	CHARACTER STRING TO DESCRIBE AXIS */
/* 	N0  (I)	NUMBER OF CHARACTERS IN STRING */
/* 		- ON CLOCKWISE SIDE OF AXIS (NORMAL FOR X) */
/* 		+ ON COUNTER CLOCKWISE SIDE OF AXIS (NORMAL FOR Y) */
/* 		HUNDREDS DIGIT	= 1 NO LABELING OF AXIS--TICKS AND LINE ONLY */
/* 		THOUSANDS DIGIT = 1 HORIZONTAL NUMBERS IN Y AXIS LABEL */
/* 	S0  (R)	LENGTH OF AXIS IN INCHES */
/* 		< 0	TICKS PLACED ON OPPOSITE SIDE OF AXIS LABEL */
/* 		= 0	NO ACTION (NOP) */
/* 		> 0	NORMAL */
/* 	A0  (R)	ANGLE OF AXIS WITH RESPECT TO X AXIS OF PAPER */
/* 			0.0 FOR X-AXIS */
/* 			90.0 FOR Y-AXIS */
/* 	B0  (R)	MINIMUM VALUE ON TICK AXIS */
/* 	C0  (R)	MAXIMUM VALUE ON TICK AXIS */
/* 	D0  (R)	INT(D0) = NUMBER OF MAJOR AXIS TICKS */
/*		INT((INT(D0)-D0)*100) = NUMBER OF MINOR AXIS TICKS BETWEEN MAJOR TICKS*/
/* 		INT(MOD((INT(D0)-D0)*10000,100) = NUMBER OF SUB MINOR AXIS TICKS */
/* 			BETWEEN MINOR TICKS */
/* 	E0  (R)	CHARACTER SIZE OF TITLE AND NUMBER (IF E0=0, DEFAULTS TO 0.15) */
/* 			< 0 THEN DO NOT AUTO SCALE BY (x10 TO POWER) */
/* 	F0  (R)	NUMBER SPECIFICATION (FORMAT FX.Y) */
/* 		 (SEE NUMBER) */
/* 		 -1 WILL AUTOMATICALLY GENERATE A "NICE" FORMAT */
/* 	ICOL (I) OPTIONAL COLOR ARRAY (ACCESSED ONLY IF 100000'S DIGIT <> 0) */
/* 		 ICOL(1) = AXIS LINE AND TICK COLOR */
/* 		 ICOL(2) = NUMERIC LABEL COLOR */
/* 		 ICOL(3) = TITLE COLOR */
/* 		 ICOL(4) = EXPONENT COLOR */
/* 		PEN COLOR ON RETURN WILL DEPEND ON OPTIONS IN THE */
/* 		ORDER LISTED ABOVE */

    tl = 12.1f;				/* TICK LENGTH */
    x01 = *x0;
    y01 = *y0;
    ang = *a0;				/* ROTATION ANGLE */
    e1 = *e0;				/* CHARACTER SIZE */
    cs = abs(e1);
    if (cs == 0.f) 
	cs = .15f;
    if (*s0 == 0.f) 			/* ZERO LENGTH AXIS */
	goto L1000;

    x1 = *d0 * 1.000002f;
    njt = abs(x1);			/* NUMBER OF MAJOR TICKS */
    nnt = (abs(x1) - njt) * 100.f;	/* NUMBER OF MINOR TICKS */
    nst = ((abs(x1) - njt) * 100.f - nnt) * 100.f; /* NUMBER OF SUB-MINOR TICKS */
    if (njt < 2)
	njt = 2;
    xj = abs(*s0) / (njt - 1);		/* INCREMENT BETWEEN MAJOR TICKS */
    xn = xj;
    if (nnt != 0) 			/* INCREMENT BETWEEN MINOR TICKS */
	xn /= nnt + 1;
    xs = xn / (nst + 1);		/* INCREMENT BETWEEN SUB-MINOR TICKS */
    n1 = *n0 % 100000;
    if (abs(n1) / 1000 != 0) 		/* ROTATION ANGLE */
	hor = 90.f;
    co = cos(ang * .017453294f);	/* AXIS ANGLE ROTATION */
    si = sin(ang * .017453294f);
    hor = ang;				/* ANGLE OF NUMBER LABELS */
    nc = abs(n1) % 100;

/* 	DECODE NUMBER FORMAT */

/* NUMBER OF CHARACTERS IN TITLE */
    fa = *f0;
    if (fa == -1.f) {			/* INPUT WAS -1.0 */
	fa = 1003.f;			/* DEFAULT AUTO SCALING FORMAT */
	if (e1 < 0.f) {			/* NO AUTO SCALING SO MAKE */
	    ng = 1;			/* FORMAT TO FIT */
	    if (*b0 != 0.f) {		/* Computing MAX */
		r__1 = abs(*b0);
		i__1 = ng, i__2 = (int) (r_lg10(&r__1) + .001f);
		ng = max(i__1,i__2);
	    }
	    if (*c0 != 0.f) {		/* Computing MAX */
		r__1 = abs(*c0);
		i__1 = ng, i__2 = (int) (r_lg10(&r__1) + .001f);
		ng = max(i__1,i__2);
	    }
	    if (*b0 < 0.f || *c0 < 0.f)
		++ng;

	    fa = (float) ng + 1e3f;
	}
    }
    if (abs(fa) > 1e3f) {		/* INPUT DESIRED IS INT */
	nd = abs(fa) - 1e3f;		/* INPUT INT VALUE */
	fa = (float) nd * 1.01f + 3.f;	/* DEFAULT FORMAT FOR NOT AUTOSCALE */
	if (e1 < 0.f) {			/* NO AUTO SCALING TO MAKE FORMAT */
	    ng = 2;			/* WHICH FITS */
	    if (*b0 != 0.f) {		/* Computing MAX */
		r__1 = abs(*b0);
		i__1 = ng, i__2 = (int) (r_lg10(&r__1) + .001f) + 1;
		ng = max(i__1,i__2);
	    }
	    if (*c0 != 0.f) {		/* Computing MAX */
		r__1 = abs(*c0);
		i__1 = ng, i__2 = (int) (r_lg10(&r__1) + .001f) + 1;
		ng = max(i__1,i__2);
	    }
	    if (*b0 < 0.f || *c0 < 0.f)
		++ng;
	    fa = (float) ng + (float) nd * 1.01f;
	}
    }
    r__1 = abs(fa);
    nd = r_mod(&r__1, &c_b94);		/* NUMBER OF DIGITS */
    r__1 = abs(fa);
    ng = (r_mod(&r__1, &c_b94) - nd + 1e-4f) * 100.f;/* NUMBER OF DIGITS RIGHT OF D.P. */
    if (ng > 17) 			/* CORRECT INPUT SIMPLE ERRORS */
	ng /= 10;
    if (fa < 0.f) 			/* EXPONENTIAL NOTATION */
	nd += -4;
    if (nd <= 0)
	nd = ng;
    ndd = nd;
    if (fa < 0.f) 			/* EXPONENTIAL NOTATION */
	ndd = nd + 4;
    if (abs(fa) > 1e3f) 		/* FORMATTED INT */
	ng = -1;

    tl1 = tl;
    if (*s0 < 0.f) 			/* REVERSE SIDE OF TICKS */
	tl1 = -tl;
    if (*s0 < 0.f) 			/* REVERSE SIDE OF TICKS */
	tl = 0.f;
    if (abs(n1) / 1000 != 0)
	goto L10;

    dnx = -ndd * cs / 2.f;		/* NUMBER LABEL DISTANCE FROM AXIS */
    dny = -tl - space - cs;	/* NUMBER LABEL DISTANCE FROM AXIS */
    dty = dny - cs - space;		/* TITLE DISTANCE FROM AXIS */
    goto L20;
L10:		/* HORIZONTAL NUMBERS ON VERTICAL AXIS */
    dnx = -cs / 2.f;	/* NUMBER LABEL DISTANCE FROM AXIS */
    dny = -tl - space;	/* NUMBER LABEL DISTANCE FROM AXIS */
    dty = dny - ndd * cs - space * 2.f;	/* TITLE DISTANCE FROM AXIS */
    hor = ang - 90.f;
L20:
    if (n1 < 0)			/* CLOCKWISE TITLES */
	goto L30;

    dny = -dny - cs;	/* COUNTER-CLOCKWISE TITLES */
    dty = -dty - cs;
    tl1 = -tl1;		/* CHANGE SIDES OF TICKS */
    if (abs(n1) < 1000)
	goto L30;

    dny += cs * ndd;
    dty = dny + space;
L30:
    x1 = 0.f;				/* FIRST MAJOR TICK */
    y1 = 0.f;
    y2 = -tl1;
    if (abs(*n0) >= 100000) {
	r__1 = (float) icol[0];
	plotvt_(&r__1, &c_b8, &c__0);
    }
    r__1 = co * x1 - si * y1 + x01;
    r__2 = si * x1 + co * y1 + y01;
    plotvt_(&r__1, &r__2, &c__3);
    r__1 = co * x1 - si * y2 + x01;
    r__2 = si * x1 + co * y2 + y01;
    plotvt_(&r__1, &r__2, &c__2);
    i__1 = njt - 1;
    for (i = 1; i <= i__1; ++i) {		/* MAJOR TICKS */
	r__1 = co * x1 - si * y1 + x01;
	r__2 = si * x1 + co * y1 + y01;
	plotvt_(&r__1, &r__2, &c__3);
	x1 = xj * i;
	y2 = -tl1;
	r__1 = co * x1 - si * y1 + x01;
	r__2 = si * x1 + co * y1 + y01;
	plotvt_(&r__1, &r__2, &c__2);
	r__1 = co * x1 - si * y2 + x01;
	r__2 = si * x1 + co * y2 + y01;
	plotvt_(&r__1, &r__2, &c__2);
	i__2 = nnt + 1;
	for (j = 1; j <= i__2; ++j) {		/* MINOR TICKS */
	    y2 = -tl1 * .7f;
	    x2 = x1 + xn * j - xj;
	    r__1 = co * x2 - si * y1 + x01;
	    r__2 = si * x2 + co * y1 + y01;
	    plotvt_(&r__1, &r__2, &c__3);
	    r__1 = co * x2 - si * y2 + x01;
	    r__2 = si * x2 + co * y2 + y01;
	    plotvt_(&r__1, &r__2, &c__2);
	    y2 = -tl1 * .4f;
	    i__3 = nst;
	    for (k = 1; k <= i__3; ++k) {	/* SUB MINOR TICKS */
		x3 = x2 + xs * k - xn;
		r__1 = co * x3 - si * y1 + x01;
		r__2 = si * x3 + co * y1 + y01;
		plotvt_(&r__1, &r__2, &c__3);
		r__1 = co * x3 - si * y2 + x01;
		r__2 = si * x3 + co * y2 + y01;
		plotvt_(&r__1, &r__2, &c__2);
	    }
	}
    }
    if (abs(n1) % 1000 > 100) 			/* NO LABELING */
	goto L1000;
    xs = 0.f;					/* EXPONENT */
    if (e1 < 0.f)
	goto L140;

/* 	COMPUTE AUTO EXPONENT SCALING SO THAT THE FORMATED LABEL */
/* 	HAS THE INT PORTION FILLED AS MUCH AS POSSIBLE */

/* NO AUTO SCALING */
    i = nd - ng - 1;
    if (*b0 < 0.f || *c0 < 0.f)
	--i;
    if (*b0 != 0.f) {
	r__1 = abs(*b0) + 1e-30f;
	x1 = r_lg10(&r__1);
	r__2 = x1 - .001f;
	if (x1 < 0.f && (r__1 = r_int(&r__2) - x1, abs(r__1)) > .001f) {
	    x1 += -1.f;
	}
	if (x1 >= 0.f)
	    x1 += 1.f;
	x1 = r_int(&x1);
    } else
	x1 = 0.f;
    if (*c0 != 0.f) {
	r__1 = abs(*c0) + 1e-30f;
	y1 = r_lg10(&r__1);
	r__2 = y1 - .001f;
	if (y1 < 0.f && (r__1 = r_int(&r__2) - y1, abs(r__1)) > .001f)
	    y1 += -1.f;
	if (y1 >= 0.f)
	    y1 += 1.f;
	y1 = r_int(&y1);
    } else 
	y1 = 0.f;

    x2 = (x1 < y1 ? x1 : y1);
    x3 = (x1 > y1 ? x1 : y1);
    if (x3 < 0.f)
	x3 += 1;
    if (x2 < 0.f && (float) ng <= 2 - x2)
	xs = nd - ng - 1 - x2;
    if ((float) i < x3 + xs)
	xs = i - x3;

L140:
    y1 = dny;
    y2 = (*c0 - *b0) / (njt - 1);
    if (abs(*n0) >= 100000) {
	r__1 = (float) icol[1];
	plotvt_(&r__1, &c_b8, &c__0);
    }
    e1 = xs;					/* EXPONENT VALUE */
    i__1 = njt;
    for (i = 1; i <= i__1; ++i) {		/* LABEL MAJOR TICKS */
	x1 = (i - 1) * xj + dnx;
	d__1 = e1;
	c1 = (y2 * (i - 1) + *b0) * pow_dd(&c_b117, &d__1);
	r__1 = co * x1 - si * y1 + x01;
	r__2 = si * x1 + co * y1 + y01;

	numbera_(&r__1, &r__2, &cs, &c1, &hor, f0, &c_n1);
    }

/* 	PLOT TITLE */    
    if (nc != 0) {
	y1 = 0.f;
	x1 = 0.f;
	syms_(&x1, &y1, &cs, t, &c_b8, &nc, &c_n3);	/* GET TITLE LENGTH */
	dtx = (abs(*s0) - x1) / 2.f;			/* CENTER TITLE */
	if (e1 != 0.f) 				/* ADD EXPONENT SPACE */
	    dtx -= cs * 3.f;
	if (abs(*n0) >= 100000) {
	    r__1 = (float) icol[2];
	    plotvt_(&r__1, &c_b8, &c__0);
	}
	r__1 = co * dtx - si * dty + x01;
	r__2 = si * dtx + co * dty + y01;

	syms_(&r__1, &r__2, &cs, t, &ang, &nc, &c_n1);

	dtx += x1;
    } else {
	dtx = abs(*s0) / 2.f;
	if (e1 != 0.f)			/* ADD EXPONENT SPACE */
	    dtx -= cs * 3.f;
    }
    x1 = dtx + cs / 2.f;
    y1 = dty;
    if (e1 == 0.f) 				/* NO EXPONENT */
	goto L1000;
    if (abs(*n0) >= 100000) {
	r__1 = (float) icol[3];
	plotvt_(&r__1, &c_b8, &c__0);
    }
    e1 = -e1;
    r__1 = co * x1 - si * y1 + x01;
    r__2 = si * x1 + co * y1 + y01;
    syms_(&r__1, &r__2, &cs, "(X10", &ang, &c__4, &c_n1);
    x1 += cs * 3.75f;
    y1 += cs * .4f;
    r__1 = co * x1 - si * y1 + x01;
    r__2 = si * x1 + co * y1 + y01;
    numbera_(&r__1, &r__2, &cs, &e1, &ang, &c_b8, &c_n1);
    r__2 = abs(e1);
    r__1 = r_lg10(&r__2);
    x2 = r_int(&r__1) + .75f;
    if (e1 < 0.f)
	x2 += 1.f;
    x1 += x2 * cs;
    y1 -= cs * .4f;
    r__1 = co * x1 - si * y1 + x01;
    r__2 = si * x1 + co * y1 + y01;
    syms_(&r__1, &r__2, &cs, ")", &ang, &c__1, &c_n1);
L1000:
    return 0;
}



int numbera_(float *x, float *y, float *hght, float *z, float *t, 
	float *f0, int *ipf)
{
    int i__2;
    float r__1;
    char ch__1[12];

    /* Builtin functions */
    double r_mod(float *, float *), r_lg10(float *);

    char b[20];
    float f, Z;
    int i;
    float t1, fa;
    char fb[8];
    float hg;
    int nd, nn;
    char fb1[8];
    extern /* Subroutine */ float syms_(float *, float *, float *, char *, float *
	    , int *, int *);
    float alg;

/* 	WRITTEN BY D. LONG    AUG, 1983 AT JPL */
/* 	REVISED:	JUNE 1990 */

/* 	PLOTS THE FLOATING POINT NUMBER Z (CAN PLOT AS AN INT) */

/* 	X,Y	(R) COORDINATES OF STRING */
/* 		    (999.,999.) CONTINUE FROM LAST POINT */
/* 	HGHT	(R) HEIGHT OF THE PLOTTED NUMBER */
/* 	Z	(R) FLOATING POINT NUMBER TO BE PLOTTED */
/* 	T	(R) ORIENTATION ANGLE */
/* 	F0	(R) PLOTTING FORMAT (Fn.j) */
/* 		  n = TOTAL NUMBER OF SPACES TO USE (INCLUDING SIGN AND D.P.) */
/* 			[MAX 18 CHARACTERS WIDE] */
/* 		  j = DIGITS TO RIGHT OF DECIMAL POINT (TWO DIGITS EXPECTED) */
/* 			(F4.2 SHOULD BE WRITTEN F4.02) */
/* 		  IF F0 < 0 USE EXPONENTIAL NOTATION (I.E., En,j) */
/* 		  F0 = 1 PLOT IN FLOATING POINT FREE FORMAT */
/* 		  F0 = 0 PLOT INT PORTION WITH NO D.P. (FREE FORMAT) */
/* 		  F0 =-1 PLOT IN EXPONENTIAL FREE FORMAT */
/* 			 NOTE: FREE FORMATS HAVE LEADING SPACES SUPPRESSED */
/* 		  F0 > 1000 PLOT INT PORTION IN FIXED FORMAT WITH */
/* 			n DIGITS AND WITHOUT D.P. */
/* 		  IF n=0 THEN PLOT INT PORTION, DECIMAL POINT, AND */
/* 			 j DIGITS TO RIGHT OF DECIMAL POINT */
/* 		  WHEN Z OVERFLOWS THIS FORMAT, SPACE IS FILLED WITH ASTERICKS */
/* 	IPF	(I) NUMBER CENTERING FLAG (SEE SYMBOL) */
/* 		=-3 X,Y ARE LOWER LEFT CORNER, END OF STRING RETURNED IN X,Y */
/* 			BUT NUMBER IS NOT PLOTTED */
/* 		=-2 X,Y ARE LOWER LEFT CORNER, END OF STRING RETURNED IN X,Y */
/* 		=-1 X,Y ARE LOWER LEFT CORNER */
/* 		= 0 X,Y ARE STRING CENTER */
/* 		=+1 X,Y ARE LOWER RIGHT CORNER */
/* 		=+2 NO PLOT OUTPUT */

    hg = *hght;
    if (hg == 0.f)
	hg = .15f;

    t1 = *t;
    nd = 0;
    nn = 0;
    fa = *f0;
    Z = *z;

    if (abs(fa) > 1022.f)
	fa = 0.f;

    if (fa == 0.f) 
	goto L10;

    if (fa > 999.f) {			/* PLOT FORMATTED INT */
	nn = r_mod(&fa, &c_b94);
	fa = 0.f;
    } else {				/* PLOT FLOAT OR EXPON NUMBER */
	f = abs(fa) * 1.000002f;
	nn = f;
	f = (f - nn) * 100.f;
	nd = f;
    }
L10:
    if (nd > 17) 			/* CORRECT SIMPLE INPUT ERRORS */
	nd /= 10;

    if (nn == 0) {			/* DIGITS TO LEFT OF DECIMAL POINT */
	nn = nd + 2;
	if (Z == 0.f && fa == 0.f)
	    nn = 1;
	if (Z != 0.f) {
	    r__1 = abs(Z);
	    alg = r_lg10(&r__1);
	    if (alg < 0.f)
		alg = 0.f;
	    nn = nd + 2 + alg;
	    if (fa == 0.f)
		nn = alg + 1;
	}
	if (Z < 0.f)
	    ++nn;
	if (fa < 0.f)
	    nn += 4;
    }
    if (nd > nn) 			/* FORMAT ERROR */
	goto L90;
    if (nn > 18) 			/* MAX CHARACTERS */
	nn = 18;

    if (fa == 0.f) {			/* INTEGER */
	i = (int) Z;
	if (nn / 10 > 0)
	    i__2 = sprintf(ch__1, "%%%2dd", nn);
	else
	    i__2 = sprintf(ch__1, "%%%1dd", nn);	
	/* if (i__2 != 0)
	    goto L90;
	*/
	i__2 = sprintf(b, ch__1, i);
	/*
	if (i__2 != 0)
	    goto L90;
	*/
    } else {				/* FLOATING POINT OR EXPONENTIAL */
	if (nn > 1) {
	  if (nd/10 > 0)
	    i__2 = sprintf(ch__1, "%%%2d.%2df", nn,nd);
	  else
	    if (nn/10 > 0)
	      i__2 = sprintf(ch__1, "%%%2d.%1df", nn,nd);
	    else
	      i__2 = sprintf(ch__1, "%%%1d.%1df", nn,nd);
	} else
	    i__2 = sprintf(ch__1, "%%f");
	/* if (i__2 != 0)
	   goto L90; */
	i__2 = sprintf(b, ch__1, Z);
	/* if (i__2 != 0)
	    goto L90; */


    }
L50:

    syms_(x, y, &hg, b, &t1, &nn, ipf);
    
    return 0;
L90:
    
    for (i = 0; i < 18; ++i) {
	b[i] = '*';
	if (i == nn - nd - 1)
	    b[i] = '.';
    }
    goto L50;
} /* numbera_ */

