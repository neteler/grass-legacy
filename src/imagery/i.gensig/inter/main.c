#include <stdlib.h>
#include "imagery.h"

int main (int argc, char *argv[])
{
	char command[1024];
	char map[50];
	char *mapset;
	char group[50];
	char subgroup[50];
	char sigfile[50];

	G_gisinit(argv[0]);

	mapset = G_ask_cell_old ("Enter map with training sites", map);
	if (mapset == NULL) exit(0);

	if (!I_ask_group_old ("", group)) exit(0);
	if(!I_ask_subgroup_old ("",group,subgroup)) exit(0);
	if(!I_ask_signature_file_any ("", group, subgroup, sigfile)) exit(0);

	sprintf (command,
	    "i.gensig train='%s' group='%s' subgroup='%s' sig='%s'",
		G_fully_qualified_name(map, mapset), group, subgroup, sigfile);
	
	exit(system(command));
}
