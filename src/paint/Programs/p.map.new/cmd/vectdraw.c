#include <stdio.h>
#include "gis.h"
#include "Vect.h"

#include "graphics.h"
#include "vtext.h"
#include "local_proto.h"


static int plot_lines (int,double *,double *,int);
static struct line_pnts *line_p;	/* digit libes */
static int first = 1;

int vectdraw (struct Map_info *Map)
{
    int type;

/* initialization required by dig library interface */


    if (first)
    {
	line_p = Vect_new_line_struct ();
	first = 0;
    }


    while ((type = Vect_read_next_line (Map, line_p)) >= 0)
	{
	plot_lines (line_p->n_points, line_p->x, line_p->y, type);
	}

	return 0;
}

static int plot_lines (int nc, double *east, double *north, int type)
{
    double e1, e2;
    double n1, n2;
    int x, y;
    int n;

    if (nc < 1)
        return 0;

    e2 = *east++;
    n2 = *north++;
    if (vect_type_is_dot(type))
    {
	G_plot_where_xy (e2, n2, &x, &y);
        diamond (x, y);
        return 0;
    }

    for (n = 1; n < nc; n++)
    {
        e1 = e2;
        n1 = n2;
	e2 = *east++;
	n2 = *north++;
	G_plot_line (e1, n1, e2, n2);
    }
    return 0;
}
