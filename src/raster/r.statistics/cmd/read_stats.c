#include <stdio.h>

read_stats (fd, cat1, cat2, value)
    FILE *fd;
    long *cat1, *cat2;
    double *value;
{
    char buf[1024];

    if (fgets(buf, sizeof buf, fd) == NULL) return 0;

    
    if (sscanf (buf, "%ld %ld %lf", cat1, cat2, value) == 3)
    {
/*      fprintf(stdout,"base: %ld  cover: %ld  val: %lf\n", *cat1, *cat2, *value);
/**/
      return 1;
    }

    
    fprintf (stderr, "ERROR reading r.stats output\n");
    exit(1);
}
