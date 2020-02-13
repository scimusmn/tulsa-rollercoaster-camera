// opencv image processing
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

// libserialport arduino communication
#include <libserialport.h>

// io
#include <iostream>

// web configuration
#include "smmServer.hpp"

// project headers
#include "callbacks.hpp"
#include "settings.h"
#include "arduino_util.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char** argv) {
  // important variables
  struct globals g;
  g.image_quality = 0.2;  // relatively low-quality video will be served to the web interface

  g.track1_run = false;   // do not run track 1 at start
  g.track2_run = false;   // do not run track 2 at start
  g.track1_running = false;
  g.track2_running = false;

  g.settings_file = "rollercoaster-settings.yaml";
  
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
  if( load_settings(&g.settings, g.settings_file) != 0 ) {
    std::cerr << "WARNING: could not find '" << g.settings_file << "', using default values" << std::endl;
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
  std::cout << "Server launched on port 8000" << std::endl;

  char run = 'a';
  char off = 'b';
  
  while(server.isRunning()) {

    g.camera1_frame_mtx.lock();
    camera1 >> g.camera1_frame;
    g.camera1_frame_mtx.unlock();

    g.camera2_frame_mtx.lock();
    camera2 >> g.camera2_frame;
    g.camera2_frame_mtx.unlock();

    g.camera1_mask_mtx.lock();
    g.camera1_mask = get_mask(g.camera1_frame, g.settings.camera1_settings);
    g.camera1_mask_mtx.unlock();
    
    g.camera2_mask_mtx.lock();
    g.camera2_mask = get_mask(g.camera2_frame, g.settings.camera2_settings);
    g.camera2_mask_mtx.unlock();
    
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

/*                        ,d88b.d88b,
                          88888888888
                          `Y8888888Y'
                            `Y888Y'
                              `Y'                                */

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
