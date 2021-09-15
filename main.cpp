#include <iostream>
#include "ScreenRecorder.h"
#include <thread>
#include <csignal>
#include <vector>

enum Command { stop, start, pause, resume, audio, out, dim, offset, help };
std::vector<std::string> commands{ "stop", "start", "pause", "resume", "audio", "out", "dim", "offset", "help"};

ScreenRecorder screenRecorder;
bool justPause = false;

void stopSignalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";


    if(!screenRecorder.getActiveMenu()){
        screenRecorder.setActiveMenu(true);


        while(!screenRecorder.getDisabledMenu());
        justPause=true;
        std::cout<< "Insert command: "<<std::flush;
    }
    else{
        justPause=false;
        screenRecorder.setActiveMenu(false);

    }

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
    std::cout << "dim --> set screen section to record" << std::endl;
    std::cout << "offset --> set top left point of screen section to record" << std::endl;
    std::cout << "out --> set output direcotry (relative or absolute path)" << std::endl;
    std::cout << "help --> show all commands" << std::endl;
    std::cout << "ctrl + c --> show menu while recording" << std::endl;
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
        if(!justPause && screenRecorder.getActiveMenu() ) std::cout << "\nInsert command: ";


        std::cin >> cmd;
        Command c = stringToInt(cmd);
        std::cin.clear();
        switch (c) {
           case stop:
               if(started){
                   stopCommand(screenRecorder);
               }

               endWhile =  true;

               break;
            case start:
                if(!started){
                    started =  true;
                    screenRecorder.setStarted(started);
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
                }
                else{
                    std::cout << "Already started." << std::endl;
                    showCommands();
                }

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
                        try {
                            screenRecorder.startRecording();
                        }
                        catch (std::exception e) {
                            std::cout << e.what() << std::endl;
                            exit(-1);
                        }
                    }
                    else {
                        std::cout << "Already started." << std::endl;
                    }
                    break;
                    case pause:
                        justPause = false;
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