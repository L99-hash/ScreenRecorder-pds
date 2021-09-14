#pragma once
//
// Created by enrico on 11/08/21.
//

#ifndef SCREENRECORDER_PDS_SCREENRECORDER_H
#define SCREENRECORDER_PDS_SCREENRECORDER_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>
#include <condition_variable>
#include <mutex>
#include <exception>
#include <string>
#include <time.h>
#include <sstream>
#include <iomanip>

#define __STDC_CONSTANT_MACROS

//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"

#include "libavdevice/avdevice.h"

#include "libavfilter/avfilter.h"
    //#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#include "libavformat/avformat.h"
#include "libavformat/avio.h"

    // libav resample

#include "libavutil/opt.h"
#include "libavutil/common.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/file.h"

    // lib swresample

#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/audio_fifo.h"
}

#if defined _WIN32
#define VIDEO "gdigrab"
#define VIDEO_DEVICE "desktop"
#define AUDIO "dshow"
#define AUDIO_DEVICE "audio="
#endif

#if defined linux
#define VIDEO "x11grab"
#define VIDEO_DEVICE ":0.0"
#define AUDIO "alsa"
#define AUDIO_DEVICE "hw:0"
#endif

#if defined __APPLE__
#define VIDEO "avfoundation"
#define VIDEO_DEVICE "0:none"
#define AUDIO "avfoudation"
#define AUDIO_DEVICE "none:0"
#endif

class ScreenRecorder {
    AVInputFormat* pAVInputFormat;
    AVFormatContext* pAVFormatContext;
    AVDictionary* options;
    AVCodecContext* pAVCodecContext;
    AVCodec* pAVCodec;
    AVFormatContext* outAVFormatContext;
    char* outputFile;
    AVOutputFormat* outputAVFormat;
    AVStream* videoSt;
    AVCodecContext* outVideoCodecContext;
    AVCodec* outVideoCodec;
    AVPacket* pAVPacket;
    AVFrame* pAVFrame;
    AVFrame* outFrame;
    int outVideoStreamIndex;

    AVInputFormat* audioInputFormat;
    AVFormatContext* inAudioFormatContext;
    AVCodecContext* inAudioCodecContext;
    AVStream* audioSt;
    AVCodecContext* outAudioCodecContext;
    AVCodec* inAudioCodec;
    AVCodec* outAudioCodec;
    int outAudioStreamIndex;
    AVAudioFifo* fifo;
    AVDictionary* audioOptions;


    int value;   //used for checking values returned from various functions
    int codec_id;
    int out_size;
    int VideoStreamIndx;
    int audioStreamIndx;
    double video_pts;
    const char* dev_name;

    int width;
    int height;
    
public:
   
    std::condition_variable cv;
    std::mutex mu;
    std::mutex write_lock;
    bool pauseCapture;
    bool stopCapture;
    bool activeMenu;
    bool started = false;
    ScreenRecorder();
    ~ScreenRecorder();
    int openVideoDevice() throw();
    int initOutputFile();
    int captureVideoFrames();
    void captureAudio();
    int initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size);
    int add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size);
    int init_fifo();
    int openAudioDevice();

    void generateVideoStream();

    void generateAudioStream();
    void setStarted(bool val) {
        started = val;
    }
    bool getActiveMenu() {
        std::lock_guard<std::mutex> lg(mu);
        return activeMenu;
    }
    void setActiveMenu(bool val) {
        std::lock_guard<std::mutex> lg(mu);
        activeMenu = val;
    }
};


#endif //SCREENRECORDER_PDS_SCREENRECORDER_H
