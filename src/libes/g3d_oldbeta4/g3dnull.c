#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "G3d_intern.h"

/*---------------------------------------------------------------------------*/

int
G3d_isNullValueNum (n, type)

     void *n;
     int type;

{
  if (type == G3D_FLOAT) 
    return G_is_f_null_value (n);
  else
    return G_is_d_null_value (n);
}

/*---------------------------------------------------------------------------*/

void
G3d_setNullValue (c, nofElts, type)

     void *c;
     int nofElts, type;

{
  if (type == G3D_FLOAT) {
    G_set_f_null_value ((float *) c, nofElts);
    return;
  }
    
  G_set_d_null_value ((double *) c, nofElts);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
