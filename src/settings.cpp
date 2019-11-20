#include "settings.h"
#include <opencv2/core.hpp>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int load_settings(rollercoaster_settings* settings, std::string settings_file) {
  cv::FileStorage fs(settings_file, cv::FileStorage::READ);
  if (!fs.isOpened()) {
    // couldn't open the settings file
    return 1;
  }

  // load camera paths
  settings->camera1_path = (std::string)fs["camera1_path"];
  settings->camera2_path = (std::string)fs["camera2_path"];

  // load settings for camera 1
  cv::FileNode n = fs["camera1_settings"];
  settings->camera1_settings.h_max        = n["h_max"];
  settings->camera1_settings.h_min        = n["h_min"];
  settings->camera1_settings.s_max        = n["s_max"];
  settings->camera1_settings.s_min        = n["s_min"];
  settings->camera1_settings.v_max        = n["v_max"];
  settings->camera1_settings.v_min        = n["v_min"];
  settings->camera1_settings.erosions     = n["erosions"];
  settings->camera1_settings.dilations    = n["dilations"];
  settings->camera1_settings.width        = n["width"];
  settings->camera1_settings.height       = n["height"];
  settings->camera1_settings.x_offset     = n["x_offset"];
  settings->camera1_settings.y_offset     = n["y_offset"];
  settings->camera1_settings.percent_min  = n["percent_min"];
  settings->camera1_settings.percent_max  = n["percent_max"];

  // load settings for camera 2
  n = fs["camera2_settings"];
  settings->camera2_settings.h_max        = n["h_max"];
  settings->camera2_settings.h_min        = n["h_min"];
  settings->camera2_settings.s_max        = n["s_max"];
  settings->camera2_settings.s_min        = n["s_min"];
  settings->camera2_settings.v_max        = n["v_max"];
  settings->camera2_settings.v_min        = n["v_min"];
  settings->camera2_settings.erosions     = n["erosions"];
  settings->camera2_settings.dilations    = n["dilations"];
  settings->camera2_settings.width        = n["width"];
  settings->camera2_settings.height       = n["height"];
  settings->camera2_settings.x_offset     = n["x_offset"];
  settings->camera2_settings.y_offset     = n["y_offset"];
  settings->camera2_settings.percent_min  = n["percent_min"];
  settings->camera2_settings.percent_max  = n["percent_max"];

  fs.release();
  return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int save_settings(rollercoaster_settings* settings, std::string settings_file) {
  cv::FileStorage fs(settings_file, cv::FileStorage::WRITE);
  if (!fs.isOpened()) {
    // couldn't open the settings file
    return 1;
  }

  // save camera paths
  fs << "camera1_path" << settings->camera1_path;
  fs << "camera2_path" << settings->camera2_path;

  // save settings for camera 1
  fs << "camera1_settings" << "{";
  fs << "h_max"       << settings->camera1_settings.h_max;      
  fs << "h_min"	      << settings->camera1_settings.h_min;      
  fs << "s_max"	      << settings->camera1_settings.s_max;      
  fs << "s_min"	      << settings->camera1_settings.s_min;      
  fs << "v_max"	      << settings->camera1_settings.v_max;      
  fs << "v_min"	      << settings->camera1_settings.v_min;      
  fs << "erosions"    << settings->camera1_settings.erosions;   
  fs << "dilations"   << settings->camera1_settings.dilations;  
  fs << "width"	      << settings->camera1_settings.width;      
  fs << "height"      << settings->camera1_settings.height;     
  fs << "x_offset"    << settings->camera1_settings.x_offset;   
  fs << "y_offset"    << settings->camera1_settings.y_offset;   
  fs << "percent_min" << settings->camera1_settings.percent_min;
  fs << "percent_max" << settings->camera1_settings.percent_max;
  fs << "}";

  // save settings for camera 2
  fs << "camera2_settings" << "{";
  fs << "h_max"       << settings->camera2_settings.h_max;      
  fs << "h_min"	      << settings->camera2_settings.h_min;      
  fs << "s_max"	      << settings->camera2_settings.s_max;      
  fs << "s_min"	      << settings->camera2_settings.s_min;      
  fs << "v_max"	      << settings->camera2_settings.v_max;      
  fs << "v_min"	      << settings->camera2_settings.v_min;      
  fs << "erosions"    << settings->camera2_settings.erosions;   
  fs << "dilations"   << settings->camera2_settings.dilations;  
  fs << "width"	      << settings->camera2_settings.width;      
  fs << "height"      << settings->camera2_settings.height;     
  fs << "x_offset"    << settings->camera2_settings.x_offset;   
  fs << "y_offset"    << settings->camera2_settings.y_offset;   
  fs << "percent_min" << settings->camera2_settings.percent_min;
  fs << "percent_max" << settings->camera2_settings.percent_max;
  fs << "}";

  
  fs.release();
  return 0;  
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void set_default(rollercoaster_settings* settings) {
  // load default settings
  settings->camera1_path = "";
  settings->camera1_path = "";

  // default settings for camera 1
  settings->camera1_settings = get_default_camera_settings();

  // default settings for camera 2
  settings->camera2_settings = get_default_camera_settings();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

camera_settings get_default_camera_settings() {
  camera_settings s;
  s.h_max        = 179;
  s.h_min        =   0;	
  s.s_max        = 255;
  s.s_min        =   0;	
  s.v_max        = 255;
  s.v_min        =   0;	
  s.erosions     =   0;	
  s.dilations    =   0;	
  s.width        =  64;	
  s.height       =  64;	
  s.x_offset     = 288;
  s.y_offset     = 178;
  s.percent_min  =   0;	
  s.percent_max  = 100;
  return s;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
