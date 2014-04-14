
/* *** LAST REVISED ON  7-JAN-1994 15:18:55.65 */
/* *** SOURCE FILE: [LONGLIB93.SOURCES.C.AUXLIB]CIRCLE.C */

typedef int integer;
typedef float real;
typedef long int logical;
typedef short int shortint;
typedef double doublereal;

#define abs(x) ((x) >= 0 ? (x) : -(x))
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))
#define dabs(x) (doublereal)abs(x)
#define dmin(a,b) ((a) <= (b) ? (a) : (b))
#define dmax(a,b) ((a) >= (b) ? (a) : (b))

#define TRUE_ (1)
#define FALSE_ (0)

/* Table of constant values */

static integer c__3 = 3;

/* Subroutine */ int circle_(x, y, a1, a2, r1, r2, d)
real *x, *y, *a1, *a2, *r1, *r2, *d;
{
    /* System generated locals */
    real r__1, r__2;

    /* Builtin functions */
    double cos(), sin(), acos(), asin(), sqrt();

    /* Local variables */
    static logical flag_;
    static integer ipen;
    static real size;
    extern /* Subroutine */ int plot_();
    static real a, r, dashl, space, x1, y1, x2, y2, ai, rd, arg, dis;
    static logical pen;
    static real tpi;

/* 	CREATED BY D. LONG    AUG, 1983		AT JPL */

/* 	THIS ROUTINE PLOTS A CIRCLE USING THE PLOTTING RESOLUTION */
/* 	AND CIRCLE SIZE TO MAKE A GOOD "APPERANCE" OF A CONTINUOUS CIRCLE. */

/* 	X,Y	(R) LOCATION OF CIRCLE CENTER */
/* 	A1	(R) ANGLE OF STARTING POINT FROM HORIZONTAL IN DEGREES */
/* 	A2	(R) ANGLE TO ENDING POINT IN DEGREES */
/* 	R1	(R) RADIUS OF STARTING POINT */
/* 	R2	(R) RADIUS OF ENDING POINT */
/* 	D	(R) D=0.0	SOLID (CONNECTED) LINE */
/* 		    D=0.5	DASHED LINE (SOFTWARE GENERATED) */

    flag_ = TRUE_;
    tpi = (float).017453292522222223;
    size = (float).1;			/* CIRCLE RESOLUTION */
    dashl = (float).3;			/* DASH LINE DASH SIZE */
    space = dashl / (float)2.1;		/* DASH LINE SPACE SIZE */
    a = *a1;
    r = *r1;
    arg = a * tpi;
    x1 = r * cos(arg) + *x;
    y1 = r * sin(arg) + *y;
    x2 = x1;
    y2 = y1;
    plot_(&x1, &y1, &c__3);		/* PLOT START POINT */
    pen = TRUE_;
    ipen = 2;
    dis = (float)0.;
    if (*a1 == *a2 || *r1 < (float)0. || *r2 < (float)0.) {
	return 0;
    }
    rd = (*r2 - *r1) / (*a2 - *a1);
L100:
    arg = a * tpi;
    if ((r__1 = acos(size / r), dabs(r__1)) < (r__2 = asin(size / r), dabs(
	    r__2))) {	/* COMPUTE ANGLE INCREM. */
	ai = (r__1 = acos(size / r) / tpi, dabs(r__1)) + (float).01;
/* BASED ON RESOLUTION */
/* OF CIRCLE */
    } else {
	ai = (r__1 = asin(size / r) / tpi, dabs(r__1)) + (float).01;
    }
    r__1 = *a2 - *a1;
    a += ai * (r__1 >= 0 ? 1. : -1. );
    if ((r__1 = a - *a2, dabs(r__1)) < ai + (float).01) {
	if (flag_) {		/* FINISH LAST POINT */
	    a = *a2;
	    flag_ = FALSE_;
	    ipen = 2;
	    goto L110;
	}
	plot_(x, y, &c__3);	/* END CURVE */
	return 0;
    }
L110:
    r = *r1 + rd * (a - *a1);
    arg = a * tpi;
    x1 = r * cos(arg) + *x;
    y1 = r * sin(arg) + *y;
    plot_(&x1, &y1, &ipen);
    if (*d == (float).5) {
/* DASHED CURVE */
/* Computing 2nd power */
	r__1 = x1 - x2;
/* Computing 2nd power */
	r__2 = y1 - y2;
	dis = sqrt(r__1 * r__1 + r__2 * r__2) + dis;
	if (dis > dashl && pen) {
	    pen = FALSE_;
	    ipen = 3;
	    dis = (float)0.;
	    goto L120;
	}
	if (dis > space && ! pen) {
	    pen = TRUE_;
	    ipen = 2;
	    dis = (float)0.;
	}
L120:
	x2 = x1;
	y2 = y1;
    }
    goto L100;
} /* circle_ */
