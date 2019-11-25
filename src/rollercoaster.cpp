// opencv image processing
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

// libserialport arduino communication
#include <libserialport.h>

// io
#include <iostream>

// project headers
#include "settings.h"
#include "arduino_util.h"

using namespace std;
using namespace cv;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define SETTINGS_FILE "rollercoaster-settings.yaml"

bool open_tracks(struct sp_port** track1, struct sp_port** track2);
bool triggered(Mat frame, camera_settings settings);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char** argv) {
  // important variables
  rollercoaster_settings settings; // structure to hold settings

  struct sp_port* track1;          // serial port for track 1
  struct sp_port* track2;          // serial port for track 2

  VideoCapture camera1;            // videocapture device for track 1 camera
  VideoCapture camera2;            // videocapture device for track 2 camera
  
  // open track ports
  if( !open_tracks(&track1, &track2) ) {
    cout << "FATAL: failed to open tracks!" << endl;
    return 1;
  }

  // load camera settings
  if( load_settings(&settings, SETTINGS_FILE) != 0 ) {
    cout << "WARNING: could not find '" << SETTINGS_FILE << "', using default values" << endl;
    set_default(&settings);
  }

  // open cameras
  camera1 = VideoCapture(settings.camera1_id);
  if ( !camera1.isOpened() ) {
    cout << "FATAL: failed to open camera 1 ( OpenCV ID " << settings.camera1_id << " )" << endl;
    return 1;
  }

  camera2 = VideoCapture(settings.camera2_id);
  if ( !camera2.isOpened() ) {
    cout << "FATAL: failed to open camera 2 ( OpenCV ID " << settings.camera2_id << " )" << endl;
    return 1;
  }

  Mat frame;

  char run = 'a';
  char off = 'b';
  
  while(true) {
    camera1 >> frame;
    if ( triggered(frame, settings.camera1_settings) ) {
      sp_blocking_write(track1, &run, 1, 100);
    }
    else {
      sp_blocking_write(track1, &off, 1, 100);
    }

    camera2 >> frame;
    if ( triggered(frame, settings.camera2_settings) ) {
      sp_blocking_write(track2, &run, 1, 100);
    }
    else {
      sp_blocking_write(track2, &off, 1, 100);
    }
  }
  
  // close track serial ports
  sp_close(track1);
  sp_close(track2);
  
  return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool triggered(Mat frame, camera_settings settings) {
  vector<Mat> chan;
  Mat hsv, mask_tmp, mask;

  // build mask
  cvtColor(frame, hsv, COLOR_BGR2HSV);
  split(hsv,chan);
  threshold(chan[0],mask,settings.h_max, 255, THRESH_BINARY_INV);
  threshold(chan[0],mask_tmp,settings.h_min-1, 255, THRESH_BINARY);
  bitwise_and(mask_tmp,mask,mask);

  threshold(chan[1],mask_tmp,settings.s_max, 255, THRESH_BINARY_INV);
  bitwise_and(mask_tmp,mask,mask);
  threshold(chan[1],mask_tmp,settings.s_min-1, 255, THRESH_BINARY);
  bitwise_and(mask_tmp,mask,mask);

  threshold(chan[2],mask_tmp,settings.v_max, 255, THRESH_BINARY_INV);
  bitwise_and(mask_tmp,mask,mask);    
  threshold(chan[2],mask_tmp,settings.v_min-1, 255, THRESH_BINARY);
  bitwise_and(mask_tmp,mask,mask);

  // erode / dilate mask
  erode(mask,mask,Mat(),Point(-1,-1),settings.erosions);
  dilate(mask,mask,Mat(),Point(-1,-1),settings.dilations);

  double percent = 100*double(countNonZero(mask(Rect(settings.x_offset, settings.y_offset, settings.width, settings.height)))) / (settings.width * settings.height);

  if ( percent >= settings.percent_min && percent <= settings.percent_max ) {
    return true;
  }
  else {
    return false;
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool open_tracks(struct sp_port** track1, struct sp_port** track2) {
  // find tracks
  enum sp_return err;
  struct sp_port** ports;
  int n_ports;
  err = enumerate_ports(&ports, &n_ports);
  if ( err != SP_OK ) {
    cerr << "FATAL: could not get list of serial devices; aborting" << endl;
    return 1;
  }
  if ( n_ports == 0 ) {
    cerr << "FATAL: no serial devices found; aborting" << endl;
    return 1;
  }

  char* track1_name = NULL;
  char* track2_name = NULL;

  for (int i=0; i<n_ports; i++) {
    char buf = 0;
    err = sp_open(ports[i], SP_MODE_READ_WRITE);
    if ( err != SP_OK ) {
      cerr << "WARNING: error trying to open port " << sp_get_port_name(ports[i]) << endl;
    }
    else {
      // port opened correctly
      err = set_arduino_config(ports[i], 9600);
      if ( err != SP_OK ) {
	cerr << "WARNING: error configuring port " << sp_get_port_name(ports[i]) << endl;
	sp_close(ports[i]);
      }
      else {
	sp_blocking_read(ports[i], &buf, 1, 10000);
	switch(buf) {
	case '1':
	  cout << "found track 1 on port " << sp_get_port_name(ports[i]) << endl;
	  track1_name = sp_get_port_name(ports[i]);
	  break;
	case '2':
	  cout << "found track 2 on port " << sp_get_port_name(ports[i]) << endl;
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
    cerr << "ERROR: could not find track 1!" << endl;
    return false;
  }
  else {
    err = sp_get_port_by_name(track1_name, track1);
    if (err != SP_OK) {
      cerr << "ERROR: could not get port for track 1" << endl;
      print_error(err);
      return false;
    }
    err = sp_open(*track1,SP_MODE_WRITE);
    if ( err != SP_OK) {
      cerr << "ERROR: could not open track 1" << endl;
      print_error(err);
      return false;
    }
    sp_blocking_read(*track1, NULL, 1, 10000);
    }

  // open track 2
  if ( track2_name == NULL ) {
    cerr << "ERROR: could not find track 2!" << endl;
    return false;
  }
  else {
    err = sp_get_port_by_name(track2_name, track2);
    if (err != SP_OK) {
      cerr << "ERROR: could not get port for track 2" << endl;
      print_error(err);
      return false;
    }
    err = sp_open(*track2,SP_MODE_WRITE);
    if ( err != SP_OK) {
      cerr << "ERROR: could not open track 2" << endl;
      print_error(err);
      return false;
    }
    sp_blocking_read(*track2, NULL, 1, 10000);
  }

  sp_free_port_list(ports);

  return true;
}
