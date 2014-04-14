
/* *** LAST REVISED ON 24-JAN-1994 07:36:09.78 */
/* *** SOURCE FILE: [LONGLIB93.SOURCES.C]UTIL.C */

#include <stdio.h>
#include <math.h>

typedef long int integer;
typedef float real;
typedef double doublereal;

integer i_nint(r) real *r;
{
	integer ret_val = *r;
	if (ret_val - *r > 0.5) ret_val--;
	if (*r - ret_val > 0.5) ret_val++;
	return ret_val;
}

integer iand_(i, j) integer *i, *j;
{
	integer ret_val = (*i) & (*j);
	return ret_val;
}

integer ior_(i, j) integer *i, *j;
{
	integer ret_val = (*i) | (*j);
	return ret_val;
}

/*
int getch()
{
	int c, d;
	c = getchar();
	d = c;
	while (d != -1 && d != 10) {
		d = getchar();
	}
	if (d == -1) c = -1;
	if (c == 10) c = 32;
	return c;
}
*/
	
double pow_dd(b, e) doublereal *b, *e;
{
	return pow( (double) *b, (double) *e);
}
	
double pow_ri(b, e) doublereal *b; integer *e;
{
	return pow( (double) *b, (double) *e);
}
	
integer pow_ii(b, e) integer *b; integer *e;
{
	return (integer) pow( (double) *b, (double) *e);
}

double r_int(x) real *x;
{	int i = *x;
	return (double) i;
}

double r_lg10(x) real *x;
{	double d = *x;
	return log10(d);
}

double r_nint(r) real *r;
{	int i = (int) *r;
	double ret_val = (double) i;
	if (ret_val - *r > 0.5) ret_val -= 1.0;
	if (*r - ret_val > 0.5) ret_val += 1.0;
	return ret_val;
}

double r_mod(a,b) real *a,*b;
{	int i;
	double junk;
	junk = *a / *b;
	i = junk;
	junk = *a - i * *b;
	return junk;
}

double r_sign(a,b) real *a,*b;
{	return (double) (*b >= 0 ? *a : *a);
}

integer i_sign(a,b) integer *a,*b;
{	return (*b >= 0 ? *a : *a);
}

double dmin(a,b) double a,b;
{	return (a <= b ? a : b);
}

double dmax(a,b) double a,b;
{	return (a >= b ? a : b);
}

int ibits_(i,j,k) short int *i; int *j,*k;
{	int temp;

	temp = (*i % (1 << (*j + *k) )) >> *j;
	return temp;
}

#define PI 3.141592654

double cosd_(a) double a;
{
   return(cos(a*PI/180.0));
}

double sind_(a) double a;
{
   return(sin(a*PI/180.0));
}


