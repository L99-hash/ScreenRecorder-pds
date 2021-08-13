#include <iostream>
#include "ScreenRecorder.h"
#include <thread>

enum Command {stop, start};
std::string commands[10] = {"stop", "start"};

Command stringToInt(std::string cmd){
    for(int i=0; i<commands->length(); i++)
        if(cmd == commands[i])
            return static_cast<Command>(i);
    return static_cast<Command>(-1);
}

void stopCommand(ScreenRecorder &sr){
    std::unique_lock<std::mutex> ul(sr.mu);
    sr.stopCapture = true;
}

int main() {
    std::string cmd;
    bool endWhile = false;
    bool started = false;

    ScreenRecorder screenRecorder;

    screenRecorder.openDevice();
    std::thread t;

    while(!endWhile){
        std::cout << "Insert command: ";
        std::cin >> cmd;

        Command c = stringToInt(cmd);

        switch (c) {
           case stop:
               stopCommand(screenRecorder);
               endWhile =  true;
               break;
            case start:
                started =  true;
                t = std::move(std::thread{ [&screenRecorder](){
                    screenRecorder.initOutputFile();
                    screenRecorder.captureVideoFrames();
                } });
                break;
           default:
               std::cout << "Command: " << cmd << " does not exist" << std::endl;
               break;
        }
    }

    if(started)
        t.join();
    return 0;
}
