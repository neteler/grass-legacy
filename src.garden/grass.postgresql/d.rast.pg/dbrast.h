#define QRY_LENGTH 1024
#define OUT_LENGTH 128
int getAllOpts (int, char **);
int getSelectOpts (int, char **);
int buildPg(char *,char *,char *,char *,char *,char *,char *);
int runPg(char *, char *,char *, int);
