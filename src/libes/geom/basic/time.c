/* basic/time.c ---  Encapsulation to get system time and other goodies. */

/*--------------------------------------------------------------------------*/

#include "config.h"

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "geom/basic.h"
#include <sys/times.h>
#include <sys/param.h>

#define STRLEN  128

/*--------------------------------------------------------------------------*/

double basic_utime (void)
     /* Returns user seconds elapsed since start of process. */
{
  /* struct tms is defined in <sys/times.h> (see man times); tms values are
     in "clock ticks per second", ie, HZ, which is defined in <sys/param.h> */
  struct tms buffer;
  (void) times (&buffer);
  return (((double) buffer.tms_utime) / ((double) HZ));
}

/*--------------------------------------------------------------------------*/

char *basic_hostname (void)
     /* Returns pointer to hostname. */
{
  static char hname[STRLEN];
  (void) gethostname (hname, STRLEN);
  return (hname);
}

/*--------------------------------------------------------------------------*/

char *basic_date (void)
     /* Gives date and time information in ASCII. */
{
  static char datime[STRLEN];
  time_t clock;
  char * ptr;
  int i;
  clock = time ((time_t *) 0);
  ptr = ctime (&clock);
  i = 1;
  while (*ptr and (i < STRLEN))
    {
      datime[i-1] = *ptr;
      i ++;
      ptr ++;
    }
  datime[i-1] = *ptr;
  if (datime[i-2] == '\n')
    datime[i-2] = 0;
  return (datime);
}

/*--------------------------------------------------------------------------*/

void basic_daytime (
    int *sec,
    int *micros,
    int *minwest,
    int *dst  /* all output */
)
     /* See %man gettimeofday  and  /usr/include/sys/time.h  for more info.
        Note that tv_sec and tv_usec are casted to int. */
{
  struct timeval tp;
  struct timezone tzp;
  (void) gettimeofday (&tp, &tzp);
  *sec = (int) tp.tv_sec;
  *micros = (int) tp.tv_usec;
  *minwest = tzp.tz_minuteswest;
  *dst = tzp.tz_dsttime;
}

/*--------------------------------------------------------------------------*/

double basic_seconds (void)
     /* Returns seconds elapsed since 00:00 GMT, Jan 1, 1970. */
{
  int seconds, microseconds, w, d;
  basic_daytime (&seconds, &microseconds, &w, &d);
  return ((double) seconds + microseconds / 1000000.0);
}

/*--------------------------------------------------------------------------*/

int basic_seed (void)
     /* Returns some int seed for random genertors, */
{
  int seconds, microseconds, w, d;
  basic_daytime (&seconds, &microseconds, &w, &d);
  return (seconds);
}
