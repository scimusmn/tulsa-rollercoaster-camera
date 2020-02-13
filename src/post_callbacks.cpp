#include "callbacks.hpp"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void save_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;

  if (save_settings(&g->settings, g->settings_file) != 0) {
    std::cerr << "WARNING: failed to save settings" << std::endl;
    message.replyHttpError(500,"Failed to save settings");
  }
  else {
    std::cout << "saved.\n";
    message.replyHttpOk();
  }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
void set_camera1_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;

  camera_settings settings;
  try {
    settings.h_max         = std::stoi(message.getHttpVariable("h_max"));
    settings.s_max         = std::stoi(message.getHttpVariable("s_max"));
    settings.v_max         = std::stoi(message.getHttpVariable("v_max"));
    settings.h_min         = std::stoi(message.getHttpVariable("h_min"));   
    settings.s_min         = std::stoi(message.getHttpVariable("s_min"));   
    settings.v_min         = std::stoi(message.getHttpVariable("v_min"));   
    settings.erosions      = std::stoi(message.getHttpVariable("erosions"));   
    settings.dilations     = std::stoi(message.getHttpVariable("dilations"));
    settings.width         = std::stoi(message.getHttpVariable("width"));
    settings.height        = std::stoi(message.getHttpVariable("height"));
    settings.x_offset      = std::stoi(message.getHttpVariable("x_offset"));
    settings.y_offset      = std::stoi(message.getHttpVariable("y_offset"));
    settings.percent_min   = std::stoi(message.getHttpVariable("percent_min"));
    settings.percent_max   = std::stoi(message.getHttpVariable("percent_max"));
  }
  catch(std::invalid_argument error) {
    std::cerr << "error: invalid argument encountered in set_camera1_settings()" << std::endl;
    message.replyHttpError(422,"Invalid number");
    return;
  }
  
  g->settings.camera1_settings = settings;

  message.replyHttpOk();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void set_camera2_settings(httpMessage message, void* data) {
  struct globals* g = (struct globals*) data;

  camera_settings settings;
  try {
    settings.h_max         = std::stoi(message.getHttpVariable("h_max"));
    settings.s_max         = std::stoi(message.getHttpVariable("s_max"));
    settings.v_max         = std::stoi(message.getHttpVariable("v_max"));
    settings.h_min         = std::stoi(message.getHttpVariable("h_min"));   
    settings.s_min         = std::stoi(message.getHttpVariable("s_min"));   
    settings.v_min         = std::stoi(message.getHttpVariable("v_min"));   
    settings.erosions      = std::stoi(message.getHttpVariable("erosions"));   
    settings.dilations     = std::stoi(message.getHttpVariable("dilations"));
    settings.width         = std::stoi(message.getHttpVariable("width"));
    settings.height        = std::stoi(message.getHttpVariable("height"));
    settings.x_offset      = std::stoi(message.getHttpVariable("x_offset"));
    settings.y_offset      = std::stoi(message.getHttpVariable("y_offset"));
    settings.percent_min   = std::stoi(message.getHttpVariable("percent_min"));
    settings.percent_max   = std::stoi(message.getHttpVariable("percent_max"));
  }
  catch(std::invalid_argument error) {
    std::cerr << "error: invalid argument encountered in set_camera2_settings()" << std::endl;
    message.replyHttpError(422,"Invalid number");
    return;
  }
  
  g->settings.camera2_settings = settings;

  message.replyHttpOk();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*                        ,d88b.d88b,
                          88888888888
                          `Y8888888Y'
                            `Y888Y'
                              `Y'                                */

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
