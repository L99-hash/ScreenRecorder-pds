#include <iostream>
#include "ScreenRecorder.h"
#include <thread>
#include <csignal>
#include <vector>

enum Command { stop, start, pause, resume, audio, help };
std::vector<std::string> commands{ "stop", "start", "pause", "resume", "audio", "help"};

ScreenRecorder screenRecorder;

void stopSignalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    std::cout << "\nInsert command: ";

    screenRecorder.setActiveMenu(true);
}

Command stringToInt(std::string cmd) {
    int len = commands.size();
    for (int i = 0; i < len; i++)
        if (cmd == commands[i])
            return static_cast<Command>(i);
    return static_cast<Command>(-1);
}

void stopCommand(ScreenRecorder& sr) {
    std::unique_lock<std::mutex> ul(sr.mu);
    sr.stopCapture = true;
    if (sr.pauseCapture)
        sr.pauseCapture = false;
    sr.cv.notify_all();
}

void pauseCommand(ScreenRecorder& sr) {
    std::unique_lock<std::mutex> ul(sr.mu);
    if (!sr.pauseCapture)
        sr.pauseCapture = true;
}

void resumeCommand(ScreenRecorder& sr) {
    std::unique_lock<std::mutex> ul(sr.mu);
    if (sr.pauseCapture) {
        sr.pauseCapture = false;
        sr.cv.notify_all();
    }
}

void showCommands() {
    std::cout << "Commands: " << std::endl;
    std::cout << "start --> begin registration" << std::endl;
    std::cout << "pause --> pause registration" << std::endl;
    std::cout << "resume --> resume registration after pause" << std::endl;
    std::cout << "stop --> stop registration" << std::endl;
    std::cout << "help --> show all commands" << std::endl;
}
int main() {
    std::string cmd;
    bool endWhile = false;
    bool started = false;
    bool inPause = false;

    // register signal SIGINT and signal handler  
    

    std::thread t_video, t_audio;

    showCommands();

    while (!endWhile) {
        signal(SIGINT, stopSignalHandler);
        if(screenRecorder.getActiveMenu()) std::cout << "\nInsert command: ";
        std::cin >> cmd;
        Command c = stringToInt(cmd);
        std::cin.clear();
        switch (c) {
            
        case stop:
            if (started) {
                stopCommand(screenRecorder);
            }

            endWhile = true;

            break;
        case start:
            //std::cin.clear();
            if (!started) {
                screenRecorder.setActiveMenu(false);
                started = true;
                screenRecorder.setStarted(started);
                //screenRecorder.openAudioDevice();
                //screenRecorder.openVideoDevice();
                //screenRecorder.initOutputFile();
                screenRecorder.startRecording();
            }
            else {
                std::cout << "Already started." << std::endl;
            }
            break;
        case pause:
            if (started) {
                pauseCommand(screenRecorder);
                inPause = true;
            }
            else {
                std::cout << "Not yet started!" << std::endl;
            }
            break;
        case resume:
            if (inPause) {
                screenRecorder.setActiveMenu(false);
                inPause = false;
                resumeCommand(screenRecorder);
            }
            else {
                std::cout << "Not in pause!" << std::endl;
            }
            break;
        case audio:
            screenRecorder.setRecordAudio(true);
            break;
        case help:
            showCommands();
            break;
        default:
            std::cout << "Command: " << "\"" << cmd << "\"" << " does not exist" << std::endl;
            break;
        }
    }

    /*if (started) {
        t_video.join();
        t_audio.join();
    }*/
    return 0;
}