#ifndef ROLLERCOASTER_SERVER_CALLBACKS_HPP
#define ROLLERCOASTER_SERVER_CALLBACKS_HPP

#include <iostream>
#include <mutex>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "settings.h"
#include "smmServer.hpp"

extern "C" {
  #include "b64/base64.h"
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct globals {
  rollercoaster_settings settings;
  double image_quality;

  cv::Mat camera1_frame;
  cv::Mat camera2_frame;
  cv::Mat camera1_mask;
  cv::Mat camera2_mask;

  std::mutex camera1_frame_mtx;
  std::mutex camera2_frame_mtx;
  std::mutex camera1_mask_mtx;
  std::mutex camera2_mask_mtx;


  bool track1_run;
  bool track2_run;
  bool track1_running;
  bool track2_running;
  
  std::string settings_file;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// server GET callbacks
void serve_camera1_settings(httpMessage message, void* data);
void serve_camera2_settings(httpMessage message, void* data);
void serve_camera1_frame(httpMessage message, void* data);
void serve_camera2_frame(httpMessage message, void* data);
void serve_camera1_mask(httpMessage message, void* data);
void serve_camera2_mask(httpMessage message, void* data);

// server POST callbacks
void save_settings(httpMessage message, void* data);
void set_camera1_settings(httpMessage message, void* data);
void set_camera2_settings(httpMessage message, void* data);

// utility functions
bool open_tracks(struct sp_port** track1, struct sp_port** track2);
cv::Mat get_mask(cv::Mat& frame, camera_settings s);
bool triggered(cv::Mat mask, camera_settings settings);
void serve_image(cv::Mat image, httpMessage& m);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#endif
