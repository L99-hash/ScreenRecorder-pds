//
// Created by enrico on 11/08/21.
//

#include "ScreenRecorder.h"

using namespace std;

//Show Dshow Device
//used on Windows to select the audio device
void show_dshow_device() {
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options, "list_devices", "true", 0);
    AVInputFormat* iformat = av_find_input_format("dshow");
    printf("========Device Info=============\n");
    avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);
    printf("================================\n");
}

//Show AVFoundation Device
void show_avfoundation_device() {
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options, "list_devices", "true", 0);
    AVInputFormat* iformat = av_find_input_format("avfoundation");
    printf("==AVFoundation Device Info===\n");
    avformat_open_input(&pFormatCtx, "", iformat, &options);
    printf("=============================\n");
}

//get dimensions of main desktop
void getScreenResolution(int& width, int& height) {
#if defined __APPLE__
    Display* disp = XOpenDisplay(NULL);
    Screen* scrn = DefaultScreenOfDisplay(disp);
    width = scrn->width;
    height = scrn->height;
#endif
#if defined _WIN32
    width = (int)GetSystemMetrics(SM_CXSCREEN);
    height = (int)GetSystemMetrics(SM_CYSCREEN);
#endif
#if defined linux
    Display* disp = XOpenDisplay(NULL);
    Screen* scrn = DefaultScreenOfDisplay(disp);
    width = scrn->width;
    height = scrn->height;
#endif
}

ScreenRecorder::ScreenRecorder() : pauseCapture(false), stopCapture(false), started(false), activeMenu(true), recordAudio(false), disabledMenu(false) {
    avcodec_register_all();
    avdevice_register_all();
#if defined _WIN32
    ::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
    deviceName = "";
#endif
    getScreenResolution(width, height);
    screen_height = height;
    screen_width = width;

    cout << "Main desktop: " << width << "x" << height << endl;

    dir_path = ".";

    x_offset = 0;
    y_offset = 0;
}

ScreenRecorder::~ScreenRecorder() {
    if (started) {
        t_video.join();
        if (recordAudio)
            t_audio.join();

        //complete write on file
        value = av_write_trailer(outAVFormatContext);
        if (value < 0) {
            cerr << "Error in writing av trailer" << endl;
            exit(-1);
        }

        //close input device for audio
        avformat_close_input(&inAudioFormatContext);
        if (inAudioFormatContext == nullptr) {
            cout << "inAudioFormatContext close successfully" << endl;
        }
        else {
            cerr << "Error: unable to close the inAudioFormatContext" << endl;
            exit(-1);
        }
        avformat_free_context(inAudioFormatContext);
        if (inAudioFormatContext == nullptr) {
            cout << "AudioFormat freed successfully" << endl;
        }
        else {
            cerr << "Error: unable to free AudioFormatContext" << endl;
            exit(-1);
        }

        //close input device for video
        avformat_close_input(&pAVFormatContext);
        if (pAVFormatContext == nullptr) {
            cout << "File close successfully" << endl;
        }
        else {
            cerr << "Error: unable to close the file" << endl;
            exit(-1);
        }
        avformat_free_context(pAVFormatContext);
        if (pAVFormatContext == nullptr) {
            cout << "VideoFormat freed successfully" << endl;
        }
        else {
            cerr << "Error: unable to free VideoFormatContext" << endl;
            exit(-1);
        }
    }
}

//permits to stop recording
void ScreenRecorder::stopCommand() {
    unique_lock<mutex> ul(mu);
    stopCapture = true;
    if (pauseCapture)
        pauseCapture = false;
    cv.notify_all();
}

//permits to pause recording in order to be resumed later
void ScreenRecorder::pauseCommand() {
    unique_lock<mutex> ul(mu);
    if (!pauseCapture)
        pauseCapture = true;
}

//permits to resume paused recording
void ScreenRecorder::resumeCommand() {
    unique_lock<mutex> ul(mu);
    if (pauseCapture) {
        pauseCapture = false;
        cv.notify_all();
    }
}

//read if recording is started or not
bool ScreenRecorder::getStarted() {
        return started;
}

//read if menu is active or not (thread safe)
bool ScreenRecorder::getActiveMenu() {
    std::lock_guard<std::mutex> lg(mu);
    return activeMenu;
}

//comunicate that the menu has been activated (thread safe)
void ScreenRecorder::setActiveMenu(bool val) {
    std::lock_guard<std::mutex> lg(mu);
    activeMenu = val;
}

//read if menu is disabled
bool ScreenRecorder::getDisabledMenu() {
    std::lock_guard<std::mutex> lg(mu);
    return disabledMenu;
}

//read if audio is enabled r not
bool ScreenRecorder::getRecordAudio() {
    return recordAudio;
};

//permits to enable audio recording
void ScreenRecorder::setRecordAudio(bool val) {
    recordAudio = val;
}

//permits to set output directory (defuault: .)
void ScreenRecorder::setOutputDir(const char* dir) {
    dir_path = dir;
}

//overide screen dimension to record (default->desktop dimension)
void ScreenRecorder::setScreenDimension(int width, int height) {
    this->width = width;
    this->height = height;

    cout << "Screen dimension setted correctly" << endl;
}

//override screen offset (default->x=0, y=0)
void ScreenRecorder::setScreenOffset(int x_offset, int y_offset) {
    this->x_offset = x_offset;
    this->y_offset = y_offset;

    cout << "Screen offset setted correctly" << endl;
}

//initialize the output file
int ScreenRecorder::initOutputFile() throw() {
    value = 0;
    outputFile = const_cast<char*>("output.mp4");
#if defined _WIN32
    string completePath = string(dir_path) + string("\\") + string(outputFile);
#else
    string completePath = string(dir_path) + string("/") + string(outputFile);
#endif

    outAVFormatContext = nullptr;
    outputAVFormat = av_guess_format(nullptr, "output.mp4", nullptr);
    if (outputAVFormat == nullptr) {
        
        throw error("Error in guessing the video format, try with correct format");
    }

    //avformat_alloc_output_context2(&outAVFormatContext, outputAVFormat, nullptr, outputFile);  //for just video, we used this
    avformat_alloc_output_context2(&outAVFormatContext, outputAVFormat, outputAVFormat->name, const_cast<char*>(completePath.c_str()));

    if (outAVFormatContext == nullptr) {
        
        throw error("Error in allocating outAVFormatContext");
    }

    /*===========================================================================*/
    try{
        this->generateVideoStream();
        if (recordAudio) this->generateAudioStream();
    }
    catch(exception e){
        throw e;
    }

    //create an empty video file
    if (!(outAVFormatContext->flags & AVFMT_NOFILE)) {
        if (avio_open2(&outAVFormatContext->pb, const_cast<char*>(completePath.c_str()), AVIO_FLAG_WRITE, nullptr, &videoOptions) < 0) {
            
            throw error("Error in creating the video file");
        }
    }

    if (outAVFormatContext->nb_streams == 0) {
        
        throw error("Error in creating the video file");
    }

    value = avformat_write_header(outAVFormatContext, &videoOptions);
    if (value < 0) {
        
        throw error("Error in writing the header context" );
    }

    return 0;
}

//start recording (audio is optional)
//open devices ans initialize the output file
void ScreenRecorder::startRecording() throw() {
    if ((width + x_offset) > screen_width || (height + y_offset) > screen_height) {
        cout << "Diemensions of screen section setted are too large for the screen.";
        setActiveMenu(true);
        
        return;
    }

    try{
        if (recordAudio) openAudioDevice();
        openVideoDevice();
        initOutputFile();
    }
    catch(error e){
        throw e;
    }
    
    //detect possible exception during lauch threads
    try{
        started = true;
        t_video = std::move(std::thread{ [this]() {
            this->captureVideoFrames();
        }
            });
        if (recordAudio) {
            t_audio = std::move(std::thread{ [this]() {
                this->captureAudio();
            }
            });
        }
    }
    catch(exception e){
        started = false;
        throw e;
    }
}

/*==================================== VIDEO ==============================*/

//open input video device
int ScreenRecorder::openVideoDevice() throw() {
    value = 0;
    videoOptions = nullptr;
    pAVFormatContext = nullptr;

    pAVFormatContext = avformat_alloc_context();


    string dimension = to_string(width) + "x" + to_string(height);
    av_dict_set(&videoOptions, "video_size", dimension.c_str(), 0);   //option to set the dimension of the screen section to record
    value = av_dict_set(&videoOptions, "framerate", "25", 0);
    if (value < 0) {
        
        throw error("Error in setting dictionary value (setting framerate)");
    }

    value = av_dict_set(&videoOptions, "preset", "ultrafast", 0);
    if (value < 0) {
        
        throw error("Error in setting dictionary value (setting preset value)");
    }
#ifdef _WIN32
    
    pAVInputFormat = av_find_input_format("gdigrab");
    //Set some options
    //grabbing frame rate
    //The distance from the left edge of the screen or desktop
    av_dict_set(&videoOptions, "offset_x", to_string(x_offset).c_str(), 0);
    //The distance from the top edge of the screen or desktop
    av_dict_set(&videoOptions, "offset_y", to_string(y_offset).c_str(), 0);
    //Video frame size. The default is to capture the full screen
    //av_dict_set(&options,"video_size","640x480",0);

    if (avformat_open_input(&pAVFormatContext, "desktop", pAVInputFormat, &videoOptions) != 0) {
        cerr << "Couldn't open input stream" << endl;
        exit(-1);
    }

#elif defined linux
    //permits to set the capturing from screen
    //Set some options
    //grabbing frame rate
    //av_dict_set(&options,"framerate","5",0);
    //Make the grabbed area follow the mouse
    //av_dict_set(&options,"follow_mouse","centered",0);
    //Video frame size. The default is to capture the full screen
    //av_dict_set(&opt, "offset_x", "20", 0);
    //av_dict_set(&opt, "offset_y", "20", 0);
    //AVDictionary* opt = nullptr;
    //int offset_x = 0, offset_y = 0;
    string url = ":0.0+" + to_string(x_offset) + "," + to_string(y_offset);  //custom string to set the start point of the screen section
    
    pAVInputFormat = av_find_input_format("x11grab");
    value = avformat_open_input(&pAVFormatContext, url.c_str(), pAVInputFormat, &videoOptions);

    if (value != 0) {
        
        throw error("Error in opening input device");
    }
    
#else

    show_avfoundation_device();

    //The distance from the left edge of the screen or desktop
    value = av_dict_set(&videoOptions, "vf", ("crop=" + to_string(width) + ":" + to_string(height) + ":" + to_string(x_offset) + ":" +
        to_string(y_offset)).c_str(), 0);

    if (value < 0) {
        
        throw error("Error in setting crop");
    }

    
    value = av_dict_set(&videoOptions, "pixel_format", "yuv420p", 0);
    if (value < 0) {
        
        throw error("Error in setting pixel format");
    }


    pAVInputFormat = av_find_input_format("avfoundation");

    if (avformat_open_input(&pAVFormatContext, "1:none", pAVInputFormat, &videoOptions) != 0) {
        
        throw error("Error in opening input device");
    }
#endif
    
    //get video stream infos from context
    value = avformat_find_stream_info(pAVFormatContext, nullptr);
    if (value < 0) {
        
        throw error("Error in retrieving the stream info");
    }

    VideoStreamIndx = -1;
    for (int i = 0; i < pAVFormatContext->nb_streams; i++) {
        if (pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            VideoStreamIndx = i;
            break;
        }
    }
    if (VideoStreamIndx == -1) {
        
        throw error("Error: unable to find video stream index");
    }

    pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec;
    
    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id/*params->codec_id*/);
    if (pAVCodec == nullptr) {
        
        throw error("Error: unable to find decoder video");
    }


    return 0;
}

//find information for video stream
//called by initOutputFile()
void ScreenRecorder::generateVideoStream() throw() {
    outVideoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);  //AV_CODEC_ID_MPEG4
    if (outVideoCodec == nullptr) {
        
        throw error("Error in finding the AVCodec, try again with the correct codec");
    }

    //Generate video stream
    videoSt = avformat_new_stream(outAVFormatContext, outVideoCodec);
    if (videoSt == nullptr) {
        
        throw error("Error in creating AVFormatStream");
    }

    

    //set properties of the video file (stream)
    outVideoCodecContext = videoSt->codec;
    outVideoCodecContext->codec_id = AV_CODEC_ID_H264;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outVideoCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outVideoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    outVideoCodecContext->bit_rate = /*10000000;*/ 2500000;
    outVideoCodecContext->width = width;   //dimension of the output video file
    outVideoCodecContext->height = height;
    outVideoCodecContext->gop_size = 15;     // 3
    //outVideoCodecContext->global_quality = 500;   //for AV_CODEC_ID_MPEG4
    outVideoCodecContext->max_b_frames = 2;
    outVideoCodecContext->time_base.num = 1;
    outVideoCodecContext->time_base.den = 25; // 15fps
    outVideoCodecContext->bit_rate_tolerance = 400000;

    if (outVideoCodecContext->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(outVideoCodecContext, "preset", "ultrafast", 0);
        av_opt_set(outVideoCodecContext, "cabac", "1", 0);
        av_opt_set(outVideoCodecContext, "ref", "3", 0);
        av_opt_set(outVideoCodecContext, "deblock", "1:0:0", 0);
        av_opt_set(outVideoCodecContext, "analyse", "0x3:0x113", 0);
        av_opt_set(outVideoCodecContext, "subme", "7", 0);
        av_opt_set(outVideoCodecContext, "chroma_qp_offset", "4", 0);
        av_opt_set(outVideoCodecContext, "rc", "crf", 0);
        av_opt_set(outVideoCodecContext, "rc_lookahead", "40", 0);
        av_opt_set(outVideoCodecContext, "crf", "10.0", 0);
    }

    if (outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        outVideoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    value = avcodec_open2(outVideoCodecContext, outVideoCodec, nullptr);
    if (value < 0) {
        
        throw error("Error in opening the AVCodec");
    }

    outVideoStreamIndex = -1;
    for (int i = 0; i < outAVFormatContext->nb_streams; i++) {
        if (outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            outVideoStreamIndex = i;
        }
    }
    if (outVideoStreamIndex < 0) {
        
        throw error("Error: cannot find a free stream index for video output");
    }
    avcodec_parameters_from_context(outAVFormatContext->streams[outVideoStreamIndex]->codecpar, outVideoCodecContext);
}

//executed in a second thread to record video
int ScreenRecorder::captureVideoFrames() {
    int64_t pts = 0;
    int flag;
    int frameFinished = 0;
    bool endPause = false;
    int numPause = 0;

    int64_t numFrame = 0;

    int frameIndex = 0;
    value = 0;

    pAVPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (pAVPacket == nullptr) {
        cerr << "Error in allocating AVPacket" << endl;
        exit(-1);
        //throw error("Error in allocating AVPacket");
    }

    pAVFrame = av_frame_alloc();
    if (pAVFrame == nullptr) {
        cerr << "Error: unable to alloc the AVFrame resources" << endl;
        exit(-1);
        //throw error("Error: unable to alloc the AVFrame resources");
    }
    

    outFrame = av_frame_alloc();
    if (outFrame == nullptr) {
        cerr << "Error: unable to alloc the AVFrame resources for out frame" << endl;
        exit(-1);
        //throw error("Error: unable to alloc the AVFrame resources for out frame");
    }

    int videoOutBuffSize;
    int nBytes = av_image_get_buffer_size(outVideoCodecContext->pix_fmt, outVideoCodecContext->width, outVideoCodecContext->height, 32);
    uint8_t* videoOutBuff = (uint8_t*)av_malloc(nBytes);

    if (videoOutBuff == nullptr) {
        cerr << "Error: unable to allocate memory" << endl;
        exit(-1);
        //throw error("Error: unable to allocate memory");
    }

    value = av_image_fill_arrays(outFrame->data, outFrame->linesize, videoOutBuff, AV_PIX_FMT_YUV420P, outVideoCodecContext->width, outVideoCodecContext->height, 1);
    if (value < 0) {
        cerr << "Error in filling image array" << endl;
    }

    SwsContext* swsCtx_;

    if (avcodec_open2(pAVCodecContext, pAVCodec, nullptr) < 0) {
        cerr << "Could not open codec" << endl;
        exit(-1);
    }

    swsCtx_ = sws_getContext(pAVCodecContext->width, pAVCodecContext->height, pAVCodecContext->pix_fmt, outVideoCodecContext->width, outVideoCodecContext->height, outVideoCodecContext->pix_fmt, SWS_BICUBIC,
        nullptr, nullptr, nullptr);


    cout << "pVCodec Context width: height " << pAVCodecContext->width << " " << pAVCodecContext->height << endl;
    cout << "outVideoCodecContext width: height " << outVideoCodecContext->width << " " << outVideoCodecContext->height << endl;
    AVPacket outPacket;
    int gotPicture;

    double pauseTime, startSeconds, precSeconds=0;
    time_t startPause, stopPause;
    time_t startTime;
    time(&startTime);

    startSeconds = difftime(startTime, 0);

    cout << endl;

    while (true) {
        unique_lock<mutex> ul(mu);
        if (pauseCapture) {

#if defined linux
        avformat_close_input(&pAVFormatContext);
        if (pAVFormatContext != nullptr) {
            cerr << "Error: unable to close the pAVFormatContext (before pause)" << endl;
            exit(-1);
        }
#endif
            endPause = true;
            time(&startPause);
            numPause++;
            
        }
        cv.wait(ul, [this]() { return !pauseCapture; });   //pause capture (not busy waiting)
        if (endPause) {
#if defined linux
            string url = ":0.0+" + to_string(x_offset) + "," + to_string(y_offset);  //custom string to set the start point of the screen section
            string dimension = to_string(width) + "x" + to_string(height);
            av_dict_set(&videoOptions, "video_size", dimension.c_str(), 0);
            value = avformat_open_input(&pAVFormatContext, url.c_str(), pAVInputFormat, &videoOptions);
            
            if (value != 0) {
                cerr << "Error in opening input device (video after pause)" << endl;
                exit(-1);
            }

            value = avformat_find_stream_info(pAVFormatContext, nullptr);
            if (value < 0) {
                cerr << "Error in retrieving the stream info" << endl;
                exit(-1);
            }
            
            VideoStreamIndx = -1;
            for (int i = 0; i < pAVFormatContext->nb_streams; i++) {
                if (pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                    VideoStreamIndx = i;
                    break;
                }
            }
            if (VideoStreamIndx == -1) {
                cerr << "Error: unable to find video stream index" << endl;
                exit(-2);
            }

            pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec;
            
            pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id/*params->codec_id*/);
            if (pAVCodec == nullptr) {
                cerr << "Error: unable to find decoder video" << endl;
                exit(-1);
            }

            if (avcodec_open2(pAVCodecContext, pAVCodec, nullptr) < 0) {
                cerr << "Could not open codec" << endl;
                exit(-1);
            }

            swsCtx_ = sws_getContext(pAVCodecContext->width, pAVCodecContext->height, pAVCodecContext->pix_fmt, outVideoCodecContext->width, outVideoCodecContext->height, outVideoCodecContext->pix_fmt, SWS_BICUBIC,
                nullptr, nullptr, nullptr);
#endif
            endPause = false;
            time(&stopPause);
            pauseTime = difftime(stopPause, startPause);
            startSeconds += pauseTime;
            
        }

        if (stopCapture) { //check if the capture has to stop
            break;
        }
        ul.unlock();

        if (av_read_frame(pAVFormatContext, pAVPacket) >= 0 && pAVPacket->stream_index == VideoStreamIndx) {
            av_packet_rescale_ts(pAVPacket, pAVFormatContext->streams[VideoStreamIndx]->time_base, pAVCodecContext->time_base);
            value = avcodec_decode_video2(pAVCodecContext, pAVFrame, &frameFinished, pAVPacket);
            if (value < 0) {
                cout << "Unable to decode video" << endl;
            }

            pAVFrame->pts = numFrame;

            if (frameFinished) { //frame successfully decoded

                numFrame++;

                av_init_packet(&outPacket);
                outPacket.data = nullptr;
                outPacket.size = 0;

                if (outAVFormatContext->streams[outVideoStreamIndex]->start_time <= 0) {
                    outAVFormatContext->streams[outVideoStreamIndex]->start_time = pAVFrame->pts;
                }

                //disable warning on the console
                outFrame->width = outVideoCodecContext->width;
                outFrame->height = outVideoCodecContext->height;
                outFrame->format = outVideoCodecContext->pix_fmt;

                outFrame->pts = pAVFrame->pts;
                sws_scale(swsCtx_, pAVFrame->data, pAVFrame->linesize, 0, pAVCodecContext->height, outFrame->data, outFrame->linesize);

                avcodec_encode_video2(outVideoCodecContext, &outPacket, outFrame, &gotPicture);

                if (gotPicture) {
                    

                    av_packet_rescale_ts(&outPacket, outVideoCodecContext->time_base, outAVFormatContext->streams[outVideoStreamIndex]->time_base);
                    

                    time_t timer;
                    double seconds;

                    mu.lock();
                    if (!activeMenu) {
                        disabledMenu = false;
                        time(&timer);
                        seconds = difftime(timer, 0) - startSeconds;
                        int h = (int)(seconds / 3600);
                        int m = (int)(seconds / 60) % 60;
                        int s = (int)(seconds) % 60;
                        if ((seconds - precSeconds) >= 1) {
                            std::cout << "\r" << std::setw(2) << std::setfill('0') << h << ':'
                                << std::setw(2) << std::setfill('0') << m << ':'
                                << std::setw(2) << std::setfill('0') << s << std::flush;
                            precSeconds = seconds;
                        }

                    }
                    else {
                        disabledMenu = true;
                        
                    }
                    mu.unlock();

                    write_lock.lock();






                    if (av_write_frame(outAVFormatContext, &outPacket) != 0) {
                        cerr << "Error in writing video frame" << endl;
                    }
                    write_lock.unlock();
                    av_packet_unref(&outPacket);
                }
                gotPicture = 0;
                av_packet_unref(&outPacket);
                av_free_packet(pAVPacket);  //avoid memory saturation
            }
            frameFinished = 0;
        }
    }

    
   

    av_free(videoOutBuff);

    return 0;
}

/*==========================================  AUDIO  ============================*/

//open audio device
int ScreenRecorder::openAudioDevice() throw() {
    audioOptions = nullptr;
    inAudioFormatContext = nullptr;

    inAudioFormatContext = avformat_alloc_context();
    value = av_dict_set(&audioOptions, "sample_rate", "44100", 0);
    if (value < 0) {
        
        throw error("Error: cannot set audio sample rate");
    }
    value = av_dict_set(&audioOptions, "async", "25", 0);
    if (value < 0) {
        
        throw error("Error: cannot set audio sample rate");
    }

#if defined linux
    audioInputFormat = av_find_input_format("alsa");
    value = avformat_open_input(&inAudioFormatContext, "hw:0", audioInputFormat, &audioOptions);
    
    if (value != 0) {
        
        throw error("Error in opening input device (audio)");
    }
#endif

#if defined _WIN32
    show_dshow_device();
    cout << "\nPlease select the audio device among those listed before: ";
    getchar();
    getline(cin, deviceName);
    deviceName = "audio=" + deviceName;

    audioInputFormat = av_find_input_format("dshow");
    value = avformat_open_input(&inAudioFormatContext, deviceName.c_str(), audioInputFormat, &audioOptions);
    
    if (value != 0) {
        
        throw error("Error in opening input device (audio)");
    }
#endif

#if defined __APPLE__
    audioInputFormat = av_find_input_format("avfoundation");
    value = avformat_open_input(&inAudioFormatContext, "none:0", audioInputFormat, &audioOptions);
#endif

    value = avformat_find_stream_info(inAudioFormatContext, nullptr);
    if (value != 0) {
        
        throw error("Error: cannot find the audio stream information");
    }

    audioStreamIndx = -1;
    for (int i = 0; i < inAudioFormatContext->nb_streams; i++) {
        if (inAudioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndx = i;
            break;
        }
    }
    if (audioStreamIndx == -1) {
        
        throw error("Error: unable to find audio stream index");
    }
}

//find information for audio stream
//called by initOutputFile()
void ScreenRecorder::generateAudioStream() throw(){
    
    AVCodecParameters* params = inAudioFormatContext->streams[audioStreamIndx]->codecpar;
    inAudioCodec = avcodec_find_decoder(params->codec_id);
    if (inAudioCodec == nullptr) {
        
        throw error("Error: cannot find the audio decoder");
    }

    inAudioCodecContext = avcodec_alloc_context3(inAudioCodec);
    if (avcodec_parameters_to_context(inAudioCodecContext, params) < 0) {
        
        throw error("Cannot create codec context for audio input");
    }

    value = avcodec_open2(inAudioCodecContext, inAudioCodec, nullptr);
    if (value < 0) {
        
        throw error("Error: cannot open the input audio codec");
    }

    //Generate audio stream
    outAudioCodecContext = nullptr;
    outAudioCodec = nullptr;
    int i;

    AVStream* audio_st = avformat_new_stream(outAVFormatContext, nullptr);
    if (audio_st == nullptr) {
        
        throw error("Error: cannot create audio stream");
    }

    outAudioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (outAudioCodec == nullptr) {
        
        throw error("Error: cannot find requested encoder");
    }

    outAudioCodecContext = avcodec_alloc_context3(outAudioCodec);
    if (outAudioCodecContext == nullptr) {
        
        throw error("Error: cannot create related VideoCodecContext");
    }

    if ((outAudioCodec)->supported_samplerates) {
        outAudioCodecContext->sample_rate = (outAudioCodec)->supported_samplerates[0];
        for (i = 0; (outAudioCodec)->supported_samplerates[i]; i++) {
            if ((outAudioCodec)->supported_samplerates[i] == inAudioCodecContext->sample_rate)
                outAudioCodecContext->sample_rate = inAudioCodecContext->sample_rate;
        }
    }
    outAudioCodecContext->codec_id = AV_CODEC_ID_AAC;
    outAudioCodecContext->sample_fmt = (outAudioCodec)->sample_fmts ? (outAudioCodec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outAudioCodecContext->channels = inAudioCodecContext->channels;
    outAudioCodecContext->channel_layout = av_get_default_channel_layout(outAudioCodecContext->channels);
    outAudioCodecContext->bit_rate = 96000;
    outAudioCodecContext->time_base = { 1, inAudioCodecContext->sample_rate };

    outAudioCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    if ((outAVFormatContext)->oformat->flags & AVFMT_GLOBALHEADER) {
        outAudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outAudioCodecContext, outAudioCodec, nullptr) < 0) {
        
        throw error("error in opening the avcodec");
    }

    //find a free stream index
    outAudioStreamIndex = -1;
    for (i = 0; i < outAVFormatContext->nb_streams; i++) {
        if (outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            outAudioStreamIndex = i;
        }
    }
    if (outAudioStreamIndex < 0) {
        
        throw error("Error: cannot find a free stream for audio on the output");
    }

    avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outAudioCodecContext);
}

int ScreenRecorder::init_fifo()
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(fifo = av_audio_fifo_alloc(outAudioCodecContext->sample_fmt,
        outAudioCodecContext->channels, 1))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

int ScreenRecorder::add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size) {
    int error;
    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void**)converted_input_samples, frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

int ScreenRecorder::initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size) {
    int _error;
    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = (uint8_t**)calloc(output_codec_context->channels,
        sizeof(**converted_input_samples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if (av_samples_alloc(*converted_input_samples, nullptr,
        output_codec_context->channels,
        frame_size,
        output_codec_context->sample_fmt, 0) < 0) {

       
        throw error("Errorr: could not allocate memeory for samples in all channels (audio)");
    }
    return 0;
}

//executed in a second thread to record audio
void ScreenRecorder::captureAudio() {
    int ret;
    AVPacket* inPacket, * outPacket;
    AVFrame* rawFrame, * scaledFrame;
    uint8_t** resampledData;

    bool endPause = false;

    init_fifo();

    //allocate space for a packet
    inPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!inPacket) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
        //throw error("Cannot allocate an AVPacket for encoded video");
    }
    av_init_packet(inPacket);

    //allocate space for a packet
    rawFrame = av_frame_alloc();
    if (!rawFrame) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
        //throw error("Cannot allocate an AVPacket for encoded video");
    }

    scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
        //throw error("Cannot allocate an AVPacket for encoded video");
    }

    outPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!outPacket) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
        //throw error("Cannot allocate an AVPacket for encoded video");
    }

    //init the resampler
    SwrContext* resampleContext = nullptr;
    resampleContext = swr_alloc_set_opts(resampleContext,
        av_get_default_channel_layout(outAudioCodecContext->channels),
        outAudioCodecContext->sample_fmt,
        outAudioCodecContext->sample_rate,
        av_get_default_channel_layout(inAudioCodecContext->channels),
        inAudioCodecContext->sample_fmt,
        inAudioCodecContext->sample_rate,
        0,
        nullptr);
    if (!resampleContext) {
        cerr << "Cannot allocate the resample context" << endl;
        exit(1);
        //throw error("Cannot allocate the resample context");
    }
    if ((swr_init(resampleContext)) < 0) {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&resampleContext);
        exit(1);
        //throw error("Could not open resample context");
    }

    

    while (true) {
        unique_lock<mutex> ul(mu);
        if (pauseCapture) {
            endPause = true;
#if defined _WIN32
            avformat_close_input(&inAudioFormatContext);
            if (inAudioFormatContext != nullptr) {
                cerr << "Error: unable to close the inAudioFormatContext" << endl;
                exit(-1);
                //throw error("Error: unable to close the inAudioFormatContext (before pause)");
            }
#endif
        }
        cv.wait(ul, [this]() { return !pauseCapture; });   //pause capture (not busy waiting)

        if (endPause) {
            endPause = false;
#if defined _WIN32
            value = avformat_open_input(&inAudioFormatContext, deviceName.c_str(), audioInputFormat, &audioOptions);
            
            if (value != 0) {
                cerr << "Error in opening input device (audio)" << endl;
                exit(-1);
                //throw error("Error in opening input device (audio after pause)");
            }

            value = avformat_find_stream_info(inAudioFormatContext, nullptr);
            if (value != 0) {
                cerr << "Error: cannot find the audio stream information" << endl;
                exit(-1);
                //throw error("Error: cannot find the audio stream information");
            }

            audioStreamIndx = -1;
            for (int i = 0; i < inAudioFormatContext->nb_streams; i++) {
                if (inAudioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                    audioStreamIndx = i;
                    break;
                }
            }
            if (audioStreamIndx == -1) {
                cerr << "Error: unable to find audio stream index" << endl;
                exit(-2);
                //throw error("Error: unable to find audio stream index");
            }
#endif
        }
        if (stopCapture) { //check if the capture has to stop
            break;
        }

        ul.unlock();

        if (av_read_frame(inAudioFormatContext, inPacket) >= 0 && inPacket->stream_index == audioStreamIndx) {
            //decode audio routing
            av_packet_rescale_ts(outPacket, inAudioFormatContext->streams[audioStreamIndx]->time_base, inAudioCodecContext->time_base);
            if ((ret = avcodec_send_packet(inAudioCodecContext, inPacket)) < 0) {
                cout << "Cannot decode current audio packet " << ret << endl;
                continue;
            }
            
            while (ret >= 0) {
                ret = avcodec_receive_frame(inAudioCodecContext, rawFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    cerr << "Error during decoding" << endl;
                    exit(1);
                    //throw error("Error during decoding");
                }
                if (outAVFormatContext->streams[outAudioStreamIndex]->start_time <= 0) {
                    outAVFormatContext->streams[outAudioStreamIndex]->start_time = rawFrame->pts;
                }
                initConvertedSamples(&resampledData, outAudioCodecContext, rawFrame->nb_samples);

                swr_convert(resampleContext,
                    resampledData, rawFrame->nb_samples,
                    (const uint8_t**)rawFrame->extended_data, rawFrame->nb_samples);

                
                add_samples_to_fifo(resampledData, rawFrame->nb_samples);

                //raw frame ready
                av_init_packet(outPacket);
                outPacket->data = nullptr;    // packet data will be allocated by the encoder
                outPacket->size = 0;

                const int frame_size = FFMAX(av_audio_fifo_size(fifo), outAudioCodecContext->frame_size);

                scaledFrame = av_frame_alloc();
                if (!scaledFrame) {
                    cerr << "Cannot allocate an AVPacket for encoded video" << endl;
                    exit(1);
                    //throw error("Cannot allocate an AVPacket for encoded video");
                }

                scaledFrame->nb_samples = outAudioCodecContext->frame_size;
                scaledFrame->channel_layout = outAudioCodecContext->channel_layout;
                scaledFrame->format = outAudioCodecContext->sample_fmt;
                scaledFrame->sample_rate = outAudioCodecContext->sample_rate;
                
                av_frame_get_buffer(scaledFrame, 0);

                while (av_audio_fifo_size(fifo) >= outAudioCodecContext->frame_size) {

                    ret = av_audio_fifo_read(fifo, (void**)(scaledFrame->data), outAudioCodecContext->frame_size);
                    scaledFrame->pts = pts;
                    pts += scaledFrame->nb_samples;
                    if (avcodec_send_frame(outAudioCodecContext, scaledFrame) < 0) {
                        cerr << "Cannot encode current audio packet " << endl;
                        exit(1);
                        //throw error("Cannot encode current audio packet");
                    }
                    while (ret >= 0) {
                        ret = avcodec_receive_packet(outAudioCodecContext, outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        else if (ret < 0) {
                            cerr << "Error during encoding" << endl;
                            exit(1);
                            //throw error("Error during encoding");
                        }
                        //outPacket ready
                        av_packet_rescale_ts(outPacket, outAudioCodecContext->time_base, outAVFormatContext->streams[outAudioStreamIndex]->time_base);

                        outPacket->stream_index = outAudioStreamIndex;

                        write_lock.lock();

                        if (av_write_frame(outAVFormatContext, outPacket) != 0)
                        {
                            cerr << "Error in writing audio frame" << endl;
                        }
                        write_lock.unlock();
                        av_packet_unref(outPacket);
                    }
                    ret = 0;
                }// got_picture
                av_frame_free(&scaledFrame);
                av_packet_unref(outPacket);
               
            }
        }
    }
}
