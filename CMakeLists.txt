cmake_minimum_required(VERSION 3.19)
project(ScreenRecorder_pds)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

add_executable(ScreenRecorder_pds main.cpp ScreenRecorder.cpp ScreenRecorder.h)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavdevice
        libavfilter
        libavformat
        libavcodec
        libswresample
        libswscale
        libavutil
        )

target_link_libraries(ScreenRecorder_pds
        PkgConfig::LIBAV
        )

##find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h)
##find_library(AVCODEC_LIBRARY avcodec)

##find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
##find_library(AVFORMAT_LIBRARY avformat)

##find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
##find_library(AVUTIL_LIBRARY avutil)

##find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
##find_library(AVDEVICE_LIBRARY avdevice)

##target_include_directories(ScreenRecorder_pds PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR})
##target_link_libraries(ScreenRecorder_pds PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY})