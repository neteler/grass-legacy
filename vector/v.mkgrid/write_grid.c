#include <stdio.h>
#include "gis.h"
#include "Vect.h"
#include "grid_structs.h"
#include "local_proto.h"

int write_grid (struct grid_description *grid_info,
  struct Map_info *Map, int quiet)
{

  int i, k;
  int rows, cols, x_cols;
  int num_v_rows, num_v_cols;
  double x, y, x_len;
  double sx, sy;
  double width, length;
  double next_x, next_y;
  double snext_x, snext_y;
  double angle, dum;

  struct line_pnts *Points;

  Points = Vect_new_line_struct ();

  num_v_rows = grid_info->num_vect_rows;
  num_v_cols = grid_info->num_vect_cols;
  rows = grid_info->num_rows;
  cols = grid_info->num_cols;
  width = grid_info->width;
  length = grid_info->length;
  angle = grid_info->angle;

/*
* For latlon, must draw in shorter sections
* to make sure that each section of the grid
* line is less than half way around the globe
*/
  x_len = length/3.;
  x_cols = cols*3.;

  /* write out all the vector lengths (x vectors) of the entire grid  */
  if (!quiet)
    fprintf (stderr,"Writing out vector rows ...\n");

  y = grid_info->origin_y;
  for (i = 0; i < num_v_rows; ++i)
  {
    x = grid_info->origin_x;
    for (k = 0; k < x_cols; ++k)
    {
      next_x = x + x_len;

      sx = x;
      sy = y;
      snext_x = next_x;
      dum = y;

      rotate (&x, &y, grid_info->origin_x, grid_info->origin_y, angle);
      rotate (&next_x, &dum, grid_info->origin_x, grid_info->origin_y, angle);
      write_vect (x, y, next_x, dum, Map, Points);

      y = sy;
      x = next_x = snext_x;
    }
    y += width;
  }

  /* write out all the vector widths (y vectors) of the entire grid  */
  if (!quiet)
    fprintf (stdout,"Writing out vector columns ...\n");
  x = grid_info->origin_x;
  for (k = 0; k < num_v_cols; ++k)
  {
    y = grid_info->origin_y;
    for (i = 0; i < rows; ++i)
    {
      next_y = y + width;

      sx = x;
      sy = y;
      snext_y = next_y;
      dum = x;
      rotate (&x, &y, grid_info->origin_x, grid_info->origin_y, angle);
      rotate (&dum, &next_y, grid_info->origin_x, grid_info->origin_y, angle);

      write_vect (x, y, dum, next_y, Map, Points);

      x = sx;
      y = next_y = snext_y;
    }
    /* To get exactly the same coordinates as above, x+=length is wrong */
    x += x_len;
    x += x_len;
    x += x_len;
  }

  /* new with Vlib */
  Vect_destroy_line_struct (Points);

  return (0);
}

static double xarray[10];
static double yarray[10];

#define  NUM_POINTS  2

int 
write_vect (
    double x1,
    double y1,
    double x2,
    double y2,
    struct Map_info *Map,
    struct line_pnts *Points	/* new with Vlib */
)
{
  static struct line_cats *Cats = NULL;

  if ( !Cats ) {
      Cats = Vect_new_cats_struct ();
  }

  xarray[0] = x1;
  xarray[1] = x2;
  yarray[0] = y1;
  yarray[1] = y2;

  if (0 > Vect_copy_xyz_to_pnts (Points, xarray, yarray, NULL, NUM_POINTS))
    G_fatal_error ("Out of memory");
  
  Vect_write_line (Map, GV_BOUNDARY, Points, Cats);

  return 0;
}
