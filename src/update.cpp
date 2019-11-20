#include "settings.h"
#include "app_data.h"
#include <iostream>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void update_widgets_from_settings(void* data_) {
  app_data* data = (app_data*) data_;  
  camera_settings current = data->all_settings[data->index];
  if (current.iscamera1) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->widgets.camera1_check), TRUE);
  }
  else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->widgets.camera1_check), FALSE);
  }  

  if (current.iscamera2) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->widgets.camera2_check), TRUE);
  }
  else {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->widgets.camera2_check), FALSE);
  }

  gtk_range_set_value(GTK_RANGE(data->widgets.h_min_scale), current.h_min);
  gtk_range_set_value(GTK_RANGE(data->widgets.h_max_scale), current.h_max);
  gtk_range_set_value(GTK_RANGE(data->widgets.s_min_scale), current.s_min);
  gtk_range_set_value(GTK_RANGE(data->widgets.s_max_scale), current.s_max);
  gtk_range_set_value(GTK_RANGE(data->widgets.v_min_scale), current.v_min);
  gtk_range_set_value(GTK_RANGE(data->widgets.v_max_scale), current.v_max);

  gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->widgets.erode_spin),  current.erosions);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->widgets.dilate_spin), current.dilations);

  gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->widgets.width_spin),  current.width);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->widgets.height_spin), current.height);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->widgets.x_spin),      current.x_offset);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->widgets.y_spin),      current.y_offset);

  gtk_range_set_value(GTK_RANGE(data->widgets.percent_min_scale), current.percent_min);
  gtk_range_set_value(GTK_RANGE(data->widgets.percent_max_scale), current.percent_max);
  
}
  
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void update_settings_from_widgets(void* data_) {
  app_data* data = (app_data*) data_;
  
  camera_settings current = data->all_settings[data->index];
  current.iscamera1 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->widgets.camera1_check));
  current.iscamera2 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data->widgets.camera2_check));

  current.h_min = (int) gtk_range_get_value(GTK_RANGE(data->widgets.h_min_scale));
  current.h_max = (int) gtk_range_get_value(GTK_RANGE(data->widgets.h_max_scale));     
  current.s_min = (int) gtk_range_get_value(GTK_RANGE(data->widgets.s_min_scale));     
  current.s_max = (int) gtk_range_get_value(GTK_RANGE(data->widgets.s_max_scale));     
  current.v_min = (int) gtk_range_get_value(GTK_RANGE(data->widgets.v_min_scale));     
  current.v_max = (int) gtk_range_get_value(GTK_RANGE(data->widgets.v_max_scale));
  
  current.erosions   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->widgets.erode_spin));
  current.dilations  = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->widgets.dilate_spin));
  current.width      = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->widgets.width_spin));
  current.height     = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->widgets.height_spin));
  current.x_offset   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->widgets.x_spin));
  current.y_offset   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(data->widgets.y_spin));
  
  current.percent_min = (int) gtk_range_get_value(GTK_RANGE(data->widgets.percent_min_scale));
  current.percent_max = (int) gtk_range_get_value(GTK_RANGE(data->widgets.percent_max_scale));

  data->all_settings[data->index] = current;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
