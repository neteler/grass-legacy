#include "dbmi.h"

int
db_driver_drop_index (name)
    dbString *name;
{
    db_procedure_not_implemented("db_drop_index");
    return DB_FAILED;
}
