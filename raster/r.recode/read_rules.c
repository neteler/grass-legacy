#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "global.h"

#define INCR 20

int report_range (void)
{
   struct FPRange drange;
   struct Range range;
   char buff[1024], buff2[300];
   RASTER_MAP_TYPE inp_type;

  inp_type = G_raster_map_type(name, mapset);
  if(inp_type != CELL_TYPE)
  {
     if(G_read_fp_range(name, mapset, &drange)<=0)
     {
        sprintf(buff, "Can't read f_range for map %s", name);
        G_fatal_error(buff);
     }
     G_get_fp_range_min_max(&drange, &old_dmin, &old_dmax);
     if(G_is_d_null_value(&old_dmin) || G_is_d_null_value(&old_dmax))
         fprintf (stdout,"Data range is empty\n");
     else
     {
         sprintf(buff, "%.10f",old_dmin);
         sprintf(buff2, "%.10f",old_dmax);
         G_trim_decimal(buff);
         G_trim_decimal(buff2);
         fprintf (stdout,"Data range of %s is %s to %s (entire map)\n", name, buff, buff2);
      }
  }
  if(G_read_range(name, mapset, &range)<=0)
  {
     sprintf(buff, "Can't read range for map %s", name);
     G_fatal_error(buff);
  }
  G_get_range_min_max(&range, &old_min, &old_max);
  if(G_is_c_null_value(&old_min) || G_is_c_null_value(&old_max))
      fprintf (stdout,"Integer data range of %s is empty\n", name);
  else
      fprintf (stdout,"Integer data range of %s is %d to %d\n", 
		   name, (int)old_min, (int)old_max);

   return 0;
}

int read_rules (void)
{
   char buf[1024];
   DCELL oLow, oHigh, nLow, nHigh;
   int line, n;

   in_type = out_type = CELL_TYPE;

   rules = (char**) G_malloc(INCR * sizeof(char *));
   rule_size = INCR;

   if(isatty(0))
   {
      report_range();
      fprintf (stdout,"\nEnter the rule or 'help' for the format description:\n");
   }
   G_fpreclass_init(&rcl_struct);
   for (line=1;;line++)
   {
      if (isatty(0)) fprintf (stdout,"> ");
      if (!fgets(buf,1024,stdin)) return nrules;
      buf[strlen(buf)-1] = '\0';
      for (n=0;buf[n];n++)
	   if(buf[n] == ',')
	      buf[n] = ' ';
      G_strip(buf);
      if (*buf == 0) continue;
      if (*buf == '#') continue;
      if (strcmp(buf, "end") == 0) break;

      if (strcmp(buf, "help") == 0)
      {
	  fprintf (stdout,"Enter a rule in one of these formats:\n");
	  fprintf (stdout,"old_low:old_high:new_low:new_high\n");
	  fprintf (stdout,"old_low:old_high:new_val      (i.e. new_high == new_low)\n");
	  fprintf (stdout,"*:old_val:new_val             (interval [inf, old_val])\n");
	  fprintf (stdout,"old_val:*:new_val             (interval [old_val, inf])\n");
	  continue;
      }	       

      /* we read and record into quant table all values, even int as doubles
	 we convert the range and domain values to the right format when we 
	 lookup the values in the quant table */
      switch (sscanf (buf, "%lf:%lf:%lf:%lf", &oLow, &oHigh, &nLow, &nHigh))
      {
        case 3:  update_type(&in_type, oLow);
		 update_type(&in_type, oHigh);
		 update_type(&out_type, nLow);
	         update_rules(buf);
                 G_fpreclass_add_rule(&rcl_struct, oLow, oHigh, nLow, nLow);
		 break;

        case 4:	 update_type(&in_type, oLow);
		 update_type(&in_type, oHigh);
		 update_type(&out_type, nLow);
		 update_type(&out_type, nHigh);
	         update_rules(buf);
                 G_fpreclass_add_rule(&rcl_struct, oLow, oHigh, nLow, nHigh);
		 break;

        default:
	    if(sscanf (buf, "%lf:*:%lf", &oLow, &nLow)==2)
	    {
	       update_type(&in_type, oLow);
	       update_type(&out_type, nLow);
	       update_rules(buf);
	       G_fpreclass_set_pos_infinite_rule(&rcl_struct, oLow, nLow);
            }
	    else if(sscanf (buf, "*:%lf:%lf", &oHigh, &nLow)==2) 
	    {
	       update_type(&in_type, oHigh);
	       update_type(&out_type, nLow);
	       update_rules(buf);
	       G_fpreclass_set_neg_infinite_rule(&rcl_struct, oHigh, nLow);
            }
	    else
	       fprintf (stdout,"%s is not a valid rule\n", buf);
	    break;
      } /* switch */
    } /* loop */
  return nrules;
}

int 
update_type (RASTER_MAP_TYPE *map_type, DCELL val)

{
   /* check if val is not an integer number */
	if (make_dcell) 
		*map_type = DCELL_TYPE;
	else {
   if((DCELL) ((CELL) val) != val) 
       *map_type = FCELL_TYPE;
	}
   return 0;
}

int update_rules(char *buf)
{
   int buf_size;

   buf_size = strlen(buf) +1; /* +1 for null char */
   if(rule_size <= nrules)
   {
      while(rule_size <= nrules)
         rule_size += INCR;
      rules = (char**) G_realloc(rules, rule_size * sizeof(char *));
   }
   rules[nrules] = (char *) G_malloc(sizeof(char) * (buf_size + 1));
   strncpy(rules[nrules], buf, buf_size);
   nrules++;

   return 0;
}
