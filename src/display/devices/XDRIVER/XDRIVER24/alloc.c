#include "gis.h"
#include <stdio.h>
#include <stdlib.h>
#include "includes.h"

static int num_alloc = 0;
static XPoint *pnts = NULL;

XPoint *AllocXPoints (int count)
{
    if (num_alloc < count) {
        if (num_alloc == 0)
            pnts = (XPoint *) G_malloc((size_t) (count * sizeof(XPoint)));
        else
            pnts = (XPoint *) G_realloc((void *) pnts, (size_t) (count *
                            sizeof(XPoint)));
        if (pnts == NULL) {
            fprintf(stderr, "AllocXPoints: can't alloc %d XPoints.\n",
                    count);
            num_alloc = 0;
            return NULL;
        }
        num_alloc = count;
    }
    return (pnts);
}
