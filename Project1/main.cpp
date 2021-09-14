#include <iostream>
#include "ScreenRecorder.h"
#include <thread>
#include <csignal>
#include <vector>

enum Command { stop, start, pause, resume, audio, out, dim, offset, help };
std::vector<std::string> commands{ "stop", "start", "pause", "resume", "audio", "out", "dim", "offset", "help"};

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

void showCommands() {
    std::cout << "Commands: " << std::endl;
    std::cout << "start --> begin registration" << std::endl;
    std::cout << "pause --> pause registration" << std::endl;
    std::cout << "resume --> resume registration after pause" << std::endl;
    std::cout << "stop --> stop registration" << std::endl;
    std::cout << "out --> set output direcotry (relative or absolute path)" << std::endl;
    std::cout << "help --> show all commands" << std::endl;
}
int main() {
    std::string cmd, dir;
    int w, h, x_off, y_off;
    bool endWhile = false;
    bool started = false;
    bool inPause = false;

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
                screenRecorder.stopCommand();
            }

            endWhile = true;

            break;
        case start:
            if (!screenRecorder.getStarted()) {
                screenRecorder.setActiveMenu(false);
                started = true;
                screenRecorder.setStarted(started);
                screenRecorder.startRecording();
            }
            else {
                std::cout << "Already started." << std::endl;
            }
            break;
        case pause:
            if (started) {
                screenRecorder.pauseCommand();
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
                screenRecorder.resumeCommand();
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
        case out:
            std::cout << "Insert path of output directory: ";
            std::cin >> dir;
            screenRecorder.setOutputDir(const_cast<char *>(dir.c_str()));
            break;
        case dim:
            std::cout << "Insert dimension of screen to record [width heigth]: ";
            std::cin >> w >> h;
            screenRecorder.setScreenDimension(w, h);
            std::cin.clear();
            break;
        case offset:
            std::cout << "Insert dimension of offset screen to record [x_offset y_offset]: ";
            std::cin >> x_off >> y_off;
            screenRecorder.setScreenOffset(x_off, y_off);
            std::cin.clear();
            break;
        default:
            std::cout << "Command: " << "\"" << cmd << "\"" << " does not exist" << std::endl;
            break;
        }
    }
    return 0;
}