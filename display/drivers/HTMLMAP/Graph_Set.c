/*
 * Start up graphics processing.  Anything that needs to be assigned, set up,
 * started-up, or otherwise initialized happens here.  This is called only at
 * the startup of the graphics driver.
 *
 * The external variables define the pixle limits of the graphics surface.  The
 * coordinate system used by the applications programs has the (0,0) origin
 * in the upper left-hand corner.  Hence,
 *    screen_left < screen_right
 *    screen_top  < screen_bottom 
 *
 */

#include <string.h>
#include <stdlib.h>
#include "gis.h"
#include "driverlib.h"


#define MAIN
#include "htmlmap.h"

int screen_left;
int screen_top;
int screen_right;
int screen_bottom;

int NCOLORS       = 256 ;

int BBOX_MINIMUM;
int MAX_POINTS;
int MINIMUM_DIST;

int Graph_Set (int argc, char **argv, int nlev) 
{
    char *p;

    G_gisinit("HTMLMAP driver") ;

    if (NULL == (p = getenv ("LOCATION"))) {
	fprintf (stderr, "You must start this driver from within GRASS\n");
        exit (1);
    }

    /*
     * set the 'screen' resolution of the driver
     */

    if (NULL != (p = getenv ("GRASS_WIDTH"))) {
	screen_right = atoi (p);
        if (screen_right <= 0) {
	    screen_right = DEF_WIDTH;
        }
    } else {
	screen_right = DEF_WIDTH;
    }

    if (NULL != (p = getenv ("GRASS_HEIGHT"))) {
	screen_bottom = atoi (p);
        if (screen_bottom <= 0) {
            screen_bottom = DEF_HEIGHT;
        }
    } else {
	screen_bottom = DEF_HEIGHT;
    }

    /*
     * set the minimum bounding box dimensions 
     */

    if (NULL != (p = getenv ("GRASS_HTMLMINBBOX"))) {
	BBOX_MINIMUM = atoi (p);
        if (BBOX_MINIMUM <= 0) {
            BBOX_MINIMUM = DEF_MINBBOX;
        }
    } else {
	BBOX_MINIMUM = DEF_MINBBOX;
    }

    /*
     * set the maximum number of points
     */

    if (NULL != (p = getenv ("GRASS_HTMLMAXPOINTS"))) {
	MAX_POINTS = atoi (p);
        if (MAX_POINTS <= 0) {
            MAX_POINTS = DEF_MAXPTS;
        }
    } else {
	MAX_POINTS = DEF_MAXPTS;
    }

    /*
     * set the minimum difference to keep a point
     */

    if (NULL != (p = getenv ("GRASS_HTMLMINDIST"))) {
	MINIMUM_DIST = atoi (p);
        if (MINIMUM_DIST <= 0) {
            MINIMUM_DIST = DEF_MINDIST;
        }
    } else {
	MINIMUM_DIST = DEF_MINDIST;
    }


    /*
     * open the output file
     */

    if (NULL != (p = getenv ("GRASS_HTMLFILE"))) {
        if (strlen(p) == 0) {
            p = FILE_NAME;
        }
    } else {
        p = FILE_NAME;
    }
    file_name = p;

    output = fopen(file_name, "w");
    if (output == NULL) {
        fprintf(stderr,"HTMLMAP: couldn't open output file %s\n",file_name);
        exit(1);
    }


    printf("HTMLMAP: collecting to file: %s\n width = %d, height = %d, ",
		file_name, screen_right, screen_bottom);

    /*
     * check type of map wanted
     */

    if (NULL == (p = getenv ("GRASS_HTMLTYPE"))) {
        p = "CLIENT";
    }

    if (strcmp(p,"APACHE") == 0) {
        html_type = APACHE;
        printf("type = APACHE\n");

    } else if (strcmp(p,"RAW") == 0) {
        html_type = RAW;
        printf("type = RAW\n");

    } else {
        html_type = CLIENT;
        printf("type = CLIENT\n");
    }


    /*
     * initialize text memory and list pointers
     */
    
    last_text = (char *) malloc(INITIAL_TEXT+1);
    last_text[0] = '\0';
    last_text_len = INITIAL_TEXT;

    head = NULL;
    tail = &head;

    return 0;
}
