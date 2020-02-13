#ifndef INCLUDE_SETTINGS_H
#define INCLUDE_SETTINGS_H

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
  int camera1_id;
  int camera2_id;
  camera_settings camera1_settings;
  camera_settings camera2_settings;
} rollercoaster_settings;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int load_settings(rollercoaster_settings* settings, std::string settings_file);
int save_settings(rollercoaster_settings* settings, std::string settings_file);
void set_default(rollercoaster_settings* settings);
camera_settings get_default_camera_settings();

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
