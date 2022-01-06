#include <iostream>
#include "ScreenRecorder.h"
#include <thread>
#include <csignal>
#include <vector>

enum Command{
    stop,
    start,
    pause,
    resume,
    audio,
    out,
    dim,
    offset,
    help
};
std::vector<std::string> commands{ "stop", "start", "pause", "resume", "audio", "out", "dim", "offset", "help" };

ScreenRecorder screenRecorder;
bool justPause = false;

void stopSignalHandler(int signum){
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";

    if(screenRecorder.getStarted()){
        if (!screenRecorder.getActiveMenu()) {
        screenRecorder.setActiveMenu(true);

        while (!screenRecorder.getDisabledMenu()) ;
        justPause = true;
        std::cout << "Insert command: " << std::flush;
        }
        else {
            justPause = false;
            screenRecorder.setActiveMenu(false);
        }
    }
    else{
        std::cout << "\nInsert command: " << std::flush;
    }
}

Command stringToInt(std::string cmd){
    int len = commands.size();
    for (int i = 0; i < len; i++)
        if (cmd == commands[i])
            return static_cast<Command>(i);
    return static_cast<Command>(-1);
}

void showCommands(){
    std::cout << "Commands: " << std::endl;
    std::cout << "start --> begin registration" << std::endl;
    std::cout << "audio --> enable audio registration" << std::endl;
    std::cout << "pause --> pause registration" << std::endl;
    std::cout << "resume --> resume registration after pause" << std::endl;
    std::cout << "stop --> stop registration and/or exit" << std::endl;
    std::cout << "dim --> set screen section to record" << std::endl;
    std::cout << "offset --> set top left point of screen section to record" << std::endl;
    std::cout << "out --> set output direcotry (relative or absolute path)" << std::endl;
    std::cout << "help --> show all commands" << std::endl;
    std::cout << "ctrl + c --> show menu while recording" << std::endl;
}
int main(){
    std::string cmd, dir;
    int w, h, x_off, y_off;
    bool endWhile = false;
    //bool started = false;
    bool inPause = false;

    showCommands();

    while (!endWhile) {
        signal(SIGINT, stopSignalHandler);
        if (!justPause && screenRecorder.getActiveMenu())
            std::cout << "\nInsert command: ";

        std::cin >> cmd;
        Command c = stringToInt(cmd);
        std::cin.clear();
        switch (c) {

        case stop:
            if (screenRecorder.getStarted()){
                screenRecorder.stopCommand();
            }

            endWhile = true;

            break;
        case start:
            if (!screenRecorder.getStarted()) {
                screenRecorder.setActiveMenu(false);
                //started = true;
                //screenRecorder.setStarted(started);
                try{
                    screenRecorder.startRecording();
                }
                catch (std::exception e){
                    std::cout << e.what() << std::endl;
                    exit(-1);
                }
            }
            else{
                std::cout << "Already started." << std::endl;
            }
            break;
        case pause:
            justPause = false;
            if (screenRecorder.getStarted()){
                screenRecorder.pauseCommand();
                inPause = true;
                while (!screenRecorder.getDisabledMenu());
                std::cout << "Pause recording" << std::endl;
            }
            else{
                std::cout << "Not yet started!" << std::endl;
            }
            break;
        case resume:
            if (inPause){
                std::cout << "Resume recording" << std::endl;
                screenRecorder.setActiveMenu(false);
                inPause = false;
                screenRecorder.resumeCommand();
            }
            else {
                std::cout << "Not in pause!" << std::endl;
            }
            break;
        case audio:
            if (!screenRecorder.getStarted()) {
                std::cout << "Audio enabled" << std::endl;
                screenRecorder.setRecordAudio(true);
            }
            else std::cout << "Command disabled during recording" << std::endl;
            break;
        case help:
            showCommands();
            break;
        case out:
            if (!screenRecorder.getStarted()) {
                std::cout << "Insert path of output directory: ";
                std::cin >> dir;
                screenRecorder.setOutputDir(const_cast<char*>(dir.c_str()));
            }
            else std::cout << "Command disabled during recording" << std::endl;
            break;
        case dim:
            if (!screenRecorder.getStarted()) {
                std::cout << "Insert dimension of screen to record [width heigth]: ";
                std::cin >> w >> h;
                screenRecorder.setScreenDimension(w, h);
                std::cin.clear();
            }
            else std::cout << "Command disabled during recording" << std::endl;
            break;
        case offset:
            if (!screenRecorder.getStarted()) {
                std::cout << "Insert dimension of offset screen to record [x_offset y_offset]: ";
                std::cin >> x_off >> y_off;
                screenRecorder.setScreenOffset(x_off, y_off);
                std::cin.clear();
            }
            else std::cout << "Command disabled during recording" << std::endl;
            break;
        default:
            std::cout << "Command: "
                << "\"" << cmd << "\""
                << " does not exist" << std::endl;
            break;
        }
    }
    return 0;
}
