/*-
 * r.simwe.hydro: main program for hydrologic and sediment transport
 * simulation (SIMWE)
 *
 * Original program (2002) and various modifications:
 * Lubos Mitas, Helena Mitasova
 *
 * GRASS5.0 version of the program:
 * J. Hofierka
 *
 * Copyright (C) 2002 L. Mitas,  H. Mitasova, J. Hofierka
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *
 *
 * Notes on modifications:
 * v. 1.0 May 2002
 *
 */

#define NWALK	"2000000"
#define DIFFC	"0.8"
#define HMAX	"0.4"
#define HALPHA	"4.0"
#define	HBETA	"0.5"
#define NITER   "1200"
#define ITEROUT "300"
#define DENSITY "200"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include "config.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "gis.h"
#include "Vect.h"
#include "linkm.h"
#include "bitmap.h"
#include "site.h"

#include "waterglobs.h"

char fncdsm[32];
char filnam[10];

/*struct BM *bitmask;*/
/*struct Cell_head cellhd;*/
struct GModule *module;
struct Map_info Map;

char msg[1024];

int main ( int argc, char *argv[]) 
{
  int i,ii,j,l;
  int ret_val;
  double x_orig, y_orig;
  static int rand1 = 12345;
  static int rand2 = 67891;

  G_gisinit (argv[0]);

  module = G_define_module();
  module->description =        
                  "Overland flow hydrologic model based on duality "
                  "particle-field concept (SIMWE) ";
                  
  if (G_get_set_window (&cellhd) == -1)
    exit (0);

  conv = G_database_units_to_meters_factor();

  mixx = conv * cellhd.west;
  maxx = conv * cellhd.east;
  miyy = conv * cellhd.south;
  mayy = conv * cellhd.north;

  stepx = cellhd.ew_res * conv;
  stepy = cellhd.ns_res * conv;
/*  step = amin1(stepx,stepy);*/
  step = (stepx + stepy) / 2.;
  mx = cellhd.cols;
  my = cellhd.rows;
  x_orig = cellhd.west * conv;
  y_orig = cellhd.south * conv;/* do we need this? */
    xmin = 0.;
    ymin = 0.;
    xp0 = xmin + stepx / 2.;
    yp0 = ymin + stepy / 2.;
    xmax = xmin + stepx * (float) mx;
    ymax = ymin + stepy * (float) my;
	
    bxmi=2093113. * conv;
    bymi=731331. * conv;
    bxma=2093461. * conv;
    byma=731529. * conv;
    bresx=2. * conv;
    bresy=2. * conv;
    maxwab=100000;

    mx2o= (int)((bxma-bxmi)/bresx);
    my2o= (int)((byma-bymi)/bresy);

/* relative small box coordinates: leave 1 grid layer for overlap */

    bxmi = bxmi - mixx + stepx;
    bymi = bymi - miyy + stepy;
    bxma = bxma - mixx - stepx;
    byma = byma - miyy - stepy;
    mx2 = mx2o - 2*((int) (stepx / bresx));
    my2 = my2o - 2*((int) (stepy / bresy)); 

  fprintf (stdout, "\n");
  fprintf (stdout, "Authors: original version L.Mitas, H.Mitasova\n");
  fprintf (stdout, "         GRASS implementation J. Hofierka\n"); 
  fprintf (stdout, "see references in manual page or at:\n");
  fprintf (stdout, "http://www2.gis.uiuc.edu:2280/modviz/papers/listsj.html\n");
  fprintf (stdout, "\n");
  fflush(stdout);

  parm.elevin = G_define_option();
  parm.elevin->key = "elevin";
  parm.elevin->type = TYPE_STRING;
  parm.elevin->required = YES;
  parm.elevin->gisprompt = "old,cell,raster";
  parm.elevin->description = "Name of the elevation raster file";

  parm.dxin = G_define_option();
  parm.dxin->key = "dxin";
  parm.dxin->type = TYPE_STRING;
  parm.dxin->required = YES;
  parm.dxin->gisprompt = "old,cell,raster";
  parm.dxin->description = "Name of the x-derivatives raster file";

  parm.dyin = G_define_option();
  parm.dyin->key = "dyin";
  parm.dyin->type = TYPE_STRING;
  parm.dyin->required = YES;
  parm.dyin->gisprompt = "old,cell,raster";
  parm.dyin->description = "Name of the y-derivatives raster file";

  parm.rain = G_define_option();
  parm.rain->key = "rain";
  parm.rain->type = TYPE_STRING;
  parm.rain->required = YES;
  parm.rain->gisprompt = "old,cell,raster";
  parm.rain->description = "Name of the rainfall excess raster file";

  parm.infil = G_define_option();
  parm.infil->key = "infil";
  parm.infil->type = TYPE_STRING;
  parm.infil->required = YES;
  parm.infil->gisprompt = "old,cell,raster";
  parm.infil->description = "Name of the infiltration excess raster file";

  parm.traps = G_define_option();
  parm.traps->key = "traps";
  parm.traps->type = TYPE_STRING;
  parm.traps->required = NO;
  parm.traps->gisprompt = "old,cell,raster";
  parm.traps->description = "Name of the flow control raster file";

  parm.manin = G_define_option();
  parm.manin->key = "manin";
  parm.manin->type = TYPE_STRING;
  parm.manin->required = YES;
  parm.manin->gisprompt = "old,cell,raster";
  parm.manin->description = "Name of the Mannings n raster file";

  parm.sfile = G_define_option ();
  parm.sfile->key = "sites";
  parm.sfile->type = TYPE_STRING;
  parm.sfile->required = NO;
  parm.sfile->gisprompt = "old,site_lists,sites";
  parm.sfile->description = "Name of the site file with x,y locations";

  parm.depth = G_define_option();
  parm.depth->key = "depth";
  parm.depth->type = TYPE_STRING;
  parm.depth->required = NO;
  parm.depth->gisprompt = "new,cell,raster";
  parm.depth->description = "Output water depth raster file";

  parm.disch = G_define_option();
  parm.disch->key = "disch";
  parm.disch->type = TYPE_STRING;
  parm.disch->required = NO;
  parm.disch->gisprompt = "new,cell,raster";
  parm.disch->description = "Output water discharge raster file";

  parm.err = G_define_option();
  parm.err->key = "err";
  parm.err->type = TYPE_STRING;
  parm.err->required = NO;
  parm.err->gisprompt = "new,cell,raster";
  parm.err->description = "Output simulation error raster file";

  parm.outwalk = G_define_option ();
  parm.outwalk->key = "outwalk";
  parm.outwalk->type = TYPE_STRING;
  parm.outwalk->required = NO;
  parm.outwalk->gisprompt = "new,site_lists,sites";
  parm.outwalk->description = "Name of the output walkers site file";

  parm.nwalk = G_define_option();
  parm.nwalk->key = "nwalk";
  parm.nwalk->type = TYPE_INTEGER;
  parm.nwalk->answer = NWALK;
  parm.nwalk->required = NO;
  parm.nwalk->description = "Number of walkers";

  parm.niter = G_define_option();
  parm.niter->key = "niter";
  parm.niter->type = TYPE_INTEGER;
  parm.niter->answer = NITER;
  parm.niter->required = NO;
  parm.niter->description = "Number of time iterations (sec.)";

  parm.outiter = G_define_option();
  parm.outiter->key = "outiter";
  parm.outiter->type = TYPE_INTEGER;
  parm.outiter->answer = ITEROUT;
  parm.outiter->required = NO;
  parm.outiter->description = "Time step for saving output maps (sec.)";

  parm.density = G_define_option();
  parm.density->key = "density";
  parm.density->type = TYPE_INTEGER;
  parm.density->answer = DENSITY;
  parm.density->required = NO;
  parm.density->description = "Density of output walkers";

  parm.diffc = G_define_option();
  parm.diffc->key = "diffc";
  parm.diffc->type = TYPE_DOUBLE;
  parm.diffc->answer = DIFFC;
  parm.diffc->required = NO;
  parm.diffc->description = "Water diffusion constant";

  parm.hmax = G_define_option();
  parm.hmax->key = "hmax";
  parm.hmax->type = TYPE_DOUBLE;
  parm.hmax->answer = HMAX;
  parm.hmax->required = NO;
  parm.hmax->description = "Threshold water depth (diffusion increases after this water depth is reached)";

  parm.halpha = G_define_option();
  parm.halpha->key = "halpha";
  parm.halpha->type = TYPE_DOUBLE;
  parm.halpha->answer = HALPHA;
  parm.halpha->required = NO;
  parm.halpha->description = "Diffusion increase constant";

  parm.hbeta = G_define_option();
  parm.hbeta->key = "hbeta";
  parm.hbeta->type = TYPE_DOUBLE;
  parm.hbeta->answer = HBETA;
  parm.hbeta->required = NO;
  parm.hbeta->description = "Weighting factor for water flow velocity vector";

  flag.mscale = G_define_flag ();
  flag.mscale->key = 'm';
  flag.mscale->description = "Multiscale simulation";

  flag.tserie = G_define_flag ();
  flag.tserie->key = 't';
  flag.tserie->description = "Time-series (dynamic) output";


  if (G_parser (argc, argv))
    exit (1);

  mscale=flag.mscale->answer;
  tserie=flag.tserie->answer;

  elevin = parm.elevin->answer;
  dxin = parm.dxin->answer;
  dyin = parm.dyin->answer;
/*  maskmap = parm.maskmap->answer;*/
  rain = parm.rain->answer;
  infil = parm.infil->answer;
  traps = parm.traps->answer;
  manin = parm.manin->answer;
  depth = parm.depth->answer;
  disch = parm.disch->answer;
  err = parm.err->answer;
  outwalk = parm.outwalk->answer;
  sfile = parm.sfile->answer;

  sscanf(parm.nwalk->answer, "%d", &maxwa);
  sscanf(parm.niter->answer, "%d", &timesec);
  sscanf(parm.outiter->answer, "%d", &iterout);
  sscanf(parm.density->answer, "%d", &ldemo);
  sscanf(parm.diffc->answer, "%lf", &frac);
  sscanf(parm.hmax->answer, "%lf", &hhmax);
  sscanf(parm.halpha->answer, "%lf", &halpha);
  sscanf(parm.hbeta->answer, "%lf", &hbeta);

    rwalk = (double) maxwa;

  if (conv != 1.0) 
  printf ("\nUsing metric conversion factor %f, step=%f",conv,step);


  /*
   * G_set_embedded_null_value_mode(1);
   */

  if ((depth == NULL) && (disch == NULL) && (err == NULL) && (outwalk == NULL))
    fprintf (stderr, "Warning -- you are not outputing any raster or site files\n");

  ret_val = input_data();
  if (ret_val != 1)
    G_fatal_error ("Input failed");

/* memory allocation for output grids */

        gama = (double **)malloc(sizeof(double)*(my));
           for(l=0;l<my;l++)
              {
                gama[l]   = (double*)malloc(sizeof(double)*(mx));
              }
           for (j = 0; j < my; j++)
              {
                for (i = 0; i < mx; i++)
                   gama[j][i] = 0.;
               }

   if (err != NULL)
      {
        gammas = (double **)malloc(sizeof(double)*(my));
           for(l=0;l<my;l++)
              {
                gammas[l]   = (double*)malloc(sizeof(double)*(mx));
              }
           for (j = 0; j < my; j++)
              {
                for (i = 0; i < mx; i++)
                   gammas[j][i] = 0.;
               }
       }

        dif = (float **)malloc(sizeof(float)*(my));
           for(l=0;l<my;l++)
              {
                dif[l]   = (float*)malloc(sizeof(float)*(mx));
              }
           for (j = 0; j < my; j++)
              {
                for (i = 0; i < mx; i++)
                   dif[j][i] = 0.;
               }




/*  if (maskmap != NULL)
    bitmask = BM_create (cols, rows);

  IL_create_bitmask (&params, bitmask);
*/


  seeds(rand1,rand2);

  grad_check();

/*  fprintf (stdout, "\n Percent complete: ");*/

  main_loop();

  if (tserie == NULL) {
  ii=output_data (0,1.);
  if (ii != 1)
    G_fatal_error ("Cannot write cell files");
  }

  if (fdwalkers != NULL)
    fclose (fdwalkers);

  if (sfile != NULL)
                fclose(fw);

  exit (0);
}

