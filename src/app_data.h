#ifndef INCLUDE_APP_DATA_H
#define INCLUDE_APP_DATA_H

#include "settings.h"
#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <libserialport.h>

typedef struct {
  cv::VideoCapture camera;
  cv::Mat frame;
  cv::Mat mask;
  cv::Mat mask_tmp;

  struct sp_port* track1;
  struct sp_port* track2;
  bool run_track1;
  bool run_track2;
  
  std::vector<camera_settings> all_settings;
  int index;
  rollercoaster_settings settings;
  settings_widgets widgets;
} app_data;

#endif
