/*!
  \file gsd_img_ppm.c
 
  \brief OGSF library - PPM stuff
 
  GRASS OpenGL gsurf OGSF Library 
 
  (C) 1999-2008 by the GRASS Development Team

  - added little/big endian test Markus Neteler
  - modified to PPM by Bob Covill <bcovill@tekmap.ns.ca>
  - changed 10/99 Jaro
  - Created new function GS_write_ppm based on RGB dump 
 
  This program is free software under the 
  GNU General Public License (>=v2). 
  Read the file COPYING that comes with GRASS
  for details.
  
  \author Bill Brown USACERL, GMSL/University of Illinois
  Markus Neteler
  Bob Covill
  Jaro Hofierka
*/

#include <stdlib.h>
#include <stdio.h>

#include <grass/ogsf_proto.h>
#include <grass/gstypes.h>

int GS_write_ppm(char *name)
{
    int y, x;
    unsigned int xsize, ysize;
    FILE *fp;
    unsigned char *pixbuf;

    gsd_getimage(&pixbuf, &xsize, &ysize);


    if (NULL == (fp = fopen(name, "w"))) {
	fprintf(stderr, "Cannot open file for output.\n");
	return(1);
    }

    fprintf(fp, "P6 %d %d 255\n", xsize, ysize);

    for (y = ysize - 1; y >= 0; y--) {
	for (x = 0; x < xsize; x++) {
	    unsigned char r = pixbuf[(y * xsize + x) * 4 + 0];
	    unsigned char g = pixbuf[(y * xsize + x) * 4 + 1];
	    unsigned char b = pixbuf[(y * xsize + x) * 4 + 2];

	    fputc((int) r, fp);
	    fputc((int) g, fp);
	    fputc((int) b, fp);
	}

    }
    free(pixbuf);
    fclose(fp);

    return (0);
}

int GS_write_zoom(char *name, unsigned int xsize, unsigned int ysize)
{
    int y, x;
    FILE *fp;
    unsigned char *pixbuf;

    gsd_writeView(&pixbuf, xsize, ysize);

    if (NULL == (fp = fopen(name, "w"))) {
	fprintf(stderr, "Cannot open file for output.\n");
	return(1);
    }

    fprintf(fp, "P6 %d %d 255\n", xsize, ysize);

    for (y = ysize - 1; y >= 0; y--) {
	for (x = 0; x < xsize; x++) {
	    unsigned char r = pixbuf[(y * xsize + x) * 4 + 0];
	    unsigned char g = pixbuf[(y * xsize + x) * 4 + 1];
	    unsigned char b = pixbuf[(y * xsize + x) * 4 + 2];

	    fputc((int) r, fp);
	    fputc((int) g, fp);
	    fputc((int) b, fp);
	}

    }
    free(pixbuf);
    fclose(fp);

    return (0);
}

