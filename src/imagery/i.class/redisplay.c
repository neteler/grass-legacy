#include "globals.h"
#include "raster.h"
#include "local_proto.h"

static int use = 1;

int 
redisplay (void)
{
  int redisplay_zoom();
  int redisplay_map();
  int redisplay_both();
  int cancel_redisplay();

  static Objects objects[] =
    {
      INFO("Redisplay Map Menu:",&use),
      MENU(" Map window ",redisplay_map,&use),
      MENU(" Zoom window ",redisplay_zoom, &use),
      MENU(" Both ", redisplay_both, &use),
      MENU(" Cancel ",cancel_redisplay,&use),
      {0}
    };

  Input_pointer (objects);
  Menu_msg("");

  return(0);
}

int 
redisplay_both (void)
{
  redisplay_map();
  redisplay_zoom();

  return(-1);
}

int 
redisplay_map (void)
{
  draw_cell(VIEW_MAP1, OVER_WRITE);
  if (VIEW_MAP1_ZOOM->cell.configured) {
    /* Outline the zoom window on the main map */
    R_standard_color(RED);
    Outline_cellhd (VIEW_MAP1, &VIEW_MAP1_ZOOM->cell.head);
  }
  return(-1);
}

int 
redisplay_zoom (void)
{
  if (VIEW_MAP1_ZOOM->cell.configured) {
    draw_cell(VIEW_MAP1_ZOOM,OVER_WRITE);
    /*
     * Outline the zoom window on the main map
     */
    R_standard_color(RED);
    Outline_cellhd (VIEW_MAP1, &VIEW_MAP1_ZOOM->cell.head);
  }
  else
    G_warning("No zoom window is defined.");

  return(-1);
}

int 
cancel_redisplay (void)
{
  return(-1);
}




