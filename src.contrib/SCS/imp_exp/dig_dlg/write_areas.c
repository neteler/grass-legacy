/* @(#)write_areas.c	1.1   06/27/90 */
/*
**  Written by Dave Gerdes  6/89
**  US Army Construction Engineering Research Lab
**  modified by RL Glenn, SCS, CGIS Division
*/
/*#include "digit.h"*/
#include "gis.h"
#include "digit.h"
#include "export_dlg.h"


#define AREA_FMT "A%5d%12.2lf%12.2lf%6d%6d%6d%6d%6d%6d      \n"
#ifdef SCS_MODS
#define FLAT_FMT "%5d\t%s\t%6d \n"
#endif

write_dlg_areas (map, fp, ftmp, ctmp)
    struct Map_info *map;
    FILE *fp, *ftmp, *ctmp;
{
    P_AREA *Area;
    P_ISLE *Isle;
    double x, y;
    int n_atts;
    register int area, i;
    plus_t zero = 0;
    int n_lines;
#ifdef SCS_MODS
    char label[50];
#endif

    for (area = 1 ; area <= map->n_areas ; area++)
    {
	Area = &(map->Area[area]);

	if (Area->att || area == 1)
	    n_atts=1;
	else
	    n_atts=0;

	if (AREA_LABELED (Area))
	{
		x = map->Att[Area->att].x;
		y = map->Att[Area->att].y;
	}
	else
	{
	    if (area == 1 || area == 2)
	    {
		/* dont really exist */
		x = (Area->E + Area->W) / 2.0;
		y = (Area->N + Area->S) / 2.0;
	    }
	    else
	    {
		double totalarea;
		/*
		** use this to calculate centroid for area 
		** NOTE! this in not guaranteed to be inside the 
		** polygon.  Or anywhere near it for that matter.
		*/
		dig_find_area (map, Area, &totalarea, &x, &y, 0.0);
	    }
	}

	/*
	** get total number of area_line entries.  This includes
	** the zero separators.
	*/
	n_lines = Area->n_lines + Area->n_isles;
	for (i = 0 ; i < Area->n_isles ; i++)
	{
	    n_lines += map->Isle[Area->isles[i]].n_lines;
	}

	fprintf (fp, AREA_FMT, 
		area, 		 	/* index of element */
		x, y,			/* coordinates */
		0,			/* unused */
		n_lines,		/* number of lines */
		0,			/* unused */
		n_atts,			/* # of att codes */
		0,			/* unused */
		Area->n_isles);		/* number of isles */

	start_ints ();
	write_ints (fp, Area->n_lines, Area->lines);
	for (i = 0 ; i < Area->n_isles ; i++)
	{
	    Isle = &(map->Isle[Area->isles[i]]);
	    write_ints (fp, 1, &zero);
	    write_ints (fp, Isle->n_lines, Isle->lines);
	}
	end_ints (fp);

	if (area == 1)  /* special area has  0000 000 attribute */
	{
	    start_att ();
	    write_dlg_att (fp, 0, 0);
	    end_att (fp);
	}
	else
	if (n_atts)
	{
	    start_att ();
	    write_dlg_att (fp, DEF_MAJOR, map->Att[Area->att].cat);
	    end_att (fp);
#ifdef SCS_MODS
	    label[0] = '\0';
	    if (get_label(label,map->Att[Area->att].cat,ctmp) == 0)
	       fprintf(stderr,"	* WARNING * no label for area %d, cat# %d\n",
		   area,map->Att[Area->att].cat);
	    fprintf (ftmp, FLAT_FMT, 
		area,			    /* area number */
		label, 			    /* category label */
		map->Att[Area->att].cat);   /* category code value */
#endif
	}
    }
}
