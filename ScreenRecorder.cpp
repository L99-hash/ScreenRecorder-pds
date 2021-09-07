//
// Created by enrico on 11/08/21.
//

#include "ScreenRecorder.h"
#include <exception>

using namespace std;

class error : exception {
private:
    const char *desc;
public:
    error(const char * description): desc(description){}
    const char* what() const noexcept {
        return desc;
    }
};

void show_avfoundation_device(){
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    AVDictionary* options = NULL;
    av_dict_set(&options,"list_devices","true",0);
    AVInputFormat *iformat = av_find_input_format("avfoundation");
    printf("==AVFoundation Device Info===\n");
    avformat_open_input(&pFormatCtx,"",iformat,&options);
    printf("=============================\n");
}

ScreenRecorder::ScreenRecorder(): stopCapture(false) , pauseCapture(false){
    avcodec_register_all();
    avdevice_register_all();

    width = 1920;//640;
    height = 1104;//480;
}

ScreenRecorder::~ScreenRecorder(){
    avformat_close_input(&pAVFormatContext);
    if(pAVFormatContext == nullptr){
        cout << "File close successfully" << endl;
    }
    else{
        cerr << "Error: unable to close the file" << endl;
        exit(-1);
        //throw "Error: unable to close the file";
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

int ScreenRecorder::openDevice() throw(){
    value = 0;

    /*=================        VIDEO      ====================*/
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
    //permits to set the capturing from screen
    //Set some options
    //grabbing frame rate
    //av_dict_set(&options,"framerate","5",0);
    //Make the grabbed area follow the mouse
    //av_dict_set(&options,"follow_mouse","centered",0);
    //Video frame size. The default is to capture the full screen
    //av_dict_set(&opt, "offset_x", "20", 0);
    //av_dict_set(&opt, "offset_y", "20", 0);
    AVDictionary* opt = nullptr;
    int offset_x = 0, offset_y = 0;
    string url = ":0.0+" + to_string(offset_x) + "," + to_string(offset_y);  //custom string to set the start point of the screen section
    string dimension = to_string(width) + "x" + to_string(height);
    av_dict_set(&opt,"video_size",dimension.c_str(),0);   //option to set the dimension of the screen section to record
    pAVInputFormat = av_find_input_format("x11grab");
    value = avformat_open_input(&pAVFormatContext, url.c_str(), pAVInputFormat, &opt);

    if(value !=0 ){
        cerr << "Error in opening input device (video)" << endl;
        exit(-1);
        //throw error("Error in opening input device");
    }
    //get video stream infos from context
    value = avformat_find_stream_info(pAVFormatContext, nullptr);
    if (value < 0) {
        cout << "\nCannot find the stream information";
        exit(1);
    }
#else

    show_avfoundation_device();
    pAVInputFormat = av_find_input_format("avfoundation");
    if(avformat_open_input(&pAVFormatContext, "1", pAVInputFormat, nullptr)!=0){  //TODO trovare un modo per selezionare sempre lo schermo (forse "Capture screen 0")
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
        cerr << "Error: unable to find video stream index" << endl;
        exit(-2);
    }

    pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec;

    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    if(pAVCodec == nullptr){
        cerr << "Error: unable to find decoder video" << endl;
        exit(-1);
    }

    /*========================  AUDIO  ============================*/
    audioOptions = nullptr;
    inAudioFormatContext = nullptr;

    inAudioFormatContext = avformat_alloc_context();
    value = av_dict_set(&audioOptions, "sample_rate", "44100", 0);
    if(value < 0){
        cerr << "Error: cannot set audio sample rate" << endl;
        exit(-1);
    }

    //audioInputFormat = av_find_input_format("alsa");
    //value = avformat_open_input(&inAudioFormatContext, "hw:0", audioInputFormat, &audioOptions);
    audioInputFormat = av_find_input_format("pulse");
    value = avformat_open_input(&inAudioFormatContext, "default", audioInputFormat, &audioOptions);

    if(value != 0){
        cerr << "Error in opening input device (audio)" << endl;
        exit(-1);
        //throw error("Error in opening input device");
    }

    value = avformat_find_stream_info(inAudioFormatContext, nullptr);
    if(value != 0){
        cerr << "Error: cannot find the audio stream information" << endl;
        exit(-1);
    }

    audioStreamIndx = -1;
    for(int i=0; i<inAudioFormatContext->nb_streams; i++){
        if(inAudioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audioStreamIndx = i;
            break;
        }
    }

    if(audioStreamIndx == -1){
        cerr << "Error: unable to find audio stream index" << endl;
        exit(-2);
    }

    //inAudioCodecContext = inAudioFormatContext->streams[audioStreamIndx]->codec;
    AVCodecParameters  *params = inAudioFormatContext->streams[audioStreamIndx]->codecpar;
    inAudioCodec = avcodec_find_decoder(params->codec_id);
    if(inAudioCodec == nullptr){
        cerr << "Error: cannot find the audio decoder" << endl;
        exit(-1);
    }

    inAudioCodecContext = avcodec_alloc_context3(inAudioCodec);
    if(avcodec_parameters_to_context(inAudioCodecContext, params) < 0){
        cout << "Cannot create codec context for audio input" << endl;
    }

    value = avcodec_open2(inAudioCodecContext, inAudioCodec, nullptr);
    if(value < 0){
        cerr << "Error: cannot open the input audio codec" << endl;
        exit(-1);
    }

    /*int h, w;
    cout << "Insert height and width [h w]: ";   //custom screen dimension to record
    cin >> h >> w;*/

    //pAVCodecContext->height = 1080;
    //pAVCodecContext->width = 1920;

    return 0;
}

int ScreenRecorder::initOutputFile() {
    value = 0;
    outputFile = "../media/output.mp4";

    outAVFormatContext = nullptr;
    outputFormat = av_guess_format(nullptr, outputFile, nullptr);
    if(outputFile == nullptr){
        cerr << "Error in guessing the video format, try with correct format" << endl;
        exit(-5);
    }

    avformat_alloc_output_context2(&outAVFormatContext, outputFormat, nullptr, outputFile);
    if(outAVFormatContext == nullptr){
        cerr << "Error in allocating AVFormatContext" << endl;
        exit(-4);
    }

    /*===================================  VIDEO  ==================================*/

    //Generate video stream
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

    //set properties of the video file (stream)
    outAVCodecContext = videoSt->codec;
    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt  = AV_PIX_FMT_YUV420P;
    outAVCodecContext->bit_rate = 10000000; // 2500000
    outAVCodecContext->width = width;   //dimension of the output video file
    outAVCodecContext->height = height;
    outAVCodecContext->gop_size = 10;     // 3
    outAVCodecContext->global_quality = 500;
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;
    outAVCodecContext->time_base.den = 30; // 15fps
    outAVCodecContext->bit_rate_tolerance = 400000;

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

    /*===============================  AUDIO  ==================================*/

    //Generate audio stream
    outAudioCodecContext = nullptr;
    outAudioCodec = nullptr;
    int i;

    AVStream* audio_st = avformat_new_stream(outAVFormatContext, nullptr);
    if (audio_st == nullptr) {
        cerr << "Error: cannot create audio stream" << endl;
        exit(1);
    }
    outAudioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (outAudioCodec == nullptr) {
        cerr << "Error: cannot find requested encoder" << endl;
        exit(1);
    }

    outAudioCodecContext = avcodec_alloc_context3(outAudioCodec);
    if (outAudioCodecContext == nullptr) {
        cerr << "Error: cannot create related VideoCodecContext" << endl;
        exit(1);
    }

    /* set properties for the audio stream encoding*/
    /*outAudioCodecContext->sample_fmt = (outAudioCodec)->sample_fmts ? (outAudioCodec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outAudioCodecContext->bit_rate = 64000;

    if((outAudioCodec)->supported_samplerates){
        outAudioCodecContext->sample_rate = (outAudioCodec)->supported_samplerates[0];

        for(i=0; (outAudioCodec)->supported_samplerates[i]; i++){
            if(outAudioCodec->supported_samplerates[i] == 44100){
                outAudioCodecContext->sample_rate = 44100;
            }
        }
    }

    outAudioCodecContext->channels = av_get_channel_layout_nb_channels(outAudioCodecContext->channel_layout);
    outAudioCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;

    if((outAudioCodec)->channel_layouts){
        outAudioCodecContext->channel_layout = (outAudioCodec)->channel_layouts[0];

        for(i=0; outAudioCodec->channel_layouts[i]; i++){
            if(outAudioCodec->channel_layouts[i] == AV_CH_LAYOUT_STEREO){
                outAudioCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
    }

    outAudioCodecContext->channels = av_get_channel_layout_nb_channels(outAudioCodecContext->channel_layout);
    outAudioCodecContext->time_base = (AVRational) {1, outAudioCodecContext->sample_rate};

    if(outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER){
        outAudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    value = avcodec_open2(outAudioCodecContext, outAudioCodec, nullptr);
    if(value < 0) {
        cerr << "Error: cannot open AVCodec" << endl;
        exit(-1);
    }*/
    if ((outAudioCodec)->supported_samplerates) {
        outAudioCodecContext->sample_rate = (outAudioCodec)->supported_samplerates[0];
        for (i = 0; (outAudioCodec)->supported_samplerates[i]; i++) {
            if ((outAudioCodec)->supported_samplerates[i] == inAudioCodecContext->sample_rate)
                outAudioCodecContext->sample_rate = inAudioCodecContext->sample_rate;
        }
    }
    outAudioCodecContext->codec_id = AV_CODEC_ID_AAC;
    outAudioCodecContext->sample_fmt  = (outAudioCodec)->sample_fmts ? (outAudioCodec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outAudioCodecContext->channels  = inAudioCodecContext->channels;
    outAudioCodecContext->channel_layout = av_get_default_channel_layout(outAudioCodecContext->channels);
    outAudioCodecContext->bit_rate = 96000;
    outAudioCodecContext->time_base = { 1, inAudioCodecContext->sample_rate };

    outAudioCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    if ((outAVFormatContext)->oformat->flags & AVFMT_GLOBALHEADER) {
        outAudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outAudioCodecContext, outAudioCodec, nullptr)< 0) {
        cout << "\nerror in opening the avcodec with error: ";
        exit(1);
    }

    //find a free stream index
    outAudioStreamIndex = -1;
    for(i=0; i < outAVFormatContext->nb_streams; i++)
        if(outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN)
            outAudioStreamIndex = i;

        if(outAudioStreamIndex < 0) {
            cout << "\nCannot find a free stream for audio on the output";
            exit(1);
        }

        avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outAudioCodecContext);

    return 0;
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

int ScreenRecorder::add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size){
    int error;
    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples, frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

int ScreenRecorder::initConvertedSamples(uint8_t ***converted_input_samples,
                                         AVCodecContext *output_codec_context,
                                         int frame_size){
    int error;
    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = (uint8_t **)calloc(output_codec_context->channels,
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

        exit(1);
    }
    return 0;
}

static int64_t pts = 0;
void ScreenRecorder::captureAudio() {
    int ret;
    AVPacket *inPacket, *outPacket;
    AVFrame *rawFrame, *scaledFrame;
    uint8_t  **resampledData;
    //allocate space for a packet
    inPacket = (AVPacket *) av_malloc(sizeof (AVPacket));
    if(!inPacket) {
        cout << "\nCannot allocate an AVPacket for encoded video";
        exit(1);
    }
    av_init_packet(inPacket);

    //allocate space for a packet
    rawFrame = av_frame_alloc();
    if(!rawFrame) {
        cout << "\nCannot allocate an AVPacket for encoded video";
        exit(1);
    }

    outPacket = (AVPacket *) av_malloc(sizeof (AVPacket));
    if(!outPacket) {
        cout << "\nCannot allocate an AVPacket for encoded video";
        exit(1);
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
                                         0, nullptr);
    if(!resampleContext){
        cout << "\nCannot allocate the resample context";
        exit(1);
    }
    if ((swr_init(resampleContext)) < 0) {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&resampleContext);
        exit(1);
    }

    cout<<"\n\n[AudioThread] thread started!";
    while(true) {
        if(av_read_frame(inAudioFormatContext, inPacket) >= 0 && inPacket->stream_index == audioStreamIndx) {
            //decode video routing
            av_packet_rescale_ts(outPacket,  inAudioFormatContext->streams[audioStreamIndx]->time_base, inAudioCodecContext->time_base);
            if((ret = avcodec_send_packet(inAudioCodecContext, inPacket)) < 0){
                cout << "Cannot decode current video packet " <<  ret;
                continue;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(inAudioCodecContext, rawFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    exit(1);
                }
                if(outAVFormatContext->streams[outAudioStreamIndex]->start_time <= 0) {
                    outAVFormatContext->streams[outAudioStreamIndex]->start_time = rawFrame->pts;
                }
                initConvertedSamples(&resampledData, outAudioCodecContext, rawFrame->nb_samples);

                swr_convert(resampleContext,
                            resampledData, rawFrame->nb_samples,
                            (const uint8_t **)rawFrame->extended_data, rawFrame->nb_samples);

                add_samples_to_fifo(resampledData,rawFrame->nb_samples);

                //raw frame ready
                av_init_packet(outPacket);
                outPacket->data = nullptr;    // packet data will be allocated by the encoder
                outPacket->size = 0;

                const int frame_size = FFMAX(av_audio_fifo_size(fifo), outAudioCodecContext->frame_size);

                scaledFrame = av_frame_alloc();
                if(!scaledFrame) {
                    cout << "\nCannot allocate an AVPacket for encoded video";
                    exit(1);
                }

                scaledFrame->nb_samples     = outAudioCodecContext->frame_size;
                scaledFrame->channel_layout = outAudioCodecContext->channel_layout;
                scaledFrame->format         = outAudioCodecContext->sample_fmt;
                scaledFrame->sample_rate    = outAudioCodecContext->sample_rate;
                // scaledFrame->best_effort_timestamp = rawFrame->best_effort_timestamp;
                // scaledFrame->pts = rawFrame->pts;
                av_frame_get_buffer(scaledFrame,0);

                while (av_audio_fifo_size(fifo) >= outAudioCodecContext->frame_size){
                    ret = av_audio_fifo_read(fifo, (void **)(scaledFrame->data), outAudioCodecContext->frame_size);
                    scaledFrame->pts = pts;
                    pts += scaledFrame->nb_samples;
                    if(avcodec_send_frame(outAudioCodecContext, scaledFrame) < 0){
                        cout << "Cannot encode current audio packet ";
                        exit(1);
                    }
                    while(ret>=0){
                        ret = avcodec_receive_packet(outAudioCodecContext, outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        else if (ret < 0) {
                            fprintf(stderr, "Error during encoding\n");
                            exit(1);
                        }
                        //outPacket ready
                        av_packet_rescale_ts(outPacket, outAudioCodecContext->time_base,  outAVFormatContext->streams[outAudioStreamIndex]->time_base);


                        outPacket->stream_index = outAudioStreamIndex;

                        if(av_interleaved_write_frame(outAVFormatContext , outPacket) != 0)
                        {
                            cout<<"\nerror in writing audio frame";
                        }
                        av_packet_unref(outPacket);
                    }
                    ret=0;
                }// got_picture
                av_frame_free(&scaledFrame);
                av_packet_unref(outPacket);
                //av_freep(&resampledData[0]);
                // free(resampledData);
            }
        }
    }
}

int ScreenRecorder::captureVideoFrames() {

    int flag;
    int frameFinished = 0;

    int frameIndex = 0;
    value = 0;

    init_fifo();

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

    //cout << "Enter No frames to capture: ";
    //cin >> noFrames;

    AVPacket outPacket;
    int j = 0;

    int gotPicture;

    while(av_read_frame(pAVFormatContext, pAVPacket) >= 0){
        unique_lock<mutex> ul(mu);
        //ul.unlock();
        //if(ii++ == noFrames)
          //  break;

        //ul.lock();
        if(pauseCapture) cout << "Pause" << endl;
        cv.wait(ul, [this](){ return !pauseCapture;});   //pause capture (not busy waiting)

        if(stopCapture)  //check if the capture has to stop
            break;

        ul.unlock();

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

                //disable warning on the console
                outFrame->width = outAVCodecContext->width;
                outFrame->height = outAVCodecContext->height;
                outFrame->format = outAVCodecContext->pix_fmt;

                avcodec_encode_video2(outAVCodecContext, &outPacket, outFrame, &gotPicture);
                if(gotPicture){
                    if(outPacket.pts != AV_NOPTS_VALUE){
                        outPacket.pts = av_rescale_q(outPacket.pts, videoSt->codec->time_base, videoSt->time_base);
                    }
                    if(outPacket.dts != AV_NOPTS_VALUE){
                        outPacket.dts = av_rescale_q(outPacket.dts, videoSt->codec->time_base, videoSt->time_base);
                    }

                    //cout << "Write frame " << j++ << " (size = " << outPacket.size / 1000 << ")" << endl;
                    //cout << "(size = " << outPacket.size << ")" << endl;

                    if(av_write_frame(outAVFormatContext, &outPacket) != 0){
                        cerr << "Error in writing video frame" << endl;
                    }

                    av_packet_unref(&outPacket);
                }

                av_packet_unref(&outPacket);
                av_free_packet(pAVPacket);  //avoid memory saturation
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
