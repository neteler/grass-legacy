/**************************************************************
 * db.columns driver=name database=name [location=name] table=name
 *
 *
 *   list the column names for a table
 ****************************************************************/

#include "gis.h"
#include "dbmi.h"
#include "codes.h"

struct {
	char *driver, *database, *location, *table;
} parms;

void parse_command_line();


main(argc, argv) char *argv[];
{
    dbDriver *driver;
    dbHandle handle;
    dbTable *table;
    dbString table_name;
    int col, ncols;

    parse_command_line (argc, argv);

    driver = db_start_driver(parms.driver);
    if (driver == NULL)
    {
	exit(ERROR);
    }
    db_init_handle (&handle);
    db_set_handle (&handle, parms.database, parms.location);
    if (db_open_database(driver, &handle) != DB_OK)
    {
	exit(ERROR);
    }
    db_init_string(&table_name);
    db_set_string(&table_name, parms.table);
    if(db_describe_table (driver, &table_name, &table) != DB_OK)
	exit(ERROR);

    db_close_database(driver);
    db_shutdown_driver(driver);

    ncols = db_get_table_number_of_columns(table);
    for (col = 0; col < ncols; col++)
	fprintf (stdout,"%s\n", db_get_column_name(db_get_table_column(table, col)));
    exit(OK);
}

void
parse_command_line(argc, argv) char *argv[];
{
    struct Option *driver, *database, *location, *table;

    driver 		= G_define_option();
    driver->key 	= "driver";
    driver->type 	= TYPE_STRING;
    driver->required 	= YES;
    driver->description = "driver name";

    database 		= G_define_option();
    database->key 	= "database";
    database->type 	= TYPE_STRING;
    database->required 	= YES;
    database->description = "database name";

    location 		= G_define_option();
    location->key 	= "location";
    location->type 	= TYPE_STRING;
    location->required 	= NO;
    location->description = "database location";

    table 		= G_define_option();
    table->key 		= "table";
    table->type 	= TYPE_STRING;
    table->required 	= YES;
    table->description 	= "table name";

    G_disable_interactive();
    if(G_parser(argc, argv))
	exit(ERROR);

    parms.driver	= driver->answer;
    parms.database	= database->answer;
    parms.location	= location->answer;
    parms.table		= table->answer;
}
