/*
 * MODULE:      v.out.dxf
 *
 * AUTHOR(S):   Chuck Ehlschlaeger
 *              Update to GRASS 5.7 by Radim Blazek
 *
 * PURPOSE:     Convert vector files into DXF files.  This program is a
 *              small demo and not to be taken seriously.
 *
 * COPYRIGHT:   (C) 1989-2006 by the GRASS Development Team
 *
 *              This program is free software under the GNU General Public
 *              License (>=v2). Read the file COPYING that comes with GRASS
 *              for details.
 *
 * Improved March 15, 1989: take it a little more seriously, but not too much.
 *
 * Revised for Grass4.0- converted for new command parser 1/91 -dks
 *   BUT--still needs to be set up for binary input before
 *   moved from alpha to main installation.
 */

#define _MAIN_C_
#include <stdio.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#include <grass/Vect.h>
#include "global.h"

/* size of text compared to screen=1 */
#define TEXT_SIZE	.003
#define CENTERED	4

static double do_limits(struct Map_info *);
static int make_layername(void);
static int add_plines(struct Map_info *, double);

int main(int argc, char *argv[])
{
    double textsize;
    char *mapset, *dxf_file;
    struct Map_info In;
    struct GModule *module;
    struct Option *input, *output;

    G_gisinit(argv[0]);

    /* Set description */
    module = G_define_module();
    module->description = _("Exports GRASS vector files to DXF file format.");

    input = G_define_standard_option(G_OPT_V_INPUT);

    output = G_define_option();
    output->key = "output";
    output->type = TYPE_STRING;
    output->required = NO;
    output->multiple = NO;
    output->gisprompt = "new_file,file,output";
    output->description = _("DXF output file");

    if (G_parser(argc, argv))
	exit(EXIT_FAILURE);

    /* open input vector */
    if ((mapset = G_find_vector2(input->answer, "")) == NULL) {
	G_fatal_error(_("Could not find input map <%s>"), input->answer);
    }

    if (output->answer)
	dxf_file = G_store(output->answer);
    else {
	dxf_file = G_malloc(strlen(input->answer) + 5);
	sprintf(dxf_file, "%s.dxf", input->answer);
    }

    Vect_set_open_level(2);
    Vect_open_old(&In, input->answer, mapset);

    dxf_open(dxf_file);		/* open output */
    G_free(dxf_file);

    textsize = do_limits(&In);	/* does header in dxf_fp */
    make_layername();
    dxf_entities();
    add_plines(&In, textsize);	/* puts plines in dxf_fp */

    dxf_endsec();
    dxf_eof();			/* puts final stuff in dxf_fp, closes file */

    exit(EXIT_SUCCESS);
}

double do_limits(struct Map_info *Map)
{
    double textsize;
    BOUND_BOX box;

    Vect_get_map_box(Map, &box);

    dxf_header();
    dxf_limits(box.N, box.S, box.E, box.W);
    dxf_endsec();

    if ((box.E - box.W) >= (box.N - box.S))
	textsize = (box.E - box.W) * TEXT_SIZE;
    else
	textsize = (box.N - box.S) * TEXT_SIZE;

    return textsize;
}

int make_layername(void)
{
    dxf_tables();
    dxf_linetype_table(1);
    dxf_solidline();
    dxf_endtable();
    dxf_layer_table(7);
    dxf_layer0();

    dxf_layer("point", 1, "CONTINUOUS", 0);
    dxf_layer("line", 2, "CONTINUOUS", 0);
    dxf_layer("boundary", 3, "CONTINUOUS", 0);
    dxf_layer("centroid", 4, "CONTINUOUS", 0);
    dxf_layer("point-label", 5, "CONTINUOUS", 0);
    dxf_layer("centroid-label", 6, "CONTINUOUS", 0);

    dxf_endtable();
    dxf_endsec();

    return 0;
}

int add_plines(struct Map_info *Map, double textsize)
{
    int nlines, line;
    struct line_pnts *Points;
    struct line_cats *Cats;
    char *layer, *llayer;
    int cat;
    char cat_num[50];

    Points = Vect_new_line_struct();
    Cats = Vect_new_cats_struct();

    nlines = Vect_get_num_lines(Map);

    for (line = 1; line <= nlines; line++) {
	int i, ltype;

	G_percent(line, nlines, 2);

	ltype = Vect_read_line(Map, Points, Cats, line);
	Vect_cat_get(Cats, 1, &cat);
	sprintf(cat_num, "%d", cat);

	if (ltype == GV_POINT) {
	    layer = "point";
	    llayer = "point-label";
	}
	else if (ltype == GV_LINE) {
	    layer = "line";
	}
	else if (ltype == GV_BOUNDARY) {
	    layer = "boundary";
	}
	else if (ltype == GV_CENTROID) {
	    layer = "centroid";
	    llayer = "centroid-label";
	}
	else {
	    continue;
	}

	if (ltype & GV_POINTS) {
	    dxf_point(layer, Points->x[0], Points->y[0], Points->z[0]);

	    dxf_text(llayer, Points->x[0], Points->y[0], Points->z[0], textsize,
		     CENTERED, cat_num);
	}
	else {			/* lines */
	    dxf_polyline(layer);

	    for (i = 0; i < Points->n_points; i++) {
		dxf_vertex(layer, Points->x[i], Points->y[i], Points->z[i]);
	    }

	    dxf_poly_end(layer);
	}
    }

    return 0;
}
