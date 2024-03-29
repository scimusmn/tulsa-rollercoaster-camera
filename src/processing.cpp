#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>

#include "processing.h"
#include "logging.h"

void build_mask(cv::Mat& destination, cv::Mat& input, struct mask_settings_t settings)
{
   log_msg(TRACE, "build_mask(): split color channels");
   cv::Mat input_hsv, tmp;
   cv::cvtColor(input, input_hsv, cv::COLOR_BGR2HSV);
   std::vector<cv::Mat> channels;
   cv::split(input_hsv, channels);
   
   // hue
   log_msg(TRACE, "build_mask(): get hue mask");
   cv::threshold(channels[0], destination,
		 settings.hue.max,
		 255, cv::THRESH_BINARY_INV);
   cv::threshold(channels[0], tmp,
		 settings.hue.min-1,
		 255, cv::THRESH_BINARY);
   cv::bitwise_and(tmp, destination, destination);

   // saturation
   log_msg(TRACE, "build_mask(): get saturation mask");
   cv::threshold(channels[1], tmp,
		 settings.saturation.max,
		 255, cv::THRESH_BINARY_INV);
   cv::bitwise_and(tmp, destination, destination);
   cv::threshold(channels[1], tmp,
		 settings.saturation.min-1,
		 255, cv::THRESH_BINARY);
   cv::bitwise_and(tmp, destination, destination);

   // value
   log_msg(TRACE, "build_mask(): get value mask");
   cv::threshold(channels[2], tmp,
		 settings.value.max,
		 255, cv::THRESH_BINARY_INV);
   cv::bitwise_and(tmp, destination, destination);    
   cv::threshold(channels[2], tmp,
		 settings.value.min-1,
		 255, cv::THRESH_BINARY);
   cv::bitwise_and(tmp, destination, destination);

   // erode & dilate
   log_msg(TRACE, "build_mask(): erode & dilate");
   cv::erode(destination, destination,
	     cv::Mat(), cv::Point(-1, -1),
	     settings.erosions);
   cv::dilate(destination, destination,
	      cv::Mat(), cv::Point(-1, -1),
	      settings.dilations);

   log_msg(TRACE, "build_mask(): done");
}


bool region_triggered(cv::Mat& mask, struct roi_settings_t settings)
{
   log_msg(TRACE, "region_triggered(): extract ROI mat");
   cv::Mat roi = mask(cv::Rect(settings.x_offset,
			       settings.y_offset,
			       settings.width, settings.height));
   log_msg(TRACE, "region_triggered(): compute percentage");
   int num_nonzero = cv::countNonZero(roi);
   double percent_nonzero = 100 * ((double)num_nonzero)/(settings.width * settings.height);
   return (percent_nonzero > settings.percent_min) && (percent_nonzero <= settings.percent_max);
   log_msg(TRACE, "region_triggered(): done");
}


void draw_window(cv::Mat& frame, struct roi_settings_t settings, bool triggered)
{
   log_msg(TRACE, "draw_window(): drawing");
   cv::Scalar color(0, 0, 255); // red (BGR)
   if (triggered)
      color = cv::Scalar(0, 255, 0); // green
   cv::rectangle(frame,
		 cv::Rect(settings.x_offset,
			  settings.y_offset,
			  settings.width,
			  settings.height),
		 color, 2);
   log_msg(TRACE, "draw_window(): done");
}
