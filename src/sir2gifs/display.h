#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <xview/xview.h>
#include <xview/cms.h>

#define PI	  3.141592653589793 
#define TWO_PI    6.283185307179586
#define SQRT2   1.41421356237 	/* square root of 2 */
#define RTD	57.2957795131	/* radians to degrees */
#define DTR	.0174532925199	/* degrees to radians */
#define C	2.99792458e8
#define FTPM    3.2808399;	/* feet per meter */

#define SCROLL_WIDTH	21	/* width of scrollbar */
#define WIN_WIDTH_MAX 	768
#define WIN_HEIGHT_MAX	512
#define LR_NORMAL 1		/* normal display */
#define LR_REV	 -1		/* display reverse of each line, left to right */
#define SCROLLBARS  1		/* include scroll_bars */
#define NO_SCROLLBARS  -1	/* no scroll_bars */

#define REL_BEGIN 0		/* fseek relative to beginning of file */
#define REL_CUR   1		/* fseek relative to current position */
#define REL_EOF   2		/* fseek relative to end of file */

#ifndef nint
static double nintarg; 
#define nint(a) ( ((nintarg=(a)) >= 0.0 )?(int)(nintarg+0.5):(int)(nintarg-0.5) )
#endif

#define SQR(a)    ( (a) * (a) )
#define CS(a,b)     ( ((b) >= 0.0) ? (a) : (-a))

#ifndef MAX
#define MAX(a,b)  ( ( (a) > (b) ) ? (a) : (b) )
#endif

#ifndef MIN
#define MIN(a,b)  ( ( (a) < (b) ) ? (a) : (b) )
#endif

#define Max(a,b)  ( ( (a) > (b) ) ? (a) : (b) )
#define Min(a,b)  ( ( (a) < (b) ) ? (a) : (b) )

typedef struct{float re,im;} fcomplex;
typedef struct{short x,y;}Xpoint;
typedef struct{
    unsigned char	red;
    unsigned char	green;
    unsigned char	blue;
}COLOR; 
