#include <string.h>
#include "raster.h"
#include "display.h"
#include "Vectpoints.h"
#include "Vect.h"
#include "globals.h"

#define VFILES 12

static int  view2on, view2zoomon, numfiles;
static char vect_file[VFILES][100];
static char vect_mapset[VFILES][100];
static char vect_color[VFILES][10];
static int get_clr_name(char *,int);
static int choose_vectfile (char *,char *);
static int drawvect(int, View *, double *,double *, int);
 
int 
plotvect (void)
{
   return( drawvect(0,(View *)NULL,(double *)NULL,(double *)NULL,0));
}

int zoomvect(View *zoom_view)
{
    return( drawvect(1, zoom_view,(double *)NULL,(double *)NULL,0));
}

int 
re_fresh (void)
{
    return( drawvect(-1,(View *)NULL,(double *)NULL,(double *)NULL,0));
}

int warpvect(
double E[],
double N[],
int trans_order)
{
    return( drawvect(2, VIEW_MAP1, E, N, trans_order));
}

static int drawvect(
int zoomit,    /* -1 = refresh, 0 = new image, 1 = zoom, 2 = warp */
View *zoom_view,
double E[],
double N[],
int trans_order)  /* order of tranformation if warping vectors */
{
    int stat;
    int i;
    int color;
    char name[100], mapset[100];
    struct Cell_head cellhd;
    struct line_pnts *Points;
    char msg[100], win_name[100];
    int t,b,l,r;
    int blank=0;
    View *active_view;
    int left, top, nrows, ncols;
    static int vectclr[VFILES];

                /* if refresh screen or overlay & no displayed files return */ 
    if( (zoomit == -1 || zoomit == 2) && ! numfiles)
	return 0;

    if(numfiles == VFILES)
        {
        G_warning("Can't display another map; reached maximum number of files");

        return 0;
        }

    select_target_env();

    if ( zoomit <= 0)       /* New Map File or Refresh Screen */
    {                       /* Draw New Map File */
        if(!zoomit)
        {
	    if (!choose_vectfile (name, mapset))
	        return 0;

            strcpy(vect_file[numfiles], name);
            strcpy(vect_mapset[numfiles], mapset);
    
            get_vector_color();    /* ask line_color to draw map */

            if(! numfiles)
            {               /* SET VECTOR WINDOW BY WIND */
                if(G_get_window( &cellhd) < 0)
                {
                    if(G_get_cellhd(vect_file[numfiles],vect_mapset[numfiles],                          &cellhd) < 0)
	            {
		        select_current_env();
	                return 0;
                    }        
	        }
                G_copy( &VIEW_MAP2->cell.head, &cellhd, sizeof(cellhd) );
            }
            else
               G_copy( &cellhd, &VIEW_MAP2->cell.head, sizeof(cellhd) );
            numfiles++;
        }
        else                   /* Refresh Screen */ 
        {                    
            G_copy(&cellhd,&VIEW_MAP2->cell.head, sizeof(VIEW_MAP2->cell.head));
            Erase_view (VIEW_MAP2_ZOOM);
            VIEW_MAP2_ZOOM->cell.configured = 0;
            blank = BLACK;
        }

        strcpy(win_name, "vect_map");

        if(!view2on)
        {
            t = VIEW_MAP2->top;
            b = VIEW_MAP2->bottom;
            l = VIEW_MAP2->left;
            r = VIEW_MAP2->right;
            D_new_window( win_name, t, b, l, r);
            view2on = 1;
            blank = BLACK;
        }

        active_view = VIEW_MAP2; 
    }
    else                              /* Zoom Map Files */
    {
        G_copy( &cellhd, &zoom_view->cell.head, sizeof(zoom_view->cell.head));

	if(! (zoom_view == VIEW_MAP1) )
	{
            VIEW_MAP2_ZOOM->cell.configured = 0;
            strcpy(win_name, "zoom_map");
            if(!view2zoomon)
            {
                t = VIEW_MAP2_ZOOM->top;
                b = VIEW_MAP2_ZOOM->bottom;
                l = VIEW_MAP2_ZOOM->left;
                r = VIEW_MAP2_ZOOM->right;
                D_new_window( win_name, t, b, l, r);
                view2zoomon = 1;

	    }
            active_view = VIEW_MAP2_ZOOM;
            blank = BLACK;
	}
	else
	{
	strcpy(win_name, "warp_map"); /* defined in drawcell routine */
	active_view = VIEW_MAP1;
        blank = 0;    /* don't erase viewport */
	}
    }

nrows = active_view->nrows;
ncols = active_view->ncols;
left  = active_view->left;
top   = active_view->top;

    D_set_cur_wind( win_name);
    R_standard_color(YELLOW);
    Outline_box(top, top + nrows-1, left, left + ncols-1);
    Points = Vect_new_line_struct(); 

    if(zoomit != 2)
    {
    	Curses_clear_window (INFO_WINDOW);
	Curses_write_window (INFO_WINDOW, 1, 13, "COORDINATES");
    	Curses_write_window (INFO_WINDOW, 3, 2, "MAIN WINDOW");

    	sprintf(msg, "N = %10.2f   E = %10.2f",                                           VIEW_MAP2->cell.head.north, VIEW_MAP2->cell.head.east);
    	Curses_write_window (INFO_WINDOW, 5, 4, msg);
    	sprintf(msg, "S = %10.2f   W = %10.2f",                                           VIEW_MAP2->cell.head.south, VIEW_MAP2->cell.head.west);
    	Curses_write_window (INFO_WINDOW, 6, 4, msg);

    	Curses_write_window (INFO_WINDOW, 9, 2, "ZOOM WINDOW");
    	sprintf(msg, "N = %10.2f   E = %10.2f",                                           VIEW_MAP2_ZOOM->cell.head.north, VIEW_MAP2_ZOOM->cell.head.east);
    	Curses_write_window (INFO_WINDOW, 11, 4, msg);
    	sprintf(msg, "S = %10.2f   W = %10.2f",                                           VIEW_MAP2_ZOOM->cell.head.south, VIEW_MAP2_ZOOM->cell.head.west);
    	Curses_write_window (INFO_WINDOW, 12, 4, msg);
    }

    if(zoomit)
    {
        dsp_setup(blank, &cellhd);

        for( i = 0; i < numfiles; i++)
        {
            sprintf(msg, "Displaying %s",vect_file[i]);
            Menu_msg(msg);
            R_standard_color( vectclr[i]);
	    if(zoomit != 2)
                stat = plot1(vect_file[i],vect_mapset[i], Points);
	    else
                stat = plot1_warp(vect_file[i],vect_mapset[i], Points, E, N, trans_order);
        }
    }
    else
    {
        if(numfiles == 1)                   /* let first file set window */
        {
            G_copy (&VIEW_MAP2->cell.head, &cellhd, sizeof(cellhd));

            cellhd.rows = VIEW_MAP2->nrows;
            cellhd.cols = VIEW_MAP2->ncols;
            cellhd.ns_res = (cellhd.north - cellhd.south) / cellhd.rows;
            cellhd.ew_res = (cellhd.east  - cellhd.west)  / cellhd.cols;
            if(cellhd.ns_res > cellhd.ew_res)
                cellhd.ew_res = cellhd.ns_res;
            else
                cellhd.ns_res = cellhd.ew_res;

            VIEW_MAP2->cell.ns_res = cellhd.ns_res;
            VIEW_MAP2->cell.ew_res = cellhd.ew_res;

            G_copy (&VIEW_MAP2->cell.head, &cellhd, sizeof(cellhd));

            G_adjust_window_to_box (&cellhd,&VIEW_MAP2->cell.head, VIEW_MAP2->nrows,            VIEW_MAP2->ncols);

            Configure_view (VIEW_MAP2,vect_file[numfiles-1],vect_mapset[numfiles-1],            cellhd.ns_res,cellhd.ew_res);

        Curses_write_window (INFO_WINDOW, 15, 2, "WHERE CURSOR-> Mid Button");
        }

    dsp_setup(blank, &cellhd);

    R_standard_color(YELLOW);
    Outline_box(top, top + nrows-1, left, left + ncols-1);
    sprintf(msg, "Displaying %s",vect_file[numfiles-1]);
    Menu_msg(msg);

    R_standard_color( line_color );
    vectclr[numfiles -1] = line_color;

    get_clr_name( vect_color[numfiles -1], line_color);

    stat = plot1(vect_file[numfiles-1],vect_mapset[numfiles-1], Points);

}

    display_points(1);

R_standard_color (WHITE);
Outline_box (top, top+nrows-1, left, left+ncols-1);

    Menu_msg("");

#ifdef NEEDED
	if(use_plot1(name, mapset))
	{
		stat = plot1(vect_file,vect_mapset, Points);
	}
	else if (stat = plot2(vect_file,vect_mapset, Points))
	{
		fprintf(stderr, "\n*** Will try another method ***\n\n");
		stat = plot1(vect_file,vect_mapset, Points);
	}

#endif

	if(stat == 0)
		D_add_to_list(G_recreate_command()); /* ??? */

	Vect_destroy_line_struct(Points);

	VIEW_MAP2->cell.configured = 1;

	select_current_env();
	if (from_screen < 0)
	{
	    from_flag = 1;
	    from_screen = 0;
	    if (from_keyboard < 0)
	    {
		from_keyboard = 0;
		from_screen = 1;
	    }
	}

    Curses_clear_window (MENU_WINDOW);
    Curses_write_window (MENU_WINDOW, 1, 5, "COLOR  MAP FILE");
    for(i = 0; i < numfiles; i++)
        {
            sprintf(msg, "%7s  %s", vect_color[i], vect_file[i]);
            Curses_write_window (MENU_WINDOW, i + 3, 3, msg);
        } 
    return 0;
}

static int choose_vectfile (char *name,char *mapset)
{
    return ask_gis_files ("vector", vect_list, name, mapset, 1);
}

static int get_clr_name(char *name,int clr )
{
    switch(clr)
	{
	case 5:    /* BLUE */
		strcpy(name, "blue");
		break;

 	case 10:   /* GRAY */
		strcpy(name, "gray");
		break;

	case 4:   /* GREEN */
		strcpy(name, "green");
		break;

	case 1:   /* RED */
		strcpy(name, "red");
		break;

	case 8:   /* WHITE */
		strcpy(name, "white");
		break;

	case 3:   /* YELLOW */
		strcpy(name, "yellow");
		break;
	}

	return 0;
}
