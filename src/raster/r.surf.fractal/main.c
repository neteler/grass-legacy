/*****************************************************************************/
/***                                                                       ***/
/***                       main() for r.frac                               ***/
/***         GRASS module to manipulate a raster map layer.		   ***/
/***                            Jo Wood					   ***/
/***                   v 1.0  19th October, 1994			   ***/
/***                                                                       ***/
/*****************************************************************************/

#define MAIN                            

#include "frac.h"            
           
int 
main (int argc, char *argv[])       
{
    /*----------------------------------------------------------------------*/
    /*                     GET INPUT FROM USER                              */
    /*----------------------------------------------------------------------*/

    interface(argc,argv);


    /*----------------------------------------------------------------------*/
    /*   		  PROCESS RASTER FILES  		            */
    /*----------------------------------------------------------------------*/

    process();
       
    return 0;
}
