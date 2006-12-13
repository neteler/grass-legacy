/*-
 * Written by H. Mitasova, I. Kosinovsky, D. Gerdes Fall 1993
 * University of Illinois
 * US Army Construction Engineering Research Lab  
 * Copyright 1993, H. Mitasova (University of Illinois),
 * I. Kosinovsky, (USA-CERL), and D.Gerdes (USA-CERL)   
 *
 * modified by McCauley in August 1995
 * modified by Mitasova in August 1995  
 * modofied by Mitasova in Nov 1999 (dmax fix)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <grass/bitmap.h>
#include <grass/linkm.h>
#include <grass/gis.h>
#include <grass/dbmi.h>
#include <grass/Vect.h>
#include <grass/glocale.h>

#include <grass/interpf.h>

int IL_vector_input_data_2d (
    struct interp_params *params,
    struct Map_info *Map,		/* input vector map */
    /* as z values may be used: 1) z coordinates in 3D file -> field = 0
     *                          2) categories -> field > 0, zcol = NULL
     *                          3) attributes -> field > 0, zcol != NULL */ 
    int    field,               /* category field number */
    char   *zcol,                /* name of the column containing z values */
    char   *scol,                /* name of the column containing smooth values */
    struct tree_info *info,	/* quadtree info */
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    double *zmin,
    double *zmax,
    int *n_points,		/* number of points used for interpolation */
    double *dmax
)

/*
 * Inserts input data inside the region into a quad tree. Also translates
 * data. Returns number of segments in the quad tree.
 */

{
  double  dmax2;	/* max distance between points squared*/
  double c1, c2, c3, c4;
  int i, line, k = 0, nnodes;
  double ns_res, ew_res;
  int npoint, OUTRANGE;
  int totsegm;
  struct quaddata *data = (struct quaddata *) info->root->data;
  double xprev, yprev, zprev, x1, y1, z1, d1, xt, yt, z, sm;
  struct line_pnts *Points;
  struct line_cats *Cats;
  int times, j1, k1, ltype, cat, zctype=0, sctype=0;
  struct field_info *Fi;
  dbDriver  *driver;
  dbHandle  handle;
  dbString  stmt;
  dbCatValArray zarray, sarray;

  OUTRANGE = 0;
  npoint = 0;

 G_debug(2, "IL_vector_input_data_2d(): field = %d, zcol = %s, scol = %s", field, zcol, scol);
  ns_res = (data->ymax - data->y_orig) / data->n_rows;
  ew_res = (data->xmax - data->x_orig) / data->n_cols;
  dmax2=*dmax * *dmax;

  Points = Vect_new_line_struct ();	/* init line_pnts struct */
  Cats = Vect_new_cats_struct ();

  if ( field == 0 && !Vect_is_3d(Map) )
	G_fatal_error(_("Vector is not 3D"));

  if ( field > 0 && zcol != NULL ) { /* open db driver */
    fprintf (stdout, "Loading data from attribute table ...\n");
    Fi = Vect_get_field( Map, field);
    if ( Fi == NULL )
	G_fatal_error(_("Cannot get layer info"));   
    G_debug ( 3, "  driver = %s database = %s table = %s", Fi->driver, Fi->database, Fi->table);
    db_init_handle (&handle);
    db_init_string ( &stmt);
    driver = db_start_driver(Fi->driver);
    db_set_handle (&handle, Fi->database, NULL);
    if (db_open_database(driver, &handle) != DB_OK)
	G_fatal_error(_("Cannot open database %s"), Fi->database);
    
    zctype = db_column_Ctype ( driver, Fi->table, zcol );
    G_debug ( 3, " zcol C type = %d", zctype );
    if ( zctype == -1 )
	G_fatal_error(_("Cannot find z column <%s> (please verify name, e.g. with v.info)"), zcol);
    if ( zctype != DB_C_TYPE_INT && zctype != DB_C_TYPE_DOUBLE ) 
	G_fatal_error(_("Column type of z column is not supported (must be integer or double)"));

    db_CatValArray_init ( &zarray );
    db_select_CatValArray ( driver, Fi->table, Fi->key, zcol, NULL, &zarray );

    if ( scol != NULL ) {
	sctype = db_column_Ctype ( driver, Fi->table, scol );
	G_debug ( 3, " scol C type = %d", sctype );
	if ( sctype == -1 )
	    G_fatal_error(_("Cannot read column type of smooth column"));
	if ( sctype == DB_C_TYPE_DATETIME )
	    G_fatal_error(_("Column type of smooth column (datetime) is not supported"));
	if ( sctype != DB_C_TYPE_INT && sctype != DB_C_TYPE_DOUBLE ) 
	    G_fatal_error(_("Column type of s column is not supported (must be integer or double)"));

	db_CatValArray_init ( &sarray );
	db_select_CatValArray ( driver, Fi->table, Fi->key, scol, NULL, &sarray );
    }

  db_close_database_shutdown_driver ( driver );
  }
	
  /* Lines without nodes */
  fprintf (stderr, "Reading lines from vector map ... ");
  sm = 0;
  line=1;
  while ( (ltype=Vect_read_next_line(Map, Points, Cats)) != -2 ) { 
      
	if ( ! (ltype & ( GV_POINT | GV_LINE | GV_BOUNDARY ) ) ) continue;
      
      if ( field > 0 ) { /* use cat or attribute */
        Vect_cat_get( Cats, field, &cat);

    /*    line++;*/
        if ( zcol == NULL  ){ /* use categories */
	    z = (double) cat;
	} else { /* read att from db */
	    int ret, intval;

	    if ( zctype == DB_C_TYPE_INT ) {
		ret = db_CatValArray_get_value_int ( &zarray, cat, &intval );
		z = intval;
	    } else { /* DB_C_TYPE_DOUBLE */
		ret = db_CatValArray_get_value_double ( &zarray, cat, &z );
	    }

	    if ( ret != DB_OK ) {
		G_warning(_("Database record for cat %d not found"), cat);
		continue;
	    }
		
            if ( scol != NULL ) {
		if ( sctype == DB_C_TYPE_INT ) {
		    ret = db_CatValArray_get_value_int ( &sarray, cat, &intval );
		    sm = intval;
		} else { /* DB_C_TYPE_DOUBLE */
		    ret = db_CatValArray_get_value_double ( &sarray, cat, &sm );
		}
		if(sm < 0.0) G_fatal_error("Negative value of smoothing detected: sm must be >= 0");
	    }
            G_debug ( 5, "  z = %f sm = %f", z, sm );
	}
      }
	
      if  ( Vect_level(Map) == 1) {
      /* Insert all points including nodes (end points) */
      for ( i = 0; i < Points->n_points; i++ ) { 
        if ( field == 0 ) z = Points->z[i];
	process_point ( Points->x[i], Points->y[i], z, sm, info, params->zmult, xmin,
	                xmax, ymin, ymax, zmin, zmax, &npoint, &OUTRANGE, &k);

      }
      }
      else {
	/* Insert all points except nodes (end points) */
	for ( i = 1; i < Points->n_points - 1; i++ ) {
          if ( field == 0 ) z = Points->z[i];
          process_point ( Points->x[i], Points->y[i], z, sm, info, params->zmult, xmin,
	                  xmax, ymin, ymax, zmin, zmax, &npoint, &OUTRANGE, &k);

        }
      }

      /* Check all segments */
      xprev = Points->x[0]; yprev = Points->y[0]; zprev = Points->z[0];
      for ( i = 1; i < Points->n_points; i++ ) { 
	  /* compare the distance between current and previous */
	  x1 = Points->x[i]; y1 = Points->y[i]; z1 = Points->z[i];
	  
	  xt = x1 - xprev; yt = y1 - yprev;
	  d1 = (xt * xt + yt * yt);
	  if ((d1 > dmax2) && (dmax2 != 0.))
	  {
	    times = (int) (d1 / dmax2 + 0.5);
	    for (j1 = 0; j1 < times; j1++)
	    {
	      xt = x1 - j1 * ((x1 - xprev) / times);
	      yt = y1 - j1 * ((y1 - yprev) / times);
              if ( field == 0 ) z = z1 - j1 * ((z1 - zprev) / times);

	      process_point (xt, yt, z, sm, info, params->zmult,
		             xmin, xmax, ymin, ymax, zmin, zmax, &npoint, &OUTRANGE, &k);
	    }
	  }
	  xprev = x1;
	  yprev = y1;
	  zprev = z1;
      }
  }

    /* Process all nodes */
  fprintf (stderr, "Reading nodes from vector map ... ");
  nnodes = Vect_get_num_nodes (Map);
  for (k1 = 1; k1 <= nnodes; k1++) {
    G_debug ( 5, "  node %d", k1 );
    G_percent ( k1, nnodes-1, 1 );
    Vect_get_node_coor ( Map, k1, &x1, &y1, &z );

    /* TODO: check more lines ? */
    if ( field > 0 ) {
        line = abs ( Vect_get_node_line ( Map, k1, 0 ) );
        ltype = Vect_read_line ( Map, NULL, Cats, line );
        Vect_cat_get( Cats, field, &cat);
        if ( zcol == NULL  ){ /* use categories */
            if ( cat < 0 ) continue;
            z = (double) cat;
        } else { /* read att from db */
            int ret, intval;
            if ( cat == 0 ) continue;

            if ( zctype == DB_C_TYPE_INT ) {
                ret = db_CatValArray_get_value_int ( &zarray, cat, &intval );
                z = intval;
            } else { /* DB_C_TYPE_DOUBLE */
                ret = db_CatValArray_get_value_double ( &zarray, cat, &z );
            }

            if ( ret != DB_OK ) {
                G_warning(_("Database record for cat %d not found"), cat);
                continue;
            }

            if ( scol != NULL ) {
                if ( sctype == DB_C_TYPE_INT ) {
                    ret = db_CatValArray_get_value_int ( &sarray, cat, &intval );
                    sm = intval;
                } else { /* DB_C_TYPE_DOUBLE */
                    ret = db_CatValArray_get_value_double ( &sarray, cat, &sm );
                }
                if(sm < 0.0) G_fatal_error("Negative value of smoothing detected: sm must be >= 0");
            }
            G_debug ( 5, "  z = %f sm = %f", z, sm );
        }
    }

    process_point (x1, y1, z, sm, info, params->zmult, xmin, xmax, ymin, ymax, zmin, zmax, 
                   &npoint, &OUTRANGE, &k);
  }

  if ( field > 0 && zcol != NULL )
      db_CatValArray_free ( &zarray );
  if ( scol != NULL ) {
      db_CatValArray_free ( &sarray );
  }

  c1 = *xmin - data->x_orig;
  c2 = data->xmax - *xmax;
  c3 = *ymin - data->y_orig;
  c4 = data->ymax - *ymax;
  if ((c1 > 5 * ew_res) || (c2 > 5 * ew_res) || (c3 > 5 * ns_res) || (c4 > 5 * ns_res))
  {
    static int once = 0;

    if (!once)
    {
      once = 1;
      G_warning(_("strip exists with insufficient data"));
    }
  }

  totsegm = translate_quad (info->root, data->x_orig, data->y_orig, *zmin, 4);
  if (!totsegm)
    return 0;
  data->x_orig = 0;
  data->y_orig = 0;

  /* G_read_vector_timestamp(name,mapset,ts); */

  fprintf (stderr, "\n");
  if (OUTRANGE > 0)
	G_warning(_("there are points outside specified 2D/3D region--ignored %d points"), OUTRANGE);
  if (npoint > 0)
	G_warning(_("ignoring %d points -- too dense"), npoint);
  npoint = k - npoint - OUTRANGE;
  if (npoint < params->kmin)
  {
    if (npoint != 0)
    {
	G_warning(_("%d points given for interpolation (after thinning) is less than given NPMIN=%d"), 
	    npoint, params->kmin);
	params->kmin = npoint;
    }
    else
    {
      fprintf (stderr, "ERROR2: zero points in the given region!\n");
      return -1;
    }
  }
  if (npoint > params->KMAX2 && params->kmin <= params->kmax)
  {
    fprintf (stderr, "ERROR: segmentation parameters set to invalid values: npmin= %d, segmax= %d \n", params->kmin, params->kmax);
    fprintf (stderr, "for smooth connection of segments, npmin > segmax (see manual) \n");
    return -1;
  }
  if (npoint < params->KMAX2 && params->kmax != params->KMAX2)
	G_warning(_("There are less than %d points for interpolation. No "
	    "segmentation is necessary, to run the program faster set "
	    "segmax=%d (see manual)"), params->KMAX2, params->KMAX2);

  fprintf (stdout, "\n");
  fprintf (stdout, "The number of points from vector map is %d\n", k);
  fprintf (stdout, "The number of points outside of 2D/3D region %d\n", OUTRANGE);
  fprintf (stdout, "The number of points being used is %d\n", npoint);
  fflush(stdout);
  *n_points = npoint;
  return (totsegm);
}

int process_point (
    double x,
    double y,
    double z,
    double sm,
    struct tree_info *info,	/* quadtree info */
    double zmult,			/* multiplier for z-values */
    double *xmin,
    double *xmax,
    double *ymin,
    double *ymax,
    double *zmin,
    double *zmax,
    int *npoint,
    int *OUTRANGE,
    int *total
)

{
  struct triple *point;
  double c1, c2, c3, c4;
  int a;
  static int first_time = 1;
  struct quaddata *data = (struct quaddata *) info->root->data;


  (*total)++;


  z = z * zmult;
  c1 = x - data->x_orig;
  c2 = data->xmax - x;
  c3 = y - data->y_orig;
  c4 = data->ymax - y;

  if (!((c1 >= 0) && (c2 >= 0) && (c3 >= 0) && (c4 >= 0)))
  {
    if (!(*OUTRANGE))
    {
	G_warning(_("some points outside of region -- will ignore..."));
    }
    (*OUTRANGE)++;
  }
  else
  {
      if (!(point = quad_point_new (x, y, z, sm)))
      {
	fprintf (stderr, "cannot allocate memory for point\n");
	return -1;
      }
      a = MT_insert (point, info, info->root, 4);
      if (a == 0)
      {
	(*npoint)++;
      }
      if (a < 0)
      {
	fprintf (stderr, "cannot insert %f,%f,%f a = %d\n", x, y, z, a);
	return -1;
      }
      free (point);
      if (first_time)
      {
	first_time = 0;
	*xmin = x;
	*ymin = y;
	*zmin = z;
	*xmax = x;
	*ymax = y;
	*zmax = z;
      }
      *xmin = amin1 (*xmin, x);
      *ymin = amin1 (*ymin, y);
      *zmin = amin1 (*zmin, z);
      *xmax = amax1 (*xmax, x);
      *ymax = amax1 (*ymax, y);
      *zmax = amax1 (*zmax, z);
  }
  return 1;
}
