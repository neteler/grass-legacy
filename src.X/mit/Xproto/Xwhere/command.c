/*  %W%  %G%  */

#include <stdio.h>
#include "options.h"


stash_away(pos, option)
char *option;
{
    char name[100];
    double a, e;

    switch (pos) {
    case SPHEROID:
        strcpy(name, option);
        G_strip(name);
        if (*name == 0 || CC_get_spheroid(name, &a, &e) == NULL) {
            fprintf(stderr, "s=%s: unknown spheroid\n", name);
            return -1;
        }
        CC_u2ll_spheroid_parameters(a, e);
        CC_u2ll_zone(G_zone());
        have_spheroid = 1;
        break;
    case MODE:
        if (!strcmp("quiet", option))
            mode = SILENT;
        else if (!strcmp("silent", option))
            mode = SILENT;
        else
            mode = LOUD;
        break;
    default:
        fprintf(stderr, "Unknown option\n");
        return -1;
    }
    return 0;
}
