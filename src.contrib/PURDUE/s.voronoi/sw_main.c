#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sw_defs.h"

int main (int argc, char **argv)
{	
int c;
struct Site *(*next)();

sorted = 0; triangulate = 0; plot = 0; debug = 0;
while((c=getopt(argc,argv,"dpst")) != EOF)
	switch(c) {
	case 'd': debug = 1;
		  break;
	case 's': sorted = 1;
		  break;
	case 't': triangulate = 1;
		  break;
	case 'p': plot = 1;
		  break;
		  };

freeinit(&sfl, sizeof *sites);
if(sorted)
{	scanf("%d %f %f %f %f", &nsites, &xmin, &xmax, &ymin, &ymax);
	next = readone;
}
else 
{	readsites();
	next = nextone;
};

siteidx = 0;
geominit();
if(plot) plotinit();

voronoi(triangulate, next); 

return 0;
}

/* sort sites on y, then x, coord */
int 
scomp (const void *ss1, const void *ss2)
{
	const struct Point *s1 = ss1, *s2 = ss2;
	if(s1 -> y < s2 -> y) return(-1);
	if(s1 -> y > s2 -> y) return(1);
	if(s1 -> x < s2 -> x) return(-1);
	if(s1 -> x > s2 -> x) return(1);
	return(0);
}

/* return a single in-storage site */
struct Site *
nextone (void)
{
struct Site *s;
if(siteidx < nsites)
{	s = &sites[siteidx];
	siteidx += 1;
	return(s);
}
else	return( (struct Site *)NULL);
}


/* read all sites, sort, and compute xmin, xmax, ymin, ymax */
int 
readsites (void)
{
int i;

nsites=0;
sites = (struct Site *) myalloc(4000*sizeof *sites);
while(scanf("%f %f", &sites[nsites].coord.x, &sites[nsites].coord.y)!=EOF)
{	sites[nsites].sitenbr = nsites;
	sites[nsites].refcnt = 0;
	nsites += 1;
	if (nsites % 4000 == 0) 
		sites = (struct Site *) realloc(sites,(nsites+4000)*sizeof*sites);
};
qsort(sites, nsites, sizeof *sites, scomp);
xmin=sites[0].coord.x; 
xmax=sites[0].coord.x;
for(i=1; i<nsites; i+=1)
{	if(sites[i].coord.x < xmin) xmin = sites[i].coord.x;
	if(sites[i].coord.x > xmax) xmax = sites[i].coord.x;
};
ymin = sites[0].coord.y;
ymax = sites[nsites-1].coord.y;
return 0;
}

/* read one site */
struct Site *
readone (void)
{
struct Site *s;

s = (struct Site *) getfree(&sfl);
s -> refcnt = 0;
s -> sitenbr = siteidx;
siteidx += 1;
if(scanf("%f %f", &(s->coord.x), &(s->coord.y)) == EOF)
	return ((struct Site *) NULL );
return(s);
}
