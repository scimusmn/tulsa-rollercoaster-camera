project(tulsa-rollercoaster-camera C CXX)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_BUILD_TYPE Debug)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/src)
set(SOURCES ${SOURCE_DIR}/rollercoaster.cpp ${SOURCE_DIR}/settings.cpp ${SOURCE_DIR}/arduino_util.cpp ${SOURCE_DIR}/smmServer.cpp ${SOURCE_DIR}/get_callbacks.cpp ${SOURCE_DIR}/post_callbacks.cpp ${SOURCE_DIR}/mg/mongoose.c ${SOURCE_DIR}/b64/base64.c)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(rollercoaster ${SOURCES})

target_link_libraries(rollercoaster Threads::Threads ${OpenCV_LIBS} m serialport)

if(WIN32)
  target_link_libraries(rollercoaster ws2_32)
endif()
