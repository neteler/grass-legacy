
# line 10 "ncgen.y"
#ifndef lint
static char SccsId[] = "$Id$";
#endif
#include        <string.h>
#include	<stdlib.h>
#include        "ncgen.h"

typedef struct Symbol {		/* symbol table entry */
	char    	*name;
	struct Symbol   *next;
	unsigned	is_dim : 1;	/* appears as netCDF dimension */
	unsigned	is_var : 1;	/* appears as netCDF variable */
	unsigned	is_att : 1;	/* appears as netCDF attribute */
	int             dnum;	        /* handle as a dimension */
	int             vnum;	        /* handle as a variable */
	} *YYSTYPE1;

#define YYSTYPE YYSTYPE1
YYSTYPE install(), lookup();
YYSTYPE symlist;		/* symbol table: linked list */

void init_netcdf();		/* initializes netcdf counts (e.g. nvars) */
void define_netcdf();		/* generates all define mode stuff */
void load_netcdf();		/* generates variable puts */
void close_netcdf();		/* generates close */

void derror(const char *);			/* varargs message emitter */
void *emalloc(), *erealloc();	/* malloc that checks for memory exhausted */
void clearout();		/* initializes symbol table */
void nc_getfill();		/* to get fill value for various types */
void nc_putfill();		/* to get fill value for various types */
char *nctype();		/* returns type name from number */
void nc_fill();		/* fills a generic array with a value */

extern int derror_count;	/* counts errors in netcdf definition */
extern int lineno;		/* line number for error messages */

static int not_a_string;	/* whether last constant read was a string */
static char termstring[MAXTRST]; /* last terminal string read */
static double double_val;	/* last double value read */
static float float_val;		/* last float value read */
static long long_val;		/* last long value read */
static short short_val;		/* last short value read */
static char char_val;		/* last char value read */
static char byte_val;		/* last byte value read */

static nc_type type_code;	/* holds declared type for variables */
static nc_type atype_code;	/* holds derived type for attributes */
static char *netcdfname;	/* to construct netcdf file name */
static void *att_space;		/* pointer to block for attribute values */
static nc_type valtype;		/* type code for list of attribute values  */

static char *char_valp;		/* pointers used to accumulate data values */
static char *byte_valp;
static short *short_valp;
static long *long_valp;
static float *float_valp;
static double *double_valp;
static void *rec_cur;		/* pointer to where next data value goes */
static void *rec_start;		/* start of space for a record of data */
# define NC_UNLIMITED_K 257
# define BYTE_K 258
# define CHAR_K 259
# define SHORT_K 260
# define LONG_K 261
# define FLOAT_K 262
# define DOUBLE_K 263
# define IDENT 264
# define TERMSTRING 265
# define BYTE_CONST 266
# define CHAR_CONST 267
# define SHORT_CONST 268
# define LONG_CONST 269
# define FLOAT_CONST 270
# define DOUBLE_CONST 271
# define DIMENSIONS 272
# define VARIABLES 273
# define NETCDF 274
# define DATA 275

#include <malloc.h>
#include <memory.h>
#if 0
#include <values.h>
#endif

#ifdef __cplusplus

#ifndef yyerror
	void yyerror(const char *);
#endif

#ifndef yylex
#ifdef __EXTERN_C__
	extern "C" { int yylex(void); }
#else
	int yylex(void);
#endif
#endif
	int yyparse(void);

#endif
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256

# line 676 "ncgen.y"


/* PROGRAMS */

/* get lexical input routine generated by lex  */
#include "msoftyy.c"

void derror();

yyerror(s)	/* called for yacc syntax error */
    const char *s;
{
	derror(s);
}

int
yywrap()			/* returns 1 on EOF if no more input */
{
    return  1;
}


/* Symbol table operations for ncgen tool */

YYSTYPE lookup(sname)       /* find sname in symbol table (linear search) */
char *sname;
{
    YYSTYPE sp;
    for (sp = symlist; sp != (YYSTYPE) 0; sp = sp -> next)
	if (STREQ(sp -> name, sname)) {
	    return sp;
	}
    return 0;			/* 0 ==> not found */
}

YYSTYPE install(sname)  /* install sname in symbol table */
char *sname;
{
    YYSTYPE sp;

    sp = (YYSTYPE) emalloc (sizeof (struct Symbol));
    sp -> name = (char *) emalloc (strlen (sname) + 1);/* +1 for '\0' */
    (void) strcpy (sp -> name, sname);
    sp -> next = symlist;	/* put at front of list */
    sp -> is_dim = 0;
    sp -> is_var = 0;
    sp -> is_att = 0;
    symlist = sp;
    return sp;
}

void
clearout()	/* reset symbol table to empty */
{
    YYSTYPE sp, tp;
    for (sp = symlist; sp != (YYSTYPE) 0;) {
	tp = sp -> next;
	free (sp -> name);
	free ((char *) sp);
	sp = tp;
    }
    symlist = 0;
}
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 71
# define YYLAST 225
yytabelem yyact[]={

    92,    93,    91,    94,    95,    96,    97,    71,    72,    70,
    73,    74,    75,    76,    42,     2,    15,     6,    34,    40,
    13,    36,    51,    52,     3,    84,    12,    68,    80,    78,
    59,    39,    19,    18,    18,    77,    62,    56,    44,    35,
    49,    46,    33,    86,    54,    50,    87,    89,    37,    17,
    82,    57,    66,    22,    10,     9,    90,    85,    83,    63,
    53,    69,    67,    48,    16,    47,    26,    79,    65,    58,
    45,    25,    24,    38,    23,    43,    21,    11,     8,    41,
    20,    14,     7,     5,     4,    55,     1,     0,     0,     0,
     0,     0,     0,    81,     0,    60,    55,    47,    61,    64,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    88,     0,     0,     0,    81,    99,    98,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    27,    28,
    29,    30,    31,    32,    36 };
yytabelem yypact[]={

  -259,-10000000,   -99,-10000000,  -255,-10000000,  -244,  -257,  -244,   -10,
-10000000,   -29,-10000000,-10000000,-10000000,   -40,   -11,-10000000,  -244,  -238,
  -261,   -40,   -21,-10000000,-10000000,  -243,-10000000,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,   -18,  -242,-10000000,-10000000,-10000000,-10000000,-10000000,
-10000000,  -102,  -243,   -22,-10000000,     7,-10000000,-10000000,   -31,  -242,
-10000000,-10000000,-10000000,  -243,   -23,-10000000,-10000000,  -243,    12,  -258,
-10000000,   -24,-10000000,   -32,-10000000,-10000000,  -244,     6,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,     2,
-10000000,-10000000,  -258,     3,-10000000,  -265,-10000000,  -244,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000 };
yytabelem yypgo[]={

     0,    86,    84,    83,    82,    81,    80,    79,    78,    55,
    54,    77,    26,    76,    53,    74,    72,    71,    70,    41,
    39,    69,    68,    67,    28,    66,    63,    62,    42,    45,
    27,    61,    60,    44,    59,    58,    25,    57,    56 };
yytabelem yyr1[]={

     0,     2,     4,     6,     1,     3,     3,     8,     8,     9,
     9,    10,    10,    11,    12,     5,     5,    13,    13,    14,
    14,    15,    17,    17,    17,    17,    17,    17,    18,    18,
    21,    19,    20,    22,    22,    23,    23,    24,    26,    16,
    25,    25,    28,    29,    27,    27,    30,    31,    31,    31,
    31,    31,    31,    31,     7,     7,    32,    32,    34,    33,
    35,    35,    37,    36,    38,    38,    38,    38,    38,    38,
    38 };
yytabelem yyr2[]={

     0,     1,     1,     1,    19,     0,     4,     4,     6,     2,
     6,     7,     7,     3,     2,     0,     4,     4,     6,     2,
     2,     4,     3,     3,     3,     3,     3,     3,     2,     6,
     1,     7,     2,     0,     6,     2,     6,     3,     1,     9,
     6,     5,     3,     3,     2,     6,     3,     3,     3,     3,
     3,     3,     3,     3,     0,     4,     4,     6,     1,     9,
     2,     6,     1,     5,     3,     3,     3,     3,     3,     3,
     3 };
yytabelem yychk[]={

-10000000,    -1,   274,   123,    -2,    -3,   272,    -4,    -8,    -9,
   -10,   -11,   -12,   264,    -5,   273,    -9,    59,    44,    61,
    -6,   -13,   -14,   -15,   -16,   -17,   -25,   258,   259,   260,
   261,   262,   263,   -28,    58,   -20,   264,    59,   -10,   269,
   257,    -7,   275,   -14,    59,   -18,   -19,   -20,   -26,    58,
   -29,   264,   125,   -32,   -33,   -28,    59,    44,   -21,    61,
   -29,   -33,    59,   -34,   -19,   -22,    40,   -27,   -30,   -31,
   267,   265,   266,   268,   269,   270,   271,    59,    61,   -23,
   -24,   -12,    44,   -35,   -36,   -37,    41,    44,   -30,    44,
   -38,   267,   265,   266,   268,   269,   270,   271,   -24,   -36 };
yytabelem yydef[]={

     0,    -2,     0,     1,     5,     2,     0,    15,     6,     0,
     9,     0,    13,    14,     3,     0,     0,     7,     0,     0,
    54,    16,     0,    19,    20,     0,    38,    22,    23,    24,
    25,    26,    27,     0,     0,    42,    32,     8,    10,    11,
    12,     0,     0,     0,    17,    21,    28,    30,     0,     0,
    41,    43,     4,    55,     0,    58,    18,     0,    33,     0,
    40,     0,    56,     0,    29,    31,     0,    39,    44,    46,
    47,    48,    49,    50,    51,    52,    53,    57,    62,     0,
    35,    37,     0,    59,    60,     0,    34,     0,    45,    62,
    63,    64,    65,    66,    67,    68,    69,    70,    36,    61 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"NC_UNLIMITED_K",	257,
	"BYTE_K",	258,
	"CHAR_K",	259,
	"SHORT_K",	260,
	"LONG_K",	261,
	"FLOAT_K",	262,
	"DOUBLE_K",	263,
	"IDENT",	264,
	"TERMSTRING",	265,
	"BYTE_CONST",	266,
	"CHAR_CONST",	267,
	"SHORT_CONST",	268,
	"LONG_CONST",	269,
	"FLOAT_CONST",	270,
	"DOUBLE_CONST",	271,
	"DIMENSIONS",	272,
	"VARIABLES",	273,
	"NETCDF",	274,
	"DATA",	275,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"ncdesc : NETCDF '{'",
	"ncdesc : NETCDF '{' dimsection",
	"ncdesc : NETCDF '{' dimsection vasection",
	"ncdesc : NETCDF '{' dimsection vasection datasection '}'",
	"dimsection : /* empty */",
	"dimsection : DIMENSIONS dimdecls",
	"dimdecls : dimdecline ';'",
	"dimdecls : dimdecls dimdecline ';'",
	"dimdecline : dimdecl",
	"dimdecline : dimdecline ',' dimdecl",
	"dimdecl : dimd '=' LONG_CONST",
	"dimdecl : dimd '=' NC_UNLIMITED_K",
	"dimd : dim",
	"dim : IDENT",
	"vasection : /* empty */",
	"vasection : VARIABLES vadecls",
	"vadecls : vadecl ';'",
	"vadecls : vadecls vadecl ';'",
	"vadecl : vardecl",
	"vadecl : attdecl",
	"vardecl : type varlist",
	"type : BYTE_K",
	"type : CHAR_K",
	"type : SHORT_K",
	"type : LONG_K",
	"type : FLOAT_K",
	"type : DOUBLE_K",
	"varlist : varspec",
	"varlist : varlist ',' varspec",
	"varspec : var",
	"varspec : var dimspec",
	"var : IDENT",
	"dimspec : /* empty */",
	"dimspec : '(' dimlist ')'",
	"dimlist : vdim",
	"dimlist : dimlist ',' vdim",
	"vdim : dim",
	"attdecl : att",
	"attdecl : att '=' attvallist",
	"att : avar ':' attr",
	"att : ':' attr",
	"avar : var",
	"attr : IDENT",
	"attvallist : aconst",
	"attvallist : attvallist ',' aconst",
	"aconst : attconst",
	"attconst : CHAR_CONST",
	"attconst : TERMSTRING",
	"attconst : BYTE_CONST",
	"attconst : SHORT_CONST",
	"attconst : LONG_CONST",
	"attconst : FLOAT_CONST",
	"attconst : DOUBLE_CONST",
	"datasection : /* empty */",
	"datasection : DATA datadecls",
	"datadecls : datadecl ';'",
	"datadecls : datadecls datadecl ';'",
	"datadecl : avar",
	"datadecl : avar '=' constlist",
	"constlist : dconst",
	"constlist : constlist ',' dconst",
	"dconst : /* empty */",
	"dconst : const",
	"const : CHAR_CONST",
	"const : TERMSTRING",
	"const : BYTE_CONST",
	"const : SHORT_CONST",
	"const : LONG_CONST",
	"const : FLOAT_CONST",
	"const : DOUBLE_CONST",
};
#endif /* YYDEBUG */
#if !defined(lint) && !defined(__cplusplus)
static  char __yaccpar_sccsid1[] = "@(#) 9/3/92 yaccpar 6.11 Copyr 1991 Sun Micro";
#endif

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yynewmax * sizeof(type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()		yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**	If i<255, i itself is the token.  If i>255 but the neither 
**	of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
	int first = 0;
	int last = YYNMBCHARS - 1;
	int mid;
	wchar_t j;

	if(i&0x60000000){/*Must convert to a token. */
		if( yymbchars[last].character < i ){
			return i;/*Giving up*/
		}
		while ((last>=first)&&(first>=0)) {/*Binary search loop*/
			mid = (first+last)/2;
			j = yymbchars[mid].character;
			if( j==i ){/*Found*/ 
				return yymbchars[mid].tvalue;
			}else if( j<i ){
				first = mid + 1;
			}else{
				last = mid -1;
			}
		}
		/*No entry in the table.*/
		return i;/* Giving up.*/
	}else{/* i is already a token. */
		return i;
	}
}
#else/*!YYNMBCHARS*/
#define YYLEX()		yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
	hacks to please C++ and lint - goto's inside switch should never be
	executed; yypvt is set to 0 to avoid "used before set" warning.
*/
	static int __yaccpar_lint_hack__ = 0;
	switch (__yaccpar_lint_hack__)
	{
		case 1: goto yyerrlab;
		case 2: goto yynewstate;
	}
	yypvt = 0;
#endif

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
			yyerror("yacc initialization error");
			YYABORT;
		}
	}
#endif

	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */
	goto yystack;	/* moved from 6 lines above to here to please C++ */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			int yynewmax;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				char *newyys = (char *)YYNEW(int);
				char *newyyv = (char *)YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
				yyerror( "yacc stack overflow" );
				YYABORT;
			}
			yymaxdepth = yynewmax;

			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
			skip_init:
				yynerrs++;
				/* FALLTHRU */
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 103 "ncgen.y"
{ init_netcdf(); } break;
case 2:
# line 105 "ncgen.y"
{
		       if (ndims > MAX_NC_DIMS)
			 derror("Too many dimensions");
		   } break;
case 3:
# line 110 "ncgen.y"
{
		       if (derror_count == 0)
			 define_netcdf(netcdfname);
		   } break;
case 4:
# line 116 "ncgen.y"
{
		       if (derror_count == 0)
			 close_netcdf();
		   } break;
case 11:
# line 131 "ncgen.y"
{ if (long_val <= 0)
			 derror("negative dimension size");
		     dims[ndims].size = long_val;
		     ndims++;
		   } break;
case 12:
# line 137 "ncgen.y"
{  if (rec_dim != -1)
			 derror("only one NC_UNLIMITED dimension allowed");
		     rec_dim = ndims; /* the unlimited (record) dimension */
		     dims[ndims].size = NC_UNLIMITED;
		     ndims++;
		   } break;
case 13:
# line 145 "ncgen.y"
{ if (yypvt[-0]->is_dim == 1) {
		        derror( "duplicate dimension declaration for %s",
		                yypvt[-0]->name);
		     }
	             yypvt[-0]->is_dim = 1;
		     yypvt[-0]->dnum = ndims;
		     dims[ndims].name = (char *) emalloc(strlen(yypvt[-0]->name)+1);
		     (void) strcpy(dims[ndims].name, yypvt[-0]->name);
		   } break;
case 22:
# line 167 "ncgen.y"
{ type_code = NC_BYTE; } break;
case 23:
# line 168 "ncgen.y"
{ type_code = NC_CHAR; } break;
case 24:
# line 169 "ncgen.y"
{ type_code = NC_SHORT; } break;
case 25:
# line 170 "ncgen.y"
{ type_code = NC_LONG; } break;
case 26:
# line 171 "ncgen.y"
{ type_code = NC_FLOAT; } break;
case 27:
# line 172 "ncgen.y"
{ type_code = NC_DOUBLE; } break;
case 30:
# line 178 "ncgen.y"
{
		    if (nvars >= MAX_NC_VARS)
		       derror("too many variables");
		    nvdims = 0;
		    /* make sure variable not re-declared */
		    if (yypvt[-0]->is_var == 1) {
		       derror( "duplicate variable declaration for %s",
		               yypvt[-0]->name);
		    }
	            yypvt[-0]->is_var = 1;
		    yypvt[-0]->vnum = nvars;
		    vars[nvars].name = (char *) emalloc(strlen(yypvt[-0]->name)+1);
		    (void) strcpy(vars[nvars].name, yypvt[-0]->name);
		    vars[nvars].type = type_code;
		    /* set default fill value.  You can override this with
		     * the variable attribute "_FillValue". */
		    nc_getfill(type_code, &vars[nvars].fill_value);
		    vars[nvars].has_data = 0; /* has no data (yet) */
		   } break;
case 31:
# line 198 "ncgen.y"
{
		    vars[nvars].ndims = nvdims;
		    nvars++;
		   } break;
case 37:
# line 212 "ncgen.y"
{
		    if (nvdims >= MAX_VAR_DIMS) {
		       derror("%s has too many dimensions",vars[nvars].name);
		    }
		    if (yypvt[-0]->is_dim == 1)
		       dimnum = yypvt[-0]->dnum;
		    else {
		       derror( "%s is not declared as a dimension",
			       yypvt[-0]->name);
	               dimnum = ndims;
		    }
		    if (rec_dim != -1 && dimnum == rec_dim && nvdims != 0) {
		       derror("unlimited dimension must be first");
		    }
		    vars[nvars].dims[nvdims] = dimnum;
                    nvdims++;
		   } break;
case 38:
# line 231 "ncgen.y"
{
		       valnum = 0;
		       valtype = NC_UNSPECIFIED;
		       /* get a large block for attributes, realloc later */
		       att_space = emalloc(MAX_NC_ATTSIZE);
		       /* make all kinds of pointers point to it */
		       char_valp = (char *) att_space;
		       byte_valp = (char *) att_space;
		       short_valp = (short *) att_space;
		       long_valp = (long *) att_space;
		       float_valp = (float *) att_space;
		       double_valp = (double *) att_space;
		   } break;
case 39:
# line 245 "ncgen.y"
{
		       if (natts >= MAX_NC_ATTS)
			 derror("too many attributes");
		       atts[natts].var = varnum ;
		       atts[natts].type = valtype;
		       atts[natts].len = valnum;
		       /* shrink space down to what was really needed */
		       att_space = erealloc(att_space, valnum*nctypelen(valtype));
		       atts[natts].val = att_space;
		       if (STREQ(atts[natts].name, _FillValue)) {
			   nc_putfill(atts[natts].type,
				       atts[natts].val,
				       &vars[atts[natts].var].fill_value);
		       }
		       natts++;
		   } break;
case 41:
# line 264 "ncgen.y"
{
		    varnum = -1;  /* handle of "global" attribute */
		   } break;
case 42:
# line 270 "ncgen.y"
{ if (yypvt[-0]->is_var == 1)
		       varnum = yypvt[-0]->vnum;
		    else {
		      derror("%s not declared as a variable, fatal error",
			     yypvt[-0]->name);
		      YYABORT;
		      }
		   } break;
case 43:
# line 280 "ncgen.y"
{
		       atts[natts].name = (char *) emalloc(strlen(yypvt[-0]->name)+1);
		       (void) strcpy(atts[natts].name,yypvt[-0]->name);
		   } break;
case 46:
# line 289 "ncgen.y"
{
		    if (valtype == NC_UNSPECIFIED)
		      valtype = atype_code;
		    if (valtype != atype_code)
		      derror("values for attribute must be all of same type");
		   } break;
case 47:
# line 298 "ncgen.y"
{
		       atype_code = NC_CHAR;
		       *char_valp++ = char_val;
		       valnum++;
		   } break;
case 48:
# line 304 "ncgen.y"
{
		       atype_code = NC_CHAR;
		       {
			   /* don't null-terminate attribute strings */
			   int len = strlen(termstring);
			   valnum += len;
			   (void)strncpy(char_valp,termstring,len);
			   char_valp += len;
		       }
		   } break;
case 49:
# line 315 "ncgen.y"
{
		       atype_code = NC_BYTE;
		       *byte_valp++ = byte_val;
		       valnum++;
		   } break;
case 50:
# line 321 "ncgen.y"
{
		       atype_code = NC_SHORT;
		       *short_valp++ = short_val;
		       valnum++;
		   } break;
case 51:
# line 327 "ncgen.y"
{
		       atype_code = NC_LONG;
		       *long_valp++ = long_val;
		       valnum++;
		   } break;
case 52:
# line 333 "ncgen.y"
{
		       atype_code = NC_FLOAT;
		       *float_valp++ = float_val;
		       valnum++;
		   } break;
case 53:
# line 339 "ncgen.y"
{
		       atype_code = NC_DOUBLE;
		       *double_valp++ = double_val;
		       valnum++;
		   } break;
case 58:
# line 354 "ncgen.y"
{
		       valtype = vars[varnum].type; /* variable type */
		       valnum = 0;	/* values accumulated for variable */
		       vars[varnum].has_data = 1;
		       /* compute dimensions product (size of a "record") */
		       var_size = nctypelen(valtype);
		       if (vars[varnum].ndims == 0)
			   var_len = 1;
		       else if (vars[varnum].dims[0] == rec_dim) {
			   var_len = 1; /* one record for unlimited vars */
			   netcdf_record_number = 0;
		       }
		       else
			 var_len = dims[vars[varnum].dims[0]].size;
		       for(dimnum = 1; dimnum < vars[varnum].ndims; dimnum++)
			 var_len = var_len*dims[vars[varnum].dims[dimnum]].size;
		       /* allocate memory for a record of variable data */
		       if (var_len*var_size != (unsigned)(var_len*var_size)) {
			   derror("too much data for this machine");
			   exit(9);
		       }
		       rec_start = malloc ((unsigned)(var_len*var_size));
		       if (rec_start == 0) {
			   derror ("out of memory\n");
			   exit(3);
		       }
		       rec_cur = rec_start;
		       switch (valtype) {
			 case NC_CHAR:
			   char_valp = (char *) rec_start;
			   break;
			 case NC_BYTE:
			   byte_valp = (char *) rec_start;
			   break;
			 case NC_SHORT:
			   short_valp = (short *) rec_start;
			   break;
			 case NC_LONG:
			   long_valp = (long *) rec_start;
			   break;
			 case NC_FLOAT:
			   float_valp = (float *) rec_start;
			   break;
			 case NC_DOUBLE:
			   double_valp = (double *) rec_start;
			   break;
		       }
		 } break;
case 59:
# line 403 "ncgen.y"
{
		       if (valnum > 0 && valnum < var_len) { /* leftovers */
			   nc_fill(valtype,
				    var_len - valnum,
				    rec_cur,
				    vars[varnum].fill_value);
			   /* put out record of var_len values */
			   if (derror_count == 0)
			     put_variable(rec_start);
		       }
		       free ((char *) rec_start);
		 } break;
case 62:
# line 420 "ncgen.y"
{
		       if(valnum >= var_len) {
			   derror("too many values for this variable");
			   exit (4);
		       }
		       not_a_string = 1;
                   } break;
case 63:
# line 428 "ncgen.y"
{
		       if (not_a_string) {
			   switch (valtype) {
			     case NC_CHAR:
			       rec_cur = (void *) char_valp;
			       break;
			     case NC_BYTE:
			       rec_cur = (void *) byte_valp;
			       break;
			     case NC_SHORT:
			       rec_cur = (void *) short_valp;
			       break;
			     case NC_LONG:
			       rec_cur = (void *) long_valp;
			       break;
			     case NC_FLOAT:
			       rec_cur = (void *) float_valp;
			       break;
			     case NC_DOUBLE:
			       rec_cur = (void *) double_valp;
			       break;
			   }
		       }
		       if (valnum >= var_len) {
			   /* put out record of var_len elements */
			   if (derror_count == 0)
			     put_variable(rec_start);
			   /* if this variable is unbounded, reset for */
			   /* next record */
			   if (vars[varnum].dims[0] == rec_dim) {
			       valnum = 0;
			       netcdf_record_number++;
			       rec_cur = rec_start;
			       switch (valtype) {
				 case NC_CHAR:
				   char_valp = (char *) rec_start;
				   break;
				 case NC_BYTE:
				   byte_valp = (char *) rec_start;
				   break;
				 case NC_SHORT:
				   short_valp = (short *) rec_start;
				   break;
				 case NC_LONG:
				   long_valp = (long *) rec_start;
				   break;
				 case NC_FLOAT:
				   float_valp = (float *) rec_start;
				   break;
				 case NC_DOUBLE:
				   double_valp = (double *) rec_start;
				   break;
			       }
			   }
		       }
		 } break;
case 64:
# line 487 "ncgen.y"
{
		       atype_code = NC_CHAR;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = char_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = char_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = char_val;
			   break;
			 case NC_LONG:
			   *long_valp++ = char_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = char_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = char_val;
			   break;
		       }
		       valnum++;
		   } break;
case 65:
# line 512 "ncgen.y"
{
		       not_a_string = 0;
		       atype_code = NC_CHAR;
		       {
			   int len = strlen(termstring);

			   valnum += len;
			   if(valnum > var_len) {
			       derror("string won't fit in this variable");
			       exit (5);
			   }
			   switch (valtype) {
			     case NC_CHAR:
			       (void)strncpy(char_valp,termstring,len);
			       char_valp += len;
			       rec_cur = (void *) char_valp;
			       break;
			     case NC_BYTE:
			       (void)strncpy(byte_valp,termstring,len);
			       byte_valp += len;
			       rec_cur = (void *) byte_valp;
			       break;
			     case NC_SHORT:
			     case NC_LONG:
			     case NC_FLOAT:
			     case NC_DOUBLE:
			       derror("string value invalid for %s variable",
				      nctype(valtype));
			       break;
			   }
		       }
		   } break;
case 66:
# line 545 "ncgen.y"
{
		       atype_code = NC_BYTE;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = byte_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = byte_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = byte_val;
			   break;
			 case NC_LONG:
			   *long_valp++ = byte_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = byte_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = byte_val;
			   break;
		       }
		       valnum++;
		   } break;
case 67:
# line 570 "ncgen.y"
{
		       atype_code = NC_SHORT;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = short_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = short_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = short_val;
			   break;
			 case NC_LONG:
			   *long_valp++ = short_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = short_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = short_val;
			   break;
		       }
		       valnum++;
		   } break;
case 68:
# line 595 "ncgen.y"
{
		       atype_code = NC_LONG;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = long_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = long_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = long_val;
			   break;
			 case NC_LONG:
			   *long_valp++ = long_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = long_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = long_val;
			   break;
		       }
		       valnum++;
		   } break;
case 69:
# line 620 "ncgen.y"
{
		       atype_code = NC_FLOAT;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = float_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = float_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = float_val;
			   break;
			 case NC_LONG:
			   *long_valp++ = float_val;
			   break;
			 case NC_FLOAT:
			   *float_valp++ = float_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = float_val;
			   break;
		       }
		       valnum++;
		   } break;
case 70:
# line 645 "ncgen.y"
{
		       atype_code = NC_DOUBLE;
		       switch (valtype) {
			 case NC_CHAR:
			   *char_valp++ = double_val;
			   break;
			 case NC_BYTE:
			   *byte_valp++ = double_val;
			   break;
			 case NC_SHORT:
			   *short_valp++ = double_val;
			   break;
			 case NC_LONG:
			   *long_valp++ = double_val;
			   break;
			 case NC_FLOAT:
			   if (double_val == FILL_DOUBLE)
			     *float_valp++ = FILL_FLOAT;
			   else
			     *float_valp++ = double_val;
			   break;
			 case NC_DOUBLE:
			   *double_valp++ = double_val;
			   break;
		       }
		       valnum++;
		   } break;
	}
	goto yystack;		/* reset registers in driver code */
}

