cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)

project(rollercoaster)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#include_directories( C:/opencv/source/modules/calib3d/include C:/opencv/source/modules/core/include C:/opencv/source/modules/cudaarithm/include C:/opencv/source/modules/cudabgsegm/include C:/opencv/source/modules/cudacodec/include C:/opencv/source/modules/cudafeatures2d/include C:/opencv/source/modules/cudafilters/include C:/opencv/source/modules/cudaimgproc/include C:/opencv/source/modules/cudalegacy/include C:/opencv/source/modules/cudaobjdetect/include C:/opencv/source/modules/cudaoptflow/include C:/opencv/source/modules/cudastereo/include C:/opencv/source/modules/cudawarping/include C:/opencv/source/modules/cudev/include C:/opencv/source/modules/dnn/include C:/opencv/source/modules/features2d/include C:/opencv/source/modules/flann/include C:/opencv/source/modules/highgui/include C:/opencv/source/modules/imgcodecs/include C:/opencv/source/modules/imgproc/include C:/opencv/source/modules/java/include C:/opencv/source/modules/js/include C:/opencv/source/modules/ml/include C:/opencv/source/modules/objdetect/include C:/opencv/source/modules/photo/include C:/opencv/source/modules/python/include C:/opencv/source/modules/shape/include C:/opencv/source/modules/stitching/include C:/opencv/source/modules/superres/include C:/opencv/source/modules/ts/include C:/opencv/source/modules/video/include C:/opencv/source/modules/videoio/include C:/opencv/source/modules/videostab/include C:/opencv/source/modules/viz/include C:/opencv/source/modules/world/include C:/opencv/source/include C:/opencv/build )

#link_directories( C:/opencv/build/lib )

#add_executable(rollercoaster rollercoaster.cpp DeviceEnumerator.cpp)
#target_link_libraries( rollercoaster opencv_core348 opencv_highgui348 opencv_videoio348 ole32 oleaut32 strmiids )

add_executable(configurator config.cpp DeviceEnumerator.cpp)
target_link_libraries(configurator stdc++ ole32 oleaut32 strmiids ${OpenCV_LIBS})
#target_link_libraries( configurator opencv_core348 opencv_highgui348 opencv_videoio348 opencv_imgproc348 ole32 oleaut32 strmiids )
