/* Function: do_labels
**
** This function is a much modified version of the p.map function of
** the same name.
**
** Author: Paul W. Carlson	March 1992
*/

#include "ps_info.h"
#include "labels.h"

#define FIELD(x) strcmp(x,field)==0
#define LEFT 0
#define RIGHT 1
#define LOWER 0
#define UPPER 1
#define CENTER 2

extern int verbose;

do_labels()
{
    FILE *fd;

    int i;

    if (!labels.count) return;

    /* default is Helvetica font */
    font_name("Helvetica");

    for (i = 0; i < labels.count; i++)
    {
	fd = G_fopen_old("paint/labels", labels.name[i], labels.mapset[i]);
	if (fd == NULL)
	{
	    char msg[100];
	    sprintf(msg, 
		"Can't open label file %s in %s", labels.name[i], labels.mapset[i]);
	    G_warning (msg);
	}
	else
	{
	    if (verbose > 1)
	    {
	        fprintf (stdout,"PS-PAINT: reading labels file <%s in %s> ...",
		    labels.name[i], labels.mapset[i]);
	        fflush(stdout);
	    }
	    if (labels.font[i] != NULL) font_name(labels.font[i]);
    	    set_font(10);
	    do_label(fd);
	    fclose(fd);
	    if (verbose > 1) fprintf (stdout,"\n");
	}
    }
}

do_label(fd)
FILE *fd;
{
    double east, north, dtmp;
    float size;
    int x, y, xoffset, yoffset, xref, yref;
    int background, border, color, hcolor, width, hwidth;
    int opaque, fontsize, margin;
    char field[1024];
    char value[1024];
    char buf[1024];

    /* initialize */
    north = PS.w.north;
    east  = PS.w.west;
    opaque = 0;
    xoffset = 0;
    yoffset = 0;
    color = BLACK;
    width = 1;
    background = WHITE;
    border = BLACK;
    hcolor = -1;
    hwidth = 1;
    xref = CENTER;
    yref = CENTER;

    /* read the labels file */
    while (fgets (buf, sizeof buf, fd))
    {
        *value = 0;
        *field = 0;
        if (sscanf(buf,"%[^:]:%[^\n]", field, value) < 1) continue;

        if (FIELD("text"))
        {
	    G_strip(value);

	    /* get reference coordinates */
	    x = XCONV(east);
	    y = YCONV(north);
            x += xoffset;
            y += yoffset;

	    /* set font size */
	    fontsize = size * PS.ns_to_y;
 	    if (fontsize < 10) fontsize = 10;
 	    if (fontsize > 50) fontsize = 50;
    	    set_font(fontsize);

	    /* set margin to 20% of font size */
	    if (opaque || border)
	    {	margin = (int)(0.2 * (double)fontsize + 0.5);
	     	if (margin < 2) margin = 2;
		if (hcolor >= 0) margin += hwidth;
	    }
	    else margin = 0;
	    fprintf(PS.fp, "/mg %d def\n", margin);

	    /* construct path for box - text is left on stack */
	    text_box_path(x, y, xref, yref, value, fontsize);

	    if (opaque)
	    {
		/* fill the box */
		set_rgb_color(background);
		fprintf(PS.fp, "F ");
		opaque = 0;
	    }

	    if (border >= 0)
	    {
		/* draw the border */
		set_rgb_color(border);
		fprintf(PS.fp, "D ");
		border = -1;
	    }

	    /* draw the text */
	    if (hcolor >= 0)
	    {   
		set_rgb_color(hcolor);
		set_line_width(hwidth);
		fprintf(PS.fp, "HC ");
	    }
	    set_rgb_color(color);
	    fprintf(PS.fp, "TIB ");

	    hcolor = -1;
	    hwidth = 1;
            continue;
        }

        if (FIELD("color"))
        {
	    color = get_color_number(value);
            if (color < 0) color = BLACK;
            continue;
        }

        if (FIELD("hcolor"))
        {
	    hcolor = get_color_number(value);
            if (hcolor < 0) hcolor = WHITE;
            continue;
        }

        if (FIELD("xoffset"))
        {
            xoffset = atoi(value);
            continue;
        }

        if (FIELD("yoffset"))
        {
            yoffset = atoi(value);
            continue;
        }

        if (FIELD("ref"))
        {
	    if (!scan_ref(value, &xref, &yref))
	    {
                yref = CENTER;
                xref = CENTER;
	    }
            continue;
        }

        if (FIELD("background"))
        {
	    background = get_color_number(value);
            if (background < 0) background = WHITE;
            continue;
        }

        if (FIELD("border"))
        {
	    border = get_color_number(value);
            if (border < 0) border = BLACK;
            continue;
        }

        if (FIELD("opaque"))
        {
	    G_strip(value);
            opaque = (strcmp(value, "no") != 0);
            continue;
        }

        if (FIELD("width"))
        {
            continue;
        }

        if (FIELD("hwidth"))
        {
	    hwidth = atoi(value);
	    if (hwidth < 1) hwidth = 1;
	    if (hwidth > 5) hwidth = 5;
            continue;
        }

        if (FIELD("size"))
        {
	    if (G_scan_resolution(value, &dtmp, PS.w.proj)) size = dtmp;
            continue;
        }

        if (FIELD("north"))
        {
	    if (G_scan_northing(value, &dtmp, PS.w.proj)) north = dtmp;
            continue;
        }

        if (FIELD("east"))
        {
	    if (G_scan_easting(value, &dtmp, PS.w.proj)) east = dtmp;
            continue;
        }

	if (FIELD("font"))
	{
	    continue;
	}

    }
    fclose (fd);
}
