/* Driver */
int driver_open (void); 
int driver_close (void); 
int driver_refresh (void);

/* Miscellaneous */
int update (int, int);
int tool_centre (void);
int end (void);

/* Symbology */
int get_symb_code ( char *); 
char *get_symb_name ( int ); 
void symb_init ( void );
void symb_init_gui ( void );
void symb_set_driver_color ( int );

int symb_line_from_map ( int );
void symb_line_set_from_map ( int );
void symb_lines_init ( void );
void symb_updated_lines_set_from_map ( void );

int symb_node_from_map ( int );
void symb_node_set_from_map ( int );
void symb_nodes_init ( void );
void symb_updated_nodes_set_from_map ( void );

void updated_lines_and_nodes_erase_refresh_display(void); 

/* Edit */
int new_line (int);
int move_vertex (void);
int add_vertex (void);
int rm_vertex (void);
int move_line (void);
int delete_line (void);

/* Display */
void display_points ( struct line_pnts *, int );
void display_icon ( double, double, int, double, int, int);
void display_line ( int, int, int );
void display_updated_lines ( int );
void display_node ( int, int, int);
void display_updated_nodes ( int );
void display_map ( void );
void display_bg ( void );
void display_erase ( void );
void display_redraw ( void );

/* Zoom */
int zoom_window (void);
int zoom_centre (double factor);

int c_cancel (ClientData , Tcl_Interp *, int, char **);
int c_next_tool (ClientData , Tcl_Interp *, int, char **);
int c_tool_centre (ClientData , Tcl_Interp *, int, char **);
int c_set_color (ClientData , Tcl_Interp *, int, char **);
int c_set_on (ClientData , Tcl_Interp *, int, char **);
int c_create_table (ClientData , Tcl_Interp *, int, char **);
int c_table_definition (ClientData , Tcl_Interp *, int, char **);
int c_var_set (ClientData , Tcl_Interp *, int, char **);

int i_prompt (char *);
int i_prompt_buttons (char *, char *, char *);
int i_coor ( double, double);
int i_set_color ( char *, int, int, int);
int i_set_on ( char *, int);
int i_update (void);
void i_new_line_options ( int );
void i_set_cat_mode ( void );
void i_var_seti ( int, int ); 
void i_var_setd ( int, double ); 
void i_var_setc ( int, char* ); 
int i_message ( int, int, char*);

/* Cats */
void cat_init ( void );
int cat_max_get ( int );
void cat_max_set ( int, int);

/* Variables */
void var_init ( void );
int var_seti ( int, int );
int var_setd ( int, double );
int var_setc ( int, char * );
int var_get_type_by_name ( char * );
int var_get_code_by_name ( char * );
char *var_get_name_by_code ( int code );
int var_geti ( int );
double var_getd ( int );
char *var_getc ( int );

/* Background */
int bg_add ( char *);

/* Utilities */
char *get_line_type_name ( int type);
