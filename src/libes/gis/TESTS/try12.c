#include "gis.h"

main()
{
    char name[30], *mapset;
    struct Colors colr;
    int r,g,b;
    CELL n;
    char buf[100];

    G_gisinit("try");
    while (mapset = G_ask_cell_old ("", name))
    {
	fprintf (stdout,"read colors\n");
	if (G_read_colors (name, mapset, &colr) < 0)
	{
	    fprintf (stdout,"OOPS\n");
	    continue;
	}
	fprintf (stdout,"ok\n");
	if (colr.min > 0)
	{
	    G_get_color ((CELL)0, &r, &g, &b, &colr);
	    fprintf (stdout,"%d: %d %d %d\n", 0, r, g, b);
	}
	for (n = colr.min; n <= colr.max; n++)
	{
	    G_get_color (n, &r, &g, &b, &colr);
	    fprintf (stdout,"%ld: %d %d %d\n", (long)n, r, g, b);
	}
	if (colr.max < 0)
	{
	    G_get_color ((CELL)0, &r, &g, &b, &colr);
	    fprintf (stdout,"%d: %d %d %d\n", 0, r, g, b);
	}
	fprintf (stdout,"Update? ");
	if (!gets(buf)) exit(0);
	if (*buf == 'y')
	{
	    fprintf (stdout,"updating\n");
	    G_write_colors (name, mapset, &colr);
	}
	G_free_colors (&colr);
    }
}
