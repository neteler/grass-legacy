#include "gis.h"
#include "local_proto.h"

/*
int zoomwindow (int quiet, int rotate, double magnify, char pan)
*/
int zoomwindow (int quiet, double magnify, char pan)
{
    struct Cell_head window, oldwindow ;
    char *err;
    int quitonly;

    G_get_set_window(&window);
    G_copy((char *) &oldwindow, (char *) &window, sizeof(window));

/*
    if (window.proj != PROJECTION_LL)
	rotate = 0;
*/

    while(1)
    {
/*
	if (rotate)
	    quitonly=make_window_center (&window, magnify, -1.0, -1.0);
	else
*/
	    quitonly=make_window_box (&window, magnify, pan);

	/* quitonly = 0: Zoom
	 * quitonly = 1: Quit
	 * quitonly = 2: Unzoom
	 */

	if (quitonly == 1) 
	  break; /* no action was taken */
	else	  
	{

/* Comment out to take window as selected 
** Not max size from map objects 
	  
	  if(window.east > U_east)
		  window.east = U_east;
	  if(window.west < U_west)
		  window.west = U_west;
	  if(window.south < U_south)
		  window.south = U_south;
	  if(window.north > U_north)
		  window.north = U_north;
*/

	  if (err = G_adjust_Cell_head (&window, 0, 0))
	  {
	    	just_click(err);
	    	continue;
	  }

          G_put_window(&window);
          G_set_window(&window);
	  redraw();
	  
	  if (!quitonly)
	  {
	    int x, y, b;

	    fprintf(stderr, "Accept new region?\n");
	    fprintf(stderr, "Left:   Accept and quit\n");
#ifdef ANOTHER_BUTTON
	    fprintf(stderr, "Middle: No\n");
	    fprintf(stderr, "Right:  Accept and continue\n\n");
#else
	    fprintf(stderr, "Middle: Accept and continue\n");
	    fprintf(stderr, "Right:  No\n\n");
#endif

	    R_get_location_with_pointer(&x, &y, &b);

	    if(b == LEFTB)
		break;
	    else
	    if(b == RIGHTB){
    	    	G_copy((char *) &window, (char *) &oldwindow, sizeof(window));
            	G_put_window(&window);
	    	G_set_window(&window);
	    	redraw();

	    	if (!yes("Try again?"))
	      		return 1;
	    }
	  }
	  else
	    break;
	}
    }

#ifdef QUIET
    if(!quiet)
    {
	fprintf(stderr, "This region now saved as current region.\n\n") ;
	fprintf(stderr, "Note: run 'd.erase' for the new region to affect the graphics.\n");
    }
#endif
    return(quitonly) ;
}
