/*********************************************************************************/
/**	                                                            		**/
/**                              r.param.scale                 			**/
/**       GRASS module for extracting multi-scale surface parameters.		**/
/**  										**/
/**                                                         			**/
/**			  Jo Wood, V 1.1, 11th December, 1994			**/
/**                                $Id$      					**/
/*********************************************************************************/

#define MAIN

#include "param.h"	

int main(int argc, char **argv)
{

    /*--------------------------------------------------------------------------*/
    /*                                 INITIALISE				*/
    /*--------------------------------------------------------------------------*/ 



    /*--------------------------------------------------------------------------*/
    /*                               GET INPUT FROM USER			*/
    /*--------------------------------------------------------------------------*/

    interface(argc,argv);


    /*--------------------------------------------------------------------------*/
    /*                        OPEN INPUT AND OUTPUT RASTER FILES		*/
    /*--------------------------------------------------------------------------*/

    open_files();


    /*--------------------------------------------------------------------------*/
    /*                       PROCESS SURFACE FOR FEATURE DETECTION 		*/
    /*--------------------------------------------------------------------------*/

    process();   

    /*--------------------------------------------------------------------------*/
    /*                     CLOSE ALL OPENED FILES AND FREE MEMORY		*/
    /*--------------------------------------------------------------------------*/

    close_down();

    if (mparam == FEATURE)
    {
	write_cols();
	write_cats();
    }

    return 0;
}
