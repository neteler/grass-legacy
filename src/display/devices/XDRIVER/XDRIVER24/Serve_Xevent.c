#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "includes.h"
#include "colors.h"
#include "pad.h"
#include "gis.h"

static void spawnRedrawProcess(void);
static void checkRedrawProcess(void);
static void handleResizeEvent(void);
static void checkFlush(void);
static void setTitleBusy(int);

static pid_t redraw_pid;

int needs_flush;

int Service_Xevent (int opened)
{
    static int do_resize;
    Atom WM_DELETE_WINDOW;
    XEvent event;

    checkRedrawProcess();

    while (XPending(dpy))
    {
	/* NOTE: This won't die if server terminates */
        XNextEvent(dpy, &event);
	switch (event.type)
	{
	case ConfigureNotify:
	    if ( event.xconfigure.width != SC_WID || 
		 event.xconfigure.height != SC_HITE ) 
		do_resize = 1; /* group requests into one */
	    break;

        case ClientMessage:
            WM_DELETE_WINDOW = XInternAtom(event.xclient.display, 
					   "WM_DELETE_WINDOW", False);
            if (event.xclient.data.l[0] != WM_DELETE_WINDOW)
		break;
	    Graph_Close();
	    exit(0);
	    break;
        }
    } /* while() */

    /* Now process resize or expose events */
    if (do_resize && !redraw_pid && !opened)
    {
	spawnRedrawProcess();
	handleResizeEvent();
	do_resize = 0;
    }

    checkFlush();

    return 0;
}

int _time_stamp (PAD *pad)
{
    delete_item(pad,"time");
    append_item(pad,"time","1");

    return 0;
}

static void checkRedrawProcess(void)
{
    int status;
    pid_t pid;

    if (!redraw_pid)
	return;

    pid = waitpid(redraw_pid, &status, WNOHANG);
    if (pid < 0)
    {
	perror("Monitor: checkRedrawProcess: waitpid");
	return;
    }

    if (pid == 0)
	return;

    if (pid != redraw_pid)
    {
	fprintf(stderr, "Monitor: waitpid: expected %d but got %d\n",
		redraw_pid, pid);
	return;
    }

    setTitleBusy(0);
    redraw_pid = 0;
}

static void spawnRedrawProcess(void)
{
    pid_t pid;
    LIST *commands;
    PAD *pad;
    ITEM *item;

    if (redraw_pid)
	return;

    pad = find_pad("full_screen");
    if (!pad)
	return;

    item = find_item(pad, "list");
    if (!item)
	return;

    commands = item->list;
    if (!commands)
	return;

    pid = fork();
    if (pid < 0)
    {
	perror("Monitor: fork");
	return;
    }

    if (pid != 0)	/* parent */
    {
	setTitleBusy(1);
	redraw_pid = pid;
	return;
    }

    /* child */

    close(0); open("/dev/null", O_RDONLY);
    close(1); open("/dev/null", O_WRONLY);
    close(2); open("/dev/null", O_WRONLY);
    for ( ; commands; commands = commands->next)
	system(commands->value);
    exit(0);
}

static void handleResizeEvent(void)
{
    PAD *curpad;
    char buf[64];
    XWindowAttributes xwa;
    XGCValues gc_values;

    /* Get the window's current attributes. */
    if (!XGetWindowAttributes(dpy, grwin, &xwa))
	return;

    SC_WID  = xwa.width;
    SC_HITE = xwa.height;
    screen_right = xwa.width;
    screen_bottom = xwa.height;

    /* do a d.frame -e (essentially) */
    /* Dclearscreen() */
    /* delete the time and current window out of the scratch pad */
    curpad = find_pad("");
    delete_item(curpad,"time");
    delete_item(curpad,"cur_w");

    /* delete all other pads */
    for ( curpad = pad_list(); curpad != NULL; curpad = curpad->next )
	if ( *curpad->name  )
	    delete_pad(curpad);

    curpad = NULL;

    /* set standard color to black and erase */
    Standard_color(BLACK);
    Erase();

    /* Dnew("full_screen") */
    /* find a pad called "full_screen" */
    create_pad("full_screen");
    sprintf(buf,"%d %d %d %d",
	    screen_top,screen_bottom,screen_left,screen_right);
    curpad = find_pad("full_screen");
    append_item(curpad, "d_win", buf);
    _time_stamp(curpad);
    /* Dchoose("full_screen") */

    /* set the time and window name in no-name pad */
    curpad = find_pad("");
    append_item(curpad, "cur_w", "full_screen");
    _time_stamp(curpad);

    /* set the window */
    Set_window(screen_top, screen_bottom, screen_left, screen_right) ;

    /* Handle backing store */
    XFreePixmap(dpy, bkupmap);
    bkupmap = XCreatePixmap(dpy, grwin, SC_WID, SC_HITE, xwa.depth);
    XGetGCValues(dpy, gc, GCForeground, &gc_values);
    XSetForeground(dpy, gc, 0);
    XFillRectangle(dpy, bkupmap, gc, 0, 0, SC_WID, SC_HITE);
    XSetForeground(dpy, gc, gc_values.foreground);
    XSetWindowBackgroundPixmap(dpy, grwin, bkupmap);
    XClearWindow(dpy, grwin);

    needs_flush = 0;
}

static void checkFlush(void)
{
	static struct timeval last_flush;
	struct timeval now;
	long delta;

	if (!needs_flush)
		return;

	if (gettimeofday(&now, NULL) < 0)
	{
		perror("Monitor: gettimeofday");
		return;
	}

	delta = (now.tv_sec - last_flush.tv_sec) * 1000000 +
		(now.tv_usec - last_flush.tv_usec);
	if (last_flush.tv_sec && delta < 250000)
		return;

	XClearWindow(dpy, grwin);

	last_flush = now;
	needs_flush = 0;
}

static void setTitleBusy(int busy)
{
#ifndef X11R3
	static const char text[] = " [redraw]";
	XTextProperty prop;
	char title[1024], *p;

	if (!XGetWMName(dpy, grwin, &prop))
	{
		fprintf(stderr, "Monitor: XGetWMName failed\n");
		return;
	}

	if (!prop.value || !prop.nitems || prop.format != 8)
	{
		fprintf(stderr, "Monitor: XGetWMName: bad result\n");
		return;
	}

	strcpy(title, prop.value);
	XFree(prop.value);

	p = strstr(title, text);
	if (p)
		*p = '\0';
	if (busy)
		strcat(title, text);

	prop.value = title;
	prop.nitems = strlen(title);

	XSetWMName(dpy, grwin, &prop);
#endif
}

/*** end Serve_Xevent.c ***/
