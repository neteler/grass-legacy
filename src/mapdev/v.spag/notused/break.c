#include "digit.h"

    static struct line_pnts Points;
    static struct line_pnts NPoints;
    static int first_time = 1;

break_line_w_point (map, line, ux, uy)
    struct Map_info *map;
    int line;
    double ux, uy;
{
    struct new_node node;
    char type;
    register int i;
    P_LINE *Line;
    double dist;
    int segment;
    int n_points;
    int label;
    int N1, N2;
    int line1, line2;
    int status;

    if (first_time)
    {
	first_time = 0;
	Points.alloc_points = 0;
	NPoints.alloc_points = 0;
    }

    Line = &(map->Line[line]);

    /* hold on to node numbers for new lines */
    N1 = Line->N1;
    N2 = Line->N2;

    type = Line->type;
    if (0 > (dig__Read_line (&Points, map->digit, Line->offset)))
    {
	BEEP;
	fprintf (stderr,  "Error reading line.");
	sleep (3);
	return (-1);
    }

    /* save attribute info */
    /* NOTE for now, I don't bother trying to restore area atts */
    if (Line->att)
    {
	label = map->Att[Line->att].cat;
    }
    else
	label = 0;
    type = Line->type;

    /* segment  ranges from 1 -> */
    segment = local_check_dist (map, &Points, ux, uy, &dist);
/*DEBUG*/  debugf2 ("Breaking on segment %d\n", segment);

    /* get nearest point ON line */
    /* replace ux, uy with the line intersection */
    dig_xy_distance3_point_to_line (&ux, &uy, 
	    Points.x[segment-1], Points.y[segment-1], 
	    Points.x[segment], Points.y[segment], &status);
/*DEBUG*/ debugf ("returned ok from distance\n");

    /* if new intersection is past end of line, blow it off */
    /*
    if ((ux == Points.x[0] && uy == Points.y[0]) || 
       (ux == Points.x[Points.n_points-1] && uy == Points.y[Points.n_points-1]))
    */
    if (status)
    {
/*DEBUG*/  debugf2 ("PAST END OF LINE\n");
	return (-1);
    }

    /* do first line */
    n_points = segment + 1;
    if (0 > dig_alloc_points (&NPoints, n_points))
    {
	fprintf (stderr, "Out of memory.  Failed");
	return (-1);
    }
    for (i = 0 ; i < n_points-1 ; i++)
    {
	NPoints.x[i] = Points.x[i];
	NPoints.y[i] = Points.y[i];
    }
    NPoints.x[n_points-1] = ux ; 
    NPoints.y[n_points-1] = uy ; 
    NPoints.n_points = n_points;
/*DEBUG*/ debugf ("calling 1st _do_break\n");

    /* TEST */
    dig_check_nodes (map, &node, &NPoints);

    /*
    node.N1 = N1;
    node.N2 = map->n_nodes + 1;
    */
    line1 = _do_break_line (map, &NPoints, label, type, &node);
/*DEBUG*/ debugf ("returned ok from 1st _do_break\n");

    /* do second line */
    n_points = Points.n_points - segment + 1;
/*DEBUG*/ debugf ("BREAK: n_points 2 = %d\n", n_points);
    if (0 > dig_alloc_points (&NPoints, n_points))
    {
	fprintf (stderr, "Out of memory.  Failed");
	sleep (1);
	return (-1);
    }
    NPoints.x[0] = ux ; 
    NPoints.y[0] = uy ; 
    NPoints.n_points = n_points;
    for (i = 1 ; i < n_points ; i++)
    {
	NPoints.x[i] = Points.x[segment-1+i];
	NPoints.y[i] = Points.y[segment-1+i];
    }
/*DEBUG*/ debugf ("calling 2nd _do_break\n");
    node.N1 = node.N2;
    node.N2 = N2;
    line2 = _do_break_line (map, &NPoints, label, type, &node);
/*DEBUG*/ debugf ("returned ok from 2nd _do_break\n");

    /* get rid of old line */
    /* remove old line AFTER adding new lines, to hold on to nodes that
    ** would have been deleted otherwise
    */
    _remove_line (map, line);

    return (0);
}

/* returns new line number */
static
_do_break_line (map, Points, label, type, node)
    struct Map_info *map;
    struct line_pnts *Points;
    int label;
    char type;
    struct new_node *node;
{
    int nline;
    int att;
    double x, y;

/*DEBUG*/ debugf ("calling new_line\n");
    nline = new_line (map, type, node, Points);
    if (nline < 0)
    {
	BEEP;
	fprintf (stderr, "Error creating new line.");
	return (-1);
    }
    /* put the label back on */
#ifdef LABELS_TOO
    if (label)
    {
	get_line_center (&x, &y, Points);
	att = dig_new_att (map, x, y, type, nline, label);
	if (att < 0)
	    return (-1);
	map->Line[nline].att = att;
    }
#endif
    return (nline);
}

/*
**  If one line intersects itself, then
**  Three lines will form:


       ---------------
       \      3      /
        \           /
         \         /
          \       /
           \     /
            \   /
             \ /
	      X
             / \
          1 /   \  2
           /     \
**
*/

#ifdef FOO
twice_break_line_w_point (map, line, ux, uy)
    struct Map_info *map;
    int line;
    double ux, uy;
{
    struct new_node node;
    char type;
    register int i;
    P_LINE *Line;
    double dist;
    int segment;
    int n_points;
    int label;
    int N1, N2;
    int line1, line2;
    int status;

    if (first_time)
    {
	first_time = 0;
	Points.alloc_points = 0;
	NPoints.alloc_points = 0;
    }

    Line = &(map->Line[line]);

    /* hold on to node numbers for new lines */
    N1 = Line->N1;
    N2 = Line->N2;

    type = Line->type;
    if (0 > (dig__Read_line (&Points, map->digit, Line->offset)))
    {
	fprintf (stderr,  "Error reading line.");
	sleep (3);
	return (-1);
    }

    /* save attribute info */
    /* NOTE for now, I don't bother trying to restore area atts */
    if (Line->att)
    {
	label = map->Att[Line->att].cat;
    }
    else
	label = 0;
    type = Line->type;

    /* segment  ranges from 1 -> */
    segment = local_check_dist (map, &Points, ux, uy, &dist);

    /* get nearest point ON line */
    /* replace ux, uy with the line intersection */
    dig_xy_distance3_point_to_line (&ux, &uy, 
	    Points.x[segment-1], Points.y[segment-1], 
	    Points.x[segment], Points.y[segment], &status);
/*DEBUG*/ debugf ("returned ok from distance\n");

    /* if new intersection is past end of line, blow it off */
    /*
    if ((ux == Points.x[0] && uy == Points.y[0]) || 
       (ux == Points.x[Points.n_points-1] && uy == Points.y[Points.n_points-1]))
    */
    if (status)
    {
	return (-1);
    }

    /* do first line */
    n_points = segment + 1;
    if (0 > dig_alloc_points (&NPoints, n_points))
    {
	fprintf (stderr, "Out of memory.  Failed");
	return (-1);
    }
    for (i = 0 ; i < n_points-1 ; i++)
    {
	NPoints.x[i] = Points.x[i];
	NPoints.y[i] = Points.y[i];
    }
    NPoints.x[n_points-1] = ux ; 
    NPoints.y[n_points-1] = uy ; 
    NPoints.n_points = n_points;
/*DEBUG*/ debugf ("calling 1st _do_break\n");

    /* TEST */
    dig_check_nodes (map, &node, &NPoints);

    /*
    node.N1 = N1;
    node.N2 = map->n_nodes + 1;
    */
    line1 = _do_break_line (map, &NPoints, label, type, &node);
/*DEBUG*/ debugf ("returned ok from 1st _do_break\n");

    /* do second line */
    n_points = Points.n_points - segment + 1;
/*DEBUG*/ debugf ("BREAK: n_points 2 = %d\n", n_points);
    if (0 > dig_alloc_points (&NPoints, n_points))
    {
	fprintf (stderr, "Out of memory.  Failed");
	sleep (1);
	return (-1);
    }
    NPoints.x[0] = ux ; 
    NPoints.y[0] = uy ; 
    NPoints.n_points = n_points;
    for (i = 1 ; i < n_points ; i++)
    {
	NPoints.x[i] = Points.x[segment-1+i];
	NPoints.y[i] = Points.y[segment-1+i];
    }
/*DEBUG*/ debugf ("calling 2nd _do_break\n");
    node.N1 = node.N2;
    node.N2 = N2;
    line2 = _do_break_line (map, &NPoints, label, type, &node);
/*DEBUG*/ debugf ("returned ok from 2nd _do_break\n");

    /* get rid of old line */
    /* remove old line AFTER adding new lines, to hold on to nodes that
    ** would have been deleted otherwise
    */
    _remove_line (map, line);

    return (0);
}
#endif

/*
** This is copied from  dig__check_dist in file mapdev/lib/point_t_line.c
**   because I needed those segments where the point of interest is NOT
**    a node on the line.
*/
local_check_dist (map, points, ux, uy, dist)
    struct Map_info *map;
    struct line_pnts *points;
    double ux;
    double uy;
    double *dist;
{
    register int i;
    register double distance;
    register double new_dist;
    register int n_points;
    int segment;


    /*  dpg  2 aug 1989
       corrected this code to work with 1 point lines,  for DOT
    if (points->n_points < 2)
    {
        *dist = 99999999.;
        return (-1);
    }
    */
    n_points = points->n_points;
    segment = 1;

    /* initialize for segment not at end of line */
    i = GREATER (n_points-2, 1);
    distance = dig_distance2_point_to_line(ux, uy, points->x[i-1], points->y[i-1],
            points->x[i], points->y[i]);
    segment = i;
    for (/* i = i */ ; i < n_points-1 ; i++)
    {
        new_dist= dig_distance2_point_to_line(ux, uy, points->x[i],points->y[i],
                points->x[i+1], points->y[i+1]);
	if (! (ux == points->x[0] && uy == points->y[0] ||
		     points->x[points->n_points-1] && uy == 
		     points->y[points->n_points-1]))
	    if (new_dist < distance)
	    {
		distance = new_dist;
		segment = i+1;
	    }
    }
    *dist = distance;
    return (segment);
}
