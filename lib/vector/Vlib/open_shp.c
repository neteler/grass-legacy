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
#include <unistd.h>
#include "Vect.h"
#include "gis.h"
#include "shapefil.h"

#include <sys/types.h>
#include <sys/stat.h>

/* Open old file.
*  Map->name and Map->mapset must be set before
*
*  Return: 0 success
*         -1 error
*/
int 
V1_open_old_shp ( struct Map_info *Map )
{
    SHPHandle hShp;
    int       ShapeType, nShapes;
    double    MinBound[4], MaxBound[4];

#ifdef GDEBUG
    G_debug ( 1, "V1_open_old_shp(): shp file = %s", Map->fInfo.shp.file );
#endif

    if ( Map->fInfo.shp.file == NULL ) {
	G_warning ("Shapefile name not defined\n");
	return (-1);
    }

    hShp = SHPOpen( Map->fInfo.shp.file, "r" );
    if ( hShp == NULL) {
	G_warning ("Cannot open shapefile: %s", Map->fInfo.shp.file);
	return (-1);
    }
    Map->fInfo.shp.hShp = hShp;
    Map->fInfo.shp.shape = 0;
    Map->fInfo.shp.part = 0;

    SHPGetInfo( hShp, &nShapes, &ShapeType, MinBound, MaxBound );
    Map->fInfo.shp.type = ShapeType;
    Map->fInfo.shp.nShapes = nShapes;

#ifdef GDEBUG
          G_debug ( 1, "V1_open_old_shp(): shptype = %d, nShapes = %d", ShapeType, nShapes );
#endif
    
    Map->head.with_z = WITHOUT_Z;

  return (0);
}

/* Open new file.
*
*  Return: 0 success
*         -1 error
*/
int
V1_open_new_shp (
    struct Map_info *Map,
    char *name,
    int with_z)
{
    G_warning ( "V1_open_new_shp() is not implemented." );
    return (-1);
}
