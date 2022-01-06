#ifndef INCLUDE_SETTINGS_H
#define INCLUDE_SETTINGS_H

#include <gtk/gtk.h>
#include <string>

#include "mask.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct roi_settings_t {
   int width;           // width of detection region
   int height;          // height of detection region
   int x_offset;        // x offset of detection region
   int y_offset;        // y offset of detection region
   int percent_min;     // minimum percentage of detected pixels for triggering
   int percent_max;     // maximum percentage of detected pixels for triggering
};

struct camera_settings_t {
   struct mask_settings_t mask;
   struct roi_settings_t roi;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct {
   GtkHScale* h_min_scale;
   GtkHScale* s_min_scale;
   GtkHScale* v_min_scale;
   GtkHScale* h_max_scale;
   GtkHScale* s_max_scale;
   GtkHScale* v_max_scale;

   GtkSpinButton* erode_spin;
   GtkSpinButton* dilate_spin;

   GtkSpinButton* width_spin;
   GtkSpinButton* height_spin;
   GtkSpinButton* x_spin;
   GtkSpinButton* y_spin;
  
   GtkHScale* percent_min_scale;
   GtkHScale* percent_max_scale;
} settings_widgets;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void update_widgets_from_settings(void* data);
void update_settings_from_widgets(void* data);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int load_settings(struct camera_settings_t* settings, std::string settings_file);
int save_settings(struct camera_settings_t* settings, std::string settings_file);
void set_default(struct camera_settings_t* settings);
struct camera_settings_t get_default_camera_settings();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
