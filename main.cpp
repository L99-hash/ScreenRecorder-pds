#include <iostream>
#include "ScreenRecorder.h"
#include <thread>

enum Command {stop, start, pause, resume};
std::string commands[10] = {"stop", "start", "pause", "resume"};

Command stringToInt(std::string cmd){
    for(int i=0; i<commands->length(); i++)
        if(cmd == commands[i])
            return static_cast<Command>(i);
    return static_cast<Command>(-1);
}

void stopCommand(ScreenRecorder &sr){
    std::unique_lock<std::mutex> ul(sr.mu);
    sr.stopCapture = true;
    if(sr.pauseCapture)
        sr.pauseCapture = false;
    sr.cv.notify_all();
}

void pauseCommand(ScreenRecorder &sr){
    std::unique_lock<std::mutex> ul(sr.mu);
    if(!sr.pauseCapture)
        sr.pauseCapture = true;
}

void resumeCommand(ScreenRecorder &sr){
    std::unique_lock<std::mutex> ul(sr.mu);
    if(sr.pauseCapture){
        sr.pauseCapture = false;
        sr.cv.notify_all();
    }
}

int main() {
    std::string cmd;
    bool endWhile = false;
    bool started = false;

    ScreenRecorder screenRecorder;

    std::thread t_video, t_audio;

    while(!endWhile){
        std::cout << "\nInsert command: ";
        std::cin >> cmd;

        Command c = stringToInt(cmd);

        switch (c) {
           case stop:
               stopCommand(screenRecorder);
               endWhile =  true;
               break;
            case start:
                started =  true;
                screenRecorder.openAudioDevice();
                screenRecorder.openVideoDevice();
                screenRecorder.initOutputFile();
                t_video = std::move(std::thread{ [&screenRecorder](){
                    screenRecorder.captureVideoFrames();
                } });
                t_audio = std::move(std::thread{ [&screenRecorder](){
                    screenRecorder.captureAudio();
                } });
                break;
            case pause:
                pauseCommand(screenRecorder);
                break;
            case resume:
                resumeCommand(screenRecorder);
                break;
           default:
               std::cout << "Command: " << "\"" << cmd << "\"" << " does not exist" << std::endl;
               break;
        }
    }

    if(started){
        t_video.join();
        t_audio.join();
    }
    return 0;
}
