/***********************************************************************
File     	:	utils.c
Function 	:	GetMouse(w, x, y)
Args	 	:	    Widget w; -- The widget the mouse is in;
  	    		    int *x; -- extraction source
  	    		    int *y; -- extraction source

Author   	:	Frank Goodman -- spanki@ced.berkeley.edu
Creation 	:	30 January 1990
Last Revised	:
Abstract 	:	Query the mouse, and return the absolute
			coordinates relative to the root window;
Returns  	:	none.

***********************************************************************/
# include "cell_editor.h"

void GetMouse(w, x, y)
    Widget w;
    int *x, *y;
    {
    Window root;
    int root_x, root_y; 
    int win_x, win_y;
    unsigned int mask;

    XQueryPointer(XtDisplay(w), XtWindow(w), &root, &w, 
		&root_x, &root_y, &win_x, &win_y, &mask);
    *x = root_x;
    *y = root_y;
    }

/***********************************************************************

File     	:	utils.c
Function 	:	void FlushExpose(popup, child)
Args	 	:	    Widget popup; -- popup shell
Args	 	:	    Widget child; -- popup child

Author   	:	Frank Goodman -- spanki@ced.berkeley.edu
Creation 	:	3 February 1990
Last Revised	:
Abstract 	:	This is a hack to trap and process the expose
			event generated by a popup widget.
Returns  	:	None.

***********************************************************************/
void FlushExpose(popup, child)
    Widget popup;
    Widget child;
    {
    register XEvent event;
    XtAppContext app = XtWidgetToApplicationContext(popup);
    Window win = XtWindow(child);

    do
        {
        XtAppNextEvent(app, &event);
        XtDispatchEvent(&event);
        }
    while ((event.xany.window != win) || (event.type != Expose));
    XFlush(XtDisplay(popup));
    }

void FlushDestroy(app, win)
    XtAppContext app;
    Window win;		/* shells don't have windows */
{
register XEvent event;

do
    {
    XtAppNextEvent(app, &event);
    XtDispatchEvent(&event);
    }
while (event.xany.window != win);
}
