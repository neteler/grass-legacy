
/*---------------------------------------------------------*
 *               AGNPS/GRASS Interface Project             *
 *  Developed in the Agriculture Engineering Department    *
 *                at Purdue University                     *
 *                        by                               *
 *         Raghavan Srinivasan and Bernard Engel           *
 *                                                         *
 *   (c)Copyright, 1992 Purdue Research Foundation, West   *
 *   Lafayette, Indiana 47907. All Rights Reserved. Unless *
 *   permission is granted, this material shall not be     *
 *   copied, reproduced or coded for reproduction by any   *
 *   electrical, mechanical or chemical processes,  or     *
 *   combinations thereof, now known or later developed.   *
 *---------------------------------------------------------*/

#include "gis.h"

int rename_maps (old_name, new_name)
  char *old_name, *new_name;
{
  int G_rename ();

  G_rename ("cell", old_name, new_name);
  G_rename ("colr", old_name, new_name);
  G_rename ("cats", old_name, new_name);
  G_rename ("cellhd", old_name, new_name);
  G_rename ("hist", old_name, new_name);
  G_rename ("cell_misc", old_name, new_name);
  return 0;
}
