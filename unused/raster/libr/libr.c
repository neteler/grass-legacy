/*
 * $Id$
 *
 ****************************************************************************
 *
 * MODULE:	GRASS type-independent raster library (libr library)
 * FILENAME:	libr.c
 * AUTHOR(S):	Huidae Cho - Korea - hdcho@geni.cemtlo.com
 * PURPOSE:	This library functions help you program a type-independent
 * 		raster modules.
 * DATE CREATED: Apr 30 2001
 * COPYRIGHT:	(C) 2001 by the GRASS Development Team
 *
 *		This program is free software under the GNU General Public
 *		License (>=v2). Read the file COPYING that comes with GRASS
 *		for details.
 *
 *****************************************************************************/
/*
 * char	*
 * r_type_name[] = { "CELL", "FCELL", "DCELL" };
 *
 *		r_type_name[CELL_TYPE]  or r_type_name[0]:	"CELL"
 *		r_type_name[FCELL_TYPE] or r_type_name[1]:	"FCELL"
 *		r_type_name[DCELL_TYPE] or r_type_name[2]:	"DCELL"
 *
 * char	*
 * r_type_spec[] = { "%%d",  "%%f",   "%%lf"  };
 *
 * 		r_type_spec[CELL_TYPE]  or r_type_spec[0]:	"%%d"
 * 		r_type_spec[FCELL_TYPE] or r_type_spec[1]:	"%%f"
 * 		r_type_spec[DCELL_TYPE] or r_type_spec[2]:	"%%lf"
 *
 *		it's not flexible for precision control.
 *		use r_str_value() instead. see main_old.c for more info.
 *
 * double
 * r_get_value(RASTER_ROW buf, int col);
 *
 *		returns double value from any types
 *
 * void
 * r_set_value(RASTER_ROW buf, int col, double val)
 *
 * 		sets buf[col] to val
 *
 * int
 * r_is_null_value(RASTER_ROW buf, int col);
 *
 * 		returns 1		if buf[col] is NULL
 * 			0		if buf[col] is not NULL or unknown type
 *
 * int
 * r_str_value(char *str, int width, int prec, RASTER_ROW buf, int col);
 *
 * 		fill str buffer with given value.
 *		for CELL type, prec is meaningless.
 *		if width <= 0, no space is included in str.
 *
 *		returns str length	if successful
 *			0		if given type is unknown
 *
 * void
 * r_copy_value(RASTER_ROW sbuf, int scol, RASTER_ROW dbuf, int dcol);
 *
 *		copies sbuf[scol] value to dbuf[dcol]
 */


#include <stdio.h>
#include "gis.h"
#include "libr.h"


/* Here is a full example 'r.show' */
#ifdef	R_SHOW
int
main(int argc, char **argv)
{
	struct	Option *opt;
	struct	Cell_head cellhd;
	char	*name, *mapset;
	int	fd, row, rows, col, cols;
	char	str[20];
	RASTER_ROW	buf;

	opt		 = G_define_option();
	opt->key         = "map";
	opt->description = "Map to manipulate";
	opt->type        = TYPE_STRING;
	opt->required    = YES;
	opt->gisprompt   = "old,cell,raster";

	G_gisinit(argv[0]);

	if(G_parser(argc, argv))
	        exit(-1);

	name   = opt->answer;
	mapset = G_mapset();
	if(!G_find_file("cell", name, mapset)){
		fprintf(stderr, "\n** %s - not found **\n", name);
		exit(1);
	}

	G_get_cellhd(name, mapset, &cellhd);

	rows = cellhd.rows;
	cols = cellhd.cols;

	buf.type  = G_raster_map_type(name, mapset);
	buf.row.v = G_allocate_raster_buf(buf.type);

	fprintf(stderr, "%s\n", r_type_name[buf.type]);

	if((fd = G_open_cell_old(name, mapset)) < 0){
		fprintf(stderr, "\n** %s - could not read **\n", name);
		exit(1);
	}

	for(row=0; row<rows; row++){
		G_percent(row, rows, 2);
		if(G_get_raster_row(fd, buf.row.v, row, buf.type) < 0){
			G_close_cell(fd);
			exit(1);
		}

		for(col=0; col<cols; col++){
			if(r_is_null_value(buf, col)){
				printf("NULL ");
			}else{
				r_str_value(str, 15, 5, buf, col);
				printf("%s ", str);
			}
		}
		printf("\n");
	}
	G_close_cell(fd);

	fprintf(stderr, "\n%d rows, %d cols\n", rows, cols);

	{
		RASTER_ROW tmp;
		double	dval;

		fprintf(stderr, "\n*** Test ***\n");

		tmp.type   = buf.type;
		tmp.row.v = G_allocate_raster_buf(buf.type);

		dval = 123.322;
		fprintf(stderr, " buf[10] = (%s) %lf\n",
				r_type_name[buf.type], dval);

		r_set_value(buf, 10, dval);
		r_str_value(str, 10, 5, buf, 10);
		fprintf(stderr, " buf[10] = %s,", str);

		r_str_value(str, 10, 5, tmp, 2);
		fprintf(stderr, " tmp[2] = %s\n", str);

		fprintf(stderr, "\n copy buf[10] to tmp[2]\n");
		r_copy_value(buf, 10, tmp, 2);
		r_str_value(str, 0, 5, tmp, 2);
		fprintf(stderr, " tmp[2] = %s,", str);

		r_set_value(tmp, 2, 10*r_get_value(tmp, 2));
		r_str_value(str, 0, 5, tmp, 2);
		fprintf(stderr, " tmp[2]*10 = %s\n", str);
	}

	exit(0);
}
#endif


double
r_get_value(RASTER_ROW buf, int col)
{
	double	ret;

	switch(buf.type)
	{
		case CELL_TYPE:
			ret = (double) buf.row.c[col];
			break;
		case FCELL_TYPE:
			ret = (double) buf.row.f[col];
			break;
		case DCELL_TYPE:
			ret = (double) buf.row.d[col];
			break;
		default:
			G_warning("Illegal raster type\n");
			return 0.0;
			break;
	}

	return ret;
}


void
r_set_value(RASTER_ROW buf, int col, double val)
{
	switch(buf.type)
	{
		case CELL_TYPE:
			buf.row.c[col] = (CELL) val;
			break;
		case FCELL_TYPE:
			buf.row.f[col] = (FCELL) val;
			break;
		case DCELL_TYPE:
			buf.row.d[col] = (DCELL) val;
			break;
		default:
			G_warning("Illegal raster type\n");
			return;
			break;
	}

	return;
}


int
r_is_null_value(RASTER_ROW buf, int col)
{
	int	ret;

	switch(buf.type)
	{
		case CELL_TYPE:
			ret = G_is_c_null_value(&buf.row.c[col]);
			break;
		case FCELL_TYPE:
			ret = G_is_f_null_value(&buf.row.f[col]);
			break;
		case DCELL_TYPE:
			ret = G_is_d_null_value(&buf.row.d[col]);
			break;
		default:
			G_warning("Illegal raster type\n");
			return 0;
			break;
	}

	return ret;
}


int
r_str_value(char *str, int width, int prec, RASTER_ROW buf, int col)
{
	switch(buf.type)
	{
		case CELL_TYPE:
			sprintf(str, "%*d", width, buf.row.c[col]);
			break;
		case FCELL_TYPE:
			sprintf(str, "%*.*f", width, prec, buf.row.f[col]);
			break;
		case DCELL_TYPE:
			sprintf(str, "%*.*lf", width, prec, buf.row.d[col]);
			break;
		default:
			G_warning("Illegal raster type\n");
			return 0;
			break;
	}

	if(width <= 0){
		int	i, j, l;

		l = strlen(str);
		for(i=0; i<l && str[i]==' '; i++);
		for(j=i; j<=l; j++)
			str[j-i] = str[j];
	}

	return strlen(str);
}


void
r_copy_value(RASTER_ROW sbuf, int scol, RASTER_ROW dbuf, int dcol)
{
	switch(dbuf.type)
	{
		case CELL_TYPE:
			switch(sbuf.type)
			{
				case CELL_TYPE:
					dbuf.row.c[dcol] =
						(CELL) sbuf.row.c[scol];
					break;
				case FCELL_TYPE:
					dbuf.row.c[dcol] =
						(CELL) sbuf.row.f[scol];
					break;
				case DCELL_TYPE:
					dbuf.row.c[dcol] =
						(CELL) sbuf.row.d[scol];
					break;
				default:
					G_warning("Illegal raster type\n");
					return;
					break;
			}
			break;
		case FCELL_TYPE:
			switch(sbuf.type)
			{
				case CELL_TYPE:
					dbuf.row.f[dcol] =
						(FCELL) sbuf.row.c[scol];
					break;
				case FCELL_TYPE:
					dbuf.row.f[dcol] =
						(FCELL) sbuf.row.f[scol];
					break;
				case DCELL_TYPE:
					dbuf.row.f[dcol] =
						(FCELL) sbuf.row.d[scol];
					break;
				default:
					G_warning("Illegal raster type\n");
					return;
					break;
			}
			break;
		case DCELL_TYPE:
			switch(sbuf.type)
			{
				case CELL_TYPE:
					dbuf.row.d[dcol] =
						(DCELL) sbuf.row.c[scol];
					break;
				case FCELL_TYPE:
					dbuf.row.d[dcol] =
						(DCELL) sbuf.row.f[scol];
					break;
				case DCELL_TYPE:
					dbuf.row.d[dcol] =
						(DCELL) sbuf.row.d[scol];
					break;
				default:
					G_warning("Illegal raster type\n");
					return;
					break;
			}
			break;
		default:
			G_warning("Illegal raster type\n");
			return;
			break;
	}

	return;
}


double
r_get_value2(RASTER_MAP_TYPE type, RASTER_MAP_ROW data, int col)
{
	double	ret;

	switch(type)
	{
		case CELL_TYPE:
			ret = (double) data.c[col];
			break;
		case FCELL_TYPE:
			ret = (double) data.f[col];
			break;
		case DCELL_TYPE:
			ret = (double) data.d[col];
			break;
		default:
			G_warning("Illegal raster type\n");
			return 0.0;
			break;
	}

	return ret;
}


void
r_set_value2(RASTER_MAP_TYPE type, RASTER_MAP_ROW data, int col, double val)
{
	switch(type)
	{
		case CELL_TYPE:
			data.c[col] = (CELL) val;
			break;
		case FCELL_TYPE:
			data.f[col] = (FCELL) val;
			break;
		case DCELL_TYPE:
			data.d[col] = (DCELL) val;
			break;
		default:
			G_warning("Illegal raster type\n");
			return;
			break;
	}

	return;
}


int
r_is_null_value2(RASTER_MAP_TYPE type, RASTER_MAP_ROW data, int col)
{
	int	ret;

	switch(type)
	{
		case CELL_TYPE:
			ret = G_is_c_null_value(&data.c[col]);
			break;
		case FCELL_TYPE:
			ret = G_is_f_null_value(&data.f[col]);
			break;
		case DCELL_TYPE:
			ret = G_is_d_null_value(&data.d[col]);
			break;
		default:
			G_warning("Illegal raster type\n");
			return 0;
			break;
	}

	return ret;
}


int
r_str_value2(char *str, int width, int prec,
		RASTER_MAP_TYPE type, RASTER_MAP_ROW data, int col)
{
	switch(type)
	{
		case CELL_TYPE:
			sprintf(str, "%*d", width, data.c[col]);
			break;
		case FCELL_TYPE:
			sprintf(str, "%*.*f", width, prec, data.f[col]);
			break;
		case DCELL_TYPE:
			sprintf(str, "%*.*lf", width, prec, data.d[col]);
			break;
		default:
			G_warning("Illegal raster type\n");
			return 0;
			break;
	}

	if(width <= 0){
		int	i, j, l;

		l = strlen(str);
		for(i=0; i<l && str[i]==' '; i++);
		for(j=i; j<=l; j++)
			str[j-i] = str[j];
	}

	return strlen(str);
}


void
r_copy_value2(RASTER_MAP_TYPE stype, RASTER_MAP_ROW sdata, int scol,
		RASTER_MAP_TYPE dtype, RASTER_MAP_ROW ddata, int dcol)
{
	switch(dtype)
	{
		case CELL_TYPE:
			switch(stype)
			{
				case CELL_TYPE:
					ddata.c[dcol] =
						(CELL) sdata.c[scol];
					break;
				case FCELL_TYPE:
					ddata.c[dcol] =
						(CELL) sdata.f[scol];
					break;
				case DCELL_TYPE:
					ddata.c[dcol] =
						(CELL) sdata.d[scol];
					break;
				default:
					G_warning("Illegal raster type\n");
					return;
					break;
			}
			break;
		case FCELL_TYPE:
			switch(stype)
			{
				case CELL_TYPE:
					ddata.f[dcol] =
						(FCELL) sdata.c[scol];
					break;
				case FCELL_TYPE:
					ddata.f[dcol] =
						(FCELL) sdata.f[scol];
					break;
				case DCELL_TYPE:
					ddata.f[dcol] =
						(FCELL) sdata.d[scol];
					break;
				default:
					G_warning("Illegal raster type\n");
					return;
					break;
			}
			break;
		case DCELL_TYPE:
			switch(stype)
			{
				case CELL_TYPE:
					ddata.d[dcol] =
						(DCELL) sdata.c[scol];
					break;
				case FCELL_TYPE:
					ddata.d[dcol] =
						(DCELL) sdata.f[scol];
					break;
				case DCELL_TYPE:
					ddata.d[dcol] =
						(DCELL) sdata.d[scol];
					break;
				default:
					G_warning("Illegal raster type\n");
					return;
					break;
			}
			break;
		default:
			G_warning("Illegal raster type\n");
			return;
			break;
	}

	return;
}

