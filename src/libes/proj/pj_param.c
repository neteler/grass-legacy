/* put parameters in linked list and retrieve */
#ifndef lint
static const char SCCSID[]="@(#)pj_param.c	4.4	93/06/12	GIE	REL";
#endif
#include <projects.h>
#include <stdio.h>
#include <string.h>
	paralist * /* create parameter list entry */
pj_mkparam(char *str) {
	paralist *new;

	if (new = (paralist *)pj_malloc(sizeof(paralist) + strlen(str))) {
		new->used = 0;
		new->next = 0;
		if (*str == '+')
			++str;
		(void)strcpy(new->param, str);
	}
	return new;
}
	PVALUE /* test for presence or get parameter value */
pj_param(paralist *pl, char *opt) {
	int type;
	unsigned l;
	PVALUE value;

	type = *opt++;
	/* simple linear lookup */
	l = strlen(opt);
	while (pl && !(!strncmp(pl->param, opt, l) &&
	  (!pl->param[l] || pl->param[l] == '=')))
		pl = pl->next;
	if (type == 't')
		value.i = pl != 0;
	else if (pl) {
		pl->used |= 1;
		opt = pl->param + l;
		if (*opt == '=')
			++opt;
		switch (type) {
		case 'i':	/* integer input */
			value.i = atoi(opt);
			break;
		case 'd':	/* simple real input */
			value.f = atof(opt);
			break;
		case 'r':	/* degrees input */
			value.f = dmstor(opt, 0);
			break;
		case 's':	/* char string */
			value.s = opt;
			break;
		case 'b':	/* boolean */
			switch (*opt) {
			case 'F': case 'f':
				value.i = 0;
				break;
			case '\0': case 'T': case 't':
				value.i = 1;
				break;
			default:
				pj_errno = -8;
				value.i = 0;
				break;
			}
			break;
		default:
bum_type:	/* note: this is an error in parameter, not a user error */
			fprintf(stderr, "invalid request to pj_param, fatal\n");
			exit(1);
		}
	} else /* not given */
		switch (type) {
		case 'b':
		case 'i':
			value.i = 0;
			break;
		case 'd':
		case 'r':
			value.f = 0.;
			break;
		case 's':
			value.s = 0;
			break;
		default:
			goto bum_type;
		}
	return value;
}
