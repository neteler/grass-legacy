#include <stdio.h>

Pmap (scriptfile, background)
    char *scriptfile;
{
    char arg[1024];

    sprintf (arg, "input=%s", scriptfile);
    if (background && (G_fork() > 0))
	exit(0);
    execlp ("color.map", "color.map", arg,0);
    G_fatal_error ("unable to exec grey.map");
}
