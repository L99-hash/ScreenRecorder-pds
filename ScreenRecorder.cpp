//
// Created by enrico on 11/08/21.
//

#include "ScreenRecorder.h"

using namespace std;
void show_avfoundation_device(){
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("avfoundation");
    printf("==AVFoundation Device Info===\n");
    avformat_open_input(&pFormatCtx,"",iformat,&options);
    printf("=============================\n");
}

ScreenRecorder::ScreenRecorder() {
    avcodec_register_all();
    avdevice_register_all();
}

ScreenRecorder::~ScreenRecorder() {
    avformat_close_input(&pAVFormatContext);
    if(pAVFormatContext == nullptr){
        cout << "File close successfully" << endl;
    }
    else{
        cerr << "Error: unable to close the file" << endl;
        exit(-1);
    }

    avformat_free_context(pAVFormatContext);
    if(pAVFormatContext == nullptr){
        cout << "AVFormat freed successfully" << endl;
    }
    else{
        cerr << "Error: unable to free AVFormatContext" << endl;
        exit(-1);
    }
}

int ScreenRecorder::openDevice() {
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
    int offset_x = 100, offset_y = 100;
    string url = ":0.0+" + to_string(offset_x) + "," + to_string(offset_y);  //custom string to set the start point of the screen section
    //permits to set the capturing from screen
    //Set some options
    //grabbing frame rate
    //av_dict_set(&options,"framerate","5",0);
    //Make the grabbed area follow the mouse
    //av_dict_set(&options,"follow_mouse","centered",0);
    //Video frame size. The default is to capture the full screen
    //av_dict_set(&opt, "offset_x", "20", 0);
    //av_dict_set(&opt, "offset_y", "20", 0);
    av_dict_set(&opt,"video_size","640x480",0);   //option to set the dimension of the screen section to record
    pAVInputFormat = av_find_input_format("x11grab");
    value = avformat_open_input(&pAVFormatContext, url.c_str(), pAVInputFormat, &opt);

    if(value !=0 ){
        cerr << "Error in opening input device" << endl;
        exit(-1);
    }
#else

    show_avfoundation_device();
    pAVInputFormat = av_find_input_format("avfoundation");
    if(avformat_open_input(&pAVFormatContext, "1", pAVInputFormat, nullptr)!=0){  //TODO trovare un modo per selezionare sempre lo schermo
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

    /*int h, w;
    cout << "Insert height and width [h w]: ";   //custom screen dimension to record
    cin >> h >> w;*/

    //pAVCodecContext->height = 1080;
    //pAVCodecContext->width = 1920;

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

    outAVCodec = nullptr;   //avoid segmentation fault on call avcodec_alloc_context3(outAVCodec)
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
    outAVCodecContext->width = 1920;   //dimension of the output video file
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

int ScreenRecorder::captureVideoFrames() {

    int flag;
    int frameFinished = 0;

    int frameIndex = 0;
    value = 0;

    pAVPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    if(pAVPacket == nullptr){
        cerr << "Error in allocating AVPacket" << endl;
        exit(-1);
    }

    pAVFrame = av_frame_alloc();
    if(pAVFrame == nullptr){
        cerr << "Error: unable to alloc the AVFrame resources" << endl;
        exit(-1);
    }
    //pAVFrame->height = 1080;
    //pAVFrame->width = 1920;

    outFrame = av_frame_alloc();
    if(outFrame == nullptr){
        cerr << "Error: unable to alloc the AVFrame resources for out frame" << endl;
        exit(-1);
    }

    int videoOutBuffSize;
    int nBytes = av_image_get_buffer_size(outAVCodecContext->pix_fmt, outAVCodecContext->width, outAVCodecContext->height, 32);
    uint8_t* videoOutBuff = (uint8_t *) av_malloc(nBytes);

    if(videoOutBuff == nullptr){
        cerr << "Error: unable to allocate memory" << endl;
        exit(-1);
    }

    value = av_image_fill_arrays(outFrame->data, outFrame->linesize, videoOutBuff, AV_PIX_FMT_YUV420P, outAVCodecContext->width, outAVCodecContext->height, 1);
    if(value < 0){
        cerr << "Error in filling image array" << endl;
    }

    SwsContext* swsCtx_;
    if(avcodec_open2(pAVCodecContext, pAVCodec, nullptr) < 0) {
        cerr << "Could not open codec" << endl;
        exit(-1);
    }
    swsCtx_ = sws_getContext(pAVCodecContext->width, pAVCodecContext->height, pAVCodecContext->pix_fmt, outAVCodecContext->width, outAVCodecContext->height, outAVCodecContext->pix_fmt, SWS_BICUBIC,
                             nullptr, nullptr, nullptr);
    int ii=0;
    int noFrames = 100;

    cout << "Enter No frames to capture: ";
    cin >> noFrames;

    AVPacket outPacket;
    int j = 0;

    int gotPicture;

    while(av_read_frame(pAVFormatContext, pAVPacket) >= 0){
        if(ii++ == noFrames)
            break;

        if(pAVPacket->stream_index == VideoStreamIndx){
            value = avcodec_decode_video2(pAVCodecContext, pAVFrame, &frameFinished, pAVPacket);
            if(value < 0){
                cout << "Unable to decode video" << endl;
            }

            if(frameFinished) { //frame successfully decoded
                sws_scale(swsCtx_, pAVFrame->data, pAVFrame->linesize, 0, pAVCodecContext->height, outFrame->data, outFrame->linesize);
                av_init_packet(&outPacket);
                outPacket.data = nullptr;
                outPacket.size = 0;
                avcodec_encode_video2(outAVCodecContext, &outPacket, outFrame, &gotPicture);
                if(gotPicture){
                    if(outPacket.pts != AV_NOPTS_VALUE){
                        outPacket.pts = av_rescale_q(outPacket.pts, videoSt->codec->time_base, videoSt->time_base);
                    }
                    if(outPacket.dts != AV_NOPTS_VALUE){
                        outPacket.dts = av_rescale_q(outPacket.dts, videoSt->codec->time_base, videoSt->time_base);
                    }

                    cout << "Write frame " << j++ << " (size = " << outPacket.size / 1000 << ")" << endl;
                    if(av_write_frame(outAVFormatContext, &outPacket) != 0){
                        cerr << "Error in writing video frame" << endl;
                    }

                    av_packet_unref(&outPacket);
                }

                av_packet_unref(&outPacket);
            }
        }
    }

    value = av_write_trailer(outAVFormatContext);
    if(value < 0){
        cerr << "Error in writing av trailer" << endl;
        exit(-1);
    }

    av_free(videoOutBuff);


    return 0;
}
