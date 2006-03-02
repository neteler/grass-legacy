
/****************************************************************************
*
* MODULE:       r3.out.vtk  
*   	    	
* AUTHOR(S):    Original author 
*               Soeren Gebbert soerengebbert at gmx de
* 		27 Feb 2006 Berlin
* PURPOSE:      Converts 3D raster maps (G3D) into the VTK-Ascii format  
*
* COPYRIGHT:    (C) 2005 by the GRASS Development Team
*
*               This program is free software under the GNU General Public
*   	    	License (>=v2). Read the file COPYING that comes with GRASS
*   	    	for details.
*
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grass/gis.h>
#include <grass/G3d.h>
#include <grass/glocale.h>


/** Parameters and global variables ******************************************/
typedef struct
{
    struct Option *input, *output, *null_val, *elevscale, *top, *bottom,
	*decimals, *rgbmaps;
    struct Flag *mask, *point, *origin, *structgrid;
    /*struct Flag *xml; *//*maybe xml support in the future */
} paramType;

void *map = NULL;		/*The 3D Rastermap */
int top = -1;			/*file descriptor for top map */
int topMapType;
int bottom = -1;		/*file descriptor for bottom map */
int bottomMapType;
paramType param;		/*Parameters */


/** prototypes ***************************************************************/
void fatalError(char *errorMsg);	/*Simple Error message */
void setParams();		/*Fill the paramType structure */
void OpenWriteRGBMaps(G3D_Region region, FILE * fp, int dp);	/*Open the rgb voxel maps and write the data to the output */
int OpenInputMap(char *name, char *mapset);	/*opens the outputmap */
void CloseInputMap(int fd);	/*close the map */
void CheckInputMaps();		/*Check if all maps are available */
void writeVTKStructuredPointHeader(FILE * fp, char *vtkFile, G3D_Region region, int dp);	/*write the vtk-header */
void writeVTKStructuredGridHeader(FILE * fp, char *vtkFile, G3D_Region region);	/*write the vtk-header */
void writeVTKPoints(FILE * fp, G3D_Region region, int dp);	/*Write the outputdata */
void writeVTKData(FILE * fp, G3D_Region region, char *varname, int dp);	/*Write the outputdata */
void writeVTKRGBVoxelData(void *map_r, void *map_g, void *map_b, FILE * fp, const char *string, G3D_Region region, int dp);	/*Write the rgb voxel data to the output */


/* ************************************************************************* */
/* Error handling ********************************************************** */
/* ************************************************************************* */
void fatalError(char *errorMsg)
{
    /* Close files and exit */
    if (map != NULL) {
	/* should unopen map here! */
	if (!G3d_closeCell(map))
	    fatalError(_("Error closing g3d file"));

    }

    if (top != -1)
	CloseInputMap(top);

    if (bottom != -1)
	CloseInputMap(bottom);

    G3d_fatalError(errorMsg);
}


/* ************************************************************************* */
/* Setg up the arguments we are expecting ********************************** */
/* ************************************************************************* */
void setParams()
{
    param.input = G_define_option();
    param.input->key = "input";
    param.input->type = TYPE_STRING;
    param.input->required = NO;
    param.input->gisprompt = "old,grid3,3d-raster";
    param.input->multiple = YES;
    param.input->description =
	_("G3D map(s) to be converted to VTK-ASCII data format");

    param.top = G_define_option();
    param.top->key = "top";
    param.top->type = TYPE_STRING;
    param.top->required = NO;
    param.top->gisprompt = "old,cell,raster";
    param.top->multiple = NO;
    param.top->description = _("2D raster map representing the top surface");

    param.bottom = G_define_option();
    param.bottom->key = "bottom";
    param.bottom->type = TYPE_STRING;
    param.bottom->required = NO;
    param.bottom->gisprompt = "old,cell,raster";
    param.bottom->multiple = NO;
    param.bottom->description =
	_("2D raster map representing the bottom surface");

    param.output = G_define_option();
    param.output->key = "output";
    param.output->type = TYPE_STRING;
    param.output->gisprompt = "file,file,file";
    param.output->required = NO;
    param.output->description = _("Name for VTK-ASCII output file");

    param.null_val = G_define_option();
    param.null_val->key = "null";
    param.null_val->type = TYPE_DOUBLE;
    param.null_val->required = NO;
    param.null_val->description =
	_("Float value to represent no data cell/points");
    param.null_val->answer = "-99999.99";

    param.rgbmaps = G_define_option();
    param.rgbmaps->key = "rgbmaps";
    param.rgbmaps->type = TYPE_STRING;
    param.rgbmaps->required = NO;
    param.rgbmaps->gisprompt = "old,grid3,3d-raster";
    param.rgbmaps->multiple = YES;
    param.rgbmaps->description =
	_
	("Three g3d raster maps (r,g,b) which are used to create rgb values [redmap,greenmap,bluemap]");

    param.elevscale = G_define_option();
    param.elevscale->key = "elevscale";
    param.elevscale->type = TYPE_DOUBLE;
    param.elevscale->required = NO;
    param.elevscale->description = _("Scale factor for elevation");
    param.elevscale->answer = "1.0";

    param.decimals = G_define_option();
    param.decimals->key = "dp";
    param.decimals->type = TYPE_INTEGER;
    param.decimals->required = NO;
    param.decimals->multiple = NO;
    param.decimals->answer = "12";
    param.decimals->options = "0-20";
    param.decimals->description =
	_("Number of significant digits (floating point only)");

    param.mask = G_define_flag();
    param.mask->key = 'm';
    param.mask->description = _("Use g3d mask (if exists) with input maps");

    param.point = G_define_flag();
    param.point->key = 'p';
    param.point->description =
	_("Create VTK pointdata instead of VTK celldata (celldata is default)");

    param.origin = G_define_flag();
    param.origin->key = 'o';
    param.origin->description = _("Scale factor effects the origin");

    param.structgrid = G_define_flag();
    param.structgrid->key = 's';
    param.structgrid->description =
	_
	("Create 3d elevation output with a top and a bottom surface, both raster maps are required.");

    /* Maybe needed in the future
     * param.xml = G_define_flag ();
     * param.xml->key = 'x';
     * param.xml->description = "Write XML-VTK-format";
     */

    return;
}


/* ************************************************************************* */
/* Open the raster input map *********************************************** */
/* ************************************************************************* */
int OpenInputMap(char *name, char *mapset)
{
    int fd;

    G_debug(3, "Open Raster file %s in Mapset %s", name, mapset);


    /* open raster file */
    fd = G_open_cell_old(name, mapset);

    if (fd < 0)
	G_fatal_error(_("Could not open map %s"), name);


    return fd;
}


/* ************************************************************************* */
/* Close the raster input map ********************************************** */
/* ************************************************************************* */
void CloseInputMap(int fd)
{
    if (G_close_cell(fd) < 0)
	G_fatal_error(_("unable to close input map"));

    return;
}


/* ************************************************************************* */
/* Check the input maps **************************************************** */
/* ************************************************************************* */
void CheckInputMaps()
{
    int i = 0;
    char *mapset, *name;

    /*Check top and bottom if surface is requested */
    if (param.structgrid->answer) {

	if (!param.top->answer || !param.bottom->answer)
	    G3d_fatalError(_("You have to specify top and bottom map\n"));

	mapset = NULL;
	name = NULL;
	name = param.top->answer;
	mapset = G_find_cell2(name, "");
	if (mapset == NULL) {
	    G3d_fatalError(_("top cell map not found\n"));
	}

	mapset = NULL;
	name = NULL;
	name = param.bottom->answer;
	mapset = G_find_cell2(name, "");
	if (mapset == NULL) {
	    G3d_fatalError(_("bottom cell map not found\n"));
	}
    }

    /*If input maps are provided, check them */
    if (param.input->answers != NULL) {
	for (i = 0; param.input->answers[i] != NULL; i++) {
	    if (NULL == G_find_grid3(param.input->answers[i], ""))
		G3d_fatalError(_("Requested g3d data map not found"));
	}
    }

    /*Check for rgb maps. */
    if (param.rgbmaps->answers != NULL) {
	if (param.rgbmaps->answers[i] != NULL) {
	    for (i = 0; i < 3; i++) {
		if (NULL == G_find_grid3(param.rgbmaps->answers[i], ""))
		    G3d_fatalError(_("Requested g3d RGB map not found"));
	    }
	}
	else {
	    G3d_fatalError(_("Please provide three g3d RGB maps"));
	}
    }

    if (param.input->answers == NULL && param.rgbmaps->answers == NULL) {
	G_warning
	    ("No g3d data or RGB maps are provided! Will only write the geometry.");
    }

    return;
}


/* ************************************************************************* */
/* Prepare the VTK RGB voxel data for writing ****************************** */
/* ************************************************************************* */
void OpenWriteRGBMaps(G3D_Region region, FILE * fp, int dp)
{
    int i, changemask[3] = { 0, 0, 0 };
    void *map_r = NULL;		/*The 3D Rastermap Red */
    void *map_g = NULL;		/*The 3D Rastermap Green */
    void *map_b = NULL;		/*The 3D Rastermap Blue */
    void *maprgb = NULL;

    if (param.rgbmaps->answers != NULL) {

	/*Loop over all input maps! */
	for (i = 0; i < 3; i++) {
	    G_debug(3, _("Open g3d raster file %s"), param.rgbmaps->answers[i]);

	    maprgb = NULL;
	    /*Open the map */
	    maprgb =
		G3d_openCellOld(param.rgbmaps->answers[i],
				G_find_grid3(param.rgbmaps->answers[i], ""),
				G3D_DEFAULT_WINDOW, G3D_TILE_SAME_AS_FILE,
				G3D_USE_CACHE_DEFAULT);
	    if (maprgb == NULL)
		G3d_fatalError(_
			       ("Error opening g3d file. No RGB Data will be written."));

	    /*if requested set the Mask on */
	    if (param.mask->answer) {
		if (G3d_maskFileExists()) {
		    changemask[i] = 0;
		    if (G3d_maskIsOff(maprgb)) {
			G3d_maskOn(maprgb);
			changemask[i] = 1;
		    }
		}
	    }

	    if (i == 0)
		map_r = maprgb;
	    if (i == 1)
		map_g = maprgb;
	    if (i == 2)
		map_b = maprgb;
	}


	G_debug(3, _("Writing VTK VoxelData\n"));
	writeVTKRGBVoxelData(map_r, map_g, map_b, fp, "RGB_Voxel", region, dp);

	for (i = 0; i < 3; i++) {
	    if (i == 0)
		maprgb = map_r;
	    if (i == 1)
		maprgb = map_g;
	    if (i == 2)
		maprgb = map_b;

	    /*We set the Mask off, if it was off before */
	    if (param.mask->answer) {
		if (G3d_maskFileExists())
		    if (G3d_maskIsOn(maprgb) && changemask[i])
			G3d_maskOff(maprgb);
	    }
	    /* Close files */
	    if (!G3d_closeCell(maprgb))
		fatalError(_("Error closing g3d file"));
	}
    }
    return;
}


/* ************************************************************************* */
/* Writes the strcutured points Header ************************************* */
/* ************************************************************************* */
void writeVTKStructuredPointHeader(FILE * fp, char *vtkFile, G3D_Region region,
				   int dp)
{
    double scale;

    scale = atof(param.elevscale->answer);
    G_debug(3,
	    _
	    ("writeVTKStructuredPointHeader: Writing VTKStructuredPoint-Header"));

    /*Simple vtk ASCII header */

    fprintf(fp, "# vtk DataFile Version 3.0\n");
    fprintf(fp, "GRASS 6 Export\n");
    fprintf(fp, "ASCII\n");
    fprintf(fp, "DATASET STRUCTURED_POINTS\n");	/*We are using the structured point dataset. */

    if (param.point->answer)
	fprintf(fp, "DIMENSIONS %i %i %i\n", region.cols, region.rows,
		region.depths);
    else
	fprintf(fp, "DIMENSIONS %i %i %i\n", region.cols + 1, region.rows + 1,
		region.depths + 1);

    fprintf(fp, "SPACING %.*f %.*f %.*f\n", dp, region.ew_res, dp,
	    region.ns_res, dp, (region.tb_res * scale));

    if (param.point->answer) {
	if (param.origin->answer)
	    fprintf(fp, "ORIGIN %.*f %.*f %.*f\n", dp,
		    region.west + region.ew_res / 2, dp,
		    region.south + region.ns_res / 2, dp,
		    region.bottom * scale + (region.tb_res * scale) / 2);
	else
	    fprintf(fp, "ORIGIN %.*f %.*f %.*f\n", dp,
		    region.west + region.ew_res / 2, dp,
		    region.south + region.ns_res / 2, dp,
		    region.bottom + (region.tb_res * scale) / 2);
    }
    else {
	if (param.origin->answer)
	    fprintf(fp, "ORIGIN %.*f %.*f %.*f\n", dp, region.west, dp,
		    region.south, dp, region.bottom * scale);
	else
	    fprintf(fp, "ORIGIN %.*f %.*f %.*f\n", dp, region.west, dp,
		    region.south, dp, region.bottom);
    }

    if (param.point->answer)
	fprintf(fp, "POINT_DATA %i\n", region.cols * region.rows * region.depths);	/*We have pointdata */
    else
	fprintf(fp, "CELL_DATA %i\n", region.cols * region.rows * region.depths);	/*We have celldata */

    return;
}


/* ************************************************************************* */
/* Writes the strcutured grid Header **************************************** */
/* ************************************************************************* */
void writeVTKStructuredGridHeader(FILE * fp, char *vtkFile, G3D_Region region)
{
    G_debug(3,
	    _
	    ("writeVTKStructuredGridHeader: Writing VTKStructuredGrid-Header"));
    fprintf(fp, "# vtk DataFile Version 3.0\n");
    fprintf(fp, "GRASS 6 Export\n");
    fprintf(fp, "ASCII\n");
    fprintf(fp, "DATASET STRUCTURED_GRID\n");	/*We are using the structured grid dataset. */
    fprintf(fp, "DIMENSIONS %i %i %i\n", region.cols, region.rows,
	    region.depths);
    /*Only point data is available */
    fprintf(fp, "POINTS %i float\n", region.cols * region.rows * region.depths);

    return;
}


/* ************************************************************************* */
/* This function writes the point coordinates ****************************** */
/* ************************************************************************* */
void writeVTKPoints(FILE * fp, G3D_Region region, int dp)
{
    int x, y, z, status = 0;
    int rows, cols, depths;
    void *rast_top = NULL;
    void *rast_bottom = NULL;
    void *ptr_top = NULL;
    void *ptr_bottom = NULL;
    double topval = 0, bottomval = 0;
    double zcoor, ycoor, xcoor;
    double scale;

    scale = atof(param.elevscale->answer);

    rows = region.rows;
    cols = region.cols;
    depths = region.depths;

    rast_top = G_allocate_raster_buf(topMapType);
    rast_bottom = G_allocate_raster_buf(bottomMapType);

    G_debug(3, _("writeVTKPoints: Writing point coordinates"));

    for (z = 0; z < depths; z++) {

	for (y = 0; y < rows; y++) {
	    G_percent(status, (rows * depths - 1), 10);
	    status++;

	    if (!G_get_raster_row(top, rast_top, y, topMapType))
		fatalError(_("Could not get top raster row \n"));

	    if (!G_get_raster_row(bottom, rast_bottom, y, bottomMapType))
		fatalError(_("Could not get bottom raster row \n"));

	    for (x = 0, ptr_top = rast_top, ptr_bottom = rast_bottom; x < cols;
		 x++, ptr_top =
		 G_incr_void_ptr(ptr_top, G_raster_size(topMapType)),
		 ptr_bottom =
		 G_incr_void_ptr(ptr_bottom, G_raster_size(bottomMapType))) {

		 /*TOP*/ if (topMapType == CELL_TYPE) {
		    if (G_is_null_value(ptr_top, topMapType)) {
			topval = 0;
		    }
		    else {
			topval = *(CELL *) ptr_top;
		    }
		}
		if (topMapType == FCELL_TYPE) {
		    if (G_is_null_value(ptr_top, topMapType)) {
			topval = 0;
		    }
		    else {
			topval = *(FCELL *) ptr_top;
		    }
		}
		if (topMapType == DCELL_TYPE) {
		    if (G_is_null_value(ptr_top, topMapType)) {
			topval = 0;
		    }
		    else {
			topval = *(DCELL *) ptr_top;
		    }
		}
		 /*BOTTOM*/ if (bottomMapType == CELL_TYPE) {
		    if (G_is_null_value(ptr_bottom, bottomMapType)) {
			bottomval = 0;
		    }
		    else {
			bottomval = *(CELL *) ptr_bottom;
		    }
		}
		if (bottomMapType == FCELL_TYPE) {
		    if (G_is_null_value(ptr_bottom, bottomMapType)) {
			bottomval = 0;
		    }
		    else {
			bottomval = *(FCELL *) ptr_bottom;
		    }
		}
		if (bottomMapType == DCELL_TYPE) {
		    if (G_is_null_value(ptr_bottom, bottomMapType)) {
			bottomval = 0;
		    }
		    else {
			bottomval = *(DCELL *) ptr_bottom;
		    }
		}
		/*Calculate the coordinates */
		xcoor = region.west + (region.ew_res / 2 + region.ew_res * (x));
		ycoor =
		    region.north - (region.ns_res / 2 + region.ns_res * (y));
		zcoor =
		    (bottomval +
		     z * (topval - bottomval) / (depths - 1)) * scale;

		fprintf(fp, "%.*f ", dp, xcoor);
		fprintf(fp, "%.*f ", dp, ycoor);
		fprintf(fp, "%.*f\n", dp, zcoor);
	    }
	}
    }

    fprintf(fp, "POINT_DATA %i\n", region.cols * region.rows * region.depths);	/*We have pointdata */

    return;
}


/* ************************************************************************* */
/* Wwrite the VTK Cell or point data *************************************** */
/* ************************************************************************* */
void writeVTKData(FILE * fp, G3D_Region region, char *varname, int dp)
{
    double dvalue;
    float fvalue;
    int x, y, z, status;
    int rows, cols, depths, typeIntern;

    rows = region.rows;
    cols = region.cols;
    depths = region.depths;


    G_debug(3,
	    _
	    ("writeVTKData: Writing Celldata %s with rows %i cols %i depths %i to vtk-ascii file"),
	    varname, rows, cols, depths);

    fprintf(fp, "SCALARS %s float 1\n", varname);
    fprintf(fp, "LOOKUP_TABLE default\n");

    typeIntern = G3d_tileTypeMap(map);

    status = 0;

    for (z = depths - 1; z >= 0; z--) {	/*From the bottom to the top */
	if (param.structgrid->answer) {
	    for (y = 0; y < rows; y++) {
		G_percent(status, (rows * depths - 1), 10);
		status++;

		for (x = 0; x < cols; x++) {
		    if (typeIntern == G3D_FLOAT) {
			G3d_getValue(map, x, y, z, &fvalue, typeIntern);
			if (G3d_isNullValueNum(&fvalue, G3D_FLOAT))
			    fprintf(fp, "%s ", param.null_val->answer);
			else
			    fprintf(fp, "%.*f ", dp, fvalue);
		    }
		    else {
			G3d_getValue(map, x, y, z, &dvalue, typeIntern);
			if (G3d_isNullValueNum(&dvalue, G3D_DOUBLE))
			    fprintf(fp, "%s ", param.null_val->answer);
			else
			    fprintf(fp, "%.*f ", dp, dvalue);
		    }
		}
		fprintf(fp, "\n");
	    }
	}
	else {
	    for (y = rows - 1; y >= 0; y--) {	/*From south to the north */
		G_percent(status, (rows * depths - 1), 10);
		status++;

		for (x = 0; x < cols; x++) {
		    if (typeIntern == G3D_FLOAT) {
			G3d_getValue(map, x, y, z, &fvalue, typeIntern);
			if (G3d_isNullValueNum(&fvalue, G3D_FLOAT))
			    fprintf(fp, "%s ", param.null_val->answer);
			else
			    fprintf(fp, "%.*f ", dp, fvalue);
		    }
		    else {
			G3d_getValue(map, x, y, z, &dvalue, typeIntern);
			if (G3d_isNullValueNum(&dvalue, G3D_DOUBLE))
			    fprintf(fp, "%s ", param.null_val->answer);
			else
			    fprintf(fp, "%.*f ", dp, dvalue);
		    }
		}
		fprintf(fp, "\n");
	    }
	}
    }
}


/* ************************************************************************* */
/* Write the VTK RGB Voxel Data ******************************************** */
/* ************************************************************************* */
void writeVTKRGBVoxelData(void *map_r, void *map_g, void *map_b,
			  FILE * fp, const char *varname,
			  G3D_Region region, int dp)
{
    double dvalue = 0;
    float fvalue = 0;
    int x, y, z, status, k;
    int rows, cols, depths;
    int typeIntern[3];
    void *maprgb = NULL;

    rows = region.rows;
    cols = region.cols;
    depths = region.depths;

    typeIntern[0] = G3d_tileTypeMap(map_r);
    typeIntern[1] = G3d_tileTypeMap(map_g);
    typeIntern[2] = G3d_tileTypeMap(map_b);

    status = 0;

    /********************** WRITE RGB VOXEL DATA; CELL OR POINT ****************/
    fprintf(fp, "COLOR_SCALARS %s 3\n", varname);

    for (z = depths - 1; z >= 0; z--) {	/*From the bottom to the top */
	if (param.structgrid->answer) {
	    for (y = 0; y < rows; y++) {
		G_percent(status, (rows * depths - 1), 10);
		status++;

		for (x = 0; x < cols; x++) {
		    for (k = 0; k < 3; k++) {

			if (k == 0)
			    maprgb = map_r;
			if (k == 1)
			    maprgb = map_g;
			if (k == 2)
			    maprgb = map_b;

			if (typeIntern[k] == G3D_FLOAT) {
			    G3d_getValue(maprgb, x, y, z, &fvalue,
					 typeIntern[k]);
			    if (G3d_isNullValueNum(&fvalue, G3D_FLOAT))
				fprintf(fp, "0 ");
			    else {
				/*Test of value range, the data should be 1 byte gray values */
				if (fvalue > 255 || fvalue < 0) {
				    G_warning(_
					      ("Wrong g3d map values! Values should in between 0 and 255!\n"));
				    fprintf(fp, "0 ");
				}
				else {

				    fprintf(fp, "%.*f ", dp, (fvalue / 255));
				}
			    }
			}
			else {
			    G3d_getValue(maprgb, x, y, z, &dvalue,
					 typeIntern[k]);
			    if (G3d_isNullValueNum(&dvalue, G3D_DOUBLE))
				fprintf(fp, "0 ");
			    else {
				/*Test of value range, the data should be 1 byte gray values */
				if (dvalue > 255 || dvalue < 0) {
				    G_warning(_
					      ("Wrong g3d map values! Values should in between 0 and 255!\n"));
				    fprintf(fp, "0 ");
				}
				else {

				    fprintf(fp, "%.*f ", dp, (dvalue / 255));
				}
			    }
			}
		    }
		    fprintf(fp, "\n");
		}
	    }
	}
	else {
	    for (y = rows - 1; y >= 0; y--) {	/*From south to the north */
		G_percent(status, (rows * depths - 1), 10);
		status++;

		for (x = 0; x < cols; x++) {
		    for (k = 0; k < 3; k++) {

			if (k == 0)
			    maprgb = map_r;
			if (k == 1)
			    maprgb = map_g;
			if (k == 2)
			    maprgb = map_b;

			if (typeIntern[k] == G3D_FLOAT) {
			    G3d_getValue(maprgb, x, y, z, &fvalue,
					 typeIntern[k]);
			    if (G3d_isNullValueNum(&fvalue, G3D_FLOAT))
				fprintf(fp, "0 ");
			    else {
				/*Test of value range, the data should be 1 byte gray values */
				if (fvalue > 255 || fvalue < 0) {
				    G_warning(_
					      ("Wrong rgb map values! Values should in between 0 and 255!\n"));
				    fprintf(fp, "0 ");
				}
				else {

				    fprintf(fp, "%.*f ", dp, (fvalue / 255));
				}
			    }
			}
			else {
			    G3d_getValue(maprgb, x, y, z, &dvalue,
					 typeIntern[k]);
			    if (G3d_isNullValueNum(&dvalue, G3D_DOUBLE))
				fprintf(fp, "0 ");
			    else {
				/*Test of value range, the data should be 1 byte gray values */
				if (dvalue > 255 || dvalue < 0) {
				    G_warning(_
					      ("Wrong rgb map values! Values should in between 0 and 255!\n"));
				    fprintf(fp, "0 ");
				}
				else {

				    fprintf(fp, "%.*f ", dp, (dvalue / 255));
				}
			    }
			}
		    }
		    fprintf(fp, "\n");
		}
	    }
	}
    }
    return;
}


/* ************************************************************************* */
/* Main function, opens most of the input and output files ***************** */
/* ************************************************************************* */
int main(int argc, char *argv[])
{
    char *output = NULL;
    G3D_Region region;
    FILE *fp = NULL;
    struct GModule *module;
    int dp, i, changemask = 0;
    int rows, cols;
    char *mapset, *name;

    /* Initialize GRASS */
    G_gisinit(argv[0]);

    module = G_define_module();
    module->description =
	_("Converts 3D raster maps (G3D) into the VTK-Ascii format");

    /* Get parameters from user */
    setParams();

    /* Have GRASS get inputs */
    if (G_parser(argc, argv))
	exit(EXIT_FAILURE);

    /*The precision of the output */
    if (param.decimals->answer) {
	if (sscanf(param.decimals->answer, "%d", &dp) != 1)
	    G_fatal_error(_("failed to interpret dp as an integer"));
	if (dp > 20 || dp < 0)
	    G_fatal_error(_("dp has to be from 0 to 20"));
    }
    else {
	dp = 8;			/*This value is taken from the lib settings in G_format_easting */
    }

    /*Check the input */
    CheckInputMaps();

    /*open the output */
    if (param.output->answer) {
	fp = fopen(param.output->answer, "w");
	if (fp == NULL) {
	    perror(param.output->answer);
	    G_usage();
	    exit(EXIT_FAILURE);
	}
    }
    else
	fp = stdout;

    /* Figure out the region from the map */
    G3d_initDefaults();
    G3d_getWindow(&region);

    /*Open the top and bottom file */
    if (param.structgrid->answer) {

	/*Check if the g3d-region is equal to the 2d rows and cols */
	rows = G_window_rows();
	cols = G_window_cols();

	if (rows != region.rows || cols != region.cols) {
	    G3d_fatalError(_
			   ("The 2d resolution (res) and the 3d resolution (res3) should be equal. Please set this with g.region."));
	}
	/*open top */
	mapset = NULL;
	name = NULL;
	name = param.top->answer;
	mapset = G_find_cell2(name, "");
	top = OpenInputMap(name, mapset);
	topMapType = G_raster_map_type(name, mapset);

	/*open bottom */
	mapset = NULL;
	name = NULL;
	name = param.bottom->answer;
	mapset = G_find_cell2(name, "");
	bottom = OpenInputMap(name, mapset);
	bottomMapType = G_raster_map_type(name, mapset);

	/* Write the vtk-header and the points */
	writeVTKStructuredGridHeader(fp, output, region);
	writeVTKPoints(fp, region, dp);

	/*Close top and bottom maps */
	CloseInputMap(top);
	CloseInputMap(bottom);
    }
    else {
	/* Write the structured point vtk-header */
	writeVTKStructuredPointHeader(fp, output, region, dp);
    }

    /*Write the normal VTK data (cell or point data) */
    /*Loop over all 3d input maps! */
    if (param.input->answers != NULL) {
	for (i = 0; param.input->answers[i] != NULL; i++) {

	    G_debug(3, _("Open g3d raster file %s"), param.input->answers[i]);

	    /*Open the map */
	    map =
		G3d_openCellOld(param.input->answers[i],
				G_find_grid3(param.input->answers[i], ""),
				G3D_DEFAULT_WINDOW, G3D_TILE_SAME_AS_FILE,
				G3D_USE_CACHE_DEFAULT);
	    if (map == NULL)
		G3d_fatalError(_("Error opening g3d file"));

	    /*if requested set the Mask on */
	    if (param.mask->answer) {
		if (G3d_maskFileExists()) {
		    changemask = 0;
		    if (G3d_maskIsOff(map)) {
			G3d_maskOn(map);
			changemask = 1;
		    }
		}
	    }

	    /* Write the point or cell data */
	    writeVTKData(fp, region, param.input->answers[i], dp);

	    /*We set the Mask off, if it was off before */
	    if (param.mask->answer) {
		if (G3d_maskFileExists())
		    if (G3d_maskIsOn(map) && changemask)
			G3d_maskOff(map);
	    }

	    /* Close the g3d map */
	    if (!G3d_closeCell(map))
		fatalError(_
			   ("Error closing g3d data map, the VTK file may be incomplete."));

	    map = NULL;
	}
    }

    /*Write the RGB voxel data */
    OpenWriteRGBMaps(region, fp, dp);

    /*Close the output file */
    if (param.output->answer && fp != NULL)
	if (fclose(fp))
	    fatalError(_("Error closing VTK-ASCII file"));

    return 0;
}
