#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

#include "mask.h"

void build_mask(cv::Mat& destination, cv::Mat& input, struct mask_settings_t settings)
{
   cv::Mat input_hsv, tmp;
   cv::cvtColor(input, input_hsv, cv::COLOR_BGR2HSV);
   std::vector<cv::Mat> channels;
   cv::split(input_hsv, channels);
   
   // hue
   cv::threshold(chan[0], destination,
		 settings.hue.max,
		 255, THRESH_BINARY_INV);
   cv::threshold(chan[0], tmp,
		 settings.hue.min-1,
		 255, THRESH_BINARY);
   cv::bitwise_and(tmp, destination, destination);

   // saturation
   cv::threshold(chan[1], tmp,
		 settings.saturation.max,
		 255, THRESH_BINARY_INV);
   cv::bitwise_and(tmp, destination, destination);
   cv::threshold(chan[1], tmp,
		 settings.saturation.min-1,
		 255, THRESH_BINARY);
   cv::bitwise_and(mask_tmp, destination, destination);

   // value
   cv::threshold(chan[2], tmp,
		 settings.value.max,
		 255, THRESH_BINARY_INV);
   cv::bitwise_and(tmp, destination, destination);    
   cv::threshold(chan[2], tmp,
		 settings.value.min-1,
		 255, THRESH_BINARY);
   cv::bitwise_and(tmp, destination, destination);

   // erode & dilate
   cv::erode(destination, destination,
	     cv::Mat(), cv::Point(-1, -1),
	     settings.erosions);
   cv::dilate(destination, destination,
	      cv::Mat(), cv::Point(-1, -1),
	      settings.dilations);
}


void print_mask_settings(struct mask_settings_t s)
{
   std::cout << "hue: (" << s.hue.min << "-" << s.hue.max << ")" << std::endl;
   std::cout << "saturation: (" << s.saturation.min << "-" << s.saturation.max << ")" << std::endl;
   std::cout << "value: (" << s.value.min << "-" << s.value.max << ")" << std::endl;
   std::cout << "erosions: " << s.erosions << std::endl;
   std::cout << "dilations: " << s.dilations << std::endl;
}
