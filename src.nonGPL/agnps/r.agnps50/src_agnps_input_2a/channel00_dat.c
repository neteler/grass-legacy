
/*   This GRASS AGNPS interface is used for user to */
/*   input fertilizer application rate and the      */
/*   fertilizer availability factors in a format    */
/*   compatible with the latest version of          */ 
/*   AGNPS-- AGNPS5.0(including AGNPS4.03).         */
/*                                                  */
/*   This function is used to allow the user to     */
/*   enter channel data for TR-55/Nongeomorphic     */
/*   AGNPS calculation.                             */
/*                                                  */
/*                        Zhian  Li                 */
/*                                                  */
/*                        Penn State University     */
/*                                                  */
/*                        August, 1995              */ 

#include "agnps_input.h"

channel00_dat(int cell_number)
{

        char	buf[1024];
        char    buf1[15];
        char    str[50];

        chlwc    = 3.425;
        chlwe    = 0.3151;
        chldc    = 0.4537;
        chlde    = 0.2192;
        chllnthc = 153.0;
        chllnthe = 0.6;
        chlm     = 0.103;

        if (cell_number == 0)
         { chlw       = 0.0;
           chld       = 10.0;
           chllnth    = 0.0;
           chlslope   = 0;
           chl_type   = 2;
           chl_cel_id = 0;
         }
        else
         { chlw       = cel[cell_number].chlw;
           chld       = cel[cell_number].chld;
           chllnth    = cel[cell_number].chllnth;
           chlslope   = cel[cell_number].chl_slope;
           chl_type   = cel[cell_number].chl_indicator;
           chl_cel_id = cell_number;
         }

          strcpy(str, "Channel Information ");
          if (geom_mod) strcat(str, "(Geomorphic Option)");
          else          strcat(str, "(Nongeomorphic Option)");

          V_clear();
          V_line(1,  "Cell number ---------->");
          V_line(3,  str);
          V_line(4,  "==========================================");
          V_line(5,  "Channel type (1-8):");
          V_line(6,  "   1.  No definitive channel");
          V_line(7,  "   2.  Drainage ditch");
          V_line(8,  "   3.  Road ditch");
          V_line(9,  "   4.  Grass waterway");
          V_line(10, "   5.  Ephemeral stream");
          V_line(11, "   6.  Intermittent stream");
          V_line(12, "   7.  Perennial stream");
          V_line(13, "   8.  User defined channel");
          V_line(15, "Channel characteristics:");
          V_line(16, "   1.  Channel width");
          V_line(17, "   2.  Channel depth");
          V_line(18, "   3.  Channel length (ft.)");
          V_line(19, "   4.  Channel slope");
          V_line(20, "   5.  Channel Manning coefficient");

          V_const(&chl_cel_id,'i',1,40,5);
          V_ques(&chl_type,'i',5,40,5);
          V_ques(&chlw,'f',16,40,5);
          V_ques(&chld,'f',17,40,5);
          V_ques(&chllnth,'f',18,40,5);
          V_ques(&chlslope,'f',19,40,5);
          V_ques(&chlm,'f',20,40,5);


          V_intrpt_ok();
          if(!V_call()) exit(1);




}
