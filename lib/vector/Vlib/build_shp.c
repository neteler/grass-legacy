/*
* $Id$
*
****************************************************************************
*
* MODULE:       Vector library 
*   	    	
* AUTHOR(S):    Original author CERL, probably Dave Gerdes or Mike Higgins.
*               Update to GRASS 5.1 Radim Blazek and David D. Gray.
*
* PURPOSE:      Higher level functions for reading/writing/manipulating vectors.
*
* COPYRIGHT:    (C) 2001 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gis.h"
#include "Vect.h"

extern FILE *Msgout;
extern int prnmsg ( char *msg, ...) ;

/* Build topology 
*  msgout - message output (stdout/stderr for example) or NULL
*
*  Returns: 1 - success
*           0 - error
*/
int
Vect_build_shp ( struct Map_info *Map, FILE *msgout ) {
    struct Plus_head *plus ;
    int    i, j, s, n, type, n_points, node, lineid, offset, ret;
    int    n_lines, side, line;
    int    area, isle, direction;
    int    found;
    plus_t lines[1];
    struct line_pnts **Points, *CPoints;
    int    alloc_parts;
    int    *ptype;
    struct line_cats *Cats;
    P_LINE *Line, *BLine;
    P_NODE *Node;
    P_AREA *Area;
    P_ISLE *Isle;
    double area_size, poly;
    int     progress;
    int     nShapes, nParts, shape, part;
    int     first, last;
    SHPObject *pShape;
    
    
    if ( Map->fInfo.shp.type != SHPT_POLYGON  &&
         Map->fInfo.shp.type != SHPT_POLYGONZ &&
         Map->fInfo.shp.type != SHPT_POLYGONM ) 
    {
        return ( Vect_build_nat ( Map, msgout) );
    } 
    else 
    {
	G_warning ("Topology for shapefile areas is not fully supported.\n");
    }
    
    plus = &(Map->plus);
    Msgout = msgout;
    
    CPoints = Vect_new_line_struct ();
    //APoints = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();

    nShapes = Map->fInfo.shp.nShapes;
  
    alloc_parts = 10; 
    Points = (struct line_pnts **) malloc ( alloc_parts * sizeof ( struct line_pnts *) );
    ptype = (int *) malloc ( alloc_parts * sizeof ( int *) );
    for ( i = 0; i < alloc_parts; i++ ) {
	Points[i] = Vect_new_line_struct (); 
    }
    
    Vect_rewind ( Map );
    prnmsg ("Registering areas: ");
    for ( shape = 0; shape < nShapes; shape++ ) {
	G_debug ( 3, "shape = %d", shape );
	pShape = SHPReadObject( Map->fInfo.shp.hShp, shape );
        nParts = pShape->nParts;
    
	if ( nParts > alloc_parts ) {
	    Points = (struct line_pnts **) realloc ( (void *) Points, nParts * sizeof ( struct line_pnts *) );
            ptype = (int *) realloc ( (void *) ptype, alloc_parts * sizeof ( int *) );
	    for ( i = alloc_parts; i < nParts; i++ ) {
		Points[i] = Vect_new_line_struct (); 
	    }
            alloc_parts = nParts;
	}
	    
	/* Read all parts to array */
	for ( part = 0; part < nParts; part++ ) {
            first = pShape->panPartStart[part];
            if( part == pShape->nParts - 1 ) {
                last = pShape->nVertices - 1;
            } else {
                last = pShape->panPartStart[part+1] - 1;
            }
	    G_debug ( 3, "part = %d from %d to %d", part, first, last );
	    Vect_reset_line ( Points[part] );
	    for ( i = first; i <= last; i++ ) {
		/* TODO do it better (speed) */
		Vect_append_point ( Points[part], pShape->padfX[i], pShape->padfY[i] );
	    }
	    
	    /* register line */
	    offset = ( ( shape << 11 ) | ( part & 0x7FF) );
	    G_debug ( 3, "Register line: offset = %d", offset );
	    line = dig_add_line ( plus, GV_BOUNDARY, Points[part], offset );
	    if ( part == 0 ) { lineid = line; }
	
	    /* Check part type: area or isle */
	    dig_find_area_poly (Points[part], &area_size);
            G_debug ( 3, "  area/isle size = %f", area_size );

	    if (area_size > 0) {  /* area */
		ptype[part] = 1;
                G_debug ( 3, "  -> area" );
	    } else if (area_size < 0) { /* island */
                G_debug ( 3, "  -> isle" );
		ptype[part] = 2;
	    }
	}
	SHPDestroyObject(pShape);

        /* Go through parts again attache isles and register */
	for ( part = 0; part < nParts; part++ ) {
            G_debug ( 3, "  part = %d ptype = %d", part, ptype[part] );
	    if ( ptype[part] == 1 ) { /* area */
		/* register area */
		lines[0] = lineid + part; 
	        area = dig_add_area (plus, 1, lines);
	        
                /* create virtual centroid */
		/* !! offset for virtual centroids is offset for part 0 */
		/* TODO calculate better centroid coordinates */
	        offset =  ( shape << 11 ) ;
	        Vect_reset_line ( CPoints );
		Vect_append_point ( CPoints, Points[part]->x[0], Points[part]->y[0] );
	        line = dig_add_line ( plus, GV_CENTROID, CPoints, offset );

		/* register centroid to area */
		Area = plus->Area[area];
		n = Area->n_centroids;
                if ( dig_area_alloc_centroid (Area, 1) == -1 )
                    return 0;
		
		Area->centroids[n] = line;
		Area->n_centroids++;
                G_debug ( 3, "n_centroids = %d", Area->n_centroids);

		/* find islands inside area */
		for ( i = 0; i < nParts; i++ ) {
		    if ( ptype[i] == 2 ) { /* not registerd island */
			ret = dig_point_in_poly ( Points[i]->x[0], 
				      Points[i]->y[0], Points[part]);
			if ( ret > 0 ) { /* isle inside ares */
			    G_debug ( 3, "isle (part = %d) in area", i );
                            /* register island */ 
		            lines[0] = lineid + i; 
	                    isle = dig_add_isle (plus, 1, lines);
			    
			    G_debug ( 3, " -> isle %d", isle );
			    Isle = plus->Isle[isle];
			    Isle->area = area;
			    
			    Area = plus->Area[area];
			    n = Area->n_isles;
			    if ( dig_area_alloc_isle (Area, 1) == -1 )
				return 0;
			    
			    Area->isles[n] = isle;
			    Area->n_isles++;
			    G_debug ( 3, "n_isles = %d", Area->n_isles);
			    ptype[i] = 0; /* used */
			}
		    }

		}
		
	    }
	    
	}
	
	/* print progress */
        progress = ( int ) 100 *  (shape + 1) / nShapes;  
        prnmsg ("%4d%%\b\b\b\b\b", progress);
    }

    
    return 1;
}


