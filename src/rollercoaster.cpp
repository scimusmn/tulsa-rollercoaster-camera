// opencv image processing
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

// io
#include <iostream>

// project headers
#include "settings.h"
#include "processing.h"
#include "logging.h"
#include "arduino_util.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define SETTINGS_FILE "settings.yaml"

static int loop(cv::VideoCapture& camera,
		struct camera_settings_t& settings,
		struct sp_port *arduino);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char** argv) {
   // parse command line options
   parse_options(argc, argv);
   log_msg(INFO, "log level: %s", level_string(log_level));
   log_msg(INFO, "libserialport version %s", sp_get_package_version_string());

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
	   "    erosions: %d\n"
	   "    dilations: %d\n"
	   "  detection region:\n"
	   "    size: (width=%d, height=%d)\n"
	   "    offset: (x=%d, y=%d)\n"
	   "    detection range: [%d-%d\%]\n",
	   settings.mask.hue.min, settings.mask.hue.max,
	   settings.mask.saturation.min, settings.mask.saturation.max,
	   settings.mask.value.min, settings.mask.value.max,
	   settings.mask.erosions,
	   settings.mask.dilations,

	   settings.roi.width, settings.roi.height,
	   settings.roi.x_offset, settings.roi.y_offset,
	   settings.roi.percent_min, settings.roi.percent_max);

   // open camera
   log_msg(DEBUG, "opening camera");
   cv::VideoCapture camera(0);
   if (!camera.isOpened()) {
      std::cerr << "FATAL: could not open camera!" << std::endl;
      return 1;
   }

   // open arduino
   log_msg(DEBUG, "opening serial port");
   struct sp_port *arduino;
   error = find_metro_mini(&arduino);
   if (error) {
      log_msg(FATAL, "could not find Metro Mini");
      return 1;
   }
   log_msg(DEBUG, "found Metro Mini at '%s'", sp_get_port_name(arduino));

   enum sp_return err = sp_open(arduino, SP_MODE_READ_WRITE);
   if (err != SP_OK) {
      char *last_error = sp_last_error_message();
      log_msg(FATAL, "could not open Metro Mini: %s", last_error);
      sp_free_error_message(last_error);
      sp_free_port(arduino);
      return 1;
   }

   error = set_arduino_config(arduino, 9600);
   if (error) {
      log_msg(FATAL, "could not configure Metro Mini");
      sp_close(arduino);
      sp_free_port(arduino);
      return 1;
   }
   show_config(arduino);

   // run main loop
   log_msg(DEBUG, "entering main loop");
   while (loop(camera, settings, arduino) != 1) {}

   // finish up
   sp_close(arduino);
   sp_free_port(arduino);
   
   return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static int loop(cv::VideoCapture& camera,
		struct camera_settings_t& settings,
		struct sp_port *arduino)
{
   log_msg(TRACE, "loop(): load frame");
   cv::Mat frame, mask;
   camera >> frame;

   log_msg(TRACE, "loop(): compute mask");
   build_mask(mask, frame, settings.mask);

   log_msg(TRACE, "loop(): check triggering");
   bool triggered = region_triggered(mask, settings.roi);
   draw_window(frame, settings.roi, triggered);

   if (triggered) {
      log_msg(TRACE, "loop(): run track");
      const char run = 'a';
      sp_blocking_write(arduino, &run, 1, 100);
   }
   else {
      log_msg(TRACE, "loop(): stop track");
      const char stop = 'b';
      sp_blocking_write(arduino, &stop, 1, 100);
   }

   log_msg(TRACE, "loop(): done");
   return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
