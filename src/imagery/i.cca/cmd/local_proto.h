/* allocate.c */
int allocate(double *[MX], int, int);
int freemem(double *[MX], int);
/* jacobi.c */
int jacobi(double [MX][MX], long, double [MX], double [MX][MX]);
int egvorder(double [MX], double [MX][MX], long);
int transpose(double [MX][MX], long);
/* main.c */
int main(int, char *[]);
/* matrix.c */
int product(double [MX], double, double [MX][MX], int);
int setdiag(double [MX], int, double [MX][MX]);
int getsqrt(double [MX][MX], int, double [MX][MX], double [MX][MX]);
int solveq(double [MX][MX], int, double [MX][MX], double [MX][MX]);
int matmul(double [MX][MX], double [MX][MX], double [MX][MX], int);
/* stats.c */
int within(int, int, double [MC], double [MC][MX][MX], double [MX][MX], int);
int between(int, int, double [MC], double [MC][MX], double [MX][MX], int);
/* transform.c */
int transform(int [MX], int [MX], int, int, double [MX][MX], int, CELL [MX], CELL [MX]);
