cmake_minimum_required(VERSION 3.13)

project(tools LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/../libs/cppformat/include)
include_directories(${CMAKE_SOURCE_DIR}/../libs/portaudio-build/include)
link_directories(${CMAKE_SOURCE_DIR}/../libs/portaudio-build/lib)

add_executable(rtsp-connect rtsp-connect.cpp ../src/rtsp.cpp ../src/rtsp.h)
target_link_libraries(rtsp-connect PRIVATE avformat swscale avcodec avutil)

find_package(ALSA REQUIRED)
add_executable(audio-devices audio-devices.cpp)
target_link_libraries(audio-devices PRIVATE portaudio portaudiocpp ${ALSA_LIBRARIES} jack)
