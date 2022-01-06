// opencv image processing
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

// io
#include <iostream>

// project headers
#include "mask.h"
#include "settings.h"
#include "logging.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define UI_FILE "config.ui"
#define SETTINGS_FILE "settings.yaml"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char** argv) {
   // parse command line options
   parse_options(argc, argv);
   log_msg(INFO, "log level: %s", level_string(log_level));

   // load settings
   log_msg(DEBUG, "loading settings from '%s'", SETTINGS_FILE);
   struct camera_settings_t settings;
   int error = load_settings(&settings, SETTINGS_FILE);
   if (error) {
      log_msg(WARN, "failed to load settings from '%s'; using default values", SETTINGS_FILE);
      set_default(&settings);
   }
   log_msg(INFO, "camera settings:\n"
	   "  mask:\n"
	   "    hue: [%d-%d]\n"
	   "    saturation: [%d-%d]\n"
	   "    value: [%d-%d]\n"
	   "  detection region:\n"
	   "    size: (width=%d, height=%d)\n"
	   "    offset: (x=%d, y=%d)\n"
	   "    detection range: [%d\%-%d\%]\n",
	   settings.mask.hue.min, settings.mask.hue.max,
	   settings.mask.saturation.min, settings.mask.saturation.max,
	   settings.mask.value.min, settings.mask.value.max,

	   settings.roi.width, settings.roi.height,
	   settings.roi.x_offset, settings.roi.y_offset,
	   settings.roi.percent_min, settings.roi.percent_max);
   
   cv::VideoCapture camera(0);
   if (!camera.isOpened()) {
      std::cerr << "FATAL: could not open camera!" << std::endl;
      return 1;
   }

   while (true) {
      cv::Mat frame;
      camera >> frame;
      cv::imshow("Frame", frame);

      if (cv::waitKey(10) == 27) // esc pressed
	 break;
   }
}
