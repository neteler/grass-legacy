#include "dbmi.h"

int
db_driver_create_database (handle)
    dbHandle *handle;
{
    db_procedure_not_implemented("db_create_database");
    return DB_FAILED;
}
