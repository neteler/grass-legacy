/*  @(#)plot_coors.c	2.1  6/26/87  */
#include "dlg.h"
#include "convert.h"

#define XFUDGE		1.

#define XADJ(x)		(int)((x - U_west ) * U_to_D_xconv + D_west + XFUDGE)
#define YADJ(y)		(int)((y - U_south) * U_to_D_yconv + D_south)


plot_all_coors(n_coors, coors)
	int n_coors ;
	double *coors ;
{
	double *xptr, *yptr ;
	int i ;

	xptr = coors ;
	yptr = coors + 1 ;

	First(xptr, yptr) ;
	xptr += 2 ;
	yptr += 2 ;
	for(i=1; i<n_coors; i++)
	{
		Next(xptr, yptr) ;
		xptr += 2 ;
		yptr += 2 ;
	}
}

plot_dots(n_coors, coors)
	int n_coors ;
	double *coors ;
{
	double *xptr, *yptr ;
	int i ;

	/* begin cycling through the coordinates */
	/* if the user has entered a list of arcs, only do those arcs */

	xptr = coors + 2 ;  /* Skip first point as it is a node */
	yptr = coors + 3 ;
	n_coors-- ;

	for(i=0; i<n_coors; i++)
	{
		Dot(*xptr, *yptr) ;
		xptr += 2 ;
		yptr += 2 ;
	}
}

static double current_x_utm, current_y_utm ;
static int	on_a_roll ;

First(x, y)
	double *x, *y ;
{
	current_x_utm = *x ;
	current_y_utm = *y ;
	on_a_roll = 0 ;
}

Next(x, y)
	double *x, *y ;
{
	int off_a_roll ;
	int xpos, ypos ;
	double new_x, new_y ;

/* check to see if entire line segment is outside window */

	if ((*x < U_west) && (current_x_utm < U_west))
		off_a_roll = 1 ;
	else if ((*x > U_east) && (current_x_utm > U_east))
		off_a_roll = 1 ;
	else if ((*y < U_south) && (current_y_utm < U_south))
		off_a_roll = 1 ;
	else if ((*y > U_north) && (current_y_utm > U_north))
		off_a_roll = 1 ;
	else
		off_a_roll = 0 ;

	if (off_a_roll)
	{
		on_a_roll = 0 ;
	}
	else
	{
		new_x = *x ;
		new_y = *y ;
		D_clip( U_south, U_north, U_west, U_east, 
			&new_x, &new_y, &current_x_utm, &current_y_utm) ;
			
		if (on_a_roll)
		{
			xpos = XADJ(new_x) ;
			ypos = YADJ(new_y) ;
			R_cont_abs(xpos, ypos) ;
		}
		else
		{
			xpos = XADJ(current_x_utm) ;
			ypos = YADJ(current_y_utm) ;
			R_move_abs(xpos, ypos) ;
			xpos = XADJ(new_x) ;
			ypos = YADJ(new_y) ;
			R_cont_abs(xpos, ypos) ;
			on_a_roll = 1 ;
		}
	}
	current_x_utm = *x ;
	current_y_utm = *y ;

	return ;
}

Adot ( x, y, string) 
	double *x, *y ;
	char *string ;
{
	int xpos, ypos ;

	if (*x < U_west)
		return ;
	else if (*x > U_east)
		return ;
	else if (*y < U_south)
		return ;
	else if (*y > U_north)
		return ;
	else
	{
		xpos = XADJ(*x) ;
		ypos = YADJ(*y) ;
		R_move_abs(xpos, ypos) ;
		R_text(string) ;
		return ;
	}
}

Dot ( x, y) 
	double x, y ;
{
	int xpos, ypos ;

	if (x < U_west)
		return ;
	else if (x > U_east)
		return ;
	else if (y < U_south)
		return ;
	else if (y > U_north)
		return ;
	else
	{
		xpos = XADJ(x) ;
		ypos = YADJ(y) ;
		R_move_abs(xpos, ypos) ;
		R_cont_abs(xpos, ypos) ;
		return ;
	}
}

Blot ( x, y) 
	double x, y ;
{
	int xpos, ypos ;

	if (x < U_west)
		return ;
	else if (x > U_east)
		return ;
	else if (y < U_south)
		return ;
	else if (y > U_north)
		return ;
	else
	{
		xpos = XADJ(x) ;
		ypos = YADJ(y) ;
		R_move_abs(xpos,   ypos  ) ;
		R_cont_abs(xpos,   ypos-1) ;
		R_cont_abs(xpos-1, ypos  ) ;
		R_cont_abs(xpos,   ypos+1) ;
		R_cont_abs(xpos+1, ypos  ) ;
		R_cont_abs(xpos,   ypos-1) ;
		return ;
	}
}
