#include "distance.h"
extern double G_geodesic_distance_lon_to_lon();

static int cur_row;

begin_distance (row)
{
    cur_row = row;

    if (window.proj == PROJECTION_LL)
	G_set_geodesic_distance_lat1(window.north - (row+.5) * window.ns_res);
}

    /* Determine number of columns for each distance zone
     *    (-1 means doesn't occur)
     * Returns: first zone to occur (-1 if none)
     * Note: modifies distances structure (global)
     */

find_distances (row)
{
    int i;
    double dist;
    double scale;
    double ns_dist;

    if (window.proj == PROJECTION_LL)
	G_set_geodesic_distance_lat2(window.north - (row+.5) * window.ns_res);

	/* if at same row, distance is constant across each cell */

    if (row == cur_row)
    {
	if (window.proj == PROJECTION_LL)
	{
	    scale = 1.0/G_geodesic_distance_lon_to_lon (0.0, window.ew_res);
	    for (i = 0; i < ndist; i++)
		distances[i].ncols = distances[i].prev_ncols 
		    = (int) (scale * distances[i].dist);
	}
	else /* note - see parse_dist.c for more info  */
	{
	    for (i = 0; i < ndist; i++)
		distances[i].ncols = distances[i].prev_ncols 
		    = (int) distances[i].dist;
	}
    }
    else
    {
	if (window.proj == PROJECTION_LL)
	    for (i = 0; i < ndist; i++)
		distances[i].ncols = find_ll_distance_ncols(i);
	else
	{
	    i = cur_row - row;
	    ns_dist = (i*i) * ns_to_ew_squared;
	    for (i = 0; i < ndist; i++)
	    {
		dist = distances[i].dist - ns_dist;
		if (dist < 0.0)
		    distances[i].ncols = -1;
		else
		    distances[i].ncols = (int) dist;
	    }
	}
    }

    for (i = 0; i < ndist; i++)
	if (distances[i].ncols >= 0)
	    return i;
    return -1;
}

reset_distances()
{
    register int i;

    for (i = 0; i < ndist; i++)
	distances[i].ncols = distances[i].prev_ncols ;
}

find_ll_distance_ncols (i)
{
    int col;
    double lon;
    register double d;
    register double dist;

	/* use the ncols from previous distances as a starting point
	 * to figure the new ncols
	 */

    col = distances[i].ncols-1;
    if (col < 0) col = 0;
    dist = distances[i].dist;

    lon = window.ew_res * col;

    d = G_geodesic_distance_lon_to_lon (0.0, lon);
    if (d > dist) /*backup */
    {
	while (d > dist)
	{
	    if(--col < 0) break;
	    lon -= window.ew_res;
	    d = G_geodesic_distance_lon_to_lon (0.0, lon);
	}
	return col;
    }
    if (d == dist)
	return col;

    while (d < dist && lon < 180.00 && col <= window.cols)
    {
	col++;
	lon += window.ew_res;
	d = G_geodesic_distance_lon_to_lon (0.0, lon);
    }
    return col-1;
}
