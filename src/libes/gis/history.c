/**********************************************************************
 *
 *  G_read_history (name, mapset, phist)
 *      char *name                   name of map
 *      char *mapset                 mapset that map belongs to
 *      struct History *phist        structure to hold history info
 *
 *  Reads the history information associated with map layer "map"
 *  in mapset "mapset" into the structure "phist".
 *
 *   returns:    0  if successful
 *              -1  on fail
 *
 *  note:   a warning message is printed if the file is incorrect
 *
 **********************************************************************
 *
 *  G_write_history (name, phist)
 *      char *name                   name of map
 *      struct History *phist        structure holding history info
 *
 *  Writes the history information associated with map layer "map"
 *  into current from the structure "phist".
 *
 *   returns:    0  if successful
 *              -1  on fail
 ***********************************************************************
 *
 *  G_short_history (name, type, hist)
 *     char *name             name of cell file
 *     char *type             type of cell file
 *     struct History *hist   History structure to be filled in
 *
 *  Puts local information like time and date, user's name, map name,
 *  and current mapset name into the hist structure
 *
 *  NOTE: use G_write_history() to write the structure.
 **********************************************************************/

#include "gis.h"

G_read_history (name, mapset, hist)
    char *name ;
    char *mapset ;
    struct History *hist ;
{
    FILE *fd;
    char msg[100];

    G_zero (hist, sizeof (struct History));
    fd = G_fopen_old ("hist", name, mapset);
    if (!fd)
	goto error;


    if (!G_getl(hist->mapid, sizeof(hist->mapid), fd))
	goto error;
    G_ascii_check(hist->mapid) ;

    if (!G_getl(hist->title, sizeof(hist->title), fd))
	goto error;
    G_ascii_check(hist->title) ;

    if (!G_getl(hist->mapset, sizeof(hist->mapset), fd))
	goto error;
    G_ascii_check(hist->mapset) ;

    if (!G_getl(hist->creator, sizeof(hist->creator), fd))
	goto error;
    G_ascii_check(hist->creator) ;

    if (!G_getl(hist->maptype, sizeof(hist->maptype), fd))
	goto error;
    G_ascii_check(hist->maptype) ;

    if (!G_getl(hist->datsrc_1, sizeof(hist->datsrc_1), fd))
	goto error;
    G_ascii_check(hist->datsrc_1) ;

    if (!G_getl(hist->datsrc_2, sizeof(hist->datsrc_2), fd))
	goto error;
    G_ascii_check(hist->datsrc_2) ;

    if (!G_getl(hist->keywrd, sizeof(hist->keywrd), fd))
	goto error;
    G_ascii_check(hist->keywrd) ;

    hist->edlinecnt = 0;
    while ((hist->edlinecnt < MAXEDLINES) &&
	(G_getl( hist->edhist[hist->edlinecnt], sizeof (hist->edhist[0]), fd)))
    {
	G_ascii_check( hist->edhist[hist->edlinecnt]) ;
	hist->edlinecnt++;
    }


    fclose(fd) ;
    return 0;

error:
    if (fd != NULL)
	fclose(fd) ;
    sprintf (msg,
	"can't get history information for [%s] in mapset [%s]",
	    name, mapset);
    G_warning (msg);
    return -1;
}

G_write_history (name, hist)
    char *name ;
    struct History *hist ;
{
    FILE *fd;
    int i;
    char msg[100];

    fd = G_fopen_new ("hist", name);
    if (!fd)
	    goto error;

    fprintf (fd, "%s\n", hist->mapid)    ; 
    fprintf (fd, "%s\n", hist->title)    ; 
    fprintf (fd, "%s\n", hist->mapset)  ; 
    fprintf (fd, "%s\n", hist->creator)  ; 
    fprintf (fd, "%s\n", hist->maptype)  ; 
    fprintf (fd, "%s\n", hist->datsrc_1) ; 
    fprintf (fd, "%s\n", hist->datsrc_2) ; 
    fprintf (fd, "%s\n", hist->keywrd)   ; 

    for(i=0; i < hist->edlinecnt; i++) 
	    fprintf (fd, "%s\n", hist->edhist[i]) ;

    fclose (fd) ;
    return 0;

error:
    if (fd)
	    fclose(fd) ;
    sprintf(msg, "can't write history information for [%s]", name);
    G_warning (msg);
    return -1;
}


G_short_history (name, type, hist)
    char *name;
    char *type;
    struct History *hist;
{
    strcpy(hist->mapid,G_date());
    strcpy(hist->title,name);
    strcpy(hist->mapset,G_mapset());
    strcpy(hist->creator,G_whoami());
    strcpy(hist->maptype,type);

    sprintf(hist->keywrd,"generated by %s", G_program_name());
    strcpy(hist->datsrc_1,"");
    strcpy(hist->datsrc_2,"");
    hist->edlinecnt = 0;

    return 1;
}
