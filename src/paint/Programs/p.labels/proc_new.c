#include <string.h>
#include <stdio.h>
#include "config.h"

int process_new (FILE *out, char *file)
{
    int i;


    while (1)
    {
	for (i = 0; i < TEXTLINES; i++)
	    config.text[i][0] = 0;
	config.xoffset[0] = config.yoffset[0] = 0;
	strcpy (config.skip,"no");
	setup (file,1);
	if(!modify())
	    return 0;
	if(!update(out))
	    return 1;
    }
}
