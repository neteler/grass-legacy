#include <stdio.h>

int main (int argc, char *argv[])
{
    int ncols;
    int i;
    char c;

    if (argc != 2 && argc !=3)
    {
	fprintf (stderr, "usage: %s #cols [file]\n", argv[0]);
	exit(1);
    }
    if ((sscanf (argv[1], "%d", &ncols) != 1) || ncols <= 0)
    {
	fprintf (stderr, "usage: %s #cols [file]\n", argv[0]);
	exit(1);
    }
    if (argc == 3)
    {
	if (!freopen (argv[2], "r", stdin))
	{
	    perror (argv[2]);
	    exit(1);
	}
    }

    for (i = 1; (c = getchar()) != EOF; i++)
    {
	if (c < ' ')
	    fprintf (stdout," ^%c", c + 0100);
	else
	    fprintf (stdout,"  %c", c);
	if (i%ncols == 0) fprintf (stdout,"\n");
    }
    fprintf (stdout,"\n");
    exit(0);
}
