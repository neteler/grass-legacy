/*
 *   d.menu - uses lines in stdin as options to prompt user.
 *
 *
 *   Lines beginning with:
 *    #      are comments and ignored
 *    .B     contain the background color
 *    .C     contain the text color
 *    .D     contain the line divider color
 *    .S     contain the text size (in pixles)
 *    .T     contain the panel's top edge
 *    .L     contain the panel's left edge
 *
 *   Of the remaining lines, the first is the menu name; the rest
 *   are the menu options.
 *
 *   Returns option number chosen.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "raster.h"
#include "gis.h"

int main(int argc, char **argv )
{
	int backcolor ;
	int textcolor ;
	int dividercolor ;
	int size ;
	int left ;
	int top ;
	char buff[128] ;
	char *cmd_ptr ;
	char *tmp ;
	char *options[128] ;
	int i ;
	int len ;
	struct GModule *module;
	struct Option *opt1, *opt2, *opt3, *opt4 ;

	/* Initialize the GIS calls */
	G_gisinit(argv[0]) ;

	module = G_define_module();
	module->description =
		"Creates and displays a menu within the active "
		"frame on the graphics monitor.";

	opt1 = G_define_option() ;
	opt1->key        = "bcolor" ;
	opt1->type       = TYPE_STRING ;
	opt1->answer     = "black" ;
	opt1->required   = NO ;
	opt1->options    = D_color_list();
	opt1->description= "Sets the color of the menu background" ;

	opt2 = G_define_option() ;
	opt2->key        = "tcolor" ;
	opt2->type       = TYPE_STRING ;
	opt2->answer     = "white" ;
	opt2->required   = NO ;
	opt2->options    = D_color_list();
	opt2->description= "Sets the color of the menu text" ;

	opt3 = G_define_option() ;
	opt3->key        = "dcolor" ;
	opt3->type       = TYPE_STRING ;
	opt3->answer     = "white" ;
	opt3->required   = NO ;
	opt3->options    = D_color_list();
	opt3->description= "Sets the color dividing lines of text" ;

	opt4 = G_define_option() ;
	opt4->key        = "size" ;
	opt4->type       = TYPE_INTEGER ;
	opt4->answer     = "3" ;
	opt4->required   = NO ;
	opt4->options    = "1-100" ;
	opt4->description= "Sets the menu text size" ;

	/* Check command line */


	if (G_parser(argc, argv))
		exit(-1);

	if (R_open_driver() != 0)
		G_fatal_error ("No graphics device selected");

	backcolor = D_translate_color(opt1->answer) ;
	if (backcolor == 0)
	{
		fprintf (stdout,"Don't know the color %s", opt1->answer);
		R_close_driver() ;
		exit(-1) ;
	}

	textcolor = D_translate_color(opt2->answer) ;
	if (textcolor == 0)
	{
		fprintf (stdout,"Don't know the color %s", opt2->answer);
		R_close_driver() ;
		exit(-1) ;
	}

	dividercolor = D_translate_color(opt3->answer) ;
	if (dividercolor == 0)
	{
		fprintf (stdout,"Don't know the color %s", opt3->answer);
		R_close_driver() ;
		exit(-1) ;
	}


	sscanf(opt4->answer,"%d",&size);

	/* Read the options */
	i = 0 ;
	while (fgets(buff,128,stdin) != NULL)
	{
		/* un-fgets it	*/
		tmp=buff;
		while(*tmp) {if(*tmp=='\n') *tmp='\0'; tmp++;}
		if (*buff == '#')
			continue ;
		if (*buff == '.')
		{
			for(cmd_ptr=buff+2; *cmd_ptr==' ' && *cmd_ptr != '\0'; cmd_ptr++) ;
			switch (buff[1] & 0x7F)
			{
			case 'F':   /* font */
				R_font(cmd_ptr) ;
				break ;
			case 'B':   /* background color */
				backcolor = D_translate_color(cmd_ptr) ;
				break ;
			case 'C':   /* text color */
				textcolor = D_translate_color(cmd_ptr) ;
				break ;
			case 'S':   /* size */
				sscanf(cmd_ptr, "%d", &size) ;
				break ;
			case 'T':   /* top edge */
				sscanf(cmd_ptr, "%d", &top) ;
				top = 100 - top ;
				break ;
			case 'L':   /* left edge */
				sscanf(cmd_ptr, "%d", &left) ;
				break ;
			default:
				break ;
			}
		}
		else
		{
			len = strlen(buff) ;
			tmp = malloc(len+1) ;
			strcpy(tmp, buff) ;
			options[i++] = tmp ;
		}
	}

	options[i] = NULL ;
	if (i<2)
	{
		fprintf(stderr,"ERROR: Must be a title and at least one option\n") ;
		R_close_driver();
		exit(-1) ;
	}

	i = D_popup(
	    backcolor ,
	    textcolor ,
	    dividercolor ,
	    top,      /* The col of the top left corner */
	left,     /* The row of the top left corner */
	size,     /* The size of the characters in pixles */
	options) ;/* The text */
	R_close_driver();

	/* Provide the result to standard output */
	fprintf (stdout,"%d\n", i) ;

	return 0;
}
