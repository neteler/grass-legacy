#include "dbmi.h"
#include "macros.h"

int
db_d_bind_update()
{
    dbToken token;
    dbCursor *cursor;
    int stat;
    int ncols;

/* get the arg(s) */
    DB_RECV_TOKEN(&token);
    cursor = (dbCursor *) db_find_token(token);
    if (cursor == NULL || !db_test_cursor_type_update(cursor))
    {
	db_error ("** not an update cursor **");
	DB_SEND_FAILURE();
	return DB_FAILED;
    }
    DB_RECV_SHORT_ARRAY (&cursor->column_flags, &ncols);
    if(!db_test_cursor_any_column_flag (cursor))
    {
	db_error ("** no columns set in cursor for binding **");
	DB_SEND_FAILURE();
	return DB_FAILED;
    }

/* call the procedure */
    stat = db_driver_bind_update (cursor);

/* send the return code */
    if (stat != DB_OK)
    {
	DB_SEND_FAILURE();
	return DB_OK;
    }
    DB_SEND_SUCCESS();

/* no results */
    return DB_OK;
}
