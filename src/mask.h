#ifndef MASK_H
#define MASK_H

#include <opencv2/core.hpp>

struct range_t {
   int min;
   int max;
};


struct mask_settings_t {
   struct range_t hue;
   struct range_t saturation;
   struct range_t value;
   int erosions;
   int dilations;
};


void build_mask(cv::Mat& destination, cv::Mat& input, struct mask_settings_t settings);
void print_mask_settings(struct mask_settings_t settings);


#endif
