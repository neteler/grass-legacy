#include "dbmi.h"
#include "macros.h"

int db_list_tables (driver, names, count, system)
    dbDriver *driver;
    dbString **names;
    int *count;
    int system;
{
    int ret_code;

/* start the procedure call */
    db__set_protocol_fds (driver->send, driver->recv);
    DB_START_PROCEDURE_CALL(DB_PROC_LIST_TABLES);

/* arguments */
    DB_SEND_INT (system);

/* get the return code for the procedure call */
    DB_RECV_RETURN_CODE(&ret_code);

    if (ret_code != DB_OK)
	return ret_code; /* ret_code SHOULD == DB_FAILED */

/* results */
    DB_RECV_STRING_ARRAY (names, count);

    return DB_OK;
}
