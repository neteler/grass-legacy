/* Structure for binary records */

typedef struct
{
 float application_rate;
 float application_efficiency;
 float canopy_cover;
 float incorporation_depth;
 float time_since_application;
 float foliar_residue_half_life;
 float remaining_pesticide_on_plant;
 float soil_residue_half_life;
 float foliar_washoff_fraction;
 float organic_carbon_sorption;
 float cell_area;
 float incorporation_efficiency;
 float foliar_washoff_threshold;
 float organic_matter;
 int   column_number;
 int   soil_type;
 float bulk_density;
 float porosity;
 float surf_storage;
 float enrich_ratio;
 float tf;
 float sediment;
 float Kd;
 float B_value;
 float effpest;
 float pestplant;
 float initpestplant;
 float total_after_app;
 float total_sed_pest;
 float pestsoil;
 float initpestsoil;
 float init_pestsoil_ppm;
 float init_pestsoil_after;
 float totpestplant;
 float totpestsoil;
 float rainfall;
 float runoff;
 float washoff;
 float comb_pestsoil;
 float efi;
 float Cav;
 float percolated;
 float percolated_per;
 float soluble_con;
 float soluble_after;
 float soluble_pest;
 float soluble_pest_per;
 float soil_con;
 float sediment_con;
 float sediment_pest_per;
 float solubility;
 float sediment_pest;
 float sediment_lbs;
 float soluble_lbs;
}*PEST_REC_PTR, PEST_REC;

typedef struct
{
  int   column;
 float initial_sediment_portion;
float sed_avail[6];
float avail_sed[6];
float sediment_avail[6];
float percent_flowing_out[6];
float sed_yield[6];
float percent_surface[6];
float pesticide_yield[6];
  float enrich_ratio;
  float sediment_yield;
  int   soil_type;
  float tf;
  float Kd_adjust;
  float Kd;
  float water_lbs;
  float water;
  int   receiving_cell_num;
  int   receiving_cell_div;
  float sediment_portion;
  float water_before;
  float sediment;
  float total_pesticide;
  float water_con_before;
  float solubility;
  float water_con_after;
  float water_portion_after;
}PEST_ROUTE_REC, *PEST_ROUTE_REC_PTR;

/* Hydrology records for source analysis */



typedef struct
{
int   temp_receiving_cell;
int   column_number;
int   impoundments;
float curve_number;
float storm_rainfall;
float retention_factor;
float equation_top;
float equation_bottom;
float runoff;
int   calc_method;
float slope;
float surface_condition;
float power;
float flow_velocity;
float flow_velocity_after;
int   slope_length;
float overland_flow_duration;
float overland_mannings;
float time_overland;
float area;
float cell_width;
int   primary_cell;
float shallow_length;
float velocity_shallow;
float velocity_shallow_after;
float time_shallow;
int   geomorphic_calc;
int   channel_indicator;
int   soil_type;
float depth;
float width;
float bank_depth;
float channel_side_slope;
float channel_side_slope_after;
float channel_slope;
float water_slope;
float water_depth;
float water_width;
float water_bank_depth;
float no_slope;
float no_depth;
float no_width;
float depth_coef;
float depth_exp;
float width_coef;
float width_exp;
}HYDRO_REC, *HYDRO_REC_PTR;



typedef struct
{
int   column;
float drainage_area_above;
float accumulated_drainage_area;
float area;
float runoff_volume_above;
float accumulated_runoff_volume_above;
float time_to_peak_in;
float K_triangle;
float runoff_flow_above;
float drainage_area_below;
float area_not_terraced;
float runoff_volume_below;
float cell_run_off;
int   calc_method;
float length_slope;
float sum_of_lengths;
float term1;
float term2;
float channel_length;
float cn_area;
float tr55_length;
SOURCEPTR source_list;
float slope_length;
float shallow_length;
float overland_mannings;
float storm_rainfall;
float average_land_slope;
float time_overland;
float velocity_shallow;
float velocity_shallow_after;
float time_shallow;
float channel_indicator;
float soil_type;
float channel_width;
float max_depth;
float r;
float channel_depth;
float channel_velocity;
float time_concentrated;
float total_time;
float time_previous;
float temp_cn;
float initial_abstraction;
float iap;
float iap_after;
char  storm_type[4];
float c0;
float c1;
float c2;
float temp1;
float temp2;
float temp3;
float temp;
float qu;
float s;
float q;
float peak_flow;
float max_depth_new;
float channel_side_slope;
float sum_psource_volumes;
float new_runoff_volume_below;
float flow_rate_below;
float sum_psource_flows;
float adjusted_flow_rate_below;
float time_to_peak_out;
float time_to_peak_ave;
float prepeak_RO_fraction;
float duration;
float longest_duration;
float new_time_to_peak;
float water_flow_rate_below;
float water_flow_rate_below_after;
float new_channel_side_slope;
float new_channel_slope;
float new_bank_depth;
float base_increment_duration;
float total_vol_below;
float max_increments;
float point_source_flow_top;
float point_source_flow_bottom;
int   increment[8];
float ave_incr_upflow_rate[8];
float ave_incr_downflow_rate[8];
float total_incremental_upflow_rate[8];
float total_incremental_downflow_rate[8];
float new_downflow_rate[8];
int   num_psources;
int   num_imp;
float total_psource_flow;
float total_psource_volume;
}HYDRO_ROUTE_REC, *HYDRO_ROUTE_REC_PTR;

typedef struct POINT_SOURCE
{
int   enter_at_top;
float water_discharge;
float volume;
float total_cell_flow_rate_top;
float total_cell_flow_rate_bottom;
float total_cell_volume_top;
float total_cell_volume_bottom;
struct POINT_SOURCE *next;
}POINT_SOURCE_REC, *POINT_SOURCE_REC_PTR;

typedef struct IMPOUND_ROUTE
{
float volume_runoff;
float peak_flow;
float duration;
int   number_increments;
int   receiving_cell_position;
float left_over_volume;
struct IMPOUND_ROUTE *next;
}IMPOUND_ROUTE_REC, *IMPOUND_REC_PTR;

typedef struct IMPOUND
{
float drainage_area;
float pipe_diameter;
float drainage_area_after;
float pipe_diameter_after;
float cell_runoff;
float impoundment_runoff;
float depth;
int   pond_area_coeff;
float b;
float cor;
float peak_flow;
struct IMPOUND *next;
}HYDRO_IMP_REC, *HYDRO_IMP_REC_PTR;
