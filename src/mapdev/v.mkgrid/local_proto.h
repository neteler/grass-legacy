/* ask_for.c */
int welcome_mat(void);
int ask_for_int(int *, char *);
int ask_for_double(double *, char *);
int ask_for_name(char *, char *, char *, char *, char *);
/* init_head.c */
int init_header(struct Cell_head *, struct dig_head *);
/* rotate.c */
void rotate(double *, double *, double, double, double);
/* write_grid.c */
int write_grid(struct grid_description *, struct Map_info *, int);
int write_vect(double, double, double, double, struct Map_info *, struct line_pnts *);
