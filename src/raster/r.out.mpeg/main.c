
/* Written by Bill Brown, USACERL (brown@zorro.cecer.army.mil)
 * May, 1994
 *
 * This code is in the public domain. Specifically, we give to the public
 * domain all rights for future licensing of the source code, all resale
 * rights, and all publishing rights.
 * 
 * We ask, but do not require, that the following message be included in
 * all derived works:
 *     "Portions developed at the US Army Construction Engineering 
 *     Research Laboratories, Champaign, Illinois."
 * 
 * USACERL GIVES NO WARRANTY, EXPRESSED OR IMPLIED,
 * FOR THE SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT
 * LIMITATION, WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A
 * PARTICULAR PURPOSE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "gis.h"

#include "rom_proto.h"

#define MAXIMAGES 100
#define DEF_MAX 500
#define DEF_MIN 200
#define MAXVIEWS    4 
#define BORDER_W    2


int     nrows, ncols, numviews, quality, quiet=0;
char 	*vfiles[MAXVIEWS][MAXIMAGES];
char 	outfile[BUFSIZ];

float 	vscale, scale;  /* resampling scale factors */
int 	irows, icols, vrows, vcols;
int 	frames;

int main ( int  argc, char **argv)
{
    int	     	i, j, d;
    int       	*sdimp, longdim, r_out;
    char 	dummy, *p;

    G_gisinit (argv[0]);
    parse_command(argc, argv, vfiles, &numviews, &frames, &quality, &r_out);

    vrows = G_window_rows();
    vcols = G_window_cols();
    nrows = vrows;
    ncols = vcols;

    /* short dimension */
    sdimp = nrows>ncols? &ncols: &nrows;

    /* these proportions should work fine for 1 or 4 views, but for
    2 views, want to double the narrow dim & for 3 views triple it */
    if(numviews == 2)
	*sdimp *= 2;
    else if(numviews == 3)
	*sdimp *= 3;

    longdim = nrows>ncols? nrows: ncols;

    scale = 1.0;

    { /* find animation image size */
    int max, min;
    char *p;

    max = DEF_MAX;
    min = DEF_MIN;

    if((p = getenv ("GMPEG_SIZE")))
	max = min = atoi(p);

    if(longdim > max)      /* scale down */
	scale = (float)max/longdim;
    else if(longdim < min) /* scale up */
	scale = (float)min/longdim;
    }
    /* TODO: align image size to 16 pixel width & height */
    
    vscale = scale;
    if(numviews == 4)
	vscale = scale / 2.;

    nrows *= scale;
    ncols *= scale;
    /* now nrows & ncols are the size of the combined - views image */
    vrows *= vscale;
    vcols *= vscale;
    /* now vrows & vcols are the size for each sub-image */

    /* add to nrows & ncols for borders */
    /* irows, icols used for vert/horizontal determination in loop below */
    irows = nrows;
    icols = ncols;
    nrows += (1 + (nrows/vrows)) * BORDER_W;
    ncols += (1 + (ncols/vcols)) * BORDER_W;

    if(numviews == 1 && r_out)
	use_r_out();
    else
	load_files();

    return(0);

}

int load_files()
{
void    *voidc;
int     rtype;
register int i, rowoff, row, col, vxoff, vyoff, offset;
int 	cnt, ret, fd, size, tsiz, coff;
int	vnum;
int	y_rows, y_cols;
char    *pr, *pg, *pb;
unsigned char *tr, *tg, *tb, *tset;
int     R, G, B;
char	*mpfilename, *mapset, name[BUFSIZ];
char 	cmd[1000], *yfiles[MAXIMAGES];
struct Colors colors;

    size = nrows * ncols;

    if(NULL == (pr = malloc(size))){
	    fprintf(stderr,"Can't malloc memory for imagebuffer\n");
	    exit(1);
    }
    if(NULL == (pg = malloc(size))){
	    fprintf(stderr,"Can't malloc memory for imagebuffer\n");
	    exit(1);
    }
    if(NULL == (pb = malloc(size))){
	    fprintf(stderr,"Can't malloc memory for imagebuffer\n");
	    exit(1);
    }
    tsiz = G_window_cols();

    if(NULL == (tr = (unsigned char *) malloc(tsiz))){
        fprintf(stderr,"Unable to malloc.\n");
        exit (0);
    }
    if(NULL == (tg = (unsigned char *) malloc(tsiz))){
        fprintf(stderr,"Unable to malloc.\n");
        exit (0);
    }
    if(NULL == (tb = (unsigned char *) malloc(tsiz))){
        fprintf(stderr,"Unable to malloc.\n");
        exit (0);
    }
    if(NULL == (tset = (unsigned char *) malloc(tsiz))){
        fprintf(stderr,"Unable to malloc.\n");
        exit (0);
    }

    for (cnt = 0; cnt < frames; cnt++)
    {
	
        if (cnt > MAXIMAGES)
	{
	    cnt--;
	    break;
	}

	for(i=0; i< size; i++)
	    pr[i] = pg[i] = pb[i] = 0;

	for(vnum = 0; vnum < numviews; vnum++){
	    if(icols == vcols){
		vxoff =  BORDER_W;
		vyoff = (irows == vrows)? BORDER_W : 
			    BORDER_W + vnum*(BORDER_W+vrows);
	    }
	    else if (irows == vrows){
		vxoff = (icols == vcols)? BORDER_W : 
			    BORDER_W + vnum*(BORDER_W+vcols);
		vyoff =  BORDER_W;
	    }
	    else{ /* 4 views */
		/* assumes we want :
		    view1	view2

		    view3	view4   
		*/
		vxoff = vnum%2? BORDER_W: vcols+2*BORDER_W;
		vyoff = vnum>1? vrows+2*BORDER_W: BORDER_W; 
	    }

	    strcpy(name,vfiles[vnum][cnt]);
	    if(!quiet)
		fprintf (stderr, "\rReading file '%s'\n", name);
	    mapset = G_find_cell2 (name, "");
	    if (mapset == NULL){
		char msg[100];	
		sprintf (msg, "%s: <%s> cellfile not found\n", 
					    G_program_name(), name);
		G_fatal_error (msg);
		exit(1);
	    }
	    fd = G_open_cell_old (name, mapset);
	    if (fd < 0)
		exit(1);

	    ret = G_read_colors(name, mapset, &colors);
	    if (ret < 0)
		exit(1);

            rtype = G_raster_map_type(name, mapset);
            if (rtype == CELL_TYPE)
                voidc = G_allocate_c_raster_buf();
            else if (rtype == FCELL_TYPE)
                voidc = G_allocate_f_raster_buf();
            else if (rtype == DCELL_TYPE)
                voidc = G_allocate_d_raster_buf();
            else
                exit(1);

	    for (row = 0; row < vrows; row++){
		if (G_get_raster_row (fd, voidc, 
                                      (int)(row/vscale), rtype) < 0)
		    exit(1);
		rowoff = (vyoff+row)*ncols;
                G_lookup_raster_colors(voidc, tr, tg, tb,
				       tset, tsiz, &colors, rtype);
                for (col = 0; col < vcols; col++){
                    coff= (int)(col/vscale);
		    offset = rowoff + col + vxoff;
                    if(!tset[coff])
                        pr[offset] = pg[offset] = pb[offset] = (char) 255;
		    else{
			pr[offset] = (char) tr[coff];	
			pg[offset] = (char) tg[coff];	
			pb[offset] = (char) tb[coff];	
		    }	
                }
	    }

	    G_close_cell(fd);
	}

	yfiles[cnt] = G_tempfile();

#ifdef USE_PPM
	write_ppm(pr, pg, pb, nrows, ncols, &y_rows, &y_cols, yfiles[cnt]);
#else
	write_ycc(pr, pg, pb, nrows, ncols, &y_rows, &y_cols, yfiles[cnt]);
#endif


    }

    mpfilename = G_tempfile();
    write_params(mpfilename, yfiles, outfile, cnt, quality, y_rows, y_cols, 0);

    if(quiet)
	sprintf(cmd, "mpeg_encode %s 2> /dev/null > /dev/null", 
		mpfilename);
    else
	sprintf(cmd, "mpeg_encode %s", mpfilename);

    if(0 != G_system(cmd))
	fprintf(stderr,"mpeg_encode ERROR\n");

    clean_files(mpfilename, yfiles, cnt);

    free(voidc);
    free(tset);
    free(tr);
    free(tg);
    free(tb);
    free(pr);
    free(pg);
    free(pb);

    return(cnt);

}


int use_r_out()
{
char	*mpfilename, cmd[1000];

    mpfilename = G_tempfile();
    write_params(mpfilename, vfiles[0], 
		outfile, frames, quality, 0, 0, 1);

    if(quiet)
	sprintf(cmd, "mpeg_encode %s 2> /dev/null > /dev/null", 
		mpfilename);
    else
	sprintf(cmd, "mpeg_encode %s", mpfilename);

    if(0 != G_system(cmd))
	fprintf(stderr,"mpeg_encode ERROR\n");

    clean_files(mpfilename, NULL, 0);

    return(1);

}

/* ###################################################### */
char **gee_wildfiles(wildarg, element, num)
char *wildarg, *element;
int *num;
{
int n, cnt=0;
char path[1000], *mapset, cmd[1000], buf[512];
char *p, *tfile;
static char *newfiles[MAXIMAGES];
FILE *tf;
   
    *num = 0;
    tfile = G_tempfile();
    /* build list of filenames */
    for(n=0; (mapset = G__mapset_name (n)); n++){
	if (strcmp (mapset,".") == 0)
	    mapset = G_mapset();
	G__file_name (path, element, "", mapset);
	if(access(path, 0) == 0) {
	    sprintf(cmd, "cd %s; \\ls %s >> %s 2> /dev/null", 
		path, wildarg, tfile);
	    G_system(cmd);
	}
    }
    if(NULL == (tf = fopen(tfile, "r"))){
	fprintf(stderr, "Error reading wildcard\n");
    }
    else{
	while(NULL != fgets(buf,512,tf)){
	    /* replace newline with null */
	    if( (p = strchr(buf, '\n')) )
		*p = '\0';
	    /* replace first space with null */
	    else if( (p = strchr(buf, ' ')) )
		*p = '\0';
	    if(strlen(buf) > 1){
		newfiles[cnt++] = G_store (buf);
	    }
	}
	fclose(tf);
    }
    *num = cnt;
    sprintf(cmd, "\\rm %s", tfile );
    G_system(cmd);
    free (tfile);
    return(newfiles);

}


/********************************************************************/
void parse_command(argc, argv, vfiles, numviews, numframes, quality, convert)
int argc;
char *argv[];
char *vfiles[MAXVIEWS][MAXIMAGES];
int *numframes, *numviews, *quality, *convert;
{
	struct GModule *module;
    struct Option *viewopts[MAXVIEWS], *out, *qual; 
    struct Flag *qt, *conv;
    char buf[BUFSIZ], **wildfiles;
    int i,j,k, numi, wildnum;

	module = G_define_module();
	module->description =
		"Raster File Series to MPEG Conversion Program.";

    *numviews = *numframes = 0;
    for(i=0; i<MAXVIEWS; i++){
	viewopts[i] = G_define_option();
	sprintf(buf,"view%d", i+1);
	viewopts[i]->key		= G_store(buf);
	viewopts[i]->type 		= TYPE_STRING;
	viewopts[i]->required 		= (i? NO: YES);
	viewopts[i]->multiple 		= YES;
	viewopts[i]->gisprompt 		= "old,cell,Raster";;
	sprintf(buf,"Raster file(s) for View%d", i+1);
	viewopts[i]->description 	= G_store(buf);
    }

    out = G_define_option();
    out->key		= "output";
    out->type 		= TYPE_STRING;
    out->required 	= NO;
    out->multiple 	= NO;
    out->answer 	= "gmovie.mpg";
    out->description 	= "Name for output file";

    qual = G_define_option();
    qual->key		= "qual";
    qual->type 		= TYPE_INTEGER;
    qual->required 	= NO;
    qual->multiple 	= NO;
    qual->answer 	= "3";
    qual->options       = "1-5" ;
    qual->description 	= 
	    "Quality factor (1 = highest quality, lowest compression)";

    qt = G_define_flag ();
    qt->key = 'q';
    qt->description = "Quiet - suppress progress report";
   
    conv = G_define_flag ();
    conv->key = 'c';
    conv->description = "Convert on the fly, use less disk space\n\t(requires r.out.ppm with stdout option)";
   
    if (G_parser (argc, argv))
	    exit (-1);

    *convert = 0; 
    if(qt->answer) quiet = 1;
    if(conv->answer) *convert = 1;

    *quality = 3; 
    if(qual->answer != NULL)
	sscanf(qual->answer,"%d", quality);
    if(*quality > 5 || *quality < 1)
	*quality = 3; 

    if(out->answer)
	strcpy(outfile, out->answer);
    else
	strcpy(outfile, "gmovie.mpg");

    for(i=0; i<MAXVIEWS; i++){
	if(viewopts[i]->answers){
	    (*numviews)++;
	    for (j = 0, numi=0 ; viewopts[i]->answers[j] ; j++){
		if((NULL != strchr(viewopts[i]->answers[j], '*')) || 
		   (NULL != strchr(viewopts[i]->answers[j], '?')) || 
		   (NULL != strchr(viewopts[i]->answers[j], '['))){
		    wildfiles = gee_wildfiles(viewopts[i]->answers[j],
				"cell", &wildnum);
		    for(k=0; k<wildnum; k++){
			vfiles[i][numi++] = wildfiles[k];
		    }
		}
		else
		    vfiles[i][numi++] = G_store(viewopts[i]->answers[j]);
	    }
	    /* keep track of smallest number of frames */
	    *numframes = *numframes? *numframes > numi? numi: *numframes: numi;
	}
    }

}

/*********************************************************************/
/*********************************************************************/
