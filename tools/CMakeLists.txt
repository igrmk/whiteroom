cmake_minimum_required(VERSION 3.13)

project(tools LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LOCAL_LIBS "${CMAKE_SOURCE_DIR}/../libs")

include_directories(${LOCAL_LIBS}/cppformat/include)
include_directories(${LOCAL_LIBS}/portaudio-build/include)
include_directories(${LOCAL_LIBS}/ffmpeg-build/include)
link_directories(${LOCAL_LIBS}/portaudio-build/lib)
link_directories(${LOCAL_LIBS}/ffmpeg-build/lib)
if(WIN32)
    link_directories(${LOCAL_LIBS}/portaudio-build/bin)
    link_directories(${LOCAL_LIBS}/ffmpeg-build/bin)
endif()


if(UNIX AND NOT APPLE)
    find_package(ALSA REQUIRED)
endif()

add_executable(rtsp-connect rtsp-connect.cpp ../src/rtsp.cpp ../src/rtsp.h)
target_link_libraries(rtsp-connect PRIVATE avformat swscale avcodec avutil)

add_executable(audio-devices audio-devices.cpp)
target_link_libraries(audio-devices PRIVATE portaudio portaudiocpp)
if(UNIX AND NOT APPLE)
    target_link_libraries(audio-devices PRIVATE ${ALSA_LIBRARIES} jack)
endif()

if(WIN32)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/../tools-bundle)
    set(RUNTIME_DIR .)

    install(
        TARGETS audio-devices rtsp-connect
        BUNDLE DESTINATION .
        COMPONENT runtime
        RUNTIME DESTINATION "${RUNTIME_DIR}")

    install(CODE [[
        set(LOCAL_LIBS "${CMAKE_SOURCE_DIR}/../libs")

        include(BundleUtilities)

        set(DIRS "${LOCAL_LIBS}/ffmpeg-build/lib;${LOCAL_LIBS}/portaudio-build/lib")
        set(DIRS "$DIRS;${LOCAL_LIBS}/ffmpeg-build/bin;${LOCAL_LIBS}/portaudio-build/bin")

        set(BUNDLE "${CMAKE_INSTALL_PREFIX}/audio-devices.exe")
        fixup_bundle("${BUNDLE}" "" "${DIRS}")

        set(BUNDLE "${CMAKE_INSTALL_PREFIX}/rtsp-connect.exe")
        fixup_bundle("${BUNDLE}" "" "${DIRS}")
    ]])
endif()
