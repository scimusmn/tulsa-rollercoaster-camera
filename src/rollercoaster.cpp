// opencv image processing
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// libserialport arduino communication
#include <libserialport.h>

// io
#include <iostream>

// mutex
#include <mutex>

// web configuration
#include "smmServer.hpp"

// base64
extern "C" {
  #include "b64/base64.h"
}

// project headers
#include "settings.h"
#include "arduino_util.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct globals {
  std::mutex access;
  rollercoaster_settings settings;
  double image_quality;
  cv::Mat camera1_frame;
  cv::Mat camera2_frame;
  cv::Mat camera1_mask;
  cv::Mat camera2_mask;
  bool track1_run;
  bool track2_run;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define SETTINGS_FILE "rollercoaster-settings.yaml"

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

int main(int argc, char** argv) {
  // important variables
  struct globals g;
  g.image_quality = 0.2;  // relatively low-quality video will be served to the web interface

  g.track1_run = false;   // do not run track 1 at start
  g.track2_run = false;   // do not run track 2 at start
  
  /*struct sp_port* track1; // arduino port for track 1
    struct sp_port* track2; // arduino port for track 2*/
  
  cv::VideoCapture camera1;   // videocapture device for track 1 camera
  cv::VideoCapture camera2;   // videocapture device for track 2 camera

  smmServer server("8000", "./web_root", &g); // webconfig server

  server.addPostCallback("set_camera1_settings", &set_camera1_settings);
  server.addPostCallback("set_camera2_settings", &set_camera2_settings);  
  server.addPostCallback("save_settings", &save_settings);

  server.addGetCallback("camera1_settings", &serve_camera1_settings);
  server.addGetCallback("camera2_settings", &serve_camera2_settings);

  server.addGetCallback("camera1_frame", &serve_camera1_frame);
  server.addGetCallback("camera2_frame", &serve_camera2_frame);
  
  server.addGetCallback("camera1_mask", &serve_camera1_mask);
  server.addGetCallback("camera2_mask", &serve_camera2_mask);

  // open track ports
  /*if( !open_tracks(&track1, &track2) ) {
    std::cerr << "FATAL: failed to open tracks!" << std::endl;
    return 1;
    }*/

  // load camera settings
  if( load_settings(&g.settings, SETTINGS_FILE) != 0 ) {
    std::cerr << "WARNING: could not find '" << SETTINGS_FILE << "', using default values" << std::endl;
    set_default(&g.settings);
  }

  // open cameras
  camera1 = cv::VideoCapture(g.settings.camera1_id);
  if ( !camera1.isOpened() ) {
    std::cerr << "FATAL: failed to open camera 1 ( OpenCV ID " << g.settings.camera1_id << " )" << std::endl;
    return 2;
  }

  camera2 = cv::VideoCapture(g.settings.camera2_id);
  if ( !camera2.isOpened() ) {
    std::cout << "FATAL: failed to open camera 2 ( OpenCV ID " << g.settings.camera2_id << " )" << std::endl;
    return 2;
  }

  // launch configuration server
  server.launch();

  char run = 'a';
  char off = 'b';
  
  while(server.isRunning()) {

    g.access.lock();
    camera1 >> g.camera1_frame;
    camera2 >> g.camera2_frame;

    g.camera1_mask = get_mask(g.camera1_frame, g.settings.camera1_settings);
    g.camera2_mask = get_mask(g.camera2_frame, g.settings.camera2_settings);
    g.access.unlock();

    // set run/stop on track 1
    if ( triggered(g.camera1_mask, g.settings.camera1_settings) || g.track1_run ) {
      //sp_blocking_write(track1, &run, 1, 100);
    }
    else {
      //sp_blocking_write(track1, &off, 1, 100);
    }

    // set run/stop on track 2
    if ( triggered(g.camera2_mask, g.settings.camera2_settings) || g.track2_run ) {
      //sp_blocking_write(track2, &run, 1, 100);
    }
    else {
      //sp_blocking_write(track2, &off, 1, 100);
    }
  }
  
  // close track serial ports
  /*sp_blocking_write(track1, &off, 1, 100);
  sp_blocking_write(track2, &off, 1, 100);
  sp_close(track1);
  sp_close(track2);*/
  
  return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cv::Mat get_mask(cv::Mat& frame, camera_settings s) {
  std::vector<cv::Mat> chan;
  cv::Mat hsv, mask_tmp, mask;

  // build mask
  cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
  cv::split(hsv,chan);
  if (s.h_min < s.h_max) {
    cv::threshold(chan[0],mask,s.h_max, 255, cv::THRESH_BINARY_INV);
    cv::threshold(chan[0],mask_tmp,s.h_min-1, 255, cv::THRESH_BINARY);
    cv::bitwise_and(mask_tmp,mask,mask);
  }
  else {
    cv::threshold(chan[0],mask,s.h_max, 255, cv::THRESH_BINARY_INV);
    cv::threshold(chan[0],mask_tmp,s.h_min-1, 255, cv::THRESH_BINARY);
    cv::bitwise_or(mask_tmp,mask,mask);
  }

  cv::threshold(chan[1],mask_tmp,s.s_max, 255, cv::THRESH_BINARY_INV);
  cv::bitwise_and(mask_tmp,mask,mask);
  cv::threshold(chan[1],mask_tmp,s.s_min-1, 255, cv::THRESH_BINARY);
  cv::bitwise_and(mask_tmp,mask,mask);

  cv::threshold(chan[2],mask_tmp,s.v_max, 255, cv::THRESH_BINARY_INV);
  cv::bitwise_and(mask_tmp,mask,mask);    
  cv::threshold(chan[2],mask_tmp,s.v_min-1, 255, cv::THRESH_BINARY);
  cv::bitwise_and(mask_tmp,mask,mask);

  // erode / dilate mask
  cv::erode(mask,mask,cv::Mat(),cv::Point(-1,-1),s.erosions);
  cv::dilate(mask,mask,cv::Mat(),cv::Point(-1,-1),s.dilations);

  return mask;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool triggered(cv::Mat mask, camera_settings settings) {
  double percent = 100*double(cv::countNonZero(mask(cv::Rect(settings.x_offset,
                                                             settings.y_offset,
                                                             settings.width,
                                                             settings.height)))
                              ) / (settings.width * settings.height);

  if ( percent >= settings.percent_min && percent <= settings.percent_max ) {
    return true;
  }
  else {
    return false;
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_image(cv::Mat image, httpMessage& m) {
  // get raw JPEG bytes from frame
  std::vector<unsigned char> rawJpegBuffer;
  cv::imencode(".jpeg", image, rawJpegBuffer);

  // convert to base64
  unsigned int bufferSize = b64e_size(rawJpegBuffer.size())+1;
  unsigned char* b64JpegBuffer = (unsigned char*) malloc(bufferSize * sizeof(unsigned char));
  b64_encode(rawJpegBuffer.data(), rawJpegBuffer.size(), b64JpegBuffer);
  
  m.replyHttpContent("image/jpeg", std::string((char*) b64JpegBuffer, bufferSize));
  free(b64JpegBuffer);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool open_tracks(struct sp_port** track1, struct sp_port** track2) {
  // find tracks
  enum sp_return err;
  struct sp_port** ports;
  int n_ports;
  err = enumerate_ports(&ports, &n_ports);
  if ( err != SP_OK ) {
    std::cerr << "FATAL: could not get list of serial devices; aborting" << std::endl;
    return 1;
  }
  if ( n_ports == 0 ) {
    std::cerr << "FATAL: no serial devices found; aborting" << std::endl;
    return 1;
  }

  char* track1_name = NULL;
  char* track2_name = NULL;

  for (int i=0; i<n_ports; i++) {
    char buf = 0;
    err = sp_open(ports[i], SP_MODE_READ_WRITE);
    if ( err != SP_OK ) {
      std::cerr << "WARNING: error trying to open port " << sp_get_port_name(ports[i]) << std::endl;
    }
    else {
      // port opened correctly
      err = set_arduino_config(ports[i], 9600);
      if ( err != SP_OK ) {
	std::cerr << "WARNING: error configuring port " << sp_get_port_name(ports[i]) << std::endl;
	sp_close(ports[i]);
      }
      else {
	sp_blocking_read(ports[i], &buf, 1, 10000);
	switch(buf) {
	case '1':
          std::cout << "found track 1 on port " << sp_get_port_name(ports[i]) << std::endl;
	  track1_name = sp_get_port_name(ports[i]);
	  break;
	case '2':
          std::cout << "found track 2 on port " << sp_get_port_name(ports[i]) << std::endl;
	  track2_name = sp_get_port_name(ports[i]);
	  break;
	default:
	  // boring port, close & move on
	  break;
	}
      }
      sp_close(ports[i]);
    }
  }

  // open track 1
  if ( track1_name == NULL ) {
    std::cerr << "ERROR: could not find track 1!" << std::endl;
    return false;
  }
  else {
    err = sp_get_port_by_name(track1_name, track1);
    if (err != SP_OK) {
      std::cerr << "ERROR: could not get port for track 1" << std::endl;
      print_error(err);
      return false;
    }
    err = sp_open(*track1,SP_MODE_WRITE);
    if ( err != SP_OK) {
      std::cerr << "ERROR: could not open track 1" << std::endl;
      print_error(err);
      return false;
    }
    sp_blocking_read(*track1, NULL, 1, 10000);
    }

  // open track 2
  if ( track2_name == NULL ) {
    std::cerr << "ERROR: could not find track 2!" << std::endl;
    return false;
  }
  else {
    err = sp_get_port_by_name(track2_name, track2);
    if (err != SP_OK) {
      std::cerr << "ERROR: could not get port for track 2" << std::endl;
      print_error(err);
      return false;
    }
    err = sp_open(*track2,SP_MODE_WRITE);
    if ( err != SP_OK) {
      std::cerr << "ERROR: could not open track 2" << std::endl;
      print_error(err);
      return false;
    }
    sp_blocking_read(*track2, NULL, 1, 10000);
  }

  sp_free_port_list(ports);

  return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_camera1_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;

  g->access.lock();
  
  std::string buffer = "{";
  buffer += "\"h_max\":";
  buffer += std::to_string(g->settings.camera1_settings.h_max);
  buffer += ",\"h_min\":";   
  buffer += std::to_string(g->settings.camera1_settings.h_min);
  buffer += ",\"s_max\":";   
  buffer += std::to_string(g->settings.camera1_settings.s_max);
  buffer += ",\"s_min\":";   
  buffer += std::to_string(g->settings.camera1_settings.s_min);
  buffer += ",\"v_max\":";   
  buffer += std::to_string(g->settings.camera1_settings.v_max);
  buffer += ",\"v_min\":";   
  buffer += std::to_string(g->settings.camera1_settings.v_min);
  buffer += ",\"erosions\":";
  buffer += std::to_string(g->settings.camera1_settings.erosions);
  buffer += ",\"dilations\":";
  buffer += std::to_string(g->settings.camera1_settings.dilations);
  buffer += ",\"width\":";
  buffer += std::to_string(g->settings.camera1_settings.width);
  buffer += ",\"height\":";
  buffer += std::to_string(g->settings.camera1_settings.height);
  buffer += ",\"x_offset\":";
  buffer += std::to_string(g->settings.camera1_settings.x_offset);
  buffer += ",\"y_offset\":";
  buffer += std::to_string(g->settings.camera1_settings.y_offset);
  buffer += ",\"percent_min\":";
  buffer += std::to_string(g->settings.camera1_settings.percent_min);
  buffer += ",\"percent_max\":";
  buffer += std::to_string(g->settings.camera1_settings.percent_max);
  buffer += "}";
  
  message.replyHttpContent("text/plain", buffer);
  g->access.unlock();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_camera2_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;

  g->access.lock();
  
  std::string buffer = "{";
  buffer += "\"h_max\":";
  buffer += std::to_string(g->settings.camera2_settings.h_max);
  buffer += ",\"h_min\":";   
  buffer += std::to_string(g->settings.camera2_settings.h_min);
  buffer += ",\"s_max\":";   
  buffer += std::to_string(g->settings.camera2_settings.s_max);
  buffer += ",\"s_min\":";   
  buffer += std::to_string(g->settings.camera2_settings.s_min);
  buffer += ",\"v_max\":";   
  buffer += std::to_string(g->settings.camera2_settings.v_max);
  buffer += ",\"v_min\":";   
  buffer += std::to_string(g->settings.camera2_settings.v_min);
  buffer += ",\"erosions\":";
  buffer += std::to_string(g->settings.camera2_settings.erosions);
  buffer += ",\"dilations\":";
  buffer += std::to_string(g->settings.camera2_settings.dilations);
  buffer += ",\"width\":";
  buffer += std::to_string(g->settings.camera2_settings.width);
  buffer += ",\"height\":";
  buffer += std::to_string(g->settings.camera2_settings.height);
  buffer += ",\"x_offset\":";
  buffer += std::to_string(g->settings.camera2_settings.x_offset);
  buffer += ",\"y_offset\":";
  buffer += std::to_string(g->settings.camera2_settings.y_offset);
  buffer += ",\"percent_min\":";
  buffer += std::to_string(g->settings.camera2_settings.percent_min);
  buffer += ",\"percent_max\":";
  buffer += std::to_string(g->settings.camera2_settings.percent_max);
  buffer += "}";
  
  message.replyHttpContent("text/plain", buffer);
  g->access.unlock();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_camera1_frame(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  cv::Mat image;
  bool ok = false;

  g->access.lock();
  if (!g->camera1_frame.empty()) {
    cv::resize(g->camera1_frame, image, cv::Size(), g->image_quality, g->image_quality);
    ok = true;
  }
  g->access.unlock();

  if (ok) {
    serve_image(image, message);
  }
  else {
    message.replyHttpError(503, "image not yet loaded");
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void serve_camera2_frame(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  cv::Mat image;
  bool ok = false;

  g->access.lock();
  if (!g->camera2_frame.empty()) {
    cv::resize(g->camera2_frame, image, cv::Size(), g->image_quality, g->image_quality);
    ok = true;
  }
  g->access.unlock();

  if (ok) {
    serve_image(image, message);
  }
  else {
    message.replyHttpError(503, "image not yet loaded");
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_camera1_mask(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  cv::Mat image;
  bool ok = false;

  g->access.lock();
  if (!g->camera1_mask.empty()) {
    cv::resize(g->camera1_mask, image, cv::Size(), g->image_quality, g->image_quality);
    ok = true;
  }
  g->access.unlock();

  if (ok) {
    serve_image(image, message);
  }
  else {
    message.replyHttpError(503, "image not yet loaded");
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_camera2_mask(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  cv::Mat image;
  bool ok = false;

  g->access.lock();
  if (!g->camera2_mask.empty()) {
    cv::resize(g->camera2_mask, image, cv::Size(), g->image_quality, g->image_quality);
    ok = true;
  }
  g->access.unlock();

  if (ok) {
    serve_image(image, message);
  }
  else {
    message.replyHttpError(503, "image not yet loaded");
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void save_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;

  g->access.lock();
  if (save_settings(&g->settings, SETTINGS_FILE) != 0) {
    std::cerr << "WARNING: failed to save settings" << std::endl;
    message.replyHttpError(500,"Failed to save settings");
  }
  else {
    std::cout << "saved.\n";
    message.replyHttpOk();
  }
  g->access.unlock();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
void set_camera1_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;

  camera_settings settings;
  try {
    settings.h_max         = std::stoi(message.getHttpVariable("h_max"));
    settings.s_max         = std::stoi(message.getHttpVariable("s_max"));
    settings.v_max         = std::stoi(message.getHttpVariable("v_max"));
    settings.h_min         = std::stoi(message.getHttpVariable("h_min"));   
    settings.s_min         = std::stoi(message.getHttpVariable("s_min"));   
    settings.v_min         = std::stoi(message.getHttpVariable("v_min"));   
    settings.erosions      = std::stoi(message.getHttpVariable("erosions"));   
    settings.dilations     = std::stoi(message.getHttpVariable("dilations"));
    settings.width         = std::stoi(message.getHttpVariable("width"));
    settings.height        = std::stoi(message.getHttpVariable("height"));
    settings.x_offset      = std::stoi(message.getHttpVariable("x_offset"));
    settings.y_offset      = std::stoi(message.getHttpVariable("y_offset"));
    settings.percent_min   = std::stoi(message.getHttpVariable("percent_min"));
    settings.percent_max   = std::stoi(message.getHttpVariable("percent_max"));
  }
  catch(std::invalid_argument error) {
    std::cerr << "error: invalid argument encountered in set_camera1_settings()" << std::endl;
    message.replyHttpError(422,"Invalid number");
    return;
  }
  
  g->access.lock();
  g->settings.camera1_settings = settings;
  g->access.unlock();

  message.replyHttpOk();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void set_camera2_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;

  camera_settings settings;
  try {
    settings.h_max         = std::stoi(message.getHttpVariable("h_max"));
    settings.s_max         = std::stoi(message.getHttpVariable("s_max"));
    settings.v_max         = std::stoi(message.getHttpVariable("v_max"));
    settings.h_min         = std::stoi(message.getHttpVariable("h_min"));   
    settings.s_min         = std::stoi(message.getHttpVariable("s_min"));   
    settings.v_min         = std::stoi(message.getHttpVariable("v_min"));   
    settings.erosions      = std::stoi(message.getHttpVariable("erosions"));   
    settings.dilations     = std::stoi(message.getHttpVariable("dilations"));
    settings.width         = std::stoi(message.getHttpVariable("width"));
    settings.height        = std::stoi(message.getHttpVariable("height"));
    settings.x_offset      = std::stoi(message.getHttpVariable("x_offset"));
    settings.y_offset      = std::stoi(message.getHttpVariable("y_offset"));
    settings.percent_min   = std::stoi(message.getHttpVariable("percent_min"));
    settings.percent_max   = std::stoi(message.getHttpVariable("percent_max"));
  }
  catch(std::invalid_argument error) {
    std::cerr << "error: invalid argument encountered in set_camera2_settings()" << std::endl;
    message.replyHttpError(422,"Invalid number");
    return;
  }
  
  g->access.lock();
  g->settings.camera2_settings = settings;
  g->access.unlock();

  message.replyHttpOk();
}  

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*                        ,d88b.d88b,
                          88888888888
                          `Y8888888Y'
                            `Y888Y'
                              `Y'                                */

