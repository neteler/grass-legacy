/* Print z coordinate value for each node */

#include "gis.h"
#include "Vect.h"
#include "display.h"
#include "raster.h"
#include "plot.h"

extern int width;

int zcoor ( struct Map_info *Map, int type, LATTR *lattr ) {
    int num, el;
    double xl, yl, zl;
    struct line_pnts *Points;
    struct line_cats *Cats;
    int X, Y, T, B, L, R, Xoffset, Yoffset, xarr[5], yarr[5];
    char text[50];
    
    G_debug (1, "display zcoor:");
    Points = Vect_new_line_struct ();
    Cats = Vect_new_cats_struct ();

    R_color(lattr->color) ;
    R_text_size(lattr->size, lattr->size) ;
    R_font(lattr->font) ;
	
    Vect_rewind ( Map );


    num = Vect_get_num_nodes(Map);
    G_debug (1, "n_nodes = %d", num);
    
    /* Nodes */
    for ( el = 1; el <= num; el++ ) {
	if ( !Vect_node_alive (Map, el ) ) continue; 
	Vect_get_node_coor (Map, el, &xl, &yl, &zl );
        G_debug (3, "node = %d", el);
	
	X = (int)(D_u_to_d_col(xl)) ;
	Y = (int)(D_u_to_d_row(yl)) ;
       
	X = X + 0.5 * lattr->size;
	Y = Y + 1.5 * lattr->size;
    
	R_move_abs(X, Y) ;
	sprintf (text, "%.2f", zl);
	R_get_text_box(text, &T, &B, &L, &R);
	    
	/* Expand border 1/2 of text size */
	T = T - lattr->size / 2 ;
	B = B + lattr->size / 2 ;
	L = L - lattr->size / 2 ;
	R = R + lattr->size / 2 ;
	   
	Xoffset = 0;
	Yoffset = 0;

	if (lattr->xref == LCENTER) Xoffset = -(R - L) / 2 ;
	if (lattr->xref == LRIGHT ) Xoffset = -(R - L) ;
	if (lattr->yref == LCENTER) Yoffset = -(B - T) / 2 ;
	if (lattr->yref == LBOTTOM) Yoffset = -(B - T) ; 
	  
	if ( lattr->bgcolor || lattr->bcolor )
	  {
	    xarr[0] = xarr[1] = xarr[4] = L + Xoffset; 
	    xarr[2] = xarr[3] = R + Xoffset; 
	    yarr[0] = yarr[3] = yarr[4] = B + Yoffset; 
	    yarr[1] = yarr[2] = T + Yoffset; 
	    
	    if( lattr->bgcolor)
	      {
		R_color( lattr->bgcolor) ;
		R_polygon_abs(xarr, yarr, 5) ;
	      }
	    
	    if( lattr->bcolor)
	      {
		 R_color( lattr->bcolor) ;
		 R_polyline_abs(xarr, yarr, 5) ;
	      }
	    R_color(lattr->color) ;
	  }
	    
	R_move_abs(X + Xoffset, Y + Yoffset) ;
	R_line_width(0);
	R_text(text);
	R_line_width(width);
    }

    Vect_destroy_line_struct (Points);
    Vect_destroy_cats_struct (Cats);
    
    return 0;
}
