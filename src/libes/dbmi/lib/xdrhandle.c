#include "dbmi.h"
#include "macros.h"

int db__send_handle (handle)
    dbHandle *handle;
{
    DB_SEND_STRING (&handle->dbName);
    DB_SEND_STRING (&handle->dbPath);

    return DB_OK;
}

int
db__recv_handle (handle)
    dbHandle *handle;
{
    DB_RECV_STRING (&handle->dbName);
    DB_RECV_STRING (&handle->dbPath);

    return DB_OK;
}
