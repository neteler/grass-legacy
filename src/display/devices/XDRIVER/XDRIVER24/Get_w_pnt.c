#include "includes.h"

/* Using mouse device, get a new screen coordinate and button number.
 * Button numbers must be the following values which correspond to the
 * following software meanings: 1 - left button 2 - middle button 3 -
 * right button
 * 
 * This is called directly by the application programs.
 * 
 * A "pointing hand" pointer is used. Upon button depression, the current
 * coordinate is returned in (*wx, *wy) and the button pressed in
 * returned in *button. */

extern Display *dpy;
extern Window grwin;
extern Cursor grcurse;

static u_long mask;

int Get_location_with_pointer (int *wx, int *wy, int *button)
{
    XEvent bpevent;
    u_int  mask;
    Window root, child;
    int    rx, ry;

    /* set the grass cursor on (defined in Graph_Set.c) */
    XDefineCursor(dpy, grwin, grcurse);

    /* wait for a button-push event in the grass window, and return the
     * x,y coord and button number */

    if(*button < 0){
	XQueryPointer(dpy, grwin, &root, &child, &rx, &ry, wx, wy, &mask);
	*button = (mask&Button1Mask ? 1
			: (mask&Button2Mask ? 2
				: (mask&Button2Mask ? 3
					: *button)));
    }else{
	XWindowEvent(dpy, grwin, ButtonPressMask, &bpevent);

	*wx = bpevent.xbutton.x;
	*wy = bpevent.xbutton.y;
	*button = bpevent.xbutton.button;
    }

    return 0;
}
