/* allocation.c */
char *falloc(int, int);
char *frealloc(char *, int, int, int);
/* do_graph.c */
int set_graph_stuff(void);
int set_text_size(void);
int do_draw(char *);
int do_move(char *);
int do_color(char *);
char *do_poly(char *, FILE *);
int do_size(char *);
int do_text(char *);
int check_alloc(int);
int do_icon(char *);
/* graphics.c */
int graphics(FILE *);
/* main.c */
int main(int, char **);
