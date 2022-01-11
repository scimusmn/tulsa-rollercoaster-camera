#include <opencv2/core.hpp>

#include "settings.h"
#include "logging.h"


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int load_settings(struct camera_settings_t* s, std::string settings_file) {
   log_msg(TRACE, "load_settings(): open file '%s'", settings_file);
   cv::FileStorage fs(settings_file, cv::FileStorage::READ);
   if (!fs.isOpened()) {
      // couldn't open the settings file
      return 1;
   }

   // mask settings
   log_msg(TRACE, "load_settings(): read mask settings");
   cv::FileNode mask = fs["mask"];
  
   cv::FileNode hue = mask["hue"];
   s->mask.hue.min = (int)hue["min"];
   s->mask.hue.max = (int)hue["max"];

   cv::FileNode saturation = mask["saturation"];
   s->mask.saturation.min = (int)saturation["min"];
   s->mask.saturation.max = (int)saturation["max"];

   cv::FileNode value = mask["value"];
   s->mask.value.min = (int)value["min"];
   s->mask.value.max = (int)value["max"];

   s->mask.erosions = (int)mask["erosions"];
   s->mask.dilations = (int)mask["dilations"];

   // roi s
   log_msg(TRACE, "load_settings(): read ROI settings");
   cv::FileNode roi = fs["detection_region"];
   s->roi.width = (int)roi["width"];
   s->roi.height = (int)roi["height"];
   s->roi.x_offset = (int)roi["x_offset"];
   s->roi.y_offset = (int)roi["y_offset"];
   s->roi.percent_min = (int)roi["percent_min"];
   s->roi.percent_max = (int)roi["percent_max"];

   fs.release();
   log_msg(TRACE, "load_settings(): done");
   return 0;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int save_settings(struct camera_settings_t s, std::string settings_file) {
   log_msg(TRACE, "save_settings(): open file '%s'", settings_file);
   cv::FileStorage fs(settings_file, cv::FileStorage::WRITE);
   if (!fs.isOpened()) {
      // couldn't open the settings file
      return 1;
   }

   // mask settings
   log_msg(TRACE, "save_settings(): write mask settings");
   fs << "mask" << "{";

   fs << "hue" << "{";
   fs << "min" << s.mask.hue.min;
   fs << "max" << s.mask.hue.max;
   fs << "}";

   fs << "saturation" << "{";
   fs << "min" << s.mask.saturation.min;
   fs << "max" << s.mask.saturation.max;
   fs << "}";

   fs << "value" << "{";
   fs << "min" << s.mask.value.min;
   fs << "max" << s.mask.value.max;
   fs << "}";

   fs << "erosions" << s.mask.erosions;
   fs << "dilations" << s.mask.dilations;

   fs << "}";

   // roi settings
   log_msg(TRACE, "save_settings(): write ROI settings");
   fs << "detection_region" << "{";
   fs << "width" << s.roi.width;
   fs << "height" << s.roi.height;
   fs << "x_offset" << s.roi.x_offset;
   fs << "y_offset" << s.roi.y_offset;
   fs << "percent_min" << s.roi.percent_min;
   fs << "percent_max" << s.roi.percent_max;
   fs << "}";
  
   fs.release();
   log_msg(TRACE, "save_settings(): done");
   return 0;  
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static struct range_t range(int min, int max)
{
   return { min, max };
}

void set_default(struct camera_settings_t* s) {
   // mask
   log_msg(TRACE, "set_default(): default mask settings");
   s->mask.hue         = range(0, 179);
   s->mask.saturation  = range(0, 255);
   s->mask.value       = range(0, 255);
   s->mask.erosions    = 0;	
   s->mask.dilations   = 0;

   // roi
   log_msg(TRACE, "set_default(): default ROI settings");
   s->roi.width        =  64;	
   s->roi.height       =  64;	
   s->roi.x_offset     = 288;
   s->roi.y_offset     = 178;
   s->roi.percent_min  =   0;	
   s->roi.percent_max  =   0;
   log_msg(TRACE, "set_default(): done");
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
