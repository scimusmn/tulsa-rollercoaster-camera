// opencv image processing
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

// io
#include <iostream>

// timing
#include <chrono>

// project headers
#include "settings.h"
#include "processing.h"
#include "logging.h"
#include "arduino_util.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define UI_FILE "config.ui"
#define SETTINGS_FILE "settings.yaml"

// save interval in seconds
#define SAVE_INTERVAL 20

struct widgets_t {
   GtkHScale* h_min_scale;
   GtkHScale* s_min_scale;
   GtkHScale* v_min_scale;
   GtkHScale* h_max_scale;
   GtkHScale* s_max_scale;
   GtkHScale* v_max_scale;

   GtkSpinButton* erode_spin;
   GtkSpinButton* dilate_spin;

   GtkSpinButton* width_spin;
   GtkSpinButton* height_spin;
   GtkSpinButton* x_spin;
   GtkSpinButton* y_spin;
  
   GtkHScale* percent_min_scale;
   GtkHScale* percent_max_scale;
};

static int build_config_window(std::string ui_file,
			       struct widgets_t *widgets,
			       struct camera_settings_t s);
static void update_settings(struct camera_settings_t *s,
			    struct widgets_t *widgets);

static int loop(cv::VideoCapture& camera,
		struct camera_settings_t& settings,
		struct widgets_t& widgets,
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

   std::chrono::duration<int> save_interval(SAVE_INTERVAL);
   std::chrono::steady_clock::time_point last_save = std::chrono::steady_clock::now();

   // open camera
   log_msg(DEBUG, "opening camera");
   cv::VideoCapture camera(0);
   if (!camera.isOpened()) {
      std::cerr << "FATAL: could not open camera!" << std::endl;
      return 1;
   }

   // construct widgets window
   log_msg(DEBUG, "building GTK window");
   struct widgets_t widgets;
   error = build_config_window(UI_FILE, &widgets, settings);
   if (error) {
      log_msg(FATAL, "failed to construct GTK window!");
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
   while (loop(camera, settings, widgets, arduino) != 1) {
      if (std::chrono::steady_clock::now() - last_save > save_interval) {
	 log_msg(INFO, "saving settings");
	 save_settings(settings, SETTINGS_FILE);
	 last_save = std::chrono::steady_clock::now();
      }
   }

   // finish up
   sp_close(arduino);
   sp_free_port(arduino);
   
   log_msg(INFO, "saving settings");
   save_settings(settings, SETTINGS_FILE);
   return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static int loop(cv::VideoCapture& camera,
		struct camera_settings_t& settings,
		struct widgets_t& widgets,
		struct sp_port *arduino)
{
   log_msg(TRACE, "loop(): update settings");
   update_settings(&settings, &widgets);
   log_msg(TRACE, "camera settings:\n"
	   "  mask:\n"
	   "    hue: [%d-%d]\n"
	   "    saturation: [%d-%d]\n"
	   "    value: [%d-%d]\n"
	   "  detection region:\n"
	   "    size: (width=%d, height=%d)\n"
	   "    offset: (x=%d, y=%d)\n"
	   "    detection range: [%d-%d\%]\n",
	   settings.mask.hue.min, settings.mask.hue.max,
	   settings.mask.saturation.min, settings.mask.saturation.max,
	   settings.mask.value.min, settings.mask.value.max,

	   settings.roi.width, settings.roi.height,
	   settings.roi.x_offset, settings.roi.y_offset,
	   settings.roi.percent_min, settings.roi.percent_max);

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

   log_msg(TRACE, "loop(): display mats");
   cv::imshow("Frame", frame);
   cv::imshow("Mask", mask);

   if (cv::waitKey(10) == 27) // esc pressed
      return 1;

   log_msg(TRACE, "loop(): done");
   return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void update_settings(struct camera_settings_t *s, struct widgets_t *widgets)
{
   log_msg(TRACE, "update_settings(): get mask settings");
   s->mask.hue.min        = (int) gtk_range_get_value(GTK_RANGE(widgets->h_min_scale));
   s->mask.hue.max        = (int) gtk_range_get_value(GTK_RANGE(widgets->h_max_scale));     
   s->mask.saturation.min = (int) gtk_range_get_value(GTK_RANGE(widgets->s_min_scale));     
   s->mask.saturation.max = (int) gtk_range_get_value(GTK_RANGE(widgets->s_max_scale));     
   s->mask.value.min      = (int) gtk_range_get_value(GTK_RANGE(widgets->v_min_scale));     
   s->mask.value.max      = (int) gtk_range_get_value(GTK_RANGE(widgets->v_max_scale));
   s->mask.erosions       = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets->erode_spin));
   s->mask.dilations      = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets->dilate_spin));

   log_msg(TRACE, "update_settings(): get roi settings");
   s->roi.width      = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets->width_spin));
   s->roi.height     = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets->height_spin));
   s->roi.x_offset   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets->x_spin));
   s->roi.y_offset   = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgets->y_spin));
   s->roi.percent_min = (int) gtk_range_get_value(GTK_RANGE(widgets->percent_min_scale));
   s->roi.percent_max = (int) gtk_range_get_value(GTK_RANGE(widgets->percent_max_scale));
   log_msg(TRACE, "update_settings(): done");
}

int build_config_window(std::string ui_file, struct widgets_t *widgets, struct camera_settings_t s)
{
   GtkBuilder *builder;
   GError *error = NULL;

   gtk_init(0, NULL);

   builder = gtk_builder_new();
   if (gtk_builder_add_from_file(builder, ui_file.c_str(), &error) == 0) {
      log_msg(ERROR, "%s", error->message);
      return 1;
   }

   // store widgets
   widgets->h_min_scale = (GtkHScale*) gtk_builder_get_object(builder, "h_min_scale");
   widgets->h_max_scale = (GtkHScale*) gtk_builder_get_object(builder, "h_max_scale");
   widgets->s_min_scale = (GtkHScale*) gtk_builder_get_object(builder, "s_min_scale");
   widgets->s_max_scale = (GtkHScale*) gtk_builder_get_object(builder, "s_max_scale");
   widgets->v_min_scale = (GtkHScale*) gtk_builder_get_object(builder, "v_min_scale");
   widgets->v_max_scale = (GtkHScale*) gtk_builder_get_object(builder, "v_max_scale");

   widgets->erode_spin = (GtkSpinButton*) gtk_builder_get_object(builder, "erode_spin");
   widgets->dilate_spin = (GtkSpinButton*) gtk_builder_get_object(builder, "dilate_spin");

   widgets->width_spin = (GtkSpinButton*) gtk_builder_get_object(builder, "width_spin");
   widgets->height_spin = (GtkSpinButton*) gtk_builder_get_object(builder, "height_spin");
   widgets->x_spin = (GtkSpinButton*) gtk_builder_get_object(builder, "x_spin");
   widgets->y_spin = (GtkSpinButton*) gtk_builder_get_object(builder, "y_spin");

   widgets->percent_min_scale = (GtkHScale*) gtk_builder_get_object(builder, "percent_min_scale");
   widgets->percent_max_scale = (GtkHScale*) gtk_builder_get_object(builder, "percent_max_scale");

   // set initial values
   gtk_range_set_value(GTK_RANGE(widgets->h_min_scale), s.mask.hue.min);
   gtk_range_set_value(GTK_RANGE(widgets->h_max_scale), s.mask.hue.max);
   gtk_range_set_value(GTK_RANGE(widgets->s_min_scale), s.mask.saturation.min);
   gtk_range_set_value(GTK_RANGE(widgets->s_max_scale), s.mask.saturation.max);
   gtk_range_set_value(GTK_RANGE(widgets->v_min_scale), s.mask.value.min);
   gtk_range_set_value(GTK_RANGE(widgets->v_max_scale), s.mask.value.max);

   gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->erode_spin),  s.mask.erosions);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->dilate_spin), s.mask.dilations);

   gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->width_spin),  s.roi.width);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->height_spin), s.roi.height);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->x_spin),      s.roi.x_offset);
   gtk_spin_button_set_value(GTK_SPIN_BUTTON(widgets->y_spin),      s.roi.y_offset);

   gtk_range_set_value(GTK_RANGE(widgets->percent_min_scale), s.roi.percent_min);
   gtk_range_set_value(GTK_RANGE(widgets->percent_max_scale), s.roi.percent_max);

   return 0;
}
