/******************************************************************************
 * $Id$
 *
 * Project:  PROJ.4
 * Purpose:  Some utility functions we don't want to bother putting in
 *           their own source files.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2001, Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log$
 * Revision 1.1  2002-04-20 19:13:44  roger
 * Updating Proj lib to 4.4.5, and adding two new functions for datum conversions
 *
 * Revision 1.1  2001/04/05 04:22:46  warmerda
 * New
 *
 */

#define PJ_LIB__

#include "projects.h"
#include <string.h>
#include <math.h>

/************************************************************************/
/*                           pj_is_latlong()                            */
/*                                                                      */
/*      Returns TRUE if this coordinate system object is                */
/*      geographic.                                                     */
/************************************************************************/

int pj_is_latlong( PJ *pj )

{
    return pj == NULL || pj->is_latlong;
}

/************************************************************************/
/*                        pj_latlong_from_proj()                        */
/*                                                                      */
/*      Return a PJ* definition defining the lat/long coordinate        */
/*      system on which a projection is based.  If the coordinate       */
/*      system passed in is latlong, a clone of the same will be        */
/*      returned.                                                       */
/************************************************************************/

PJ *pj_latlong_from_proj( PJ *pj_in )

{
    char	defn[512];
    int		got_datum = FALSE;

    pj_errno = 0;
    strcpy( defn, "+proj=latlong" );

    if( pj_param(pj_in->params, "tdatum").i )
    {
        got_datum = TRUE;
        sprintf( defn+strlen(defn), " +datum=%s", 
                 pj_param(pj_in->params,"sdatum").s );
    }
    else if( pj_param(pj_in->params, "tellps").i )
    {
        sprintf( defn+strlen(defn), " +ellps=%s", 
                 pj_param(pj_in->params,"sellps").s );
    }
    else if( pj_param(pj_in->params, "ta").i )
    {
        sprintf( defn+strlen(defn), " +a=%s", 
                 pj_param(pj_in->params,"sa").s );
            
        if( pj_param(pj_in->params, "tb").i )
            sprintf( defn+strlen(defn), " +b=%s", 
                     pj_param(pj_in->params,"sb").s );
        else if( pj_param(pj_in->params, "tes").i )
            sprintf( defn+strlen(defn), " +es=%s", 
                     pj_param(pj_in->params,"ses").s );
        else if( pj_param(pj_in->params, "tf").i )
            sprintf( defn+strlen(defn), " +f=%s", 
                     pj_param(pj_in->params,"sf").s );
        else
            sprintf( defn+strlen(defn), " +es=%.4f", 
                     pj_in->es );
    }
    else
    {
        pj_errno = -13;

        return NULL;
    }

    if( !got_datum )
    {
        if( pj_param(pj_in->params, "ttowgs84").i )
            sprintf( defn+strlen(defn), " +towgs84=%s", 
                     pj_param(pj_in->params,"stowgs84").s );

        if( pj_param(pj_in->params, "ttowgs84").i )
            sprintf( defn+strlen(defn), " +towgs84=%s", 
                     pj_param(pj_in->params,"stowgs84").s );

        if( pj_param(pj_in->params, "tnadgrids").i )
            sprintf( defn+strlen(defn), " +nadgrids=%s", 
                     pj_param(pj_in->params,"snadgrids").s );
    }

    /* copy over some other information related to ellipsoid */
    if( pj_param(pj_in->params, "tR").i )
        sprintf( defn+strlen(defn), " +R=%s", 
                 pj_param(pj_in->params,"sR").s );

    if( pj_param(pj_in->params, "tR_A").i )
        sprintf( defn+strlen(defn), " +R_A" );

    if( pj_param(pj_in->params, "tR_V").i )
        sprintf( defn+strlen(defn), " +R_V" );

    if( pj_param(pj_in->params, "tR_a").i )
        sprintf( defn+strlen(defn), " +R_a" );

    if( pj_param(pj_in->params, "tR_lat_a").i )
        sprintf( defn+strlen(defn), " +R_lat_a=%s", 
                 pj_param(pj_in->params,"sR_lat_a").s );

    if( pj_param(pj_in->params, "tR_lat_g").i )
        sprintf( defn+strlen(defn), " +R_lat_g=%s", 
                 pj_param(pj_in->params,"sR_lat_g").s );

    printf( "pj_latlong_from_proj->%s\n", defn );

    return pj_init_plus( defn );
}

