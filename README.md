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

Before build the preject, perform: 
- ```sudo apt-get update```
- ```sudo apt-get install libavcodec-dev libavformat-dev libavfilter-dev libavutil-dev libavdevice-dev libswscale-dev libswresample-dev libx11-dev cmake pkg-config -y```

To build and execute the project:
- move in the project directory
- ```cmake .```
- ```make```
- ```./ScreenRecorder_pds```
