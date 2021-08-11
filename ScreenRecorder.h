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
}

class ScreenRecorder {
    AVInputFormat* pAVInputFormat;
    AVFormatContext* pAVFormatContext;
    AVDictionary* options;
    AVCodecContext* pAVCodecContext;
    AVCodec* pAVCodec;
    AVFormatContext* outAVFormatContext;
    const char* outputFile;
    AVOutputFormat* outputFormat;
    AVStream* videoSt;
    AVCodecContext* outAVCodecContext;
    AVCodec* outAVCodec;

    int value;   //used for checking values returned from various functions
    int codec_id;
    int out_size;
    int VideoStreamIndx;
    double video_pts;
    const char* dev_name;
public:
    ScreenRecorder();
    ~ScreenRecorder();
    int openCamera();
    int initOutputFile();
};


#endif //SCREENRECORDER_PDS_SCREENRECORDER_H
