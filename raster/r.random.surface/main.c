/* main.c */
#include <stdlib.h>
#include <stdio.h>
#include <grass/gis.h>
#include <grass/glocale.h>

#undef TRACE
#undef DEBUG

#define MAIN
#include "ransurf.h"
#include "local_proto.h"
#undef MAIN


int main(int argc, char **argv)
{
	struct GModule *module;
	int	DoMap, DoFilter, MapSeed;
	double	ran1();
	FUNCTION(main);

	G_gisinit( argv[0]);

	module = G_define_module();
	module->keywords = _("raster");
    module->description =
		_("Generates random surface(s) with spatial dependence.");

	Init( argc, argv);
	if( Uniform->answer)
		GenNorm();
	CalcSD();
	for( DoMap = 0; DoMap < NumMaps; DoMap++) {
		OutFD = G_open_cell_new ( OutNames[ DoMap]);
        	if (OutFD < 0)
                	G_fatal_error("%s: unable to open [%s] random raster map",
                        	G_program_name(), OutNames[ DoMap]);

                G_message(_("Starting map [%s]"), OutNames[DoMap]);

		if( Seeds[DoMap] == SEED_MIN - 1)
			Seeds[ DoMap] = (int) (ran1() * SEED_MAX);

		MapSeed = Seed = Seeds[DoMap];
		ZeroMapCells();

		for( DoFilter = 0; DoFilter < NumFilters; DoFilter++) {
		    CopyFilter( &Filter, AllFilters[DoFilter]);
                    G_message(_(
"Starting filter #%d, distance: %.*lf, exponent: %.*lf, flat: %.*lf"),
                    DoFilter,
                    Digits( 2.0 * Filter.MaxDist, 6),
                    2.0 * Filter.MaxDist,
                    Digits( 1.0 / Filter.Exp, 6),
                    1.0 / Filter.Exp,
                    Digits( Filter.Mult, 6),
                    Filter.Mult);

                    G_message(_("Percent done:"));

		    MakeBigF();
		    CalcSurface();
		}

		SaveMap( DoMap, MapSeed);
	}

	return 0;
}
