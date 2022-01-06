macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

project(ui-test CXX)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(ROLLERCOASTER_SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/src)
set(CONFIG_SOURCES
  ${ROLLERCOASTER_SOURCE_DIR}/config.cpp
  ${ROLLERCOASTER_SOURCE_DIR}/settings.cpp
  ${ROLLERCOASTER_SOURCE_DIR}/logging.cpp
#  ${ROLLERCOASTER_SOURCE_DIR}/update.cpp
#  ${ROLLERCOASTER_SOURCE_DIR}/arduino_util.cpp
)
set(ROLLERCOASTER_SOURCES
  ${ROLLERCOASTER_SOURCE_DIR}/rollercoaster.cpp
  ${ROLLERCOASTER_SOURCE_DIR}/settings.cpp
  ${ROLLERCOASTER_SOURCE_DIR}/arduino_util.cpp )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

#find_package(libserialport REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})

add_definitions(${GTK_CFLAGS_OTHER})

# add_executable(rollercoaster ${ROLLERCOASTER_SOURCES})
add_executable(config ${CONFIG_SOURCES})

# target_link_libraries(rollercoaster ${OpenCV_LIBS} ${GTK_LIBRARIES} m serialport)
target_link_libraries(config ${OpenCV_LIBS} ${GTK_LIBRARIES} m serialport)

#print_all_variables()
configure_file(res/config.ui bin/config.ui COPYONLY)
configure_file(res/icon.png bin/icon.png COPYONLY)
