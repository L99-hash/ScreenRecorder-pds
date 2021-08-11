//
// Created by enrico on 11/08/21.
//

#include "ScreenRecorder.h"

using namespace std;

ScreenRecorder::ScreenRecorder() {
    avcodec_register_all();
    avdevice_register_all();
}

ScreenRecorder::~ScreenRecorder() {

}

int ScreenRecorder::openCamera() {
    value = 0;
    options = nullptr;
    pAVFormatContext = nullptr;

    pAVFormatContext = avformat_alloc_context();
#ifdef _WIN32
    AVDictionary* opt = nullptr;
    pAVInputFormat = av_find_input_format("gdigrab");
    //Set some options
    //grabbing frame rate
    //av_dict_set(&options,"framerate","5",0);
    //The distance from the left edge of the screen or desktop
    //av_dict_set(&options,"offset_x","20",0);
    //The distance from the top edge of the screen or desktop
    //av_dict_set(&options,"offset_y","40",0);
    //Video frame size. The default is to capture the full screen
    //av_dict_set(&options,"video_size","640x480",0);
    if(avformat_open_input(&pAVFormatContext, "desktop", pAVInputFormat, &opt)!=0){
        cerr << "Couldn't open input stream" << endl;
        exit(-1);
    }
#elif defined linux
    AVDictionary* opt = nullptr;
    //permits to set the capturing from screen
    //Set some options
    //grabbing frame rate
    //av_dict_set(&options,"framerate","5",0);
    //Make the grabbed area follow the mouse
    //av_dict_set(&options,"follow_mouse","centered",0);
    //Video frame size. The default is to capture the full screen
    //av_dict_set(&options,"video_size","640x480",0);
    pAVInputFormat = av_find_input_format("x11grab");
    value = avformat_open_input(&pAVFormatContext, ":0.0+10,250", pAVInputFormat, &opt);

    if(value !=0 ){
        cerr << "Error in opening input device" << endl;
        exit(-1);
    }
#else

    show_avfoundation_device();
    pAVInputFormat = av_find_input_format("avfoundation");
    if(avformat_open_input(&pAVFormatContext, "1", pAVInputFormat, nullptr)!=0){
        cerr << "Error in opening input device" << endl;
        exit(-1);
    }

#endif

    //set frame per second
    value = av_dict_set(&options, "framerate", "30", 0);
    if(value < 0){
        cerr << "Error in setting dictionary value (setting framerate)" << endl;
        exit(-1);
    }

    value = av_dict_set(&options, "preset", "medium", 0);
    if(value < 0){
        cerr << "Error in setting dictionary value (setting preset value)" << endl;
        exit(-1);
    }

    VideoStreamIndx = -1;

    for(int i=0; i<pAVFormatContext->nb_streams; i++){
        if(pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            VideoStreamIndx = i;
            break;
        }
    }

    if(VideoStreamIndx == -1){
        cerr << "Error: unable to find stream index" << endl;
        exit(-2);
    }

    pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec;

    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    if(pAVCodec == nullptr){
        cerr << "Error: unable to find the decoder" << endl;
        exit(-3);
    }

    return 0;
}

int ScreenRecorder::initOutputFile() {
    outAVFormatContext = nullptr;
    value = 0;
    outputFile = "../media/output.mp4";

    avformat_alloc_output_context2(&outAVFormatContext, nullptr, nullptr, outputFile);
    if(outAVFormatContext == nullptr){
        cerr << "Error in allocating AVFormatContext" << endl;
        exit(-4);
    }

    outputFormat = av_guess_format(nullptr, outputFile, nullptr);
    if(outputFile == nullptr){
        cerr << "Error in guessing the video format, try with correct format" << endl;
        exit(-5);
    }

    videoSt = avformat_new_stream(outAVFormatContext, nullptr);
    if(videoSt == nullptr){
        cerr << "Error in creating AVFormatStream" << endl;
        exit(-6);
    }

    outAVCodecContext = avcodec_alloc_context3(outAVCodec);
    if(outAVCodecContext == nullptr){
        cerr << "Error in allocating the codec context" << endl;
        exit(-7);
    }

    //set properties of the video file
    outAVCodecContext = videoSt->codec;
    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 400000; // 2500000
    outAVCodecContext->width = 1920;
    outAVCodecContext->height = 1080;
    outAVCodecContext->gop_size = 3;
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30; // 15fps

    if(codec_id == AV_CODEC_ID_H264){
        av_opt_set(outAVCodecContext->priv_data, "preset", "slow", 0);
    }

    outAVCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    if(outAVCodec == nullptr){
        cerr << "Error in finding the AVCodec, try again with the correct codec" << endl;
        exit(-8);
    }

    if(outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER){
        outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    value = avcodec_open2(outAVCodecContext, outAVCodec, nullptr);
    if(value < 0){
        cerr << "Error in opening the AVCodec" << endl;
        exit(-9);
    }

    //create an empty video file
    if(!(outAVFormatContext->flags & AVFMT_NOFILE)){
        if(avio_open2(&outAVFormatContext->pb, outputFile, AVIO_FLAG_WRITE, nullptr, nullptr) < 0){
            cerr << "Error in creating the video file" << endl;
            exit(-10);
        }
    }

    if(outAVFormatContext->nb_streams == 0){
        cerr << "Output file does not contain any stream" << endl;
        exit(-11);
    }

    value = avformat_write_header(outAVFormatContext, &options);
    if(value < 0){
        cerr << "Error in writing the header context" << endl;
        exit(-12);
    }

    return 0;
}