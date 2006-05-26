#include "global.h"

int do_add(struct Map_info *Map)
{
    int next_cat;
	    
    next_cat = cat_max_get(1);
    if(strcmp(typ_opt->answer, "point")==0) {
	int i;
	struct line_pnts *Points;

	if(pnt_opt->answers[2] != NULL) {
	    G_warning(_("Adding many points with same attributes"));
	}

	for(i=0; pnt_opt->answers[i] != NULL; i++) {
	    double x,y;
	    
	    next_cat++;
	    x = atof(pnt_opt->answers[i]);
	    y = atof(pnt_opt->answers[++i]);
	    G_debug (1, "Adding a point to map [%s], x=%.10f y=%.10f with cat %d",
		     map_opt->answer, x, y, next_cat);
	    
	    Points = Vect_new_line_struct ();
	    Vect_append_point ( Points, x, y, 0 );

	    if(!add_line ( Map, GV_POINT, Points, 1, next_cat )) {
		return 0;
	    }
	    cat_max_set(1, next_cat);
	}
    }
    else {
	G_warning("Sorry this is not yet implemented");
    }
    return 1;
}

int add_line(struct Map_info *Map, int type, struct line_pnts *Points, 
	     int field, int cat)
{
    int ret;
    struct line_cats *Cats;
    Cats = Vect_new_cats_struct();
    if(Cats== NULL)
	return 0;
    if(!d_flg->answer) {
	if(!attr_new(Map, field, cat, val_opt->answer))
	    return 0;

	if(Vect_cat_set ( Cats, field, cat ) <=0) {
	    attr_del(Map, field, cat);
	    return 0;
	}
    }
    
    if(!Vect_write_line(Map, type, Points, Cats )) {
	attr_del(Map, field, cat);
	return 0;
    }

    return 1;
}
