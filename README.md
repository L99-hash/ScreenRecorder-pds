# ScreenRecorder-pds

Screen recorder built using FFmpeg 4.1 library. Project realized for System and Device Programming course of Politecnico di Torino

# Linux
Dependencies:
- libavcodec
- libavformat
- libavfilter
- libavutil
- libavdevice
- libswscale
- libswresample
- libx11

Before build the preject, perform (check that ```install_dependencies.sh``` and ```build.sh``` have execution permission): 
- ```sudo ./install_dependencies.sh```

To build and execute the project:
- ```./build.sh```
- ```./ScreenRecoder-pds```

### Last test on linux with versions:

- libavutil      56. 51.100 / 56. 51.100
- libavcodec     58. 91.100 / 58. 91.100
- libavformat    58. 45.100 / 58. 45.100
- libavdevice    58. 10.100 / 58. 10.100
- libavfilter    7. 85.100 /  7. 85.100
- libavresample  4.  0.  0 /  4.  0.  0
- libswresample  3.  7.100 /  3.  7.100
- libx11         2: 1.7.2-1

# Develop your own project

To develop your own project, you can use the ScreenRecorder.cpp as wrapper APIs for FFmpeg librarry.

## Documentation

* `void getScreenResolution(int& width, int& height)`

      Get dimensions of main desktop

* `void stopCommand()`

      Permits to stop recording

* `void pauseCommand()`

      Permits to pause recording in order to be resumed later

* `void resumeCommand()`

      Permits to resume paused recording

* `bool getStarted()`

      Read if recording is started or not

* `bool getActiveMenu()`

      Read if menu is active or not (thread safe)

* `void setActiveMenu(bool val)`

      Comunicate that the menu has been activated (thread safe)

* `bool getDisabledMenu()`

      Read if menu is disabled

* `bool getRecordAudio()`

      Read if audio is enabled or not

* `void setRecordAudio(bool val)` 

      Permits to enable audio recording

* `void setOutputDir(const char* dir)` 

      Permits to set output directory (defuault: .)

* `void setScreenDimension(int width, int height)` 

      Override screen dimension to record (default->desktop dimension)

* `void setScreenOffset(int x_offset, int y_offset)`

      Override screen offset (default->x=0, y=0)

* `int initOutputFile() throw()` 

      Initialize the output file

* `void startRecording() throw()`

      Start recording (audio is optional)
      Open devices ans initialize the output file

* `int openVideoDevice() throw()`

      Open input video device

* `void generateVideoStream() throw()`

      Find information for video stream
      Called by initOutputFile()    

* `int captureVideoFrames()`

      Start recording video in a different thread

* `int openAudioDevice() throw()`

      Open audio device

* `void generateAudioStream() throw()`

      Find information for audio stream
      Called by initOutputFile()

* `void captureAudio()` 

      Start recording video in a different thread