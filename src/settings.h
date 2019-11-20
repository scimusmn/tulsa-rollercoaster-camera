#ifndef INCLUDE_SETTINGS_H
#define INCLUDE_SETTINGS_H

#include <gtk/gtk.h>
#include <string>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct {
  bool iscamera1;      // is this camera cam1?
  bool iscamera2;      // is this camera cam2?
  std::string path;    // the system path of this camera
  int opencv_id;       // opencv id for this camera
  int h_max;           // maximum hue
  int h_min;           // minimum hue
  int s_max;           // maximum saturation
  int s_min;           // minimum saturation
  int v_max;           // maximum value
  int v_min;           // minimum value
  int erosions;        // number of erode() iterations
  int dilations;       // number of dilate() iterations
  int width;           // width of detection region
  int height;          // height of detection region
  int x_offset;        // x offset of detection region
  int y_offset;        // y offset of detection region
  int percent_min;     // minimum percentage of detected pixels for triggering
  int percent_max;     // maximum percentage of detected pixels for triggering
} camera_settings;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct {
  GtkToggleButton* camera1_check;
  GtkToggleButton* camera2_check;
  GtkButton* track1_but;
  GtkButton* track2_but;

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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct {
  std::string camera1_path;
  std::string camera2_path;
  camera_settings camera1_settings;
  camera_settings camera2_settings;
} rollercoaster_settings;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void update_widgets_from_settings(void* data);
void update_settings_from_widgets(void* data);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int load_settings(rollercoaster_settings* settings, std::string settings_file);
int save_settings(rollercoaster_settings* settings, std::string settings_file);
void set_default(rollercoaster_settings* settings);
camera_settings get_default_camera_settings();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
