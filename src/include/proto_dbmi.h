#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

void        db_Cstring_to_lowercase   P((char *s ));
void        db_Cstring_to_uppercase   P((char *s ));
int         db_add_column             P((dbDriver *driver , dbString *tableName , dbColumn *column ));
void        db__add_cursor_to_driver_state P((dbCursor *cursor ));
int         db_alloc_cursor_column_flags P((dbCursor *cursor ));
int         db_alloc_cursor_table     P((dbCursor *cursor , int ncols ));
dbDirent *  db_alloc_dirent_array     P((int count ));
dbHandle *  db_alloc_handle_array     P((int count ));
dbIndex *   db_alloc_index_array      P((int count ));
int         db_alloc_index_columns    P((dbIndex *index , int ncols ));
dbString *  db_alloc_string_array     P((int count ));
dbTable *   db_alloc_table            P((int ncols ));
int         db_append_string          P((dbString *x , char *s ));
void        db_auto_print_errors      P((int flag ));
void        db_auto_print_protocol_errors P((int flag ));
int         db_bind_update            P((dbCursor *cursor ));
void *      db_calloc                 P((int n , int m ));
void        db_char_to_lowercase      P((char *s ));
void        db_char_to_uppercase      P((char *s ));
void        db_clear_error            P((void ));
void        db__close_all_cursors     P((void ));
int         db_close_cursor           P((dbCursor *cursor ));
int         db_close_database         P((dbDriver *driver ));
int         db_convert_Cstring_to_column_default_value P((char *Cstring , dbColumn *column ));
int         db_convert_Cstring_to_column_value P((char *Cstring , dbColumn *column ));
int         db_convert_Cstring_to_value P((char *Cstring , int sqltype , dbValue *value ));
int         db_convert_Cstring_to_value_datetime P((char *buf , int sqltype , dbValue *value ));
int         db_convert_column_default_value_to_string P((dbColumn *column , dbString *string ));
int         db_convert_column_value_to_string P((dbColumn *column , dbString *string ));
int         db_convert_value_datetime_into_string P((dbValue *value , int sqltype , dbString *string ));
int         db_convert_value_to_string P((dbValue *value , int sqltype , dbString *string ));
void        db_copy_dbmscap_entry     P((dbDbmscap *dst , dbDbmscap *src ));
int         db_copy_string            P((dbString *dst , dbString *src ));
int         db_create_database        P((dbDriver *driver , dbHandle *handle ));
int         db_create_index           P((dbDriver *driver , dbIndex *index ));
int         db_create_table           P((dbDriver *driver , dbTable *table ));
int         db_d_add_column           P((void ));
int         db_d_bind_update          P((void ));
char *      db_dbmscap_filename       P((void ));
int         db_d_close_cursor         P((void ));
int         db_d_close_database       P((void ));
int         db_d_create_database      P((void ));
int         db_d_create_index         P((void ));
int         db_d_create_table         P((void ));
int         db_d_delete               P((void ));
int         db_d_delete_database      P((void ));
int         db_d_describe_table       P((void ));
int         db_d_drop_column          P((void ));
int         db_d_drop_index           P((void ));
int         db_d_drop_table           P((void ));
void        db_debug                  P((char *s ));
void        db_debug_off              P((void ));
void        db_debug_on               P((void ));
int         db_delete                 P((dbCursor *cursor ));
int         db_delete_database        P((dbDriver *driver , dbHandle *handle ));
int         db_describe_table         P((dbDriver *driver , dbString *name , dbTable **table ));
int         db_d_execute_immediate    P((void ));
int         db_d_fetch                P((void ));
int         db_d_find_database        P((void ));
int         db_d_insert               P((void ));
dbDirent *  db_dirent                 P((char *dirname , int *n ));
int         db_d_list_databases       P((void ));
int         db_d_list_indexes         P((void ));
int         db_d_list_tables          P((void ));
int         db_d_open_database        P((void ));
int         db_d_open_insert_cursor   P((void ));
int         db_d_open_select_cursor   P((void ));
int         db_d_open_update_cursor   P((void ));
int         db_driver                 P((int argc , char *argv []));
int         db_driver_mkdir           P((char *path , int mode , int parentdirs ));
int         db_drop_column            P((dbDriver *driver , dbString *tableName , dbString *columnName ));
void        db__drop_cursor_from_driver_state P((dbCursor *cursor ));
int         db_drop_index             P((dbDriver *driver , dbString *name ));
int         db_drop_table             P((dbDriver *driver , dbString *name ));
void        db_drop_token             P((dbToken token ));
int         db_d_update               P((void ));
int         db_d_version              P((void ));
int         db_enlarge_string         P((dbString *x , int len ));
void        db_error                  P((char *s ));
int         db_execute_immediate      P((dbDriver *driver , dbString *SQLstatement ));
int         db_fetch                  P((dbCursor *cursor , int position , int *more ));
int         db_find_database          P((dbDriver *driver , dbHandle *handle , int *found ));
dbAddress   db_find_token             P((dbToken token ));
void        db_free_column            P((dbColumn *column ));
void        db_free_cursor            P((dbCursor *cursor ));
void        db_free_cursor_column_flags P((dbCursor *cursor ));
void        db_free_dbmscap           P((dbDbmscap *list ));
void        db_free_dirent_array      P((dbDirent *dirent , int count ));
void        db_free_handle            P((dbHandle *handle ));
void        db_free_handle_array      P((dbHandle *handle , int count ));
void        db_free_index             P((dbIndex *index ));
void        db_free_index_array       P((dbIndex *list , int count ));
void        db_free_string            P((dbString *x ));
void        db_free_string_array      P((dbString *a , int n ));
void        db_free_table             P((dbTable *table ));
dbValue *   db_get_column_default_value P((dbColumn *column ));
char *      db_get_column_description P((dbColumn *column ));
int         db_get_column_host_type   P((dbColumn *column ));
int         db_get_column_length      P((dbColumn *column ));
char *      db_get_column_name        P((dbColumn *column ));
int         db_get_column_precision   P((dbColumn *column ));
int         db_get_column_scale       P((dbColumn *column ));
int         db_get_column_select_priv P((dbColumn *column ));
int         db_get_column_sqltype     P((dbColumn *column ));
int         db_get_column_update_priv P((dbColumn *column ));
dbValue *   db_get_column_value       P((dbColumn *column ));
int         db_get_connection         P((dbConnection *connection ));
int         db_get_cursor_number_of_columns P((dbCursor *cursor ));
dbTable *   db_get_cursor_table       P((dbCursor *cursor ));
dbToken     db_get_cursor_token       P((dbCursor *cursor ));
dbDriverState * db__get_driver_state      P((void ));
int         db_get_error_code         P((void ));
char *      db_get_error_msg          P((void ));
char *      db_get_error_who          P((void ));
char *      db_get_handle_dbname      P((dbHandle *handle ));
char *      db_get_handle_dbpath      P((dbHandle *handle ));
char *      db_get_index_column_name  P((dbIndex *index , int column_num ));
char *      db_get_index_name         P((dbIndex *index ));
int         db_get_index_number_of_columns P((dbIndex *index ));
char *      db_get_index_table_name   P((dbIndex *index ));
char *      db_get_string             P((dbString *x ));
dbColumn *  db_get_table_column       P((dbTable *table , int n ));
int         db_get_table_delete_priv  P((dbTable *table ));
char *      db_get_table_description  P((dbTable *table ));
int         db_get_table_insert_priv  P((dbTable *table ));
char *      db_get_table_name         P((dbTable *table ));
int         db_get_table_number_of_columns P((dbTable *table ));
int         db_get_table_select_priv  P((dbTable *table ));
int         db_get_table_update_priv  P((dbTable *table ));
int         db_get_value_day          P((dbValue *value ));
double      db_get_value_double       P((dbValue *value ));
int         db_get_value_hour         P((dbValue *value ));
int         db_get_value_int          P((dbValue *value ));
int         db_get_value_minute       P((dbValue *value ));
int         db_get_value_month        P((dbValue *value ));
double      db_get_value_seconds      P((dbValue *value ));
char *      db_get_value_string       P((dbValue *value ));
int         db_get_value_year         P((dbValue *value ));
int         db_has_dbms               P((void ));
void        db_init_column            P((dbColumn *column ));
void        db_init_cursor            P((dbCursor *cursor ));
void        db__init_driver_state     P((void ));
void        db_init_handle            P((dbHandle *handle ));
void        db_init_index             P((dbIndex *index ));
void        db_init_string            P((dbString *x ));
void        db_init_table             P((dbTable *table ));
int         db_insert                 P((dbCursor *cursor ));
void        db_interval_range         P((int sqltype , int *from , int *to ));
int         db_isdir                  P((char *path ));
int         db_list_databases         P((dbDriver *driver , dbString *path , int npaths , dbHandle **handles , int *count ));
int         db_list_indexes           P((dbDriver *driver , dbString *table_name , dbIndex **list , int *count ));
int         db_list_tables            P((dbDriver *driver , dbString **names , int *count , int system ));
void *      db_malloc                 P((int n ));
void        db__mark_database_closed  P((void ));
void        db__mark_database_open    P((char *dbname , char *dbpath ));
void        db_memory_error           P((void ));
dbToken     db_new_token              P((dbAddress address ));
int         db_nocase_compare         P((char *a , char *b ));
void        db_noproc_error           P((int procnum ));
int         db_open_database          P((dbDriver *driver , dbHandle *handle ));
int         db_open_insert_cursor     P((dbDriver *driver , dbCursor *cursor ));
int         db_open_select_cursor     P((dbDriver *driver , dbString *select , dbCursor *cursor , int mode ));
int         db_open_update_cursor     P((dbDriver *driver , dbString *table_name , dbString *select , dbCursor *cursor , int mode ));
void        db_print_column_definition P((FILE *fd , dbColumn *column ));
void        db_print_error            P((void ));
int         db_print_index            P((FILE *fd , dbIndex *index ));
void        db_print_table_definition P((FILE *fd , dbTable *table ));
void        db_procedure_not_implemented P((char *name ));
void        db_protocol_error         P((void ));
dbDbmscap * db_read_dbmscap           P((void ));
void *      db_realloc                P((void *s , int n ));
int         db_rcls                   P((dbRclsRule *rule , dbCatValI **new, int *num ));
void        db_rcls_rule_extend       P((dbRclsRule *rule , int n ));
dbRclsRule * db_rcls_rule_init        P((int n ));
int         db__recv_char             P((char *d ));
int         db__recv_column_default_value P((dbColumn *column ));
int         db__recv_column_definition P((dbColumn *column ));
int         db__recv_column_value     P((dbColumn *column ));
int         db__recv_datetime         P((dbDateTime *t ));
int         db__recv_double           P((double *d ));
int         db__recv_double_array     P((double **x , int *n ));
int         db__recv_float            P((float *d ));
int         db__recv_float_array      P((float **x , int *n ));
int         db__recv_handle           P((dbHandle *handle ));
int         db__recv_index            P((dbIndex *index ));
int         db__recv_index_array      P((dbIndex **list , int *count ));
int         db__recv_int              P((int *n ));
int         db__recv_int_array        P((int **x , int *n ));
int         db__recv_procnum          P((int *n ));
int         db__recv_return_code      P((int *ret_code ));
int         db__recv_short            P((short *n ));
int         db__recv_short_array      P((short **x , int *n ));
int         db__recv_string           P((dbString *x ));
int         db__recv_string_array     P((dbString **a , int *n ));
int         db__recv_table_data       P((dbTable *table ));
int         db__recv_table_definition P((dbTable **table ));
int         db__recv_token            P((dbToken *token ));
int         db__recv_value            P((dbValue *value , int Ctype ));
int         db__send_Cstring          P((char *s ));
int         db__send_char             P((int d ));
int         db__send_column_default_value P((dbColumn *column ));
int         db__send_column_definition P((dbColumn *column ));
int         db__send_column_value     P((dbColumn *column ));
int         db__send_datetime         P((dbDateTime *t ));
int         db__send_double           P((double d ));
int         db__send_double_array     P((double *x , int n ));
int         db__send_failure          P((void ));
int         db__send_float            P((float d ));
int         db__send_float_array      P((float *x , int n ));
int         db__send_handle           P((dbHandle *handle ));
int         db__send_index            P((dbIndex *index ));
int         db__send_index_array      P((dbIndex *list , int count ));
int         db__send_int              P((int n ));
int         db__send_int_array        P((int *x , int n ));
int         db__send_procedure_not_implemented P((int n ));
int         db__send_procedure_ok     P((int n ));
int         db__send_short            P((int n ));
int         db__send_short_array      P((short *x , int n ));
int         db__send_string           P((dbString *x ));
int         db__send_string_array     P((dbString *a , int count ));
int         db__send_success          P((void ));
int         db__send_table_data       P((dbTable *table ));
int         db__send_table_definition P((dbTable *table ));
int         db__send_token            P((dbToken *token ));
int         db__send_value            P((dbValue *value , int Ctype ));
int         db_set_column_description P((dbColumn *column , char *description ));
void        db_set_column_has_defined_default_value P((dbColumn *column ));
void        db_set_column_has_undefined_default_value P((dbColumn *column ));
void        db_set_column_host_type   P((dbColumn *column , int type ));
void        db_set_column_length      P((dbColumn *column , int length ));
int         db_set_column_name        P((dbColumn *column , char *name ));
void        db_set_column_null_allowed P((dbColumn *column ));
void        db_set_column_precision   P((dbColumn *column , int precision ));
void        db_set_column_scale       P((dbColumn *column , int scale ));
void        db_set_column_select_priv_granted P((dbColumn *column ));
void        db_set_column_select_priv_not_granted P((dbColumn *column ));
void        db_set_column_sqltype     P((dbColumn *column , int sqltype ));
void        db_set_column_update_priv_granted P((dbColumn *column ));
void        db_set_column_update_priv_not_granted P((dbColumn *column ));
void        db_set_column_use_default_value P((dbColumn *column ));
int         db_set_connection         P((dbConnection *connection ));
void        db_set_cursor_column_flag P((dbCursor *cursor , int col ));
void        db_set_cursor_column_for_update P((dbCursor *cursor , int col ));
void        db_set_cursor_mode        P((dbCursor *cursor , int mode ));
void        db_set_cursor_mode_insensitive P((dbCursor *cursor ));
void        db_set_cursor_mode_scroll P((dbCursor *cursor ));
void        db_set_cursor_table       P((dbCursor *cursor , dbTable *table ));
void        db_set_cursor_token       P((dbCursor *cursor , dbToken token ));
void        db_set_cursor_type_insert P((dbCursor *cursor ));
void        db_set_cursor_type_readonly P((dbCursor *cursor ));
void        db_set_cursor_type_update P((dbCursor *cursor ));
void        db_set_error_who          P((char *me ));
int         db_set_handle             P((dbHandle *handle , char *dbName , char *dbPath ));
int         db_set_index_column_name  P((dbIndex *index , int column_num , char *name ));
int         db_set_index_name         P((dbIndex *index , char *name ));
int         db_set_index_table_name   P((dbIndex *index , char *name ));
int         db_set_index_type_non_unique P((dbIndex *index ));
int         db_set_index_type_unique  P((dbIndex *index ));
void        db__set_protocol_fds      P((FILE *send , FILE *recv ));
int         db_set_string             P((dbString *x , char *s ));
int         db_set_string_no_copy     P((dbString *x , char *s ));
void        db_set_table_delete_priv_granted P((dbTable *table ));
void        db_set_table_delete_priv_not_granted P((dbTable *table ));
int         db_set_table_description  P((dbTable *table , char *description ));
void        db_set_table_insert_priv_granted P((dbTable *table ));
void        db_set_table_insert_priv_not_granted P((dbTable *table ));
int         db_set_table_name         P((dbTable *table , char *name ));
void        db_set_table_select_priv_granted P((dbTable *table ));
void        db_set_table_select_priv_not_granted P((dbTable *table ));
void        db_set_table_update_priv_granted P((dbTable *table ));
void        db_set_table_update_priv_not_granted P((dbTable *table ));
void        db_set_value_datetime_current P((dbValue *value ));
void        db_set_value_datetime_not_current P((dbValue *value ));
void        db_set_value_day          P((dbValue *value , int day ));
void        db_set_value_double       P((dbValue *value , double d ));
void        db_set_value_hour         P((dbValue *value , int hour ));
void        db_set_value_int          P((dbValue *value , int i ));
void        db_set_value_minute       P((dbValue *value , int minute ));
void        db_set_value_month        P((dbValue *value , int month ));
void        db_set_value_not_null     P((dbValue *value ));
void        db_set_value_null         P((dbValue *value ));
void        db_set_value_seconds      P((dbValue *value , double seconds ));
int         db_set_value_string       P((dbValue *value , char *s ));
void        db_set_value_year         P((dbValue *value , int year ));
int         db_shutdown_driver        P((dbDriver *driver ));
char *      db_sqltype_name           P((int sqltype ));
int         db_sqltype_to_Ctype       P((int sqltype ));
dbDriver *  db_start_driver           P((char *name ));
int         db__start_procedure_call  P((int procnum ));
char *      db_store                  P((char *s ));
void        db_strip                  P((char *buf ));
void        db_syserror               P((char *s ));
int         db_test_column_has_default_value P((dbColumn *column ));
int         db_test_column_has_defined_default_value P((dbColumn *column ));
int         db_test_column_has_undefined_default_value P((dbColumn *column ));
int         db_test_column_null_allowed P((dbColumn *column ));
int         db_test_column_use_default_value P((dbColumn *column ));
int         db_test_cursor_any_column_flag P((dbCursor *cursor ));
int         db_test_cursor_any_column_for_update P((dbCursor *cursor ));
int         db_test_cursor_column_flag P((dbCursor *cursor , int col ));
int         db_test_cursor_column_for_update P((dbCursor *cursor , int col ));
int         db_test_cursor_mode_insensitive P((dbCursor *cursor ));
int         db_test_cursor_mode_scroll P((dbCursor *cursor ));
int         db_test_cursor_type_fetch P((dbCursor *cursor ));
int         db_test_cursor_type_insert P((dbCursor *cursor ));
int         db_test_cursor_type_update P((dbCursor *cursor ));
int         db__test_database_open    P((void ));
int         db_test_index_type_unique P((dbIndex *index ));
int         db_test_value_datetime_current P((dbValue *value ));
int         db_test_value_isnull      P((dbValue *value ));
void        db_unset_column_has_default_value P((dbColumn *column ));
void        db_unset_column_null_allowed P((dbColumn *column ));
void        db_unset_column_use_default_value P((dbColumn *column ));
void        db_unset_cursor_column_flag P((dbCursor *cursor , int col ));
void        db_unset_cursor_column_for_update P((dbCursor *cursor , int col ));
void        db_unset_cursor_mode      P((dbCursor *cursor ));
void        db_unset_cursor_mode_insensitive P((dbCursor *cursor ));
void        db_unset_cursor_mode_scroll P((dbCursor *cursor ));
int         db_update                 P((dbCursor *cursor ));
int         db_version                P((dbDriver *driver , dbString *client_version , dbString *driver_version ));
char *      db_whoami                 P((void ));
void        db_zero                   P((void *s , int n ));
int         db_zero_string            P((dbString *x ));
unsigned    int                       db_sizeof_string P((dbString *x ));

#undef P
