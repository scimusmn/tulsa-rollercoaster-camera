#include "callbacks.hpp"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// image-serving utility functions
static void serve_img_(cv::Mat image, httpMessage& m) {
  // get raw JPEG bytes from frame
  std::vector<unsigned char> rawJpegBuffer;
  cv::imencode(".jpeg", image, rawJpegBuffer);

  // convert to base64
  unsigned int bufferSize = b64e_size(rawJpegBuffer.size())+1;
  unsigned char* b64JpegBuffer = (unsigned char*) malloc(bufferSize * sizeof(unsigned char));
  b64_encode(rawJpegBuffer.data(), rawJpegBuffer.size(), b64JpegBuffer);
  
  m.replyHttpContent("image/jpeg", std::string((char*) b64JpegBuffer, bufferSize));
  free(b64JpegBuffer);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

static void serve_img(httpMessage& msg, std::mutex& img_mtx, cv::Mat img, double scale) {
  // send base64-encoded JPEG to client, respecting mutex locks and doing error checking
  cv::Mat small;
  bool ok = false;

  img_mtx.lock();
  if (!img.empty()) {
    cv::resize(img, small, cv::Size(), scale, scale);
    ok = true;
  }
  img_mtx.unlock();

  if (ok) {
    serve_img_(small, msg);
  }
  else {
    msg.replyHttpError(503, "image not yet loaded");
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// image-serving callbacks
  
void serve_camera1_frame(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  serve_img(message, g->camera1_frame_mtx, g->camera1_frame, g->image_quality);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void serve_camera2_frame(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  serve_img(message, g->camera2_frame_mtx, g->camera2_frame, g->image_quality);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_camera1_mask(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  serve_img(message, g->camera1_mask_mtx, g->camera1_mask, g->image_quality);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_camera2_mask(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  serve_img(message, g->camera2_mask_mtx, g->camera2_mask, g->image_quality);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// settings-serving utility function
static void serve_settings(httpMessage& msg, camera_settings settings) {
  std::string buffer = "{";
  buffer += "\"h_max\":";
  buffer += std::to_string(settings.h_max);
  buffer += ",\"h_min\":";   
  buffer += std::to_string(settings.h_min);
  buffer += ",\"s_max\":";   
  buffer += std::to_string(settings.s_max);
  buffer += ",\"s_min\":";   
  buffer += std::to_string(settings.s_min);
  buffer += ",\"v_max\":";   
  buffer += std::to_string(settings.v_max);
  buffer += ",\"v_min\":";   
  buffer += std::to_string(settings.v_min);
  buffer += ",\"erosions\":";
  buffer += std::to_string(settings.erosions);
  buffer += ",\"dilations\":";
  buffer += std::to_string(settings.dilations);
  buffer += ",\"width\":";
  buffer += std::to_string(settings.width);
  buffer += ",\"height\":";
  buffer += std::to_string(settings.height);
  buffer += ",\"x_offset\":";
  buffer += std::to_string(settings.x_offset);
  buffer += ",\"y_offset\":";
  buffer += std::to_string(settings.y_offset);
  buffer += ",\"percent_min\":";
  buffer += std::to_string(settings.percent_min);
  buffer += ",\"percent_max\":";
  buffer += std::to_string(settings.percent_max);
  buffer += "}";
  
  msg.replyHttpContent("text/plain", buffer);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
// settings-serving callbacks

void serve_camera1_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  serve_settings(message, g->settings.camera1_settings);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void serve_camera2_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;
  serve_settings(message, g->settings.camera2_settings);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*                        ,d88b.d88b,
                          88888888888
                          `Y8888888Y'
                            `Y888Y'
                              `Y'                                */
