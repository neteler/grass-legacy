#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "gis.h"
#include "Vect.h"
#include "shapefil.h"
#include "shp2dig.h"
#include "dbutils.h"
#include "writelin.h"

/******************************************************************/
/*                                                                */
/* v.in.shape -- import an ESRI Shapefile			  */
/*                                                                */
/* Frank Warmerdam, warmerda@home.com				  */
/* Based on my shapelib library:				  */
/*   http://gdal.velocet.ca/projects/shapelib/			  */
/******************************************************************/

/* 10/1999 added dig_cats file support
 *         Markus Neteler neteler@geog.uni-hannover.de
 *
 ******************************************************************/
 
/*03/2000 minor modifications to read data from shp2dig
 *         structures, after processing
 *04/2000 further modifications. Now splits lines at nodes
 *         and removes duplicate shapes.
 *
 *        Point file import now handled by separate module
 *         s.in.shape.
 *          David D Gray  <ddgray@armadce.demon.co.uk>
 *
 ******************************************************************/

enum {ANALYSE, RASTER, LINES, VECTOR, ALL} todo;

int debug = 0;			/* debug level (verbosity) */
FILE *fde00, *fdlog;		/* input and log file descriptors */

double scale = 1.0;

int main( int   argc, char *argv[])
{
    SHPHandle	hShapeDB;
    DBFHandle   hDBF;
    double	adfMinBound[4], adfMaxBound[4];
    int		nShapeType, nShapes, iShape, iPart, iArc;
    int         iPoint, iRec, iField;
    int         pntCount;
    int		cat_field;
    int 	pgdmp, no_rattle;

    char name[128], *p;	/* name of output files */

    char *infile, *newmapset;
    int cover_type;		/* type of coverage (line, point, area) */

    FILE *f_att = NULL;
    struct Map_info map;
    struct line_pnts *points;

    struct Categories cats;  /* added MN 10/99 */
    char    AttText[512];    /* added MN 10/99 */


    /* DDG: Create structures for processing of shapefile contents */
    lineList *ll0;
    fieldDescript *fd0;
    segmentList *segl;
    BTREE *hVB;
    int fc1;

    char errbuf[1000];

    /* DDG: variables for controlling snap distance and scale */

    float sd0;
    char *sdc;

    int init_scale;

    /************************************/

    double *pntxlist, *pntylist;
    double *xlab, *ylab;
    
    int (*btrkeycmp)(char *, char *);
    char buf[256];

    struct {
	struct Option *input, *mapset, *logfile, *verbose, *attribute, *snapd;
	struct Option *scale, *pgdump, *dumpmode;
    } parm;

    /* Are we running in Grass environment ? */

    G_gisinit (argv[0]);

    /* define the different options */

    parm.input = G_define_option() ;
    parm.input->key        = "input";
    parm.input->type       = TYPE_STRING;
    parm.input->required   = YES;
    parm.input->description= "Name of .shp (or just .dbf) file to be imported";

    parm.mapset = G_define_option() ;
    parm.mapset->key        = "mapset";
    parm.mapset->type       = TYPE_STRING;
    parm.mapset->required   = NO;
    parm.mapset->description= "Name of mapset to hold resulting files (Default = current)";

    parm.verbose = G_define_option() ;
    parm.verbose->key        = "verbose";
    parm.verbose->type       = TYPE_INTEGER;
    parm.verbose->required   = NO;
    parm.verbose->description= "Debugging level : 0 (silent) - 9 (verbose)" ;
    parm.verbose->answer     = "0" ;

    parm.logfile = G_define_option() ;
    parm.logfile->key        = "logfile";
    parm.logfile->type       = TYPE_STRING;
    parm.logfile->required   = NO;
    parm.logfile->description= "Name of file where log operations";

    parm.snapd = G_define_option() ;
    parm.snapd->key        = "snapdist";
    parm.snapd->type       = TYPE_STRING;
    parm.snapd->required   = NO;
    parm.snapd->description= "Snap distance in ground units (Default = 0.1)";
    parm.snapd->answer     = "0.1";

    parm.scale = G_define_option() ;
    parm.scale->key        = "scale";
    parm.scale->type       = TYPE_INTEGER;
    parm.scale->required   = NO;
    parm.scale->description= "Set initial scale [1:2400]";
    parm.scale->answer     = "2400";

    parm.attribute = G_define_option() ;
    parm.attribute->key        = "attribute";
    parm.attribute->type       = TYPE_STRING;
    parm.attribute->required   = NO;
    parm.attribute->description= "Name of attribute to use as category";
    parm.attribute->answer     = "";
    

    /* get options and test their validity */

    if (G_parser(argc, argv))
	exit(-1);
    
    infile = parm.input->answer;
    newmapset = parm.mapset->answer;
    
    debug = atoi( parm.verbose->answer);
    if (parm.logfile->answer == NULL)
	fdlog = stderr;
    else
	if ((fdlog = fopen( parm.logfile->answer, "w")) == NULL) {    
	    sprintf (buf, "Cannot open log file \"%s\"", parm.logfile->answer);
	    G_fatal_error( buf);
	}

    sdc = (char *)malloc(20);
    strncpy( sdc, parm.snapd->answer, 19 );
    sd0 = (float)atof(sdc);
    if( sd0 < 0 ) sd0 = -sd0;
    if( fabs( (double)sd0 ) < 0.001 ) sd0 = 0.1;

    free(sdc);

    if( procSnapDistance( SET_SD, &sd0 ) != 0 ) {
      G_fatal_error( "Error setting snap distance" );
    }

    init_scale = atoi(parm.scale->answer);
    
    /* Open input file and verify that's a good shapefile file */

    hShapeDB = SHPOpen( infile, "r" );
    if (hShapeDB == NULL)
    {
	sprintf (buf, "%s - shapefile not found, or wrong format.\n", infile);
	G_fatal_error (buf);
    }

    if (debug)
	fprintf( fdlog, "\"%s\" successfully opened\n", infile);

    /* Create a mapset and made it current mapset for this program */

    if (newmapset != NULL) {
	if (G_legal_filename( newmapset) < 0) {
	    sprintf (buf, "MAPSET <%s> - illegal name\n", newmapset);
	    G_fatal_error( buf);
	}
	if (todo == ANALYSE)
	    fprintf( fdlog, "Mapset %s not created (analyse only)\n", newmapset);
	else {    
	    sprintf( buf, "%s/%s", G_location_path(), newmapset);
	    if (access( buf, F_OK) == -1)
		if (mkdir( buf, 0755) == -1) {
		    sprintf( buf, "Cannot create MAPSET %s", newmapset);
		    G_fatal_error( buf);
		}
	    G__setenv( "MAPSET", newmapset);
	    if (debug > 2)
		fprintf( fdlog, "Mapset \"%s\" created for import\n", G_mapset());
	}
    }
    
    
    	
    /* Establish the shape types and corresponding GRASS type */
    
    SHPGetInfo( hShapeDB, &nShapes, &nShapeType, adfMinBound, adfMaxBound );

    if( nShapeType == SHPT_MULTIPATCH ) {
      sprintf( buf, "Multipatch type not yet supported" );
      SHPClose( hShapeDB );
      G_fatal_error( buf );
    }

    if( nShapeType == SHPT_POINT || nShapeType == SHPT_MULTIPOINT || nShapeType == SHPT_POINTZ ||
	nShapeType == SHPT_MULTIPOINTZ || nShapeType == SHPT_POINTM || 
	nShapeType == SHPT_MULTIPOINTM ) {
      sprintf( buf, "Point map import not now supported by this module: use s.in.shape" );
      SHPClose( hShapeDB );
      G_fatal_error( buf );
    }
    
    switch (nShapeType) {
      case SHPT_ARC:
      case SHPT_ARCZ:
      case SHPT_ARCM:
        cover_type = LINE;
        break;

      case SHPT_POLYGON:
      case SHPT_POLYGONZ:
      case SHPT_POLYGONM:
        cover_type = AREA;
        break;
    }

    if( procMapType( SET_MT, &cover_type ) != 0 ) 
      G_fatal_error( "Could not set map type to. Aborting\n"  );
    



/* -------------------------------------------------------------------- */
/*      Extract basename of shapefile.                                  */
/* -------------------------------------------------------------------- */
    for( p = infile+strlen(infile)-1;
         p != infile-1 && (isalnum(*p) || *p == '_' || *p == '.' );
         p-- ) {}
    strcpy( name, p+1);
    
    p = strrchr( name, '.');
    if (p != NULL)
        *p = '\0';

    if (debug > 4)
	fprintf( fdlog, "Name of output file is \"%s\"\n", name);

/* -------------------------------------------------------------------- */
/*      Create the GRASS vector layer based on the basename of the      */
/*      shapefile.							*/
/* -------------------------------------------------------------------- */
    Vect_open_new( &map, name);

/* -------------------------------------------------------------------- */
/*	Identify the attribute (if any) to be extracted.		*/
/* -------------------------------------------------------------------- */
    hDBF = NULL;
    if( strcmp(parm.attribute->answer,"") == 0 ) {
        cat_field = -1;
        
    } else if( strcmp(parm.attribute->answer,"list") == 0 ) {
        int	i;
        
        hDBF = DBFOpen( infile, "r" );
        if( hDBF == NULL )
        {
            sprintf (buf, "%s - DBF not found, or wrong format.\n", infile);
            G_fatal_error (buf);
        }

        fprintf (stdout, "Attributes available in %s\n", infile );
        for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
        {
            char	field_name[15];

            DBFGetFieldInfo( hDBF, i, field_name, NULL, NULL );
            fprintf (stdout, "%s\n", field_name );
        }
        
        DBFClose( hDBF );
        SHPClose( hShapeDB );

        exit( 0 );
        
    } else {
        int	i;
        
        hDBF = DBFOpen( infile, "r" );
        if( hDBF == NULL )
        {
            sprintf (buf, "%s - DBF not found, or wrong format.\n", infile);
            G_fatal_error (buf);
        }

        for( i = 0; i < DBFGetFieldCount(hDBF); i++ )
        {
            char	field_name[15];

            DBFGetFieldInfo( hDBF, i, field_name, NULL, NULL );
            if( strcasecmp( parm.attribute->answer, field_name ) == 0 )
                cat_field = i;
        }

        if( cat_field == -1 ) {
            sprintf( buf,
                     "No attribute `%s' found on %s.\n"
                     "Use attribute=list to get a list of attributes.\n",
                     parm.attribute->answer, infile );
            
            DBFClose( hDBF );
            SHPClose( hShapeDB );

            G_fatal_error( buf );
        }

        if (debug > 4)
            fprintf( fdlog, "Selected attribute field %d.\n", cat_field);

        /*
         * Create the dig_att file (or append).
         */
        if (G_find_file( "dig_att", name, G_mapset()) == NULL) {
            f_att = G_fopen_new( "dig_att", name);
            if (debug)
                fprintf( fdlog, "Creating dig_att(L) file \"%s\"\n", name);
        } else {
            f_att = G_fopen_append( "dig_att", name);
            if (debug)
                fprintf( fdlog, "Updating dig_att(L) file \"%s\"\n", name);
        }
        if (f_att == NULL)
        {
            sprintf( buf, "Unable to create attribute file `%s'.", name );
            G_fatal_error( buf );
        }

    }


  /* -------------------------------------------------------------------- */
  /*      DDG: Create the line descriptor list and field descriptor.      */
  /*           Also create line segments fro vertex database.             */
  /* -------------------------------------------------------------------- */

    if( cat_field == -1 ) {
      hDBF = DBFOpen( infile, "r" );
      if( hDBF == NULL )
	{
	  sprintf (buf, "%s - DBF not found, or wrong format.\n", infile);
	  G_fatal_error (buf);
	}
    }

    /*****************************/

    ll0 = ( lineList *)malloc( sizeof( lineList ));
    fd0 = ( fieldDescript *)malloc( (DBFGetFieldCount(hDBF) + 4) * 
				    sizeof( fieldDescript ));

    hVB = (BTREE *)malloc(sizeof (BTREE) );
  
    /* Create V-base */
    btrkeycmp = btree_compare;
    if( !btree_create( hVB, btrkeycmp, 200 )) {
      sprintf( errbuf, "Cannot create database. Aborting" );
      G_fatal_error( errbuf );;
    }

    segl = ( segmentList *)malloc( sizeof( segmentList ));
    /* Initialise segment list */
    segl->origID = 0;
    segl->numSegments = 0;
    segl->segments = NULL;

    /* Read shape into line list and fill out V-base */
    linedCreate( ll0, hShapeDB, hDBF, fd0, hVB, &fc1 );

    /* Extract arcs from V-base into segment list */
    vbase2segd( segl, hVB );

    
    /*
     * Create the dig_cats file
     */
    G_init_cats( (CELL)0,(char *)NULL,&cats);
    /* if (G_write_vector_cats(name, &cats) != 1)
       G_fatal_error("Writing dig_cats file"); */
                        




    /* Check the number of records is the same as the number of lines */
    xlab = (double *)malloc( ll0->totalValidParts * sizeof( double ) );
    ylab = (double *)malloc( ll0->totalValidParts * sizeof( double ) );

    pntCount = 0;
    for( iShape = 0; iShape < nShapes; ++iShape ) {
      for( iPart = 0; iPart < ll0->lines[iShape].numParts; ++iPart ) {
	if( ll0->lines[iShape].parts[iPart].duff ) continue;
	xlab[pntCount] = ll0->lines[iShape].parts[iPart].centroid->xcentroid;
	ylab[pntCount++] = ll0->lines[iShape].parts[iPart].centroid->ycentroid;
      }	 
    } 

    /* -------------------------------------------------------------------- */
    /*      Scan segment list to extract and write arcs.                    */
    /* -------------------------------------------------------------------- */

    for( iArc = 0; iArc < segl->numSegments; ++iArc ) {
      if( segl->segments[iArc].duff ) continue;

      pntxlist = (double *)malloc( segl->segments[iArc].numVertices *
				   sizeof( double ) );
      pntylist = (double *)malloc( segl->segments[iArc].numVertices *
				   sizeof( double ) );
      for( iPoint = 0; iPoint < segl->segments[iArc].numVertices; ++iPoint ) {
	pntxlist[iPoint] = segl->segments[iArc].vertices[iPoint]->xPosn;
	pntylist[iPoint] = segl->segments[iArc].vertices[iPoint]->yPosn;
      }
      points = Vect_new_line_struct();
      Vect_copy_xy_to_pnts( points, pntxlist, pntylist,
			    segl->segments[iArc].numVertices );
      Vect_write_line( &map, cover_type, points);
      Vect_destroy_line_struct( points );
	    
      free(pntxlist);
      free(pntylist);
      
    }
    

    /* Write attributes and category file */
	
    if( f_att != NULL ) {
      for( iRec = 0; iRec < fd0[0].nRec; ++iRec ) {
	if( cover_type == LINE )
	  fprintf( f_att, "L  %-12f  %-12f  %-8d \n",
		   xlab[iRec], ylab[iRec],
		   fd0[cat_field+4].fldRecs[iRec].intField );
	else
	  fprintf( f_att, "A  %-12f  %-12f  %-8d \n",
		   xlab[iRec], ylab[iRec],
		   fd0[cat_field+4].fldRecs[iRec].intField );

                     
	/* set cat for dig_cats file*/ /* M Neteler 10/99 */
	sprintf(AttText, "%-8d", fd0[cat_field+4].fldRecs[iRec].intField );
	if (G_set_cat(iRec, AttText, &cats) != 1)
	  G_fatal_error("Error setting category in dig_cats");
	    
      }

      free( xlab );
      free( ylab );
    }

    map.head.orig_scale = (long)init_scale;
    G_strncpy( map.head.your_name, G_whoami(), 20);
    G_strncpy( map.head.date, G_date(), 20);
    G_strncpy( map.head.map_name, name, 20);
    map.head.W = adfMinBound[0];
    map.head.S = adfMinBound[1];
    map.head.E = adfMaxBound[0];
    map.head.N = adfMaxBound[1];

    Vect_close( &map);

    SHPClose( hShapeDB );

    if( hDBF != NULL )
    {
        DBFClose( hDBF );
        fclose( f_att );
    }
    segLDispose( segl );
    btree_free( hVB );
    linedDispose( ll0, fd0, fc1 );
    
    exit(0);
}

