#include "dbmi.h"

db_driver_drop_column (tableName, columnName)
    dbString *tableName;
    dbString *columnName;
{
    db_procedure_not_implemented("db_drop_column");
    return DB_FAILED;
}
