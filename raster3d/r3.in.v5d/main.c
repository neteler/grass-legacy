/*
 * r3.in.v5d - program for data conversion from the V5D format 
 * of the VIS5D visualization software to G3D GRASS
 * data format.
 *
 * Copyright Jaroslav Hofierka
 * GeoModel,s.r.o., Bratislava, Slovakia 2000
 *
 * Comments or bug reports please send to hofierka@geomodel.sk
 * The use of the software is without restrictions except
 * for the use of the code in any commercial software.
 */        

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "binio.h"
#include "v5d.h"
#include <gis.h>
#include <G3d.h>

/*---------------------------------------------------------------------------*/

void *map = NULL;

/*---------------------------------------------------------------------------*/
static void
fatalError (errorMsg)

     char *errorMsg;

{
  if (map != NULL) {
    /* should unopen map here! */
  }

  G3d_fatalError (errorMsg);
}

/*---------------------------------------------------------------------------*/

typedef	struct {
  struct Option *input, *output, *nv;
} paramType;

static paramType param;

static void
setParams ()

{
  param.input = G_define_option();
  param.input->key = "input";
  param.input->type = TYPE_STRING;
  param.input->required = YES;
  param.input->description = "v5d raster file to be imported";

  param.output = G_define_option();
  param.output->key = "output";
  param.output->type = TYPE_STRING;
  param.output->required = YES;
  param.output->multiple = NO ;
  param.output->gisprompt = "new,cell,raster(2d+3d)" ;
  param.output->description = "Name for G3d raster map";

  param.nv = G_define_option();
  param.nv->key = "nv";
  param.nv->type = TYPE_STRING;
  param.nv->required = NO;
  param.nv->multiple = NO;
  param.nv->answer = "none";
  param.nv->description = 
    "String representing NULL value data cell (use 'none' if no such value)";
}

/*---------------------------------------------------------------------------*/

static void
getParams (input, output, convertNull, nullValue)

     char **input, **output;
     int *convertNull;
     double *nullValue;

{
 *input = param.input->answer;
 *output = param.output->answer; 
 *convertNull = (strcmp(param.nv->answer, "none") != 0);
 if (*convertNull)
   if (sscanf (param.nv->answer, "%lf", nullValue) != 1)
     fatalError ("getParams: NULL-value value invalid");
}

/*---------------------------------------------------------------------------*/

void convert (openFile, region, convertNull, nullValue)

     char *openFile;
     G3D_Region *region;
     int convertNull;
     double nullValue;

{
  double tmp;
   v5dstruct v5d;
   int time, var;
         int nrncnl,cnt;
         float min, max, sum, sumsum;
         int missing, good; 
  int x, y, z;
  float value; 
  float res_r,res_c,res_l;
	float *data1;

 if (!v5dOpenFile( openFile, &v5d )) {
      printf("Error: couldn't open %s for reading\n", openFile );
      exit(0);
   } 


/* Eventually change to write the time and/or vars series of g3d files.. */
/*   for (time=0; time<v5d.NumTimes; time++) {

      for (var=0; var<v5d.NumVars; var++) {
*/

   for (time=0; time<1; time++) {

      for (var=0; var<1; var++) {

         nrncnl = v5d.Nr * v5d.Nc * v5d.Nl[var];
        region->rows = v5d.Nr;
        region->cols = v5d.Nc;
        region->depths = v5d.Nl[var];
	region->north = v5d.ProjArgs[0];
	res_r = v5d.ProjArgs[2];
	res_c = v5d.ProjArgs[3];
	res_l = v5d.VertArgs[1];
	region->south = region->north - region->rows * res_r; 
	region->west = v5d.ProjArgs[1];
        region->east = region->west + region->cols * res_c;
	region->bottom = v5d.VertArgs[0];
	region->top = region->bottom + region->depths * res_l;

        data1 = (float *) malloc( nrncnl * sizeof(float) );
        if (!data1)
        G_fatal_error("Not enough memory for data1");

         if (!v5dReadGrid( &v5d, time, var, data1 )) {
            printf("Error while reading grid (time=%d,var=%s)\n",
                   time+1, v5d.VarName[var] );
            exit(0);
         }
          cnt = 0;

  for (z = 0; z < region->depths; z++) {
    for (y = 0; y < region->rows; y++) {
      for (x = 0; x < region->cols; x++) {
        value = data1[cnt++];
        if (convertNull && (value == MISSING))
          G3d_setNullValue (&value, 1, G3D_FLOAT);
        G3d_putFloat (map, x, y, z, value);
      	  }
         }
       }

        free(data1);

        }
   }


   v5dCloseFile( &v5d ); 
}

/*---------------------------------------------------------------------------*/

#define MAX(a,b) (a > b ? a : b)

main (argc, argv) 
     
     int argc;
     char *argv[];

{
  char *input, *output;
  int convertNull;
  double nullValue;
  int useTypeDefault, type, useLzwDefault, doLzw, useRleDefault, doRle;
  int usePrecisionDefault, precision, useDimensionDefault, tileX, tileY, tileZ;
  G3D_Region region;
  FILE *fp;

  map = NULL;

  G_gisinit(argv[0]);

  setParams ();
  G3d_setStandart3dInputParams ();

  if (G_parser (argc, argv)) exit(1);

  getParams (&input, &output, &convertNull, &nullValue);
  if (! G3d_getStandart3dParams (&useTypeDefault, &type, 
				 &useLzwDefault, &doLzw, 
				 &useRleDefault, &doRle, 
				 &usePrecisionDefault, &precision, 
				 &useDimensionDefault, &tileX, &tileY, &tileZ))
    fatalError ("main: error getting standard parameters");

  G3d_getWindow (&region);
  map = G3d_openCellNew (output, G3D_FLOAT, G3D_USE_CACHE_XY,&region);
  if (map == NULL) fatalError ("main: error opening g3d file");

  convert(input, &region, convertNull, nullValue);
  
  if (! G3d_closeCell (map)) 
    fatalError ("main: error closing new g3d file");
  map = NULL;
}

/*---------------------------------------------------------------------------*/
