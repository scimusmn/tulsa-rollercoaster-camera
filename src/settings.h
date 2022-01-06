#ifndef INCLUDE_SETTINGS_H
#define INCLUDE_SETTINGS_H

#include <gtk/gtk.h>
#include <string>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct range_t {
   int min;
   int max;
};

struct mask_settings_t {
   struct range_t hue;
   struct range_t saturation;
   struct range_t value;
   int erosions;
   int dilations;
};

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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void update_widgets_from_settings(void* data);
void update_settings_from_widgets(void* data);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int load_settings(struct camera_settings_t* settings, std::string settings_file);
int save_settings(struct camera_settings_t settings, std::string settings_file);
void set_default(struct camera_settings_t* settings);
struct camera_settings_t get_default_camera_settings();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
