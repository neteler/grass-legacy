/* sos/rho1_num.c */

/*--------------------------------------------------------------------------*/

#include "geom/basic.h"
#include "geom/sos.h"
#include "internal.h"
#include "primitive.h"

/*--------------------------------------------------------------------------*/

Static_Declarations ("rho1_num", 3, 2, 2);

#include "../sos/primitive.i.c"

/*--------------------------------------------------------------------------*/

SoS_primitive_result * sos_rho1_num (i, j)
     int i, j;
     /* Returns significant term of Rho1 numerator. */
{
#ifdef __DEBUG__
  if (sos_proto_e_flag)
    {
      lia_clear ();
      print ("sos_rho1_num (%d,%d)", i, j);
      print (" (");
      print ("%s,%s,%s;", Pi(i,1), Pi(i,2), Pi(i,3));
      print ("%s,%s,%s;", Pi(j,1), Pi(j,2), Pi(j,3));
      print (")\n");
    }
#endif
/* C code generated by 'ccode' from 'gee' file "Rho1.num" */
Initialize ();
Epsilon_Term (0);
Push (Minor2 (i, j, 1, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 2, 0));
Push (Integer (2));
Power ();
Push (Minor2 (i, j, 3, 0));
Push (Integer (2));
Power ();
Plus ();
Plus ();
Coefficient (Pop ());
Epsilon_Term (1);
Epsilon (i,3);
Push (Integer (2));
Push (Minor2 (i, j, 3, 0));
Times ();
Coefficient (Pop ());
Epsilon_Term (2);
Epsilon (i,3);
Epsilon (i,3);
Push (Integer (1));
Coefficient (Pop ());
Finish ();
}
