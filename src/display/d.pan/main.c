/*
 *   d.pan
 *
 *   Change region through graphics - select new center
 */

#include "gis.h"
#include "raster.h"
#include "display.h"
#include "local_proto.h"

int main (int argc, char **argv)
{
    int stat;
    char rast[128], vect[128];
    char command[128];
    struct Flag *quiet;
    struct Option *map, *vmap, *zoom;
    double magnify;
    char *mapset;

/* Initialize the GIS calls */
    G_gisinit(argv[0]) ;
    R_open_driver();

    if(D_get_cell_name (rast) < 0)
               *rast = 0;
    if(D_get_dig_name (vect) < 0)
               *vect = 0;
    R_close_driver();
                    
    map = G_define_option();
    map->key = "rast";
    map->type = TYPE_STRING;
    if (*rast)
          map->answer = rast;
    if (*rast)
          map->required = NO;
       else
          map->required = YES;
    map->gisprompt = "old,cell,raster" ;
    map->description = "Name of raster map";
                                                        
    vmap = G_define_option();
    vmap->key = "vector";
    vmap->type = TYPE_STRING;
    if (*vect)
          vmap->answer = vect;
    if (*vect)
          vmap->required = NO;
       else
          vmap->required = YES;
    vmap->gisprompt = "old,dig,vector" ;
    vmap->description = "Name of vector map";
                                                        
    quiet = G_define_flag();
    quiet->key = 'q';
    quiet->description = "Quiet";

    zoom = G_define_option() ;
    zoom->key        = "zoom" ;
    zoom->type       = TYPE_DOUBLE ;
    zoom->required   = NO ;
    zoom->answer     = "1.0" ;
    zoom->options    = "0.001-1000.0" ;
    zoom->description= "magnification: >1.0 zooms in, <1.0 zooms out" ;

    if (argc > 1 && G_parser(argc,argv))
	exit(1);

    sscanf(zoom->answer,"%lf", &magnify); 

/* Make sure map is available */
    if (map->answer == NULL) exit(0);
       mapset = G_find_cell2 (map->answer, "");
    if (mapset == NULL)
    {
	char msg[256];
	sprintf(msg,"Raster file [%s] not available", map->answer);
	G_fatal_error(msg) ;
    }

    /* if map was found in monitor: */
    if (*rast || *vect) 
       quiet->answer=1;
        
    R_open_driver();

    D_setup(0);

    if (G_projection() == PROJECTION_LL)
    {
    }

/* Do the pan */
    stat = pan(quiet->answer,magnify) ;

    R_close_driver();

/* Redraw raster map */
    if (*rast)
    {
      sprintf(command, "d.erase; d.rast map=%s", map->answer);
      system(command);
    }

/* Redraw vector map */
    if (*vect)
    {
      sprintf(command, "d.vect map=%s", vmap->answer);
      system(command);
    }
    
    exit(stat);
}

