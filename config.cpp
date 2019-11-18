#include "DeviceEnumerator.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

using namespace cv;
using namespace std;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main() {
  // important variables
  const char* win_settings = "Settings";     // name of the settings window
  map<int, Device> devices;                  // list of all enumerated camera devices
  map<int, Device>::iterator current_device; // iterator pointing to current camera enumeration
  VideoCapture camera;                       // the current camera

  // struct for all the settings we care about
  struct {
    string camera1_path; // path to camera 1
    string camera2_path; // path to camera 2
    int h_max;	         // maximum hue
    int h_min;	         // minimum hue
    int s_max;	         // maximum saturation
    int s_min;	         // minimum saturation
    int v_max;	         // maximum value
    int v_min;	         // minimum value
    int erosions;        // number of erode() iterations 
    int dilations;       // number of dilate() iterations
    int width;	         // width of detection region
    int height;	         // height of detection region
    int x_offset;        // x offset of detection region
    int y_offset;        // y offset of detection region
    int percent_min;     // minimum percentage of detected pixels for triggering
    int percent_max;     // maximum percentage of detected pixels for triggering
  } settings;

  // read current settings
  {
    FileStorage fs("rollercoaster_settings.yaml", FileStorage::READ);
    if (fs.isOpened()) {
      settings.camera1_path = (string)fs["camera1_path"];
      settings.camera2_path = (string)fs["camera2_path"];    
      settings.h_max	    = fs["h_max"];
      settings.h_min	    = fs["h_min"];     
      settings.s_max	    = fs["s_max"];     
      settings.s_min	    = fs["s_min"];     
      settings.v_max	    = fs["v_max"];	     
      settings.v_min	    = fs["v_min"];	     
      settings.erosions     = fs["erosions"];   
      settings.dilations    = fs["dilations"];  
      settings.width	    = fs["width"];	     
      settings.height	    = fs["height"];	     
      settings.x_offset     = fs["x_offset"];   
      settings.y_offset     = fs["y_offset"];   
      settings.percent_min  = fs["percent_min"];
      settings.percent_max  = fs["percent_max"];
      fs.release();
    }
    else {
      // file failed to open
      cerr << "WARNING: could not open rollercoaster_settings.yaml, proceeding with default values." << endl;
      settings.camera1_path =  "";
      settings.camera2_path =  "";      
      settings.h_max	    = 180; 
      settings.h_min	    =   0;
      settings.s_max	    = 255;
      settings.s_min	    =   0;
      settings.v_max	    = 255;
      settings.v_min	    =   0;
      settings.erosions     =   0;
      settings.dilations    =   0;
      settings.width	    =  64;
      settings.height	    =  64;
      settings.x_offset     = 288;
      settings.y_offset     = 178;
      settings.percent_min  =   0;
      settings.percent_max  = 100;
    }
  }
  
  // configure settings window
  namedWindow(win_settings, WINDOW_NORMAL);

  // hue trackbars
  createTrackbar("H (max)", win_settings, &(settings.h_max), 180, NULL, NULL);
  createTrackbar("H (min)", win_settings, &(settings.h_min), 180, NULL, NULL);

  // saturation trackbars
  createTrackbar("S (max)", win_settings, &(settings.s_max), 255, NULL, NULL);
  createTrackbar("S (min)", win_settings, &(settings.s_min), 255, NULL, NULL);

  // value trackbars
  createTrackbar("V (max)", win_settings, &(settings.v_max), 255, NULL, NULL);
  createTrackbar("V (min)", win_settings, &(settings.v_min), 255, NULL, NULL);

  // erode() / dilate() trackbars
  createTrackbar("Erosions", win_settings, &(settings.erosions), 10, NULL, NULL);  
  createTrackbar("Dilations", win_settings, &(settings.dilations), 10, NULL, NULL);

  // detection region trackbars
  createTrackbar("Width", win_settings, &(settings.width), 255, NULL, NULL);
  createTrackbar("Height", win_settings, &(settings.height), 255, NULL, NULL);
  createTrackbar("X Offset", win_settings, &(settings.x_offset), 640, NULL, NULL);
  createTrackbar("Y Offset", win_settings, &(settings.y_offset), 480, NULL, NULL);

  // detection percentage trackbars
  createTrackbar("% Max", win_settings, &(settings.percent_max), 100, NULL, NULL);
  createTrackbar("% Min", win_settings, &(settings.percent_min), 100, NULL, NULL);

  // get list of all cameras connected to the system
  {
    DeviceEnumerator de;
    devices = de.getVideoDevicesMap();
  }
  
  if (devices.empty()) {
    // abort if no cameras detected
    cerr << "FATAL: no cameras detected!" << endl;
    return -1;
  }
  
  // list cameras
  for (auto dev : devices) {
    cout << "Camera " << dev.first << endl;
    cout << "  Name: " << dev.second.deviceName << endl;
    cout << "  Path: " << dev.second.devicePath << endl;
  }

  // point current_device to the first enumerated device & open it
  current_device = devices.begin();
  camera = VideoCapture(current_device->first);

  // mats and variables for main loop
  bool running = true;
  Mat frame, hsv, mask_tmp, mask;
  vector<Mat> chan;

  // BEGIN MAIN LOOP
  while(running) {
    // get frame
    camera >> frame;

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
    
    //draw window and instructions on frame
    Scalar color(0,0,255);
    if ( percent >= settings.percent_min && percent <= settings.percent_max ) {
      // within detection threshold, draw ROI green
      color = Scalar(0,255,0);
    }
    rectangle(frame, Rect(settings.x_offset, settings.y_offset, settings.width, settings.height), color, 2);
    putText(frame, "[SPACE] for next camera", Point(10,20), FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));
    if (settings.camera1_path != current_device->second.devicePath) {
      putText(frame, "[1] to set as Camera 1", Point(10,35), FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));
    }
    else {
      putText(frame, "This is Camera 1", Point(10,35), FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));
    }
    if (settings.camera2_path != current_device->second.devicePath) {
      putText(frame, "[2] to set as Camera 2", Point(10,50), FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));
    }
    else {
      putText(frame, "This is Camera 2", Point(10,50), FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));
    }
    putText(frame, "[ESC] to quit", Point(10,65), FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));

    // show frame and mask
    imshow("Camera", frame);
    imshow("Mask", mask);

    switch(waitKey(1)) {
    case (32):
      //space -> switch to next camera
      current_device++;
      if ( current_device == devices.end() ) {
	current_device = devices.begin();
      }
      camera = VideoCapture(current_device->first);
      break;
    case(27):
      //escape -> end main loop & exit
      running = false;
      break;
    case (49):
      // [1] -> set current camera as Camera 1
      settings.camera1_path = current_device->second.devicePath;
      break;
    case (50):
      // [2] -> set current camera as Camera 2
      settings.camera2_path = current_device->second.devicePath;
      break;      
    default:
      break;
      //do nothing
    }
  }

  // close windows
  destroyAllWindows();

  // write settings
  {
    FileStorage fs("rollercoaster_settings.yaml", FileStorage::WRITE);
    if (!fs.isOpened()) {
      // error opening file
      cerr << "FATAL: could not save settings. dumping to screen!" << endl;
      cout << "camera1_path:" << settings.camera1_path << endl;
      cout << "camera2_path:" << settings.camera2_path << endl;
      cout << "h_max:" << settings.h_max << endl;
      cout << "h_min:" << settings.h_min << endl;
      cout << "s_max:" << settings.s_max << endl;
      cout << "s_min:" << settings.s_min << endl;
      cout << "v_max:" << settings.v_max << endl;
      cout << "v_min:" << settings.v_min << endl;
      cout << "erosions:" << settings.erosions << endl;
      cout << "dilations:" << settings.dilations << endl;
      cout << "width:" << settings.width << endl;
      cout << "height:" << settings.height << endl;	   
      cout << "x_offset:" << settings.x_offset << endl;
      cout << "y_offset:" << settings.y_offset << endl;
      cout << "percent_min:" << settings.percent_min << endl;
      cout << "percent_max:" << settings.percent_max << endl;
      return -1; 
    }

    // save normally
    fs << "camera1_path" << settings.camera1_path;
    fs << "camera2_path" << settings.camera2_path;
    fs << "h_max" << settings.h_max;
    fs << "h_min" << settings.h_min;
    fs << "s_max" << settings.s_max;
    fs << "s_min" << settings.s_min;
    fs << "v_max" << settings.v_max;
    fs << "v_min" << settings.v_min;
    fs << "erosions" << settings.erosions;
    fs << "dilations" << settings.dilations;
    fs << "width" << settings.width;
    fs << "height" << settings.height;
    fs << "x_offset" << settings.x_offset;
    fs << "y_offset" << settings.y_offset;
    fs << "percent_max" << settings.percent_max;
    fs << "percent_min" << settings.percent_min;
    fs.release();
  }
  
  return 0;
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
