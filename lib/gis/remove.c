/***********************************************************************
 *
 *  G_remove (element, name)
 *     char *element          mapset element containing name
 *     char *name             file name to be removed
 *
 *  Only files in current mapset can be removed
 *
 *  Returns  -1  on fail
 *            0  if no file
 *            1  if successful
 *
 ***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <grass/gis.h>

#ifdef __MINGW32__
#define lstat(path, sb) stat(path, sb)
#endif

static int recursive_remove(const char *path);

/*!
 * \brief remove a database file
 *
 * The file or directory <b>name</b> under the database <b>element</b> directory
 * in the current mapset is removed.
 * Returns 1 if successful, 0 if <b>name</b> does not exist, and -1 if there
 * was an error.
 * <b>Note.</b> If <b>name</b> is a directory, everything within the
 * directory is removed as well.
 *
 *  \param element
 *  \param name
 *  \return int
 */

int G_remove ( char *element, char *name)
{
    char path[1024];
    char *mapset;
    char xname[512], xmapset[512];

    if (G_legal_filename(name) < 0)
	    return -1;

/* name in mapset legal only if mapset is current mapset */
    mapset = G_mapset();
    if (G__name_is_fully_qualified (name, xname, xmapset)
    && strcmp (mapset, xmapset))
	    return -1;

/* if file does not exist, return 0 */
    if (access (G__file_name (path, element, name, mapset),0) != 0)
	    return 0;

    if (recursive_remove(path) == 0)
	    return 1;

    return -1;
}

/* equivalent to rm -rf path */
static int
recursive_remove(const char *path)
{
	DIR *dirp;
	struct dirent *dp;
	struct stat sb;
	char path2[4096];

	if(lstat(path, &sb))
		return 1;
	if(!S_ISDIR(sb.st_mode))
		return remove(path) == 0 ? 0 : 1;

	if((dirp = opendir(path)) == NULL)
		return 1;
	while((dp = readdir(dirp)) != NULL)
	{
		if(dp->d_name[0] == '.')
			continue;
		if (strlen(path) + strlen(dp->d_name) + 2 > sizeof(path2))
			continue;
		sprintf(path2, "%s/%s", path, dp->d_name);
		recursive_remove(path2);
	}
	closedir(dirp);

	return rmdir(path) == 0 ? 0 : 1;
}
