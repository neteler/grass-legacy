#include <math.h>
#include "driverlib.h"
# define STOP  -1

static int text_move(double,double);
static int text_draw(double,double);
static int remember(double,double);
static short int *CharIndex[128] ;
static int am_inside = 0 ;
static int dont_draw = 0 ;
static int t, b, l, r ;
double basex, basey ;
double curx, cury ;

int drawchar (double text_size_x,double text_size_y,
	register double sinrot,register double cosrot, unsigned char character)
{
	unsigned char *X ;
	unsigned char *Y ;
	int n_vects ;
	int i ;
	register int ax, ay ;
	double x, y ;
	int (*Do)();
	int ix, iy ;

	x = basex ;  y = basey ;

	get_char_vects(character, &n_vects, &X, &Y) ;

	Do = text_move ;
	for(i=1;i<n_vects;i++)
	{
		if (X[i] == ' ')
		{
			Do = text_move ;
			continue ;
		}
		ix = 10 + X[i] - 'R' ;
		iy = 10 - Y[i] + 'R' ;
		ax = (int)(text_size_x * (double)ix) ;
		ay = (int)(text_size_y * (double)iy) ;
		if(dont_draw)
		{
			remember(x + (ax * cosrot - ay * sinrot),
				y - (ax * sinrot + ay * cosrot)) ;
		}
		else
		{
			(*Do)(x + (ax * cosrot - ay * sinrot),
				y - (ax * sinrot + ay * cosrot)) ;
			Do = text_draw ;
		}
	}
	/*  This seems to do variable spacing
	ix = 10 + X[i] - 'R' ;
	*/
	ix = 20 ;
	iy = 0 ;
	ax = (int)(text_size_x * (double)ix) ;
	ay = (int)(text_size_y * (double)iy) ;
	if (! dont_draw)
		text_move(basex + (ax * cosrot - ay * sinrot),
			basey - (ax * sinrot + ay * cosrot)) ;
	else
		remember(basex + (ax * cosrot - ay * sinrot),
			basey - (ax * sinrot + ay * cosrot)) ;

	return 0;
}

static int remember(double x,double y)
{
	if ((int)x > r) r = (int)x ;
	if ((int)x < l) l = (int)x ;
	if ((int)y > b) b = (int)y ;
	if ((int)y < t) t = (int)y ;
	curx = x ;  cury = y ;

	return 0;
}

static int text_draw (double x,double y)
{
	double X1, Y1 ;
	double X2, Y2 ;
	int mod ;

	X1 = x ;
	Y1 = y ;
	X2 = curx ;
	Y2 = cury ;

	mod = window_clip(&X1, &Y1, &X2, &Y2) ;

	if (am_inside)
	{
		Cont_abs((int)X1, (int)Y1) ;
		if (mod)
			am_inside = 0 ;
	}
	else
	{
		if (!mod)
		{
			Move_abs((int)X2, (int)Y2) ;
			Cont_abs((int)X1, (int)Y1) ;
			am_inside = 1 ;
		}
	}

	curx = x ;  cury = y ;

	return 0;
}

static int text_move (double x,double y)
{
	double X1, Y1 ;
	double X2, Y2 ;
	int mod ;

	X1 = x ;
	Y1 = y ;
	X2 = curx ;
	Y2 = cury ;

	mod = window_clip(&X1, &Y1, &X2, &Y2) ;
	
	if(am_inside)
		Move_abs((int)X1, (int)Y1) ;

	if(mod)
		am_inside = 0 ;

	curx = x ;  cury = y ;

	return 0;
}

int soft_text_ext(int x,int y,
	double text_size_x,double text_size_y,double text_rotation,
	char *string)
{
	t = 999999 ;
	b = 0 ;
	l = 999999 ;
	r = 0 ;
	dont_draw = 1 ;
	soft_text(x, y, text_size_x, text_size_y, text_rotation, string) ;
	dont_draw = 0 ;

	return 0;
}

int get_text_ext (int *top,int *bot,int *left,int *rite)
{
	*top = t ;
	*bot = b ;
	*left = l ;
	*rite = r ;

	return 0;
}

# define RpD ((2 * 3.14159265358979323846) / 360.)	/* radians/degree */
# define D2R(d) (double)(d * RpD)	/* degrees->radians */

int soft_text(int x,int y,
	double text_size_x,double text_size_y,double text_rotation,
	char *string)
{
	double sinrot = sin (D2R (text_rotation)) ;
	double cosrot = cos (D2R (text_rotation)) ;

	am_inside = 0 ;
	curx = basex = (double)x ;
	cury = basey = (double)y ;
	while(*string != 00)
	{
		drawchar (text_size_x, text_size_y, sinrot, cosrot, *string++) ;
		basex = curx ; basey = cury ;
	}

	return 0;
}

int onechar(int x,int y,
	double text_size_x,double text_size_y,double text_rotation,
	register unsigned char achar)
{
	double sinrot = sin (D2R (text_rotation)) ;
	double cosrot = cos (D2R (text_rotation)) ;

	am_inside = 0 ;
	curx = basex = (double)x ;
	cury = basey = (double)y ;
	drawchar (text_size_x,text_size_y,sinrot, cosrot, achar) ;

	return 0;
}
