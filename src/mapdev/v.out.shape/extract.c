/******************************************************************************
 * extract.c [v.out.shape2]
 * Routines to transfer shapes from dig files to shapefiles.

 * @Copyright David D.Gray <ddgray@armadce.demon.co.uk>`
 * 29th. Jun. 2000
 * Last updated 2nd. Jul. 2000
 *

 * This file is part of GRASS GIS. It is free software. You can 
 * redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 ******************************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "gis.h"
#include "Vect.h"
#include "shapefil.h"
#include "local_structs.h"
#include "local_proto.h"

int extract_lines( SHPObject **hObj, struct Map_info *Map, int *indx_list, int *nIndices,
		   const int curr_indx, int ltype) {

  /* Extract lines from a LINE dig file and add to shapefile */

  SHPObject *obj1;
  struct line_pnts *line1;
  char buf[128] = "";
  int cindx;

  /* Fields for shape object */
  int partoffset = 0, tvertices = 0;

  cindx = curr_indx + 1;

  /* If we have come to the end of the lines, exit with 0 */
  if( curr_indx >= Map->n_lines ) return 0;

  /* Deal only with linear elements */

    if(Map->Line[cindx].type == DOT)
    return -1;

  /* Initialise line structure */
  line1 = Vect_new_line_struct();


  /* Read the line */

  /* the test below will accept a DOT if one appears */
  if (V2_read_line(Map, line1, cindx ) != ltype && ltype != 3) return 1;

  tvertices = line1->n_points;

  /* Build the shape object */

  if( (obj1 = SHPCreateObject( SHPT_ARC, *nIndices + 1, 1, &partoffset, NULL,
			       tvertices, line1->x, line1->y, NULL, NULL )) == NULL ) {
    sprintf(buf, "Could not build shape object for line %d\n", cindx );
    G_fatal_error(buf);
  }

  *hObj = obj1;

  Vect_destroy_line_struct(line1);

  /* Clean up and finish */
  indx_list[*nIndices] = cindx;
  (*nIndices)++;
  return 1;
}

int extract_points( SHPObject **hObj, struct Map_info *Map, int *indx_list, int *nIndices,
		   const int curr_indx ) {

  /* Extract points from a POINT dig file and add to shapefile */

  SHPObject *obj1;
  struct line_pnts *line1;
  char buf[128] = "";
  int cindx;

  /* Fields for shape object */

  cindx = curr_indx + 1;

  /* If we have come to the end of the lines, exit with 0 */
  if( curr_indx >= Map->n_lines ) return 0;

  /* Deal only with point elements (reject area edges and lines) */
  if(Map->Line[cindx].type != DOT)
    return -1;

  /* Initialise line structure */
  line1 = Vect_new_line_struct();

  /* Read the line */
  if( V2_read_line(Map, line1, cindx ) != DOT ) return -1;

  if( (obj1 = SHPCreateSimpleObject( SHPT_POINT, 1, line1->x, line1->y, NULL )) == NULL ) {
    sprintf(buf, "Could not build shape object for point %d\n", cindx );
    G_fatal_error(buf);
  }

  *hObj = obj1;

  Vect_destroy_line_struct(line1);

  /* Clean up and finish */
  indx_list[*nIndices] = cindx;
  (*nIndices)++;
  return 1;
}

int extract_ring( SHPObject **sh1, struct Map_info *Map, 
        int *indx_list, int *nIndices, const int curr_indx, const int all) {

  /* Extract a ring from the vector map, based on the current index position,
     and write it to the area shapefile.
  */

  SHPObject *obj1;
  P_AREA *Area;
  struct line_pnts *Points;

  char *logfile_name;
  FILE *lfp;
  char buf[512];

  /* loop */
  int i, j, k, k1;

  /* Fields for shape object */
  int numparts, *partoffsets, numvertices, totalvertices;
  double *listX, *listY;
  int cindx;

  /* Fields for dig file area and topology analysis */
  int nIsles;
  int startOffset, endOffset;

  cindx = curr_indx + 1;
  if( curr_indx >= Map->n_areas ) return 0;
 
  logfile_name = G_malloc(128);

  proc_logfile( GET_VAL, logfile_name );

  if( (lfp = fopen( logfile_name, "a" )) == NULL ) {
    lfp = stdout;
  }
  free (logfile_name);

  if (!all && !V2_area_att(Map, cindx)) {
    fprintf ( lfp, "Skipping unlabeled area (hole?) #%d\n", cindx);
    return -1;
  }

  if( V2_get_area( Map, cindx, &Area ) != 0 ) {
    fprintf( lfp, "Area %d unassigned\n", cindx );
    return -1;
  }

  Points = Vect_new_line_struct();
  Points->alloc_points = 0;
  Points->n_points = 0;
  Points->x = NULL;
  Points->y = NULL;
 
  /* Determine initial information on shape */
  numparts = Area->n_isles + 1;

  if( (numvertices = Vect_get_area_points( Map, cindx, Points )) < 0 ) {
    fprintf( lfp, "Could not build area %d\n", cindx );
    return 1;
  }

  partoffsets = G_malloc( numparts * sizeof *partoffsets );

  /* Set first offset (of main ring) */
  partoffsets[0] = 0;
  startOffset = 0;
  endOffset = numvertices - 1;

  /* Loop through the main ring and assign X and Y points */

  listX = G_malloc( numvertices * sizeof listX[0] );
  listY = G_malloc( numvertices * sizeof listY[0] );

  
  for( i = 0; i < numvertices; ++i ) {
    listX[i] = Points->x[i];
    listY[i] = Points->y[i];
  }
  totalvertices = numvertices;

  /* Determine the rings of the isles */

  nIsles = Area->n_isles;

  for( i = 0; i < nIsles; ++i ) {

    j = Area->isles[i];

    if( (numvertices = Vect_get_isle_points( Map, j, Points )) < 0 ) {
      sprintf( buf, "Could not extract vertices for island %d of shape %d\n",
	       i, cindx );
      G_fatal_error(buf);
    }

    partoffsets[i+1] = startOffset = endOffset + 1;
    endOffset += numvertices;

    /* Loop through this ring and add points to the shape */
    
    totalvertices += numvertices;
    listX = G_realloc( listX, totalvertices * sizeof listX[0] );
    listY = G_realloc( listY, totalvertices * sizeof listY[0] );
    
    for( k = startOffset, k1 = 0; k <= endOffset && k1 < numvertices; ++k, ++k1 ) {
      listX[k] = Points->x[k1];
      listY[k] = Points->y[k1];      
    }
  }

  Vect_destroy_line_struct(Points);

  /* Log vertex lists */
  k = 0;
  for(i = 0; i < totalvertices; i++ ) {
    if( i == partoffsets[k++] ) {
      if( i != 0 ) fprintf( lfp, "  END\n" );
      fprintf(lfp, "  Ring %d\n", k );
      }
    fprintf(lfp, "    Vertex %d: %.6f  %.6f\n", i, listX[i], listY[i] );
  }
  fprintf( lfp, "END\n" );

  /* Build the shape object */


  if( (obj1 = SHPCreateObject( SHPT_POLYGON, *nIndices + 1, numparts, partoffsets,
			       NULL, totalvertices, listX, listY, NULL, NULL ))
      == NULL ) {
    
    sprintf(buf, "Could not build shape object for area %d\n", cindx );
    G_fatal_error(buf);
  }

  free(listX);
  free(listY);
  free(partoffsets);

  *sh1 = obj1;

  if(lfp != stdout && lfp != stderr) fclose(lfp);

  /* Update indices */
  indx_list[*nIndices] = cindx;
  (*nIndices)++;
  return 1;
}




