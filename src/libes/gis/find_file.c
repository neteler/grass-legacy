/*
 **********************************************************************
 *  char *
 *  G_find_file (element, name, mapset)
 *        char *element    database element (eg, "cell", "cellhd", etc)
 *        char *name       file name to look for
 *        char *mapset     mapset to search. if mapset is ""
 *                         will search in mapset search list
 *
 *	searches for a file from the mapset search list
 *      or in a specified mapset.
 *	returns the mapset name where the file was found.
 *
 *  returns:
 *      char *  pointer to a string with name of mapset
 *              where file was found, or NULL if not found
 *  note:
 *      rejects all names that begin with .
 *
 *      if name is of the form nnn in ppp then only mapset ppp
 *      is searched
 *
 *  G_find_file2 (element, name, mapset)
 *
 *      exactly the same as G_find_file() except that if name is in the
 *      form nnn in ppp, and is found, name is changed to nnn.
 **********************************************************************/

#include "gis.h"

char *
G_find_file (element, name, mapset)
    char *element;
    char *name;
    char *mapset;
{
    char path[1000];
    char cpx_name[256];
    char cpx_maps[256];
    int n;

    if (*name == 0)
	return NULL;
    *path = 0;

/*
 * if name is in the name_in_mapset format, split it into
 * name, mapset (overrides what was in mapset)
 */
    if (G__name_in_mapset(name, cpx_name, cpx_maps))
    {
	name = cpx_name;
	mapset = cpx_maps;
    }

/*
 * reject illegal names and mapsets
 */
    if (G_legal_filename (name) == -1)
	    return NULL;

    if (mapset && *mapset && G_legal_filename (mapset) == -1)
	    return NULL;

/*
* if no specific mapset is to be searched
* then serach all mapsets in the mapset search list
*/
    if (mapset == 0 || *mapset == 0)
    {
	for (n = 0; mapset = G__mapset_name(n); n++)
	    if (access(G__file_name (path, element, name, mapset), 0) == 0)
		    return mapset;
    }
/*
 * otherwise just look for the file in the specified mapset
 * since the name may have been split and mapset pointing
 * to the automatic array cpx_maps[], so we must copy it to
 * permanent storage via G_store().
 */
    else
    {
	if (access(G__file_name (path, element, name, mapset),0) == 0)
		return G_store (mapset);
    }
    return NULL;
}

char *
G_find_file2 (element, name, mapset)
    char *element;
    char *name;
    char *mapset;
{
    char *mp;

    mp = G_find_file (element, name, mapset);
    if (mp)
    {
	while (*name && (*name != ' ' && *name != '\t'))
	    name++;
	*name = 0;
    }

    return mp;
}
