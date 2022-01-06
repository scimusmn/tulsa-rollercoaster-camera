#ifndef PROCESSING_H
#define PROCESSING_H

#include <opencv2/core.hpp>
#include "settings.h"

void build_mask(cv::Mat& destination, cv::Mat& input, struct mask_settings_t settings);
bool region_triggered(cv::Mat& mask, struct roi_settings_t settings);
void draw_window(cv::Mat& frame, struct roi_settings_t settings, bool triggered);

#endif
